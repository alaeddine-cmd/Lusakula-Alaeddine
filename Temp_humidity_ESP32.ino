#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_MQTT_FONA.h>


#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11
#define LED1               5
#define MOT               4
                                                                                                                                                                                                                                                                                                                                                                                           

DHT dht(DHTPIN, DHTTYPE);

char str_hum[16];
char str_temp[16];


#define WLAN_SSID       "SSID"
#define WLAN_PASS       "PASS"


// Adafruit IO

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "USERNAME"
#define AIO_KEY         "KEY_xx" 

WiFiClient client;


Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);


Adafruit_MQTT_Subscribe Light1= Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/LED"); 
Adafruit_MQTT_Subscribe Light2= Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/MOTOR");

Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature");
Adafruit_MQTT_Publish hum = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity");





void MQTT_connect();

void setup() {

 Serial.begin(115200);

  delay(10);
  dht.begin();
  pinMode(LED1, OUTPUT);
  pinMode(MOT, OUTPUT);

  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());

  mqtt.subscribe(&Light1);
  mqtt.subscribe(&Light2);
}

void loop() {
 
  MQTT_connect();

      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      float f = dht.readTemperature(true);


      if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
      }
        

      float hif = dht.computeHeatIndex(f, h);
      float hic = dht.computeHeatIndex(t, h, false);
      
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.print(" *C ");
      Serial.print(f);
      Serial.print(" *F\t");
      Serial.print("Heat index: ");
      Serial.print(hic);
      Serial.print(" *C ");
      Serial.print(hif);
      Serial.println(" *F");


      Serial.print("Temperature in Celsius:");
      Serial.println(String(t).c_str());

      Serial.print("Temperature in Fahrenheit:");
      Serial.println(String(f).c_str());

      Serial.print("Humidity:");
      Serial.println(String(h).c_str());




  Serial.print(F("\nSending Humidity value: "));
  Serial.print(String(h).c_str());
  Serial.print("...");
  if (! hum.publish(String(h).c_str())) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
delay(1000);
  Serial.print(F("\nSending Temperature value: "));
  Serial.print(String(t).c_str());
  Serial.print("...");
  if (! temp.publish(String(t).c_str())) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }


  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    // Check if its the onoff button feed
    if (subscription == &Light1) {
      Serial.print(F("On-Off button: "));
      Serial.println((char *)Light1.lastread);
      
      if (strcmp((char *)Light1.lastread, "ON") == 0) {
        digitalWrite(LED1, LOW); 
        digitalWrite(MOT, LOW);
      }
     else if (strcmp((char *)Light1.lastread, "OFF") == 0) {
        digitalWrite(LED1, HIGH); 
        digitalWrite(MOT, HIGH); 
      }
    }

}
  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

}
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");

}
