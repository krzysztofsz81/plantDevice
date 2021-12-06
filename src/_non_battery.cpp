// #include <Arduino.h>
// #include <WiFi.h>
// #include <WiFiMulti.h>
// #include <ArduinoJson.h>
// #include <WebSocketsClient.h>
// #include <secrets.h>

// WiFiMulti WIFI;
// WebSocketsClient webSocket;

// #define LED_GREEN_PIN GPIO_NUM_27
// #define SOIL_MOISTURE_PIN A2
// #define SOIL_TEMP_PIN A3
// #define LIPO_PIN A13

// String action_id;
// String action_state;

// char cstr[7];
// char message[32];

// bool readyToSendData = false;

// String getValueFromPayload(String data, char separator, int index)
// {
//     int found = 0;
//     int strIndex[] = {0, -1};
//     int maxIndex = data.length() - 1;

//     for (int i = 0; i <= maxIndex && found <= index; i++)
//     {
//         if (data.charAt(i) == separator || i == maxIndex)
//         {
//             found++;
//             strIndex[0] = strIndex[1] + 1;
//             strIndex[1] = (i == maxIndex) ? i + 1 : i;
//         }
//     }
//     return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
// }

// void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
// {

//     switch (type)
//     {
//     case WStype_DISCONNECTED:
//         Serial.println("[WS] Disconnected");
//         break;
//     case WStype_CONNECTED:
//         Serial.printf("[WS] Connected: %s\n", payload);
//         webSocket.sendTXT("REGISTER_OUTPUT/led_green:Boolean/false");
//         webSocket.sendTXT("REGISTER_INPUT/soil:Percent");

//         strcpy(message, "UPDATE/soil:Percent/");
//         strcat(message, itoa(map(analogRead(SOIL_MOISTURE_PIN), 0, 4095, 100, 0), cstr, DEC));
//         webSocket.sendTXT(message);
//         break;
//     case WStype_TEXT:
//         Serial.printf("[WS] Response: %s\n", payload);
//         action_id = getValueFromPayload((char *)payload, '/', 0);
//         action_state = getValueFromPayload((char *)payload, '/', 1);
//         if (action_id == "led_green") {
//             digitalWrite(LED_GREEN_PIN, action_state == "true" ? HIGH : LOW);
//         }
//         break;
//     case WStype_BIN:
//     case WStype_ERROR:
//         Serial.printf("[WS] Error: %s\n", payload);
//     case WStype_FRAGMENT_TEXT_START:
//     case WStype_FRAGMENT_BIN_START:
//     case WStype_FRAGMENT:
//     case WStype_FRAGMENT_FIN:
//     case WStype_PING:
//     case WStype_PONG:
//         break;
//     }
// }

// void connectToWiFi()
// {
//     WIFI.addAP(WIFI_SSID, WIFI_PASS);
//     while (WIFI.run() != WL_CONNECTED) delay(100);
// }

// void initWebSocket()
// {
//     webSocket.begin(WS_HOST, WS_PORT, "/?deviceId=plant_1");
//     webSocket.onEvent(webSocketEvent);
//     // webSocket.setAuthorization("user", "Password");
//     webSocket.setReconnectInterval(5000);
// }

// void setup()
// {
//     pinMode(LED_GREEN_PIN, OUTPUT);
//     pinMode(LIPO_PIN, OUTPUT);

//     Serial.begin(115200);
//     Serial.setDebugOutput(true);

//     connectToWiFi();
//     initWebSocket();
// }

// void loop()
// {
//     webSocket.loop();
// }