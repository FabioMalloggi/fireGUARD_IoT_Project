import xml.etree.ElementTree as ET
import json
import time
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


# Global Definitions
db_config = load_db_config("config.xml")
devices = load_devices("config.xml")

safety_levels_default = load_safety_levels_default("config.xml")
safety_levels = safety_levels_default.copy()


# ==================== Safety Parameter Checks ====================

def is_safe_param(param: str, allowed_params: set) -> bool:
    if param in allowed_params:
        return True
    print(f"[Invalid Command] '{param}' is not a valid parameter.")
    return False

def is_safe_integer(input_str: str) -> bool:
    if input_str.isdigit():
        return True
    print(f"[Invalid Command] '{input_str}' is not a valid positive integer.")
    return False


# ==================== DB Interactions ====================

def query_sensor(sensor: str, n: str | None):
    print("----------------------------")
    allowed_sensors = { "temp", "hum", "pressure", "tvoc", "raw_h2", "raw_ethanol", "pm1_0", "pm2_5", "nc0_5", "status" }
    if not is_safe_param(sensor, allowed_sensors):
        return
    n_measurements = int(n) if n and is_safe_integer(n) else 10 # Default if not provided
    try:
        db = pymysql.connect(**db_config)
        with db.cursor() as cursor:
            sql = f"SELECT timestamp, value FROM {sensor} ORDER BY timestamp DESC LIMIT %s"
            cursor.execute(sql, (n_measurements,))
            results = cursor.fetchall()
            if results:
                print(f"Last {len(results)} values from '{sensor}' (DB):")
                for ts, v in results:
                    print(f"  Timestamp: {ts} | Value: {v}")
            else:
                print(f"No data found in '{sensor}' table.")
    except Exception as e:
        print(f"[Error] Database query failed: {e}")
    finally:
        if db:
            db.close()
    print("----------------------------")

def daily_hazard_levels():
    print("----------------------------")
    hazard_sensors = ["pm1_0", "pm2_5", "nc0_5"]
    max_values = 100
    try:
        db = pymysql.connect(**db_config)
        with db.cursor() as cursor:
            print("Daily Average hazard parameter values:")
            for sensor in hazard_sensors:
                sql = f"SELECT value FROM {sensor} ORDER BY time DESC LIMIT %s"
                cursor.execute(sql, (max_values))
                results = cursor.fetchall()
                values = [v[0] for v in results]
                if values:
                    avg_val = sum(values) / len(values)
                    print(f"   '{sensor}': {avg_val:.2f}")
                else:
                    print(f"No data found for '{sensor}'.")
    except Exception as e:
        print(f"[Error] Failed to compute daily hazard levels: {e}")
    finally:
        if db:
            db.close()
    print("----------------------------")

def show_devices():
    print("----------------------------")
    for device in devices:
        print(json.dumps(device, indent=4))
    print("----------------------------")

def show_safety():
    print("----------------------------")
    print("Current safety levels:")
    for k, v in safety_levels.items():
        print(f"  {k}: {v}")
    print("----------------------------")

# ==================== Direct Device Interactions ====================

def get_dev_by_cat(cat: str):
    return next((d for d in devices if d["cat"] == cat), None)
    
def dev_status():
    print("----------------------------")
    device = get_dev_by_cat("SSD")
    if not device:
        print("[Error] No SSD device found.")
        return
    
    # CoAP request
    address = device["address"]
    port = device["port"]
    path = "/status"
    try:
        client = HelperClient(server=(address, port))
        response = client.get(path)
        if response:
            print(f"CoAP GET response from coap://[{address}]:{port}{path}:")
            print(response.pretty_print())
        else:
            print("[No Response]")
    except Exception as e:
        print(f"[Error] Failed to query sensor: {e}")
    finally:
        try:
            client.stop()
        except:
            pass
    print("----------------------------")


def dev_sensor(sensor: str, n: str | None):
    print("----------------------------")
    allowed_sensors = {"temp", "hum", "pressure", "tvoc", "raw_h2", "raw_ethanol", "pm1_0", "pm2_5", "nc0_5"}
    if not is_safe_param(sensor, allowed_sensors):
        return
    n_measurements = int(n) if n and is_safe_integer(n) else 10 # default value
    device = get_dev_by_cat("SSD")
    if not device:
        print("[Error] No SSD device found.")
        return
    
    # CoAP request
    address = device["address"]
    port = device["port"]
    path = f"/{sensor}?n={n_measurements}"
    try:
        client = HelperClient(server=(address, port))
        response = client.get(path)
        if response:
            print(f"CoAP GET response from coap://[{address}]:{port}{path}:")
            print(response.pretty_print())
        else:
            print("[No Response]")
    except Exception as e:
        print(f"[Error] Failed to query sensor: {e}")
    finally:
        try:
            client.stop()
        except:
            pass
    print("----------------------------")


def set_safety(param: str, val_str: str | None):
    print("----------------------------")
    allowed_params = set(safety_levels_default.keys())
    if param not in allowed_params:
        print(f"[Invalid Command] '{param}' is not a recognized safety parameter.")
        return
    val = int(val_str) if val_str and val_str.isdigit() else safety_levels_default[param] # Default value
    device = get_dev_by_cat("SSD")
    if not device:
        print("[Error] No SSD device found.")
        return
    
    # CoAP request
    address = device["address"]
    port = device["port"]
    path = f"/{param}"
    payload = f"limit={val}"
    try:
        client = HelperClient(server=(address, port))
        response = client.post(path, payload)
        if response:
            print(f"CoAP POST to coap://[{address}]:{port}{path} successful:")
            print(response.pretty_print())
            # Update local safety level only after successful POST
            safety_levels[param] = val
        else:
            print("[No Response] Safety level update not confirmed.")
    except Exception as e:
        print(f"[Error] CoAP POST failed: {e}")
    finally:
        try:
            client.stop()
        except:
            pass
    print("----------------------------")
    
def start_stop_vent(action: str, vent_type: str):
    print("----------------------------")
    valid_actions = {"start", "stop"}
    valid_vent_types = {"filter", "smoke"}
    if action not in valid_actions or vent_type not in valid_vent_types:
        print("[Invalid Command] Usage: start|stop <filter|smoke> vent")
        return
    device = get_dev_by_cat("SV")
    if not device:
        print("[Error] No SV device found.")
        return
    
    # CoAP request
    address = device["address"]
    port = device["port"]
    path = f"/vent?system={vent_type}"
    mode = "on" if action == "start" else "off"
    payload = f"mode={mode}"
    try:
        client = HelperClient(server=(address, port))
        response = client.post(path, payload)
        if response:
            print(f"CoAP POST to coap://[{address}]:{port}{path} successful:")
            print(response.pretty_print())
        else:
            print("[No Response]")
    except Exception as e:
        print(f"[Error] Failed to control ventilation: {e}")
    finally:
        try:
            client.stop()
        except:
            pass
    print("----------------------------")


# ==================== CLI Test ====================
def test_DB():
    print("============> START TESTING DB INTERACTION <============")
        
    print("-----------> CMD: \'query x\'")
    query_sensor("x", None)
    print("-----------> CMD: \'query x 2\'")
    query_sensor("x", "2")
    print("-----------> CMD: \'query temp x\'")
    query_sensor("temp", "x")
    print("-----------> CMD: \'query temp 2\'")
    query_sensor("temp", "2")
    print("-----------> CMD: \'query temp\'")
    query_sensor("temp", None)
    
    print("-----------> CMD: \'query status 10\'")
    query_sensor("status", "10")
    
    print("-----------> CMD: \'daily hazard levels\'")
    daily_hazard_levels()
    
    print("============> END TESTING DB INTERACTION <============")

def test_dev():
    print("============> START TESTING DEVICE INTERACTION <============")
        
    print("-----------> CMD: \'dev x\'")
    dev_sensor("x", None)
    print("-----------> CMD: \'dev x 2\'")
    dev_sensor("x", "2")
    print("-----------> CMD: \'dev temp x\'")
    dev_sensor("temp", "x")
    print("-----------> CMD: \'dev temp 2\'")
    dev_sensor("temp", "2")
    print("-----------> CMD: \'dev temp\'")
    dev_sensor("temp", None)
    
    print("-----------> CMD: \'dev status\'")
    dev_status()
    
    print("-----------> CMD: \'set safety x\'")
    set_safety("x", None)
    print("-----------> CMD: \'set safety x 200\'")
    set_safety("x", "200")
    print("-----------> CMD: \'set safety pm1_0 x\'")
    set_safety("pm1_0", "x")
    print("-----------> CMD: \'set safety pm1_0 200\'")
    set_safety("pm1_0", "x")
    print("-----------> CMD: \'set safety pm1_0\'")
    set_safety("pm1_0", "x")
    
    
    print("-----------> CMD: \'x filter vent\'")
    start_stop_vent("x","filter")
    print("-----------> CMD: \'start x vent\'")
    start_stop_vent("start","x")
    print("-----------> CMD: \'start filter vent\'")
    start_stop_vent("start","filter")
    print("-----------> CMD: \'start smoke vent\'")
    start_stop_vent("start","smoke")
    
    print("-----------> CMD: \'stop x vent\'")
    start_stop_vent("stop","x")
    print("-----------> CMD: \'stop filter vent\'")
    start_stop_vent("stop","filter")
    print("-----------> CMD: \'stop smoke vent\'")
    start_stop_vent("stop","smoke")
    
    print("============> END TESTING DEVICE INTERACTION <============")

def test_general():
    print("============> CLI GENERAL INTERACTION TEST ERRORS: <============")
    print("-----------> CMD: \'x\'")
    print("-----------> CMD: \'set safety \'")
    print("-----------> CMD: \'start \'")
    print("-----------> CMD: \'start x\'")
    print("============> CLI GENERAL INTERACTION TESTS ERRORS <============")

# ==================== CLI prints ====================

def print_help():
    print("""
Commands:
  help                         - Show this help
  show devices                 - Show devices info and resources
  show safety                  - Show current safety levels
  query <sensor> (<n>)         - Query DB last 'n' sensor measurements
  query status (<n>)           - Query DB last 'n' status records
  dev <sensor> (<n>)           - Query dev last 'n' sensor measurements
  dev status                   - Query dev current environment status
  daily hazard levels          - Show the daily average of hazard parameters
  set safety <param> (<value>) - Set levels by given (or default) parameters
  start <filter|smoke> vent    - Start ventilation
  stop  <filter|smoke> vent    - Stop ventilation
  exit                         - Exit the server
""")

def print_welcome():
    print("""
============================================================================
                  Welcome to FireGUARD IoT System (Remote)
============================================================================""")
    print_help()
    print("============================================================================")

    
# ==================== CLI Loop ====================
if __name__ == "__main__":
    print_welcome()
    while True:
        try:
            cmd = input("fireGUARD > ").strip()
            parts = cmd.split()
            if cmd in {"exit", "quit"}:
                break
            elif cmd == "help":
                print_help()
            elif cmd == "show devices":
                show_devices()
            elif cmd == "show safety":
                show_safety()
            elif cmd.startswith("query ") and len(parts) <= 3:
                query_sensor(parts[1], parts[2] if len(parts) == 3 else None)
            elif cmd.startswith("dev status"):
                dev_status()
            elif cmd.startswith("dev ") and len(parts) <= 3:
                dev_sensor(parts[1], parts[2] if len(parts) == 3 else None)
            elif cmd == "daily hazard levels":
                daily_hazard_levels()
            elif cmd.startswith("set safety ") and (len(parts) == 3 or len(parts) == 4):
                set_safety(parts[2], parts[3] if len(parts) == 4 else None)
            elif (cmd.startswith("start ") or cmd.startswith("stop ")) and len(parts) == 3:
                if len(parts) == 3 and parts[2] == "vent":
                    start_stop_vent(parts[0], parts[1])
                else:
                    print("[Invalid Command] Usage: start|stop <filter|smoke> vent")
            elif cmd == "test DB":
                test_DB()
            elif cmd == "test dev":
                test_dev()
            elif cmd == "test gen":
                test_general()
            else:
                print("[Invalid Command] Type 'help' for available commands.")
        except KeyboardInterrupt:
            print("[Interrupt] KeyboardInterrupt received. Shutting down...")
            break
        except Exception as e:
            print(f"[Error] {e}")
    print("Server shutdown.")

