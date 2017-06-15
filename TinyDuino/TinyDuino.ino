
/**  LED  ***  LED  ***  LED  **/

/**  GPS  ***  GPS  ***  GPS  **/

/**  SD   ***  SD   ***  SD   **/

/** WIFI  *** WIFI  *** WIFI  **/

#include <SPI.h>
#include <WiFi101.h>
#include <MQTTClient.h>

/********************************
*** SETUP *** SETUP *** SETUP ***
********************************/

void setup() {

	Serial.begin(115200);
	wifiSetup();
}

/********************************
*** LOOP  *** LOOP  *** LOOP  ***
********************************/


void loop() {

	wifiConnect() && mqttConnect() && mqttPublish("Hi there.");
	delay(10000);
}

/********************************
***  LED  ***  LED  ***  LED  ***
********************************/

/********************************
***  GPS  ***  GPS  ***  GPS  ***
********************************/

/********************************
***  SD   ***  SD   ***  SD   ***
********************************/

/********************************
*** WIFI  *** WIFI  *** WIFI  ***
********************************/

const char * const ssid = "visitor-76437";
const char * const pass = "12ab34cd";
int status = WL_IDLE_STATUS;

WiFiClient net;
MQTTClient client;

void wifiSetup() { WiFi.setPins(8, 2, A3, -1); }

bool wifiConnect() {

	if ((status = WiFi.status()) == WL_NO_SHIELD) {
		Serial.println("WiFi shield not present!");
		while (true);
	}

	if (status == WL_CONNECTED) return true;

	Serial.print("Attempting to connect to WPA SSID: ");
	Serial.println(ssid);
	status = WiFi.begin(ssid, pass);

	if (status != WL_CONNECTED) return false;

	Serial.println("You're connected to the network.");
	return true;
}

bool mqttConnect() {

	if (client.connected()) return true;

	if (!client.begin("siot.net", net))	return false;
	if (client.connect("TinyDuino")) return false;

	Serial.println("You're connected to the MQTT server.");
	return true;
}

bool mqttPublish(const char * const message) {

	if (!client.publish("siot/DAT/F386-09CA-1F9F-9A1F-BAD3-F573-64A0-2A22/14760cc8-4cea-c63c-a825-cef162c16146", message)) return false;

	Serial.println("You've successfully published a MQTT message.");
	return true;
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {}
