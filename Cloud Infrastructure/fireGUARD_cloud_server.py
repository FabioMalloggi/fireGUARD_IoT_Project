import xml.etree.ElementTree as ET
import json
import time
from datetime import datetime
import threading
import pymysql
from coapthon.client.helperclient import HelperClient


# ==================== Load configuration file info ====================
def load_db_config(config_path="config.xml"):
    tree = ET.parse(config_path)
    root = tree.getroot()
    db_node = root.find("database")
    if db_node is None:
        raise Exception("No <database> node found in config.xml")
    return {
        "host": db_node.get("host", "localhost"),
        "user": db_node.get("user", "root"),
        "password": db_node.get("password", "PASSWORD"),
        "database": db_node.get("name", "fireGUARD_DB"),
        "port": int(db_node.get("port", "3306"))
    }

def load_cooja_mode(config_path="config.xml"):
    tree = ET.parse(config_path)
    root = tree.getroot()
    cooja_mode = root.get("cooja", "0") == "1"
    return cooja_mode

def load_devices(config_path="config.xml"):
    tree = ET.parse(config_path)
    root = tree.getroot()
    devices = []
    cooja_mode = root.get("cooja", "0") == "1"

    for dev in root.findall("device"):
        dev_id = int(dev.get("id", "-1"))
        dev_cat = dev.get("cat", "")
        # Use cooja_address if in cooja mode, else normal address
        address = dev.get("cooja_address") if cooja_mode else dev.get("address")
        port = int(dev.get("port", "5683"))
        resources = [r.text for r in dev.findall("resource")]
        base_uri = f"coap://[{address}]"
        devices.append({
            "id": dev_id,
            "cat": dev_cat,
            "address": address,
            "port": port,
            "resources": resources,
            "base_uri": base_uri
        })
    return devices

def load_safety_levels_default(config_path="config.xml"):
    tree = ET.parse(config_path)
    root = tree.getroot()
    safety_levels_node = root.find('safety_levels')
    safety_levels_default = {
        "pm1_0": int(safety_levels_node.findtext('pm1_0', default='250')) if safety_levels_node else 250,
        "pm2_5": int(safety_levels_node.findtext('pm2_5', default='250')) if safety_levels_node else 250,
        "nc0_5": int(safety_levels_node.findtext('nc0_5', default='2500')) if safety_levels_node else 2500
    }
    return safety_levels_default



# ==================== Database Initialization ====================

def run_schema_sql(cursor, filepath="SchemaDB.sql"):
    with open(filepath, "r") as f:
        schema_sql = f.read()
    statements = [stmt.strip() for stmt in schema_sql.split(';') if stmt.strip()]
    for stmt in statements:
        cursor.execute(stmt)



# ==================== SenML Parsing and DB Insert ====================

def get_device_id(base_uri):
    return devices_cache.get(base_uri)



def insert_measurement(cursor, table, base_time, time_offset, max_offset, device_id, value):
    measurement_time = base_time + time_offset  # BIGINT column

    # Compute equivalent real timestamp:
    # current_unix_time - max_offset aligns the oldest sample with current time
    # then add this sample's offset
    current_unix_time = int(time.time())
    measurement_unix_time = current_unix_time - max_offset + time_offset

    timestamp_value = datetime.fromtimestamp(measurement_unix_time)

    query = f"""
        INSERT INTO {table} (time, timestamp, device, value)
        VALUES (%s, %s, %s, %s)
    """
    cursor.execute(query, (measurement_time, timestamp_value, device_id, value))


def parse_and_store(payload, cursor):
    try:
        if isinstance(payload, bytes):
            payload = payload.decode('utf-8')

        data = json.loads(payload)
        uri = data.get("bn")
        if uri is None:
            print("Missing base URI (bn) in payload")
            return

        table = uri.rstrip('/').split('/')[-1]  # e.g., "temp" or "status"
        base_uri = "/".join(uri.rstrip('/').split('/')[:-1])  # e.g., "coap://[fd00::202:2:2:2]"

        device_id = get_device_id(base_uri)
        if device_id is None:
            print(f"Unknown base URI: {base_uri}")
            return

        bt = int(data.get("bt", 0))

        if table == "status":
            status_value = data.get("status")
            if status_value is not None:
                insert_measurement(cursor, "status", bt, 0, 0, device_id, status_value)
            else:
                print("Missing 'status' field in status payload.")
            return  # Exit early for status payload

        entries = data.get("e", [])
        if not isinstance(entries, list) or not entries:
            print("No measurement entries found.")
            return
        
        max_offset = max(int(entry.get("t", 0)) for entry in entries)
        for entry in entries:
            t = int(entry.get("t", 0))
            v = entry.get("v")
            v = entry.get("v")
            if v is not None:
                # Apply scaling only for specific tables
                if table in ("temp", "hum", "pressure"):
                    v = v / 100.0
                insert_measurement(cursor, table, bt, t, max_offset, device_id, v)

    except Exception as e:
        print("Error parsing/storing SenML:", e)


# ==================== CoAP Observation Threads ====================
def make_on_response(conn):
    def on_response(response):
        if response and response.payload:
            try:
                with conn.cursor() as cursor:
                    response.pretty_print()
                    parse_and_store(response.payload, cursor)
                conn.commit()
            except Exception as e:
                print("Database error:", e)
        else:
            print("Empty or invalid CoAP response received.")
    return on_response
    

def observe_resource(address, port, resource, stop_event):
    try:
        # Persistent DB connection for this observer
        conn = pymysql.connect(
            host=db_config['host'],
            user=db_config['user'],
            password=db_config['password'],
            database=db_config['database'],
            port=db_config['port'],
            autocommit=False  # Let commit be explicit
        )
	
        client = HelperClient(server=(address, port))
        print(f"Observing coap://[{address}]:{port}/{resource}")
        client.observe(resource, make_on_response(conn))

        # Stay alive while stop_event is not set
        while not stop_event.is_set():
            time.sleep(1)

        print(f"Stopping observation of {resource} at {address}")

    except Exception as e:
        print(f"Observation failed for {resource} at {address}:{port}:", e)

    finally:
        if client:
            try:
                client.stop()  # Stop the CoAP client loop
                print(f"CoAP client stopped for {resource} at {address}")
            except Exception as e:
                print(f"Error stopping CoAP client: {e}")

        if conn:
            try:
                conn.close()
                print(f"Connection closed for {resource}")
            except Exception as e:
                print(f"Error closing DB connection: {e}")


def observer_thread(address, port, resource, stop_event):
    observe_resource(address, port, resource, stop_event)





# ==================== Main ====================

def main():
    print("""
===============================================================
               FireGUARD Monitoring Cloud Server
===============================================================""")

    global db_config, devices, devices_cache
    
    db_config = load_db_config("config.xml")
    devices = load_devices("config.xml")
    
    is_cooja_mode = load_cooja_mode("config.xml")
    
    devices_cache = {}
    for device in devices:
        devices_cache[device["base_uri"]] = device["id"]
    
    safety_levels_default = load_safety_levels_default("config.xml")
    safety_levels = safety_levels_default.copy()

    
    # Connect to MySQL (without specifying a DB yet)
    init_conn = pymysql.connect(
        host=db_config['host'], 
        user=db_config['user'], 
        password=db_config['password'],
        port=db_config['port'])
        
    # Run SchemaDB.sql to initialize schema if not already present
    with init_conn.cursor() as cursor:
        run_schema_sql(cursor)
        init_conn.commit()
    init_conn.close()
    
    # Connect to DB to populate devices table
    conn = pymysql.connect(
        host=db_config['host'],
        user=db_config['user'],
        password=db_config['password'],
        database=db_config['database'],
        port=db_config['port'],
        autocommit=False
    )
    
    try:
        with conn.cursor() as cursor:
            for device in devices:
                res_json = json.dumps(device["resources"])
                cursor.execute("""
                    INSERT INTO devices (id, base_uri, port, category, resources)
                    VALUES (%s, %s, %s, %s, %s)
                    ON DUPLICATE KEY UPDATE base_uri=VALUES(base_uri), port=VALUES(port), category=VALUES(category), resources=VALUES(resources)
                """, (device["id"], device["base_uri"], device["port"], device["cat"], res_json))
        conn.commit()
    except Exception as e:
        print("Error inserting/updating device info:", e)
    conn.close()
    
    # === Global stop event to signal threads to stop ===
    stop_event = threading.Event()
    threads = []
    
    
    
    for device in [d for d in devices if d.get('cat') == "SSD"]:
        resources_list = device['resources'] if is_cooja_mode else ["temp", "pm1_0", "status"]
        for resource in resources_list:
            thread = threading.Thread(
                target=observer_thread,
                args=(device['address'], device['port'], resource, stop_event),
                daemon=False  # threads closed manually
            )
            thread.start()
            threads.append(thread)
            time.sleep(1)  # slight delay to stagger observations

    try:
        print("\nType 'server stop' to shut down the server.")
        while True:
            command = input("> ").strip().lower()
            if command == "server stop":
                print("Shutting down server...")
                break
            else:
                print("Unknown command. Type 'server stop' to exit.")
    except KeyboardInterrupt:
        print("KeyboardInterrupt received. Shutting down...")
    finally:
        stop_event.set()
        for thread in threads:
            thread.join()
        print("All observer threads have been stopped.")
    
if __name__ == "__main__":
    main()

