#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <MQTT.h>

#ifndef STASSID
#define WIFI_SSID "wifi name"
#define WIFI_PASS  "wifi password"
#endif

ESP8266WiFiMulti WiFiMulti;

const char mqttServer[] = "broker.shiftr.io";
const int mqttServerPort = 1883;
const char key[] = "try";
const char secret[] = "try";
const char device[] = "hellomqtt"; // broker device identifier

Servo myservo;
unsigned long lastTimeOpenend = 0;
bool isOpen = false;
int OPENING_TIME = 200;

int status = WL_IDLE_STATUS;
WiFiClient net;
MQTTClient client;

int POSITION_OPEN = 90;
int POSITION_CLOSED = 180;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected to WiFi!\n");

  client.begin(mqttServer, mqttServerPort, net);

  Serial.println("connecting to broker...");
  while (!client.connect(device, key, secret)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("Connected to MQTT");

  client.onMessage(messageReceived);

  client.subscribe("/tims-channel/feeder/#");
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if (topic == "/tims-channel/feeder/feed") {
    Serial.println("Command to feed received");
    lastTimeOpenend = millis();
    isOpen = true;
    myservo.write(POSITION_OPEN);
  }
}

void setup() {
  Serial.begin(9600);
  myservo.attach(5,650,2400);
  myservo.write(POSITION_CLOSED);
  connect();
}

void loop() {
  client.loop();
  // delay(1000); // helps eventually

  if (!net.connected()) {
    connect();
  }

  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    client.publish("/tims-channel/feeder/ping", "feeder is online");
    lastMillis = millis();
  }
  while (Serial.available() > 0) {
    int inputValue = Serial.parseInt();
    if (inputValue == 1) {
      lastTimeOpenend = millis();
      isOpen = true;
      myservo.write(POSITION_OPEN);
    }
  }
  if (isOpen) {
    if (millis() > lastTimeOpenend + OPENING_TIME) {
      myservo.write(POSITION_CLOSED);
      isOpen = false;
    }   
  }
}
