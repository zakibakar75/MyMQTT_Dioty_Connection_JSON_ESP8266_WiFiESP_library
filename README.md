# MyMQTT_Dioty_Connection_JSON_ESP8266_WiFiESP_library
MQTT connection using ESP8266 Cytron WiFi Shield to Dioty MQTT broker 

This is not a library. This is my code to publish my Dust Sensor reading to Dioty MQTT broker.

The hardware used here are :
1) Arduino UNO clone.
2) Cytron Technologies WiFi ESP8266 shield.
3) Dust Sensor : ShinYei PPD42

Library used :
1) PubSubClient : https://github.com/knolleary/pubsubclient
2) WiFiESP : https://github.com/bportaluri/WiFiEsp

Don't use Cytron ESP8266 library (https://github.com/CytronTechnologies/CytronWiFiShield) because it won't work on MQTT connection.
Took me some time to try different libaries to make MQTT to work.

Initially, i use HiveMQ MQTT public broker, but since it always up and down, i decided to change to another MQTT public broker, ie Dioty.
You just need to register an account there (http://www.dioty.co) and follow my example for topic creation.
