#include <Arduino.h>
#include <secrets.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

// dht sensor lib
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

OneWire oneWire(SOIL_TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);
DHT_Unified dht(DHT_PIN, DHT21);
WebSocketsClient webSocket;
WiFiMulti WIFI;

String action_id;
String action_periphal;
String action_state;

int waterPumpTime = 2000;

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
    // if (isnan(event.temperature)) {
    //     Serial.println(F("Error reading temperature!"));
    // } else {
    //     Serial.print(F("Temperature: "));
    //     Serial.print(event.temperature);
    //     Serial.println(F("°C"));
    // }
    return event.temperature;
}

float getDHTHumidity() {
    sensors_event_t event;
    dht.humidity().getEvent(&event);
    // if (isnan(event.relative_humidity)) {
    //     Serial.println(F("Error reading humidity!"));
    // } else {
    //     Serial.print(F("Humidity: "));
    //     Serial.print(event.relative_humidity);
    //     Serial.println(F("%"));
    // }
    return event.relative_humidity;
}

float getSoilTemperature() {
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    Serial.print("webSocketEvent type: ");
    Serial.println(type); 
    switch (type) {
        case WStype_ERROR:
            Serial.println("[WS] WStype_ERROR");
            break;
        case WStype_DISCONNECTED:
            Serial.println("[WS] WStype_DISCONNECTED");
            break;
        case WStype_CONNECTED:
            Serial.printf("[WS] WStype_CONNECTED: %s\n", payload);
            webSocket.sendTXT("REGISTER_INPUT/water_pump:Boolean/false");
            webSocket.sendTXT("REGISTER_INPUT/water_pump_time:Int/2000");
            webSocket.sendTXT("REGISTER_OUTPUT/soil_moisture:Int:Percent:true/" + String(analogRead(SOIL_MOISTURE_PIN)));
            webSocket.sendTXT("REGISTER_OUTPUT/soil_temperature:Float:Celsius/" + String(getSoilTemperature()));
            webSocket.sendTXT("REGISTER_OUTPUT/dht_humidity:Float:Percent/" + String(getDHTTemperature()));
            webSocket.sendTXT("REGISTER_OUTPUT/dht_temperature:Float:Celsius/" + String(getDHTHumidity()));
            break;
        case WStype_TEXT:
            Serial.printf("[WS] WStype_TEXT: %s\n", payload);
            action_id = getValueFromPayload((char *)payload, '/', 0);
            if (action_id == "RETRIEVE_PERIPHAL_DATA") {
                webSocket.sendTXT("UPDATE/soil_moisture:Int:Percent:true/" + String(analogRead(SOIL_MOISTURE_PIN)));
                webSocket.sendTXT("UPDATE/soil_temperature:Float:Celsius/" + String(getSoilTemperature()));
                webSocket.sendTXT("UPDATE/dht_humidity:Float:Percent/" + String(getDHTTemperature()));
                webSocket.sendTXT("UPDATE/dht_temperature:Float:Celsius/" + String(getDHTHumidity()));
            }
            if (action_id == "SET_PERIPHAL_DATA") {
                action_periphal = getValueFromPayload((char *)payload, '/', 1);
                action_state = getValueFromPayload((char *)payload, '/', 2);
                if (action_periphal == "water_pump_time") {
                    Serial.println("[WATER PUMP] Set time: " + String(action_state.toInt()));
                    waterPumpTime = action_state.toInt();
                }
                if (action_periphal == "water_pump") {
                    if (action_state == "true") {
                        Serial.println("[WATER PUMP] Enabled");
                        digitalWrite(WATER_PUMP_PIN, LOW);

                        Serial.print("[WATER PUMP] Delay: ");
                        Serial.println(waterPumpTime);
                        delay(waterPumpTime);
                        digitalWrite(WATER_PUMP_PIN, HIGH);
                        webSocket.sendTXT("UPDATE/water_pump:Boolean/false");
                    } else {
                        Serial.println("[WATER PUMP] Disabled");
                        digitalWrite(WATER_PUMP_PIN, HIGH);
                    }
                }
            }
            break;
        case WStype_BIN:
            Serial.println("[WS] WStype_BIN");
            break;
        case WStype_FRAGMENT_TEXT_START:
            Serial.println("[WS] WStype_FRAGMENT_TEXT_START");
            break;
        case WStype_FRAGMENT_BIN_START:
            Serial.println("[WS] WStype_FRAGMENT_BIN_START");
            break;
        case WStype_FRAGMENT:
            Serial.println("[WS] WStype_FRAGMENT");
            break;
        case WStype_FRAGMENT_FIN:
            Serial.println("[WS] WStype_FRAGMENT_FIN");
            break;
        case WStype_PING:
            Serial.println("[WS] WStype_PING");
            break;
        case WStype_PONG:
            Serial.println("[WS] WStype_PONG");
            break;
    }
}

void connectToWiFi() {
    WIFI.addAP(WIFI_SSID, WIFI_PASS);
    while (WIFI.run() != WL_CONNECTED) {
        Serial.println("[WIFI] Connecting ...");
         delay(WIFI_TIMEOUT);
    }
    Serial.println("[WIFI] Connected");
}

void initWebSocket() {
    Serial.println("[WebSocket] Connecting ...");
    webSocket.begin(WS_HOST, WS_PORT, "/?deviceId=plant_1");
    webSocket.onEvent(webSocketEvent);
    // webSocket.setAuthorization("user", "Password");
    webSocket.setReconnectInterval(5000);
}

void setup() {
    pinMode(WATER_PUMP_PIN, OUTPUT);
    digitalWrite(WATER_PUMP_PIN, HIGH);

    pinMode(SOIL_MOISTURE_PIN, INPUT);
    pinMode(SOIL_TEMPERATURE_PIN, INPUT);
    pinMode(DHT_PIN, INPUT);

    dht.begin();
    sensors.begin();

    Serial.begin(115200);
    Serial.setDebugOutput(true);

    connectToWiFi();
    initWebSocket();
}

void loop() {
    webSocket.loop();
}