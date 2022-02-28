#include <Arduino.h>
#include <Preferences.h>

#include <secrets.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

// DHT21 sensor lib
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// DS18B20 sensor lib
#include <OneWire.h>
#include <DallasTemperature.h>


// pins
#define DHT_PIN 4
#define SOIL_MOISTURE_PIN A0
#define SOIL_TEMPERATURE_PIN 17
#define WATER_PUMP_PIN 13

#define WIFI_TIMEOUT 1000

Preferences preferences;

OneWire oneWire(SOIL_TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);
DHT_Unified dht(DHT_PIN, DHT21);
WebSocketsClient webSocket;
WiFiMulti WIFI;

String action_id;
String action_periphal;
String action_state;

String waterPumpDef = "water_pump:BUTTON/";
String waterPumpTimeDef = "water_pump_time:RANGE(1000-10000)/";
String soilMoistureDef = "soil_moisture:RANGE(0-4095)/";
String soilTemperatureDef = "soil_temperature:CELSIUS/";
String dhtHumidityDef = "dht_humidity:PERCENT/";
String dhtTemperatureDef = "dht_temperature:CELSIUS/";

int waterPumpTime = preferences.getUInt("time", 2000);
int rangedWaterPumpTime;

String getValueFromPayload(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

float getDHTTemperature() {
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    return event.temperature;
}

float getDHTHumidity() {
    sensors_event_t event;
    dht.humidity().getEvent(&event);
    return event.relative_humidity;
}

float getSoilTemperature() {
    sensors.requestTemperatures();
    delay(1000);
    return sensors.getTempCByIndex(0);
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            webSocket.sendTXT("REGISTER_INPUT/" + waterPumpDef + "false");            
            webSocket.sendTXT("REGISTER_INPUT/" + waterPumpTimeDef + String(waterPumpTime));
            webSocket.sendTXT("REGISTER_OUTPUT/" + soilMoistureDef + String(4095 - analogRead(SOIL_MOISTURE_PIN)));
            webSocket.sendTXT("REGISTER_OUTPUT/" + soilTemperatureDef + String(getSoilTemperature()));
            webSocket.sendTXT("REGISTER_OUTPUT/" + dhtHumidityDef + String(getDHTHumidity()));
            webSocket.sendTXT("REGISTER_OUTPUT/" + dhtTemperatureDef + String(getDHTTemperature()));
            break;
        case WStype_TEXT:
            action_id = getValueFromPayload((char *)payload, '/', 0);
            if (action_id == "RETRIEVE_PERIPHAL_DATA") {
                webSocket.sendTXT("UPDATE/" + soilMoistureDef + String(4095 - analogRead(SOIL_MOISTURE_PIN)));
                webSocket.sendTXT("UPDATE/" + soilTemperatureDef + String(getSoilTemperature()));
                webSocket.sendTXT("UPDATE/" + dhtHumidityDef + String(getDHTHumidity()));
                webSocket.sendTXT("UPDATE/" + dhtTemperatureDef + String(getDHTTemperature()));
            }
            if (action_id == "SET_PERIPHAL_DATA") {
                action_periphal = getValueFromPayload((char *)payload, '/', 1);
                action_state = getValueFromPayload((char *)payload, '/', 2);
                if (action_periphal == "water_pump_time") {
                    waterPumpTime = action_state.toInt();
                    preferences.putUInt("time", waterPumpTime);
                }
                if (action_periphal == "water_pump") {
                    if (action_state == "true") {
                        digitalWrite(WATER_PUMP_PIN, LOW);
                        if (waterPumpTime < 1000) rangedWaterPumpTime = 1000;
                        if (waterPumpTime > 10000) rangedWaterPumpTime = 10000;
                        if (waterPumpTime >= 1000 && waterPumpTime <= 10000) rangedWaterPumpTime = waterPumpTime;
                        delay(rangedWaterPumpTime);
                        digitalWrite(WATER_PUMP_PIN, HIGH);
                        webSocket.sendTXT("UPDATE/" + waterPumpDef + "false");
                    } else {
                        digitalWrite(WATER_PUMP_PIN, HIGH);
                    }
                }
            }
            break;
    }
}

void connectToWiFi() {
    WIFI.addAP(WIFI_SSID, WIFI_PASS);
    while (WIFI.run() != WL_CONNECTED) delay(WIFI_TIMEOUT);
}

void initWebSocket() {
    webSocket.begin(WS_HOST, WS_PORT, "/?deviceId=plant_1");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void setup() {
    pinMode(WATER_PUMP_PIN, OUTPUT);
    digitalWrite(WATER_PUMP_PIN, HIGH);

    pinMode(SOIL_MOISTURE_PIN, INPUT);
    pinMode(SOIL_TEMPERATURE_PIN, INPUT);
    pinMode(DHT_PIN, INPUT);

    preferences.begin("plantDevice", false); 
    dht.begin();
    sensors.begin();
    sensors.requestTemperatures();

    Serial.begin(115200);
    Serial.setDebugOutput(true);

    connectToWiFi();
    initWebSocket();
}

void loop() {
    webSocket.loop();
}