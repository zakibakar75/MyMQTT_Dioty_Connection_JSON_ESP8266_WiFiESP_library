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
char temp[20],temp2[20];
int value = 0;
/* end for JSON packet */

/****** for dust sensor *******/
#define CHILD_ID_DUST_PM10            0
#define CHILD_ID_DUST_PM25            1
#define DUST_SENSOR_DIGITAL_PIN_PM10  3

unsigned long SLEEP_TIME = 7*1000; // Sleep time between reads (in milliseconds)
//VARIABLES
int val = 0;           // variable to store the value coming from the sensor
float valDUSTPM25 =0.0;
float lastDUSTPM25 =0.0;
float valDUSTPM10 =0.0;
float lastDUSTPM10 =0.0;
unsigned long duration;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 7000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
long concentrationPM25 = 0;
long concentrationPM10 = 0;
int temp=28; //external temperature, if you can replace this with a DHT11 or better 
long ppmv;
/***********************************************************************/


void setup()
{
  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);
  
  /*****for dust sensor********/
  pinMode(DUST_SENSOR_DIGITAL_PIN_PM10,INPUT);
  /**************************/

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


      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();  

      if (!mqttClient.connected()) {
          reconnect();
      }

      delay(1000);
      mqttClient.loop();
  
     /************** For Dust Sensor ********************/
      //get PM 1.0 - density of particles over 1 μm.
      concentrationPM10=getPM(DUST_SENSOR_DIGITAL_PIN_PM10);
      Serial.print("PM10: ");
      Serial.println(concentrationPM10);
      Serial.print("\n");
      //ppmv=mg/m3 * (0.08205*Tmp)/Molecular_mass
      //0.08205   = Universal gas constant in atm·m3/(kmol·K)
      ppmv=(concentrationPM10*0.0283168/100/1000) *  (0.08205*temp)/0.01;
  
      if ((ceil(concentrationPM10) != lastDUSTPM10)&&((long)concentrationPM10>0)) 
      {
          //gw.send(dustMsgPM10.set((long)ppmv));
          Serial.print("ppmv PM10: ");
          Serial.println(ppmv);
          Serial.print("\n");
          lastDUSTPM10 = ceil(concentrationPM10);
        
          dtostrf( lastDUSTPM10, 3, 2, temp );
          root["PM10Count"] = temp;
        
          if(ppmv > 0)
          {  
             dtostrf( ppmv, 3, 2, temp2 );       
             root["ppmvPM10Count"] = temp2;
          }
          else
          {
             root["ppmvPM10Count"] = 0;
          }
            
      }
      else
      {
          root["PM10Count"] = 0;
          root["ppmvPM10Count"] = 0;
      }

      delay(1000);

      root.printTo(msg);

      Serial.println();
      Serial.println("Publish message : ");
      Serial.println(msg);
      mqttClient.publish("/zaki.bm@gmail.com/1234", msg);
     
      /******* end for Dust Sensor *********/
  }
  
}

float conversion10(long concentrationPM10) {
  double pi = 3.14159;
  double density = 1.65 * pow (10, 12);
  double r10 = 0.44 * pow (10, -6);
  double vol10 = (4/3) * pi * pow (r10, 3);
  double mass10 = density * vol10;
  double K = 3531.5;
  return (concentrationPM10) * K * mass10;
}

long getPM(int DUST_SENSOR_DIGITAL_PIN) {

  starttime = millis();

  while (1) {
  
    duration = pulseIn(DUST_SENSOR_DIGITAL_PIN, LOW);
    lowpulseoccupancy += duration;
    endtime = millis();
    
    if ((endtime-starttime) > sampletime_ms)
    {
    ratio = (lowpulseoccupancy-endtime+starttime)/(sampletime_ms*10.0);  // Integer percentage 0=>100
                long concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print("lowpulseoccupancy:");
    Serial.print(lowpulseoccupancy);
    Serial.print("\n");
    Serial.print("ratio:");
    Serial.print(ratio);
    Serial.print("\n");
    Serial.print("PPDNS42:");
    Serial.println(concentration);
    Serial.print("\n");
    
    lowpulseoccupancy = 0;
    return(concentration);    
    }
  }  
}
