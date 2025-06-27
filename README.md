# fireGUARD_IoT_Project
This project was developed as part of the Internet of Things course within the Computer Engineering curriculum, selected as a free-choice activity for the Master's Degree in Artificial Intelligence and Data Engineering at the University of Pisa.

This README provides complete instructions to switch between the simulation environment (COOJA) and real hardware deployment using USB Dongles. It includes setup steps, configuration files to modify, and CLI commands for compiling, flashing, and launching the backend services and Grafana interface for data monitoring.


---

## Switching Between COOJA Simulation and Dongle Deployment

> To switch **to Dongle deployment**, follow the steps below.  
> To revert to COOJA simulation, reverse these changes.

### 1. Disable COOJA Mode in Device Firmware

In the following files, **comment out** the line:

`#define COOJA 1`

- `smart_smoke_detector/project_conf.h`  
- `smart_vent/project_conf.h`


### 2. Update Server Configuration

In `config.xml`, **set** the `cooja` attribute in the `<config>` tag to:

`<config cooja="0">`

(Set it back to `"1"` to switch to COOJA)


### 3. Update Smart Smoke Detector (SSD) Address

In `smart_smoke_detector/network_config.h`:

- **Comment out** the line:

  `#define LOCAL_HOST "coap://[fd00::202:2:2:2]/"`

- **Uncomment** the line:

  `#define LOCAL_HOST "coap://[fd00::f6ce:36ed:babb:5620]/"`


### 4. Update Smart Vent (SV) Connection

In `smart_vent/network_config.h`:

- **Comment out** the line:

  `#define SSD_SERVER_EP "coap://[fe80::202:2:2:2]:5683"`

- **Uncomment** the line:

  `#define SSD_SERVER_EP "coap://[fd00::f6ce:36ed:babb:5620]:5683"`

---

## COOJA Project: Simulation Setup

Follow these steps to set up and run the COOJA simulation environment.

### Step 1: Launch COOJA Simulator

1. Open a terminal and navigate to the folder `contiki-ng/tools/Cooja/`.
2. Run the command `./gradlew run`.
3. When COOJA opens, load the simulation file named `malloggi_iot_project_sim.csc`.
4. Start the simulation.


### Step 2: Connect Border Router

1. Open a new terminal.
2. Navigate to `contiki-ng/project/border_router/`.
3. Run the command `make TARGET=cooja connect-router-cooja`.


### Step 3: Start the Cloud Server

1. Open another terminal.
2. Navigate to `contiki-ng/project/`.
3. Run the command `python3 fireGUARD_cloud_server.py`.


### Step 4: Start the Remote Control Application

1. Open a new terminal.
2. Navigate to `contiki-ng/project/`.
3. Run the command `python3 fireGUARD_remote_control_app.py`.

### Step 5: Access the MySQL Database

1. Open a terminal.
2. Run `mysql -u root -p`.
3. Inside the MySQL shell, run the following commands:
   - `USE fireGUARD_DB;`
   - `SHOW TABLES;`
   - `SELECT * FROM temp;`

### Step 6: Open Grafana Dashboard

1. Open a web browser.
2. Go to `http://127.0.0.1:3000`.
3. Select the dashboard named `iot_project_dashboard`.

---

## Dongle Project: Real Deployment Setup

### Goal
- Dongle 1 = Border Router  
- Dongle 2 = Smart Smoke Detector  
- Dongle 3 = Smart Vent  


### Clean Built Artifacts

Before each new dongle deployment, remember to clean previous builds by running:

`make distclean`


### Dongles Deployment

#### Dongle 1: Border Router Deployment

To deploy and flash the Border Router on Dongle 1, follow these steps:

1. Navigate to `contiki-ng/project/border_router/`.
2. Run the commands:
   - `make savetarget TARGET=nrf52840 BOARD=dongle`
   - `make border_router.dfu-upload PORT=/dev/ttyACM0`

#### Dongle 2: Smart Smoke Detector (SSD) Deployment

To deploy and flash the Smart Smoke Detector on Dongle 2, follow these steps:

1. Navigate to `contiki-ng/project/smart_smoke_detector/`.
2. Run the commands:
   - `make savetarget TARGET=nrf52840 BOARD=dongle`
   - `make smart_smoke_detector.dfu-upload PORT=/dev/ttyACM0`


#### Dongle 3: Smart Vent (SV) Deployment

To deploy and flash the Smart Vent on Dongle 3, follow these steps:

1. Navigate to `contiki-ng/project/smart_vent/`.
2. Run the commands:
   - `make savetarget TARGET=nrf52840 BOARD=dongle`
   - `make smart_vent.dfu-upload PORT=/dev/ttyACM0`

### Connect Border Router

To connect to the Border Router on Dongle 1 (using tunslip6):

- Run the command:  
  `make PORT=/dev/ttyACM0 connect-router`

### Launch Servers

To start backend services:

1. Open a terminal, navigate to `contiki-ng/project/`, and run:  
   `python3 fireGUARD_cloud_server.py`

2. In another terminal, navigate to `contiki-ng/project/` and run:  
   `python3 fireGUARD_remote_control_app.py`


### Optional: Log Visualization

To view logs from dongles, use these commands depending on the device port:

- For Dongle 1:  
  `make login PORT=/dev/ttyACM0`

- For Dongle 2:  
  `make login PORT=/dev/ttyACM1`

- For Dongle 3:  
  `make login PORT=/dev/ttyACM2`

### Optional: Access Database

To access the MySQL database:

1. Run `mysql -u root -p` and enter your password.
2. Then, run the following commands inside the MySQL shell:
   - `USE fireGUARD_DB;`
   - `SHOW TABLES;`
   - `SELECT * FROM temp;`

### Browser: Grafana Dashboard

- Open a browser and go to:  
  `http://127.0.0.1:3000`
- Select the dashboard named:  
  `iot_project_dashboard`

