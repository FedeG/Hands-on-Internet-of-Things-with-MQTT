/**
 * Example to send and receive MQTT messages via the MQTT server shiftr.io
 * to an Arduino MKR 1010 Wifi.
 * 
 * Depends on libraries:
 *   - ESP8266WiFi
 *   - MQTT (by Joel Gaehwiler), there are multiple with a similar name, make sure to pick the right one
 * 
 * By default it uses the openly accessible login via username: "try" and password: "try".
 * You could easily replace these with your personal login credentials after signing up on shiftr.io.
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#ifndef STASSID
#define STASSID "wifi name"
#define STAPSK  "wifi password"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

ESP8266WiFiMulti WiFiMulti;

#include <MQTT.h>

const char MQTT_SERVER[] = "broker.shiftr.io";
const int MQTT_SERVER_PORT = 1883;
const char MQTT_USERNAME[] = "try";
const char MQTT_PASSWORD[] = "try";
const char MQTT_DEVICE_NAME[] = "hellomqtt"; // can be freely picked

int status = WL_IDLE_STATUS;
WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  // first connect to the wifi
  Serial.print("Checking wifi...");
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(); Serial.print("Connected to WiFi!"); Serial.println();

  // second connect to the MQTT server
  client.begin(MQTT_SERVER, MQTT_SERVER_PORT, net);
  Serial.println("Connecting to MQTT server...");
  while (!client.connect(MQTT_DEVICE_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to MQTT server");

  // define what should happen when messages are incoming
  client.onMessage(messageReceived);
  
  // subscribe to MQTT topics
  client.subscribe("/tims-channel");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void setup() {
  Serial.begin(115200);
  connect();
}

void loop() {
  client.loop();

  if (!net.connected()) {
    connect();
  }

  // send a message every second
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/tims-channel", "hello");
    lastMillis = millis();
  }
}