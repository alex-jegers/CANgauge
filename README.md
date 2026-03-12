# Features Overview
- A robust, modern, and universal approach to automotive telemetry and data logging.
- Read data directly from the ECU and other control modules through the OBD2 port over CANbus.
- High resolution, touch screen display.
### Does it work on my car?
CANgauge requires an OBD2 port and an ISO 15765 compliant CANbus network to communicate with your car...\
\
**Model year before 2005:** No. The Diagnostics on CAN standard was not yet released (ISO 15765)..\
**Model year 2005 to 2007:** Maybe! CANbus was not yet required but many manufacturers used it anyways. See how to check your car here (TODO: add link)\
**Model year 2008+:** Yes!
### What data can I read?
The available sensor data varies from car-to-car. There is no way to be 100% sure what data will be available without checking on a CANgauge or similar scanner. You can check or contribute to our Available PIDs database here(TODO add link).\
Some popular and frequently available data includes:
- Boost pressure/intake manifold pressure
- Air/fuel ratio
- Commanded air/fuel ratio
- Intake air temp
- Charge air temp
- Long/short term fuel trims
- Timing angle advance
- Coolant temp
- Exhaust temp
- And more\
For a full list of potential data see here (TODO add link).\
# Open Source
- Arduino IDE compatible (coming soon).
- Software and hardware docs are released and free. Add custom features or use it as a starting point for your project.
- Software built around LVGL and FreeRTOS.
- Extra CAN port (2 in total), FS USB interface, and (optional) I2C port exposed on the connector for easy feature expansion.
## Info for Developers
### Processor
- A dual core STM32H745 with an Arm Cortex M7 and Arm Cortex M4F core.
- Clock speeds up to 480MHz.
### Memory
- 2MB of embedded flash, 864kB of embedded RAM (TODO check these numbers.)
- 32MB of external SDRAM.
- Optional 512kB of QSPI flash and/or 64kB of I2C EEPROM (accessible from the external connector).
# Future Plans
