/*
# ################################################################################################ #
# File: main.cpp                                                                                   #
# Project: Garage Lights Remote                                                                    #
# Created Date: Sunday, August 27th 2023, 4:42:22 pm                                               #
# Author: Jeffery Seiffert                                                                         #
# -----                                                                                            #
# Last Modified: Tue Apr 29 2025                                                                   #
# Modified By: Jeffery Seiffert                                                                    #
# -----                                                                                            #
# Copyright (c) 2025 Jeffery A. Seiffert                                                           #
#                                                                                                  #
# GNU General Public License v3.0                                                                  #
# -----                                                                                            #
# HISTORY:                                                                                         #
# Date      	By	Comments                                                                       #
# ----------	---	----------------------------------------------------------                     #
#                                                                                                  #
# ################################################################################################ #
*/

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define PRINT_DEBUG 1

#if PRINT_DEBUG == 1
#define debugp(x) Serial.print(x)
#define debugpln(x, ...) Serial.println(x)
#define debugpf(x, ...) Serial.printf(x)
#else
#define debugp(x)
#define debugpln(x, ...)
#define debugpf(x, ...)
#endif

#define BUTTON_ON 2
#define BUTTON_OFF 4
#define LED_RED 20
#define LED_GREEN 6
#define LED_BLUE 7

/* Your WiFi Credentials */
const char *ssid = "ATT";       // SSID
const char *password = "pwd"; // Password

unsigned long button_time = 0;
unsigned long last_button_time = 0;

void ledClear();
String httpGETRequest(const char *serverName);
void setupWiFi();
void WiFiEvent(WiFiEvent_t event);


void setup()
{
    Serial.begin(115200);

    setupWiFi();

    // initialize the pushbutton pin as an input:
    pinMode(BUTTON_ON, INPUT_PULLUP);
    pinMode(BUTTON_OFF, INPUT_PULLUP);

    // initialize the LED Send pin as an output
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);

    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, HIGH);

} // void setup()

void loop()
{

    // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
    if (digitalRead(BUTTON_ON) == LOW)
    {
        button_time = millis();
        String response = "";
        if (button_time - last_button_time > 250)
        {
            debugpln("Lights On.");
            const char *serverName = "http://192.168.15.95/cm?cmnd=Power%20On";
            response = httpGETRequest(serverName);
            debugpln(response);
            ledClear();
            if(response.substring(10,12) == "ON") {
                digitalWrite(LED_GREEN, LOW);
                delay(2000);
                digitalWrite(LED_GREEN, HIGH);
            }
            last_button_time = button_time;
        }
    }
    if (digitalRead(BUTTON_OFF) == LOW)
    {
        button_time = millis();
        String response = "";
        if (button_time - last_button_time > 250)
        {
            debugpln("Lights Off.");
            const char *serverName = "http://192.168.15.95/cm?cmnd=Power%20Off";
            response = httpGETRequest(serverName);
            debugpln(response);
            debugpln(response.substring(10,13) == "OFF");
            ledClear();
            if(response.substring(10,13) == "OFF") {
                digitalWrite(LED_RED, LOW);
                delay(2000);
                digitalWrite(LED_RED, HIGH);
            }
            last_button_time = button_time;
        }
    }
    if(WiFi.isConnected()) {
        ledClear();
        digitalWrite(LED_BLUE, LOW);
    }
} // void loop()

void ledClear() {
    // Turn all colors off
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, HIGH);
}

String httpGETRequest(const char *serverName)
{
    WiFiClient client;
    HTTPClient http;
    String payload = "{}";

    debugpln(serverName);

    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
        digitalWrite(LED_BLUE, LOW);
        // Your Domain name with URL path or IP address with path
        http.begin(client, serverName);

        // Send HTTP POST request
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            payload = http.getString();
        }
        else
        {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }
        // Free resources
        http.end();
    } // if (WiFi.status() == WL_CONNECTED)

    return payload;
}

void setupWiFi()
{
    // Connect WiFi
    debugpln("---------- Start WiFi Connect ----------");

    // Auto reconnect is set true as default
    // To set auto connect off, use the following function
    WiFi.setAutoReconnect(true);

    WiFi.onEvent(WiFiEvent);

    debugpln(F("Connect to WiFi"));

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    debugpln("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        debugp(".");
    }
    debugpln("");
    debugp("Connected to WiFi network with IP Address: ");
    debugpln(WiFi.localIP());
    debugpln("---------- End WiFi Connect ----------");

} // void setupWiFi()

void WiFiEvent(WiFiEvent_t event)
{
    //debugpf("[WiFi-event] event: %d\n", event);

    switch (event)
    {
    case ARDUINO_EVENT_WIFI_READY:
        debugpln("WiFi interface ready");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        debugpln("Completed scan for access points");
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        debugpln("WiFi client started");
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        debugpln("WiFi clients stopped");
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        debugpln("Connected to access point");
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        debugpln("Disconnected from WiFi access point");
        digitalWrite(LED_BLUE, HIGH);
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        debugpln("Authentication mode of access point has changed");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        debugp("Obtained IP address: ");
        debugpln(WiFi.localIP());
        debugp("WiFi Signal Strength: ");
        debugpln(WiFi.RSSI());
        digitalWrite(LED_BLUE, LOW);
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        debugpln("Lost IP address and IP address is reset to 0");
        break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
        debugpln("WiFi Protected Setup (WPS): succeeded in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
        debugpln("WiFi Protected Setup (WPS): failed in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
        debugpln("WiFi Protected Setup (WPS): timeout in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_PIN:
        debugpln("WiFi Protected Setup (WPS): pin code in enrollee mode");
        break;
    case ARDUINO_EVENT_WIFI_AP_START:
        debugpln("WiFi access point started");
        break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
        debugpln("WiFi access point  stopped");
        break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        debugpln("Client connected");
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        debugpln("Client disconnected");
        break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        debugpln("Assigned IP address to client");
        break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        debugpln("Received probe request");
        break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        debugpln("AP IPv6 is preferred");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        debugpln("STA IPv6 is preferred");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP6:
        debugpln("Ethernet IPv6 is preferred");
        break;
    case ARDUINO_EVENT_ETH_START:
        debugpln("Ethernet started");
        break;
    case ARDUINO_EVENT_ETH_STOP:
        debugpln("Ethernet stopped");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        debugpln("Ethernet connected");
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        debugpln("Ethernet disconnected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        debugpln("Obtained IP address");
        break;
    default:
        break;
    }
}
