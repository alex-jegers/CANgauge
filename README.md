# Features Overview
- Real time data logging and monitoring without the need for a computer or aftermarket sensor installation - read and display data directly from the ECU and control modules.
- High resolution, touch screen display.
### Does it work on my car?
CANgauge requires an ISO 15765 compliant CANbus network to communicate with your car...\
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
# Future Plans
