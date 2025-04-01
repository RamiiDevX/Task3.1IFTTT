#include <WiFiNINA.h>
#include <BH1750.h>
#include <ArduinoHttpClient.h>
#include <Wire.h>

// WiFi info
char ssid[] = "iphone";  //WiFi username
char pass[] = "Rameesha"; //Wifi pw

WiFiClient client;  //declare WiFi client
BH1750 lightMeter; //Declare sensor

// IFTTT settings
const int port = 80;
const char* sunlightEvent = "sun_light_start"; //event name for detect sun light
const char* noSunlightEvent = "sun_light_stop"; //event name for no sun light
const char* key = "b3Wnv0dB-A-knn1M2EvEoC"; //IFTTT webhook key
char host_name[] = "maker.ifttt.com"; //IFTTT host

// Thresholds
const int sunlightThreshold = 700; //light level to consider detect light
const int darknessThreshold = 500; //light level to consider no light
bool sunlightDetected = false;

void setup() {
    Serial.begin(9600);
    while (!Serial);

    Wire.begin();
    lightMeter.begin(); //initialize light sensor.

    Serial.println("\nSystem initialized");
    Connect_To_WiFi();  //function call connect wifi
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Connect_To_WiFi();
    }

    int lightLevel = lightMeter.readLightLevel();
    Serial.print("Light level is "); //print the light
    Serial.println(lightLevel);

    if (lightLevel > sunlightThreshold && !sunlightDetected) {
        Serial.println("Sunlight detected!");
        sendToIFTTT(sunlightEvent);
        sunlightDetected = true;
    } else if (lightLevel < darknessThreshold && sunlightDetected) {
        Serial.println("No sunlight detected!");
        sendToIFTTT(noSunlightEvent);
        sunlightDetected = false;
    }
    delay(3000);
}

void Connect_To_WiFi() {
    Serial.print("Try to connect to WiFi");
    Serial.println(ssid);

    WiFi.begin(ssid, pass);
    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > 30000) {
            Serial.println("WiFi connection is failed, Please try again!");
            return;
        }
        Serial.print("??");
        delay(5000);
    }

    Serial.println("\nConnected to the WiFi.");
}

void sendToIFTTT(String eventName) {
    Serial.println("Send event to the IFTTT");

    if (client.connect(host_name, port)) {
        Serial.println("Connected to IFTTT server");

        String url = "/trigger/" + eventName + "/with/key/" + key;

        // Send HTTP GET request
        client.print("GET " + url + " HTTP/1.1\r\n");
        client.print("Host: " + String(host_name) + "\r\n");
        client.print("Connection: close\r\n\r\n");

        // Wait for response
        unsigned long timeout = millis();
        bool success = false;

        while (client.connected() && millis() - timeout < 5000) {
            if (client.available()) {
                String line = client.readStringUntil('\n'); 
                if (line.startsWith("{\"status\":\"ok\"}")) { // Check whether response is success or not
                    success = true;
                    break;
                }
            }
        }

        client.stop();

        //print the msg based on response
        if (success) {
            Serial.println("Event sent successfully!");
        } else {
            Serial.println("Fail to verify the IFTTT response");
        }
    } else {
        Serial.println("Connection to IFTTT failed!");
    }
}
