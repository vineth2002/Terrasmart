TerraSmart: Automated Terrarium for Plant Care
Overview
TerraSmart is an IoT-based automated terrarium system designed to simplify plant care by monitoring and controlling environmental conditions such as temperature, soil moisture, water levels, and fertilizer application. The system uses an ESP32 microcontroller to collect sensor data, automate tasks like watering, heating, cooling, and fertilization, and provide real-time monitoring through a web dashboard. Data is also logged to ThingSpeak for visualization and analysis.
Features

Temperature Control: Monitors terrarium temperature using a DS18B20 sensor and activates heating or cooling (via Peltier and fan) to maintain optimal conditions (23°C–30°C).
Soil Moisture Monitoring: Uses a soil moisture sensor to detect moisture levels and automatically waters plants when levels fall below a 30% threshold.
Water and Fertilizer Level Monitoring: Float switches detect low water and fertilizer levels, displaying status on the LCD and web dashboard.
Automated Lighting: Controls an LED light with scheduled daily on/off times (19:00–00:00) and manual override via the web interface.
Fertilization Schedule: Automatically dispenses fertilizer weekly (Sundays at 10:15 AM for 10 seconds) with manual activation and a 24-hour cooldown to prevent overuse.
Real-Time Dashboard: A responsive web interface displays temperature, soil moisture, water/fertilizer levels, and last watering time, with controls for LED and fertilizer pump.
Data Logging: Sends temperature and soil moisture data to ThingSpeak for graphical visualization.
LCD Display: Shows real-time sensor readings and system status on a 16x2 I2C LCD.
WiFi Connectivity: Supports dual WiFi networks with automatic fallback and displays connection status.

Hardware Requirements

ESP32 Microcontroller: Core processing unit.
DS18B20 Temperature Sensor: Measures terrarium temperature.
Soil Moisture Sensor: Monitors soil moisture levels.
Float Switches (2): Detect water and fertilizer tank levels.
LCD Display (16x2 I2C): Displays sensor data and system status.
Relay Modules (4): Control water pump, fertilizer pump, LED light, and heating/cooling system.
Peltier Module and Fan: For temperature regulation.
LED Light: For plant growth.
Power Supply: Compatible with ESP32 and peripherals.
WiFi Router: For internet connectivity and web dashboard access.

Software Requirements

Arduino IDE: For uploading the code to the ESP32.
Libraries:
WiFi.h
WebServer.h
OneWire.h
DallasTemperature.h
SPIFFS.h
HTTPClient.h
ThingSpeak.h
time.h
LiquidCrystal_I2C.h
ArduinoJson.h
Wire.h


ThingSpeak Account: For data logging and visualization (update myChannelNumber and myWriteAPIKey in the code).
SPIFFS: For storing JSON data locally on the ESP32.

Installation

Hardware Setup:

Connect the DS18B20 sensor to pin 32.
Connect the soil moisture sensor to analog pin 33.
Connect float switches to pins 12 (water) and 13 (fertilizer).
Connect relay modules to pins 18 (water pump), 4 (LED), 5 (fertilizer pump), 26 (cooling), and 27 (heating).
Connect the I2C LCD to the ESP32 (SDA/SCL pins).
Ensure all components are powered appropriately.


Software Setup:

Install the Arduino IDE and required libraries.
Clone or download this repository.
Open the Arduino sketch (terrasmart.ino) in the Arduino IDE.
Update WiFi credentials (ssid1, password1, ssid2, password2) in the code.
Update ThingSpeak settings (myChannelNumber, myWriteAPIKey) with your account details.
Upload the code to the ESP32 using the Arduino IDE.


SPIFFS Configuration:

Use the Arduino IDE’s SPIFFS uploader plugin to upload the data.json file to the ESP32’s filesystem.


ThingSpeak Setup:

Create a ThingSpeak channel with fields for soil moisture (Field 1) and temperature (Field 2).
Note the channel ID and write API key for use in the code.



Usage

Power On: Connect the ESP32 to power. The LCD will display a "Welcome" message, followed by WiFi connection status.
WiFi Connection: The system attempts to connect to the primary WiFi network, falling back to the secondary if needed.
Monitoring:
The LCD displays real-time temperature and soil moisture readings.
Access the web dashboard by navigating to the ESP32’s IP address (displayed on the LCD) in a browser.
View graphs on ThingSpeak at https://thingspeak.com/channels/2583019.


Controls:
Use the web dashboard to toggle the LED or activate the fertilizer pump manually.
The system automatically waters the plant if soil moisture falls below 30%.
Temperature is maintained between 23°C and 30°C using heating/cooling systems.
Fertilizer is dispensed weekly, with manual activation limited by a 24-hour cooldown.


Alerts: Low water or fertilizer levels are indicated on the LCD and dashboard.

Folder Structure
TerraSmart/
├── terrasmart.ino          # Main Arduino sketch
├── data.json              # Stores sensor data (uploaded to SPIFFS)
└── README.md              # This file

Web Dashboard
The dashboard (index.html embedded in the Arduino code) provides:

Real-time display of temperature, soil moisture, water/fertilizer levels, and last watering time.
Interactive buttons to control the LED and fertilizer pump.
Embedded ThingSpeak graphs for soil moisture and temperature.
Responsive design for mobile and desktop access.

Notes

Ensure stable WiFi connectivity for ThingSpeak logging and web dashboard access.
Calibrate the soil moisture sensor (DRY_VALUE, WET_VALUE) for accurate readings.
Adjust temperature thresholds (upperThreshold, lowerThreshold) based on plant requirements.
The fertilizer pump’s weekly schedule and duration can be modified via relayWeeklyOnDay, relayWeeklyOnHour, relayWeeklyOnMinute, and fertilizerDuration.

Contributing
Contributions are welcome! Please submit issues or pull requests to improve the code, add features, or fix bugs.
License
This project is licensed under the MIT License. See the LICENSE file for details.
