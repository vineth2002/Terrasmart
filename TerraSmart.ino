#include <WiFi.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include "ThingSpeak.h"
#include <time.h>  // Include the time.h library for time functions
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <wire.h>

// Initialize the LCD library with the I2C address and LCD dimensions
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi credentials
/*#define AP_SSID "UoM_Wireless"
#define AP_PASSWORD "" 
const String username = "nallaperumavn.22";
const String password = "Vineth@200235301700";*/

const char* ssid2 = "Vineth A12";
const char* password2 = "dlrc3491";

/*const char* ssid1 = "UoM_Wireless";
const char* password1 = "Vineth@200235301700";
const char* password1 = "rgvk5417";*/
const char* ssid1 = "Aadii";
const char* password1 = "rgvk5417";



// ThingSpeak settings
const char* server = "api.thingspeak.com";
unsigned long myChannelNumber = 25830194;
const char* myWriteAPIKey = "X9LFONU73VCZUVZ1";

//Temperature
#define ONE_WIRE_BUS 32
#define COOLING_PIN 26
#define HEATING_PIN 27
const float upperThreshold = 30.0;
const float lowerThreshold = 23.0;

//Soil Misture
#define SOIL_SENSOR_PIN 33  // Analog pin for soil moisture sensor
#define WATER_RELAY_PIN 18  // Digital pin for relay module
#define DRY_VALUE 4095 // Sensor value when soil is completely dry
#define WET_VALUE 450 // Sensor value when soil is completely wet
#define MOISTURE_THRESHOLD 30  // Threshold value for watering (0-100 scale)
int soilMoistureValue = 0; 
bool waterPumpActivated=false;
unsigned long wateractivated=0;

//Floater switch
#define WATER_LEVEL 12
#define FERTILIZER_LEVEL 13 
bool waterLevelLow = false;
bool fertilizerLevelLow=false;

//RTC
const char* ntpServer = "in.pool.ntp.org";
const long gmtOffset_sec = 19800;  //Adjusted to SL timezone (+5.30)
const int daylightOffset_sec = 0;

//LED
const int LEDRelayPin = 4;
const int FertilizerRelayPin = 5;
const int relayDailyOnHour = 19; //LED ON time
const int relayDailyOffHour = 24; //LED OFF time
bool ledState = false; // Store LED state

//Fertilization
const int relayWeeklyOnHour = 10;  
const int relayWeeklyOnMinute =15;
const int relayWeeklyOnDay= 0;
const unsigned long fertilizerDuration = 10000;  // 10 seconds
unsigned long fertilizerStartTime = 0;
bool fertilizerActivated = false;

unsigned long lastManualFertilizerTime = 0;
const unsigned long manualFertilizerCooldown = 24 * 60 * 60 * 1000; 

 
WiFiClient client;
WebServer Webserver(80);

//DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

struct tm lastWateredTime;


void connectWiFi() {
    WiFi.begin(ssid1, password1);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
      lcd.setCursor(0, 0);
      lcd.println("Connecting...");
    }
    lcd.clear();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Failed to connect to WiFi, trying backup...");
      lcd.setCursor(0, 0);
      lcd.println("WiFi Failed");
      lcd.setCursor(0, 1);
      lcd.println("Trying backup...");
      WiFi.begin(ssid2, password2);
      startAttemptTime = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(1000);
        Serial.println("Connecting to backup WiFi...");
      }
    }
    lcd.clear();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to WiFi");
      lcd.setCursor(3, 0);
      lcd.print("Connected");
    } else {
      Serial.println("Failed to connect to any WiFi");
      lcd.setCursor(0, 0);
      lcd.println("WiFi Failed");
    }
      
/*  WiFi.begin(AP_SSID, AP_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
    }

    Serial.println("Connected to the WiFi network");

    // Login to Captive Portal
    HTTPClient login;
    login.begin("https://wlan.uom.lk/login.html");
    String body = "user=" + String(username) + "&password=" + String(password) + "&cmd=authenticate&Login=Log%2BIn";
    int res = login.POST(body);

    Serial.println(res);

    if (res == 200) {
      Serial.println("Authentication UOM successful");
      lcd.clear();
      lcd
    }

    login.end();*/


}

void DisplaySensorReadings(float temperature, int soilMoisturePercent) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp :");
  lcd.print(temperature);
  lcd.print("°C");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  lcd.setCursor(0, 1);
  lcd.print("Soil Moist:");
  lcd.print(soilMoisturePercent);
  lcd.print("%");
  delay(1000);
  Serial.print(" - Moisture Level: ");
  Serial.println(soilMoisturePercent);
  Serial.println("Sensor Value");
  Serial.println(soilMoistureValue);
}

void clearRow(int row) {
  lcd.setCursor(0, row);
  for (int i = 0; i < 16; i++) {
    lcd.print(" ");
  }
  lcd.setCursor(0, row);
}

//Fertilizer Application
      /* void checkFertilizer() {
         struct tm timeinfo;
         if (!getLocalTime(&timeinfo)) {
           return;
         }
        
         if (timeinfo.tm_wday == relayWeeklyOnDay && timeinfo.tm_hour == relayWeeklyOnHour && timeinfo.tm_min == relayWeeklyOnMinute &&!fertilizerActivated){
           digitalWrite(FertilizerRelayPin, LOW);
           fertilizerStartTime = millis();
           fertilizerActivated = true;
         }
        
         if (fertilizerActivated && millis() - fertilizerStartTime >= fertilizerDuration) {
           digitalWrite(FertilizerRelayPin, HIGH);
           fertilizerActivated = false;
           lcd.clear();
           lcd.setCursor(0,0);
           lcd.print("Weekly fertilizer");
           lcd.setCursor(0,1);
           lcd.print("dose given");
         }
        
         // Reset weekly flag
         if (timeinfo.tm_wday == 0 && timeinfo.tm_hour == 0 && timeinfo.tm_min == 0) {
          fertilizerActivated = false;
         }
       }*/

void checkFertilizer() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  
  // Check if enough time has passed since last manual activation
  if (millis() - lastManualFertilizerTime < manualFertilizerCooldown) {
    return;  // Skip automatic activation if manual activation was recent
  }
  
  if (timeinfo.tm_wday == relayWeeklyOnDay && timeinfo.tm_hour == relayWeeklyOnHour && timeinfo.tm_min == relayWeeklyOnMinute && !fertilizerActivated) {
    digitalWrite(FertilizerRelayPin, LOW);
    fertilizerStartTime = millis();
    fertilizerActivated = true;
  }
  
  if (fertilizerActivated && millis() - fertilizerStartTime >= fertilizerDuration) {
    digitalWrite(FertilizerRelayPin, HIGH);
    fertilizerActivated = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Weekly fertilizer");
    lcd.setCursor(0,1);
    lcd.print("dose given");
  }
  
  // Reset weekly flag
  if (timeinfo.tm_wday == 0 && timeinfo.tm_hour == 0 && timeinfo.tm_min == 0) {
    fertilizerActivated = false;
  }
}

void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");
  lcd.init();
  lcd.backlight();
  lcd.setCursor(4, 0);
  lcd.print("Welcome");
  delay(1000);
  lcd.clear();
  connectWiFi();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  lcd.setCursor(2, 0);
  lcd.print("IP address: ");
  lcd.setCursor(0, 1);
  lcd.println(WiFi.localIP());
  delay(4000);
  ThingSpeak.begin(client);
  sensors.begin();
  Webserver.on("/", handleRoot);
  Webserver.on("/data", handleData);
  Webserver.on("/toggleLED", handleToggleLED); // Register the new handler
  Webserver.on("/ActiveFertilizer", handleActiveFertilizer);
  Webserver.begin();
  Serial.println("HTTP server started");
  pinMode(SOIL_SENSOR_PIN, INPUT);
  pinMode(COOLING_PIN, OUTPUT);
  pinMode(HEATING_PIN, OUTPUT);
  pinMode(WATER_RELAY_PIN, OUTPUT);
  pinMode(LEDRelayPin, OUTPUT);
  pinMode(FertilizerRelayPin, OUTPUT);
  pinMode(WATER_LEVEL, INPUT_PULLUP);
  pinMode(FERTILIZER_LEVEL, INPUT_PULLUP);
  digitalWrite(COOLING_PIN, HIGH);
  digitalWrite(HEATING_PIN, HIGH);
  digitalWrite(WATER_RELAY_PIN, LOW);
  delay(4000);
  digitalWrite(LEDRelayPin, HIGH);
  digitalWrite(FertilizerRelayPin, HIGH);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void loop() {

  //soil moisture
  soilMoistureValue = analogRead(SOIL_SENSOR_PIN);
  int soilMoisturePercent = map(soilMoistureValue, DRY_VALUE, WET_VALUE, 0, 100);

  //check if wifi connection is not failed
  Webserver.handleClient();
  if (WiFi.status() == !WL_CONNECTED){ //changed   
    connectWiFi();
  }
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }

  // Read water level & fertilizer
  waterLevelLow = digitalRead(WATER_LEVEL) == HIGH;
  //Serial.println(waterLevelLow ? "Water level is OK" : "Water level is LOW");
  fertilizerLevelLow=digitalRead(FERTILIZER_LEVEL)==HIGH;
  //Serial.println(fertilizerLevelLow? "Fertilizer level is OK" : "Fertilizer level is LOW");

  //temperature
  sensors.requestTemperatures();
 /* float temperature = sensors.getTempCByIndex(0)-2; ---> */ float temperature = sensors.getTempCByIndex(0);


    //LED 
  if(ledState==true){
    digitalWrite(LEDRelayPin, LOW);
    ledState=true;
  }else{
    if (timeinfo.tm_hour >= relayDailyOnHour && timeinfo.tm_hour < relayDailyOffHour) {
      digitalWrite(LEDRelayPin, LOW);
      ledState=true;
      Serial.println("Daily Relay ON");
    } else {
      digitalWrite(LEDRelayPin, HIGH);
      Serial.println("Daily Relay OFF");
      ledState=false;
    }
  }
  // Check and turn off fertilizer pump if duration exceeded
  if (fertilizerActivated && millis() - fertilizerStartTime >= fertilizerDuration) {
    digitalWrite(FertilizerRelayPin, HIGH);
    fertilizerActivated = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Fertilizer dose");
    lcd.setCursor(0,1);
    lcd.print("completed");
  }
  //sending data to json file 
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["soilMoisture"] = soilMoisturePercent;
  doc["WateringTimeHour"] = lastWateredTime.tm_hour;
  doc["WateringTimeMin"] = lastWateredTime.tm_min;
  doc["WateringMonth"] = lastWateredTime.tm_mon;
  doc["WateringDay"] = lastWateredTime.tm_mday;
  doc["waterLevelLow"] = waterLevelLow ?  "OK":"LOW" ;
  doc["fertilizerLevelLow"] = fertilizerLevelLow ? "OK":"LOW" ;
  doc["ledState"] = ledState;
  doc["fertilizerActivated"]=fertilizerActivated;
  doc["waterPumpActivated"]=waterPumpActivated;

  File file = SPIFFS.open("/data.json", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
  }
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write to file");
  }

  //Display Sensor Readings
  DisplaySensorReadings(temperature, soilMoisturePercent);

  if (WiFi.status() == WL_CONNECTED) {
    ThingSpeak.setField(1, soilMoisturePercent);
    ThingSpeak.setField(2, temperature);
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
  }

  if(temperature > upperThreshold) {
      digitalWrite(COOLING_PIN, LOW);
      digitalWrite(HEATING_PIN, HIGH);
      clearRow(1);
      lcd.println("Cooling On");
      Serial.println("Cooling Peltier and Fan ON");
      delay(5000);
    }else if (temperature < lowerThreshold) {
      digitalWrite(HEATING_PIN, LOW);
      digitalWrite(COOLING_PIN, HIGH);
      Serial.println("Heating ON");
      clearRow(1);
      lcd.println("Heating ON");
      delay(5000);
    }else {
      digitalWrite(COOLING_PIN, HIGH);
      digitalWrite(HEATING_PIN, HIGH);
      Serial.println("In the preferred temperature range");
  }
  delay(1000);
  DisplaySensorReadings(temperature, soilMoisturePercent);

  if ( soilMoisturePercent < MOISTURE_THRESHOLD) { 
      digitalWrite(WATER_RELAY_PIN, LOW);
      Serial.println("Watering the plant...");
      clearRow(0);
      lcd.println("Watering ");
      lastWateredTime = timeinfo; // Update last watered time
  } else {
    digitalWrite(WATER_RELAY_PIN, HIGH);
    Serial.println("Plant has enough water.");
  }

/*  if(waterPumpActivated==true){
    digitalWrite(WATER_RELAY_PIN, LOW);
    clearRow(0);
    lcd.println("Watering ");
  }
*/
  checkFertilizer();

  Webserver.handleClient();
  delay(3000);
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void handleData() {
  File file = SPIFFS.open("/data.json", FILE_READ);
  if (!file) {
    Webserver.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to open file\"}");
    return;
  }
  String json = file.readString();
  file.close();
  Webserver.send(200, "application/json", json);
}

void handleToggleLED() {
  ledState = !ledState;
  digitalWrite(LEDRelayPin, ledState ? LOW : HIGH);
  Serial.println(ledState ? "LED is ON" : "LED is OFF");
  Webserver.send(200, "text/plain", ledState ? "LED is ON" : "LED is OFF");
}


void handleActiveFertilizer() {
  fertilizerActivated = !fertilizerActivated;
  if (fertilizerActivated) {
    digitalWrite(FertilizerRelayPin, LOW);
    fertilizerStartTime = millis();
    lastManualFertilizerTime = millis();
  } else {
    digitalWrite(FertilizerRelayPin, HIGH);
  }
  Webserver.send(200, "text/plain", fertilizerActivated ? "Fertilizer Pump ON" : "Fertilizer Pump OFF");
}
void handleActivewaterPump(){
  waterPumpActivated=!waterPumpActivated;
  if(waterPumpActivated){
    digitalWrite(WATER_RELAY_PIN,LOW);
    wateractivated=millis();
  }
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Plant Monitoring Dashboard</title>
    <style>
        :root {
            --primary-color: #4CAF50;
            --secondary-color: #2196F3;
            --background-color: #f0f4f8;
            --card-background: #ffffff;
            --text-color: #333333;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
        }
        .header {
            background-color: var(--primary-color);
            color: white;
            padding: 20px;
            text-align: center;
            position: fixed;
            display: flex;
            width: 100%;
            justify-content: space-between;
            /* align-items: center; */
            top: 0;
            left: 0;
            z-index: 1000;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .header h1 {
            margin: 0;
            font-size: 24px;
        }
        .datetime {
            font-size: 14px;
            margin-top: 5px;
            font-weight: bold;
            margin-right: 30px; 
        }
        .container {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            padding: 100px 20px 20px;
            max-width: 1200px;
            margin: 0 auto;
        }
        .card {
            background-color: var(--card-background);
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            padding: 20px;
            text-align: center;
            transition: transform 0.3s ease;
        }
        .card:hover {
            transform: translateY(-5px);
        }
        .card h2 {
            color: var(--primary-color);
            font-size: 18px;
            margin-top: 0;
        }
        .sensor-value {
            font-size: 24px;
            font-weight: bold;
            color: var(--secondary-color);
        }
        .graph {
            grid-column: span 2;
        }
        iframe {
            width: 100%;
            height: 300px;
            border: none;
            border-radius: 10px;
        }
        button {
            background-color: var(--primary-color);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            cursor: pointer;
            transition: background-color 0.3s ease;
        }
        button:hover {
            background-color: #45a049;
        }
        @media (max-width: 768px) {
            .container {
                grid-template-columns: 1fr;
            }
            .graph {
                grid-column: span 1;
            }
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>TERRASMART</h1>
        <div class="datetime" id="datetime"></div>
    </div>
    <div class="container">
        <div class="card" id="temperature">
            <h2>Temperature</h2>
            <p class="sensor-value" id="temp">Loading...</p>
        </div>
        <div class="card" id="soil-moisture">
            <h2>Soil Moisture</h2>
            <p class="sensor-value" id="soilMoisture">Loading...</p>
        </div>
        <div class="card" id="last-watered">
            <h2>Last Watering Time</h2>
            <p class="sensor-value" id="lastWateringTime">Loading...</p>
        </div>
        <div class="card" id="water-level">
            <h2>Water Level Status</h2>
            <p class="sensor-value" id="WaterLevel">Loading...</p>
        </div>

        <div class="card graph" id="thingspeak-1">
            <h2>Soil Moisture Graph</h2>
            <iframe src="https://thingspeak.com/channels/2583019/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&type=line&update=15"></iframe>
        </div>
        <div class="card graph" id="thingspeak-2">
            <h2>Temperature Graph</h2>
            <iframe src="https://thingspeak.com/channels/2583019/charts/2?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&type=line"></iframe>
        </div>
        <div class="card" id="fertilizer-level">
            <h2>Fertilizer Level Status</h2>
            <p class="sensor-value" id="FertilizerLevel">Loading...</p>
        </div>
        <div class="card">
            <h2>LED Control</h2>
            <button id="ledButton" onclick="toggleLED()">LED OFF</button>
        </div>
  
        <div class="card">
            <h2>Fertilizer Pump</h2>
            <button id="fertilizerButton" onclick="ActiveFertilizer()">OFF</button>
        </div>
        <div class="card">
            <h2>Water Pump</h2>
            <button id="waterButton" onclick="ActiveWater()">OFF</button>
        </div>
    </div>
    <script>
        let ledState = false;

        async function fetchData() {
            const response = await fetch('/data');
            const data = await response.json();
            document.getElementById('temp').innerText = data.temperature + ' °C';
            document.getElementById('soilMoisture').innerText = data.soilMoisture + ' %';
            document.getElementById('lastWateringTime').innerText = data.WateringMonth+'/'+data.WateringDay+' , '+data.WateringTimeHour + ':' + data.WateringTimeMin;
            document.getElementById('WaterLevel').innerText = data.waterLevelLow ;
            document.getElementById('FertilizerLevel').innerText = data.fertilizerLevelLow ;
            ledState = data.ledState;
            updateLEDButton();
            fertilizerActivated=data.fertilizerActivated;
            updateFertilizerButton();

        }

        async function toggleLED() {
            const response = await fetch('/toggleLED');
            const newState = await response.text();
            ledState = newState.trim() === "LED is ON";
            updateLEDButton();

        }

        function updateLEDButton() {
            const button = document.getElementById('ledButton');
            button.textContent = ledState ? "LED ON" : "LED OFF";
        }

        async function ActiveFertilizer(){
          const response=await fetch('/ActiveFertilizer');
          const newState=await response.text();
          fertilizerActivated=newState.trim()==="Fertilizer Pump ON";
          updateFertilizerButton();
        }

        function updateFertilizerButton() {
          const button = document.getElementById('fertilizerButton');
          button.textContent = fertilizerActivated ? "ON" : "OFF";
        }
        
        function updateDateTime() {
            const now = new Date();
            const dateTimeString = now.toLocaleString();
            document.getElementById('datetime').innerText = dateTimeString;
        }

        fetchData();
        setInterval(fetchData, 20000);
        setInterval(updateDateTime, 1000);
        updateDateTime();
    </script>
</body>
</html>><!DOCTYPE HTML>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Plant Monitoring Dashboard</title>
    <style>
        :root {
            --primary-color: #4CAF50;
            --secondary-color: #2196F3;
            --background-color: #f0f4f8;
            --card-background: #ffffff;
            --text-color: #333333;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
        }
        .header {
            background-color: var(--primary-color);
            color: white;
            padding: 20px;
            text-align: center;
            position: fixed;
            display: flex;
            width: 100%;
            justify-content: space-between;
            /* align-items: center; */
            top: 0;
            left: 0;
            z-index: 1000;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .header h1 {
            margin: 0;
            font-size: 24px;
        }
        .datetime {
            font-size: 14px;
            margin-top: 5px;
            font-weight: bold;
            margin-right: 30px; 
        }
        .container {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            padding: 100px 20px 20px;
            max-width: 1200px;
            margin: 0 auto;
        }
        .card {
            background-color: var(--card-background);
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            padding: 20px;
            text-align: center;
            transition: transform 0.3s ease;
        }
        .card:hover {
            transform: translateY(-5px);
        }
        .card h2 {
            color: var(--primary-color);
            font-size: 18px;
            margin-top: 0;
        }
        .sensor-value {
            font-size: 24px;
            font-weight: bold;
            color: var(--secondary-color);
        }
        .graph {
            grid-column: span 2;
        }
        iframe {
            width: 100%;
            height: 300px;
            border: none;
            border-radius: 10px;
        }
        button {
            background-color: var(--primary-color);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            cursor: pointer;
            transition: background-color 0.3s ease;
        }
        button:hover {
            background-color: #45a049;
        }
        @media (max-width: 768px) {
            .container {
                grid-template-columns: 1fr;
            }
            .graph {
                grid-column: span 1;
            }
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>TERRASMART</h1>
        <div class="datetime" id="datetime"></div>
    </div>
    <div class="container">
        <div class="card" id="temperature">
            <h2>Temperature</h2>
            <p class="sensor-value" id="temp">Loading...</p>
        </div>
        <div class="card" id="soil-moisture">
            <h2>Soil Moisture</h2>
            <p class="sensor-value" id="soilMoisture">Loading...</p>
        </div>
        <div class="card" id="last-watered">
            <h2>Last Watering Time</h2>
            <p class="sensor-value" id="lastWateringTime">Loading...</p>
        </div>
        <div class="card" id="water-level">
            <h2>Water Level Status</h2>
            <p class="sensor-value" id="WaterLevel">Loading...</p>
        </div>

        <div class="card graph" id="thingspeak-1">
            <h2>Soil Moisture Graph</h2>
            <iframe src="https://thingspeak.com/channels/2583019/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&type=line&update=15"></iframe>
        </div>
        <div class="card graph" id="thingspeak-2">
            <h2>Temperature Graph</h2>
            <iframe src="https://thingspeak.com/channels/2583019/charts/2?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&type=line"></iframe>
        </div>
        <div class="card" id="fertilizer-level">
            <h2>Fertilizer Level Status</h2>
            <p class="sensor-value" id="FertilizerLevel">Loading...</p>
        </div>
        <div class="card">
            <h2>LED Control</h2>
            <button id="ledButton" onclick="toggleLED()">LED OFF</button>
        </div>
  
        <div class="card">
            <h2>Fertilizer Pump</h2>
            <button id="fertilizerButton" onclick="ActiveFertilizer()">OFF</button>
        </div>
        <div class="card">
            <h2>Water Pump</h2>
            <button id="waterButton" onclick="ActiveWater()">OFF</button>
        </div>
    </div>
    <script>
        let ledState = false;

        async function fetchData() {
            const response = await fetch('/data');
            const data = await response.json();
            document.getElementById('temp').innerText = data.temperature + ' °C';
            document.getElementById('soilMoisture').innerText = data.soilMoisture + ' %';
            document.getElementById('lastWateringTime').innerText = data.WateringMonth+'/'+data.WateringDay+' , '+data.WateringTimeHour + ':' + data.WateringTimeMin;
            document.getElementById('WaterLevel').innerText = data.waterLevelLow ;
            document.getElementById('FertilizerLevel').innerText = data.fertilizerLevelLow ;
            ledState = data.ledState;
            updateLEDButton();
            fertilizerActivated=data.fertilizerActivated;
            updateFertilizerButton();

        }

        async function toggleLED() {
            const response = await fetch('/toggleLED');
            const newState = await response.text();
            ledState = newState.trim() === "LED is ON";
            updateLEDButton();

        }

        function updateLEDButton() {
            const button = document.getElementById('ledButton');
            button.textContent = ledState ? "LED ON" : "LED OFF";
        }

        async function ActiveFertilizer(){
          const response=await fetch('/ActiveFertilizer');
          const newState=await response.text();
          fertilizerActivated=newState.trim()==="Fertilizer Pump ON";
          updateFertilizerButton();
        }

        function updateFertilizerButton() {
          const button = document.getElementById('fertilizerButton');
          button.textContent = fertilizerActivated ? "ON" : "OFF";
        }
        
        function updateDateTime() {
            const now = new Date();
            const dateTimeString = now.toLocaleString();
            document.getElementById('datetime').innerText = dateTimeString;
        }

        fetchData();
        setInterval(fetchData, 20000);
        setInterval(updateDateTime, 1000);
        updateDateTime();
    </script>
</body>
</html>
  )rawliteral";
  Webserver.send(200, "text/html", html);
}

