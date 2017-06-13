/*
MQTT Connection - Sending Dust Sensor Information
*/

#include "WiFiEsp.h"
#include "SoftwareSerial.h"
#include <PubSubClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ArduinoJson.h>

#define BUILTIN_LED 13

SoftwareSerial Serial1(2, 3); // RX, TX

WiFiEspClient client;
PubSubClient mqttClient(client);

const char* server = "mqtt.dioty.co"; //MQTT server (of your choice)
char ssid[] = "myAlixAP";            // your network SSID (name)
char pass[] = "mimosian";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status


/* For JSON packet */
long lastMsg = 0;
char msg[180];
char temp[20],temp2[20],temp3[20],temp4[20];
int value = 0;
/* end for JSON packet */

/* For Dust Sensor */
unsigned long starttime;
unsigned long triggerOnP1;
unsigned long triggerOffP1;
unsigned long pulseLengthP1;
unsigned long durationP1;
boolean valP1 = HIGH;
boolean triggerP1 = false;

unsigned long triggerOnP2;
unsigned long triggerOffP2;
unsigned long pulseLengthP2;
unsigned long durationP2;
boolean valP2 = HIGH;
boolean triggerP2 = false;

float ratioP1 = 0;
float ratioP2 = 0;
unsigned long sampletime_ms = 10000;
float countP1;
float countP2;
/* end for Dust Sensor */

void setup()
{
  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");

  delay(1500);
  mqttClient.setServer(server, 1883);
  mqttClient.setCallback(callback);

  starttime = millis();
  Serial.println("\nPM10 count,PM2.5 count,PM10 conc,PM2.5 conc");

}


void printWifiData()
{
  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print your MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  char buf[20];
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  Serial.print("MAC address: ");
  Serial.println(buf);
}

void printCurrentNet()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to
  byte bssid[6];
  WiFi.BSSID(bssid);
  char buf[20];
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", bssid[5], bssid[4], bssid[3], bssid[2], bssid[1], bssid[0]);
  Serial.print("BSSID: ");
  Serial.println(buf);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.println(rssi);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(NULL,"zaki.bm@gmail.com","aa3ca97e")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("/zaki.bm@gmail.com/1234", "hello world");
      // ... and resubscribe
      //mqttClient.subscribe("/zaki.bm@gmail.com/1234");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {

  float PM10count;
  float PM25count;
  float concLarge;
  float concSmall;

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();  

  if (!mqttClient.connected()) {
    reconnect();
  }

  delay(1000);
  mqttClient.loop();

  /* For Dust Sensor */
  valP1 = digitalRead(8);
  valP2 = digitalRead(9);  
  
  if(valP1 == LOW && triggerP1 == false){
    triggerP1 = true;
    triggerOnP1 = micros();
  }
  
  if (valP1 == HIGH && triggerP1 == true){
      triggerOffP1 = micros();
      pulseLengthP1 = triggerOffP1 - triggerOnP1;
      durationP1 = durationP1 + pulseLengthP1;
      triggerP1 = false;
  }
  
  if(valP2 == LOW && triggerP2 == false){
     triggerP2 = true;
     triggerOnP2 = micros();
  }
  
  if (valP2 == HIGH && triggerP2 == true){
      triggerOffP2 = micros();
      pulseLengthP2 = triggerOffP2 - triggerOnP2;
      durationP2 = durationP2 + pulseLengthP2;
      triggerP2 = false;
  }
  
  if ((millis() - starttime) > sampletime_ms) {
      
      ratioP1 = durationP1/(sampletime_ms*10.0);  // Integer percentage 0=>100
      ratioP2 = durationP2/(sampletime_ms*10.0);
      countP1 = 1.1*pow(ratioP1,3)-3.8*pow(ratioP1,2)+520*ratioP1+0.62;
      countP2 = 1.1*pow(ratioP2,3)-3.8*pow(ratioP2,2)+520*ratioP2+0.62;
      PM10count = countP2;
      PM25count = countP1 - countP2;
      
      // first, PM10 count to mass concentration conversion
      double r10 = 2.6*pow(10,-6);
      double pi = 3.14159;
      double vol10 = (4.0/3.0)*pi*pow(r10,3);
      double density = 1.65*pow(10,12);
      double mass10 = density*vol10;
      double K = 3531.5;
      concLarge = (PM10count)*K*mass10;
      
      // next, PM2.5 count to mass concentration conversion
      double r25 = 0.44*pow(10,-6);
      double vol25 = (4.0/3.0)*pi*pow(r25,3);
      double mass25 = density*vol25;
      concSmall = (PM25count)*K*mass25;

      Serial.print(PM10count);     
      Serial.print(",");
      Serial.print(PM25count);      
      Serial.print(",");
      Serial.print(concLarge);      
      Serial.print(",");
      Serial.print(concSmall);

      Serial.println();

      dtostrf( PM10count, 3, 2, temp );      
      dtostrf( PM25count, 3, 2, temp2 );      
      dtostrf( concLarge, 3, 2, temp3 );      
      dtostrf( concSmall, 3, 2, temp4 );
      
      root["PM10Count"] = temp;
      root["PM25Count"] = temp2;
      root["concLarge"] = temp3;
      root["concSmall"] = temp4;

      delay(1000);

      root.printTo(msg);

      Serial.println();
      Serial.println("Publish message : ");
      Serial.println(msg);
      mqttClient.publish("/zaki.bm@gmail.com/1234", msg);
          
      durationP1 = 0;
      durationP2 = 0;
      starttime = millis();

      /* end for Dust Sensor */
  }

  
}

