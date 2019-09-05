/*
 * Hands-On Internet of Things with MQTT – Chapter 6 (03 - MQTT)
 * 
 * Displays MQTT messages sent to the channel "/tims-channel/todo-device/text" on
 * an e-paper display.
 * 
 * Based on GxEPD2_Example by Jean-Marc Zingg, which is itself based on Demo Example from
 * Good Display: http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
 * 
 * Libraries (install via library manager):
 *   - GxEPD2
 *   - Adafruit GFX
 * 
 * Waveshare e-paper discussion on Arduino forum: http://forum.arduino.cc/index.php?topic=487007.0
 */ 

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <MQTT.h>

#define MAX_DISPAY_BUFFER_SIZE 15000ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#define MAX_HEIGHT_3C(EPD) (EPD::HEIGHT <= (MAX_DISPAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
GxEPD2_3C<GxEPD2_420c, MAX_HEIGHT_3C(GxEPD2_420c)> display(GxEPD2_420c(/*CS=4*/ 4, /*DC=*/ 7, /*RST=*/ 6, /*BUSY=*/ 5));

#ifndef STASSID
#define WIFI_SSID "wifi name"
#define WIFI_PASS  "wifi password"
#endif

ESP8266WiFiMulti WiFiMulti;

const char MQTT_SERVER[] = "broker.shiftr.io";
const int MQTT_SERVER_PORT = 1883;
const char MQTT_USERNAME[] = "try";
const char MQTT_PASSWORD[] = "try";
const char MQTT_DEVICE_NAME[] = "hellomqtt"; // can be freely picked

int status = WL_IDLE_STATUS;
WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;
String lastText = "";

void setup() {
  Serial.begin(115200);
  Serial.println("starting...");
  display.init(115200);
  setText("Hello display");
  display.powerOff();
  connect();
}

void loop() {
  if(Serial.available() > 0) {
    String text = Serial.readStringUntil('\n');
    Serial.print("Received: "); Serial.println(text);
    setText(text);
  }
  client.loop();

  if (!net.connected()) {
    connect();
  }
}

void setText(String text) {
  if (text.equals(lastText)) {
    return;  
  }
  display.setRotation(0);
  display.setFont(&FreeSansBold18pt7b);
  char lastCharacter = text.charAt(text.length() - 1);
  if (lastCharacter == '!') {
    display.setTextColor(GxEPD_RED);  
  } else {
    display.setTextColor(GxEPD_BLACK);  
  }
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = (display.width() - tbw) / 2;
  uint16_t y = (display.height() + tbh) / 2; // y is base line!
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(text);
  } while (display.nextPage());
  lastText = text;
}

void connect() {
  // first connect to the wifi
  Serial.print("Checking wifi...");
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
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
  client.subscribe("/tims-channel/todo-device/text", 1);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  setText(payload);
}
