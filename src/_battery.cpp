// #include <Arduino.h>
// #include <WiFi.h>
// #include <WiFiMulti.h>
// #include <ArduinoJson.h>
// #include <WebSocketsClient.h>
// #include <secrets.h>
// #include "driver/adc.h"
// #include <esp_wifi.h>
// #include <esp_bt.h>

// WiFiMulti WIFI;
// WebSocketsClient webSocket;

// #define LED_GREEN_PIN GPIO_NUM_27
// #define SOIL_MOISTURE_PIN A2
// #define SOIL_TEMP_PIN A3
// #define LIPO_PIN A13

// #define uS_TO_MIN_FACTOR 60L * 1000000L /* Conversion factor for micro seconds to minutes */
// #define DEEP_SLEEP_TIME 15              /* Time ESP32 will go to sleep (in minutes) */

// String action_id;
// String action_state;

// char cstr[7];
// char message[32];

// char voltageBuffer[8];
// float voltage;

// bool readyToSendData = false;

// void goToDeepSleep()
// {
//     Serial.println("Going to sleep...");
//     WiFi.disconnect(true);
//     WiFi.mode(WIFI_OFF);
//     btStop();

//     adc_power_off();
//     esp_wifi_stop();
//     esp_bt_controller_disable();

//     // Configure the timer to wake us up!
//     esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME * uS_TO_MIN_FACTOR);

//     // Go to sleep! Zzzz
//     esp_deep_sleep_start();
// }

// void print_wakeup_reason()
// {
//     esp_sleep_wakeup_cause_t wakeup_reason;
//     wakeup_reason = esp_sleep_get_wakeup_cause();

//     switch (wakeup_reason)
//     {
//     case ESP_SLEEP_WAKEUP_EXT0:
//         Serial.println("Wakeup caused by external signal using RTC_IO");
//         break;
//     case ESP_SLEEP_WAKEUP_EXT1:
//         Serial.println("Wakeup caused by external signal using RTC_CNTL");
//         break;
//     case ESP_SLEEP_WAKEUP_TIMER:
//         Serial.println("Wakeup caused by timer");
//         break;
//     case ESP_SLEEP_WAKEUP_TOUCHPAD:
//         Serial.println("Wakeup caused by touchpad");
//         break;
//     case ESP_SLEEP_WAKEUP_ULP:
//         Serial.println("Wakeup caused by ULP program");
//         break;
//     default:
//         Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
//         break;
//     }
// }

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
//         Serial.println("Deep sleep: START");
//         goToDeepSleep();
//         break;
//     case WStype_CONNECTED:
//         Serial.printf("[WS] Connected: %s\n", payload);
//         webSocket.sendTXT("REGISTER_OUTPUT/led_green:Boolean/false");
//         webSocket.sendTXT("REGISTER_INPUT/soil:Percent");
//         webSocket.sendTXT("REGISTER_INPUT/battery:Float");

//         strcpy(message, "UPDATE/soil:Percent/");
//         strcat(message, itoa(map(analogRead(SOIL_MOISTURE_PIN), 0, 4095, 100, 0), cstr, DEC));
//         webSocket.sendTXT(message);

//         voltage = (float)analogRead(LIPO_PIN) / 4095 * 2 * 3.9;
//         dtostrf(voltage, 1, 3, voltageBuffer);
//         strcpy(message, "UPDATE/battery:Float/");
//         strcat(message, voltageBuffer);
//         webSocket.sendTXT(message);

//         break;
//     case WStype_TEXT:
//         Serial.printf("[WS] Response: %s\n", payload);
//         action_id = getValueFromPayload((char *)payload, '/', 0);
//         action_state = getValueFromPayload((char *)payload, '/', 1);

//         if (action_id == "led_green")
//         {
//             if (action_state == "true")
//             {
//                 digitalWrite(LED_GREEN_PIN, HIGH);
//                 gpio_hold_en(LED_GREEN_PIN);
//             }
//             else
//             {
//                 digitalWrite(LED_GREEN_PIN, LOW);
//                 gpio_hold_dis(LED_GREEN_PIN);
//             }
//         }

//         Serial.println("Deep sleep: START");
//         goToDeepSleep();
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
//     Serial.print("Connecting to WiFi... ");
//     WiFi.mode(WIFI_STA);
//     WiFi.begin(WIFI_SSID, WIFI_PASS);

//     unsigned long startAttemptTime = millis();

//     while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT)
//     {
//         delay(10);
//     }

//     if (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.println("FAILED");
//         goToDeepSleep();
//     }

//     Serial.println("OK");
// }

// void initWebSocket()
// {
//     webSocket.begin(WS_HOST, WS_PORT, "/?deviceId=plant_1");
//     webSocket.onEvent(webSocketEvent);
//     // webSocket.setAuthorization("user", "Password");
//     // webSocket.setReconnectInterval(5000);

//     while (true)
//     {
//         webSocket.loop();
//     }
// }

// void setup()
// {
//     setCpuFrequencyMhz(80);

//     print_wakeup_reason();
//     adc_power_on();

//     pinMode(LED_GREEN_PIN, OUTPUT);
//     pinMode(LIPO_PIN, OUTPUT);

//     Serial.begin(115200);
//     Serial.setDebugOutput(true);

//     connectToWiFi();
//     initWebSocket();
// }

// void loop()
// {
// }