
#define DEBUG_DISABLE

/**  LED  ***  LED  ***  LED  **/

#define LED_DISABLED

#ifndef LED_DISABLED

void ledSetup();
void ledOn();
void ledOff();
void ledShowString(const char* s);
void ledShowChar(const char c);
#endif

/**  GPS  ***  GPS  ***  GPS  **/

//#define GPS_FLOAT
#define GPS_NO_STATS
#define GPS_MIN_INFO

#ifdef GPS_FLOAT
typedef float GPS_LAT_LONG_ALT_TYPE, GPS_COURSE_SPEED_TYPE;
#else
typedef long GPS_LAT_LONG_ALT_TYPE;
typedef unsigned long GPS_COURSE_SPEED_TYPE;
#endif

typedef struct {
	int year;
	byte month, day, hour, minute, second;
	unsigned short nofSatellites;
	GPS_LAT_LONG_ALT_TYPE latitude, longitude;
#ifndef GPS_MIN_INFO
	GPS_LAT_LONG_ALT_TYPE altitude, fromHomeM;
	GPS_COURSE_SPEED_TYPE course, speed;
#endif
} GpsData;

void gpsSetup();
void gpsWakeUp();
void gpsShutDown();
void gpsDelay(const unsigned long ms);
bool gpsTryReadData(GpsData* const data);
#ifndef DEBUG_DISABLE
void gpsPrintData(const GpsData* const data);
#endif

/**  SD   ***  SD   ***  SD   **/

bool sdSetup();
bool sdWriteData(const GpsData* const data);

/** WIFI  *** WIFI  *** WIFI  **/

/********************************
*** SETUP *** SETUP *** SETUP ***
********************************/

bool error = false;

void setup() {

#ifndef DEBUG_DISABLE
	Serial.begin(115200);
#endif
#ifndef LED_DISABLED
	ledSetup();
#endif
	gpsSetup();
	if ((error = !sdSetup())) {
#ifndef DEBUG_DISABLE
		Serial.println("Could not setup SD card!");
#endif
		return;
	}
}

/********************************
*** LOOP  *** LOOP  *** LOOP  ***
********************************/

void loop() {

	if (error)
		return;

	GpsData data;
	gpsWakeUp();
	//gpsShutDown();
	if (!gpsTryReadData(&data)) {
#ifndef DEBUG_DISABLE
		Serial.println("Could not read valid GPS data!");
#endif
		return;
	}

#ifndef DEBUG_DISABLE
	gpsPrintData(&data);
	Serial.println("GPS printed.");
#endif
	if ((error = !sdWriteData(&data))) {
#ifndef DEBUG_DISABLE
		Serial.println("Could not write GPS data!");
#endif
		return;
	}
#ifndef DEBUG_DISABLE
	Serial.println("GPS written.");
#endif
}

/********************************
***  LED  ***  LED  ***  LED  ***
********************************/

#ifndef LED_DISABLED

#define MORSE_SPEED (100)
#define CHAR_FIRST ('a')
#define CHAR_LAST ('z')
const uint8_t morseCodes[] = {
	(uint8_t)0x70, (uint8_t)0xD5, (uint8_t)0xDD, (uint8_t)0xD4, (uint8_t)0x40, (uint8_t)0x5D, (uint8_t)0xF4, (uint8_t)0x55, (uint8_t)0x50, (uint8_t)0x7F,
	(uint8_t)0xDC, (uint8_t)0x75, (uint8_t)0xF0, (uint8_t)0xD0, (uint8_t)0xFC, (uint8_t)0x7D, (uint8_t)0xF7, (uint8_t)0x74, (uint8_t)0x54, (uint8_t)0xC0,
	(uint8_t)0x5C, (uint8_t)0x57, (uint8_t)0x7C, (uint8_t)0xD7, (uint8_t)0xDF, (uint8_t)0xF5
};

void ledSetup() { pinMode(LED_BUILTIN, OUTPUT); }

void ledOn() { digitalWrite(LED_BUILTIN, HIGH); }
void ledOff() { digitalWrite(LED_BUILTIN, LOW); }

void ledShowString(const char* s) {

	for (auto i = 0; s[i] != 0; i++) {
		if (s[i] == ' ')
			delay(MORSE_SPEED * (7 - 3)); //exclude the next three units
		else {
			if (i > 0) delay(MORSE_SPEED * 3);
			ledShowChar(s[i]);
		}
	}
}

void ledShowChar(const char c) {

	if (CHAR_FIRST > c || c > CHAR_LAST)
		return;

	const auto m = morseCodes[c - CHAR_FIRST];
	for (uint8_t i = 0, j = 0b11000000, k = 6; i < 4; i++, j >>= 2, k -= 2) {
		auto n = (m & j) >> k;
		if (n == 0) break;
		if (i > 0) delay(MORSE_SPEED);
		ledOn();
		delay(MORSE_SPEED * n);
		ledOff();
	}
}

#endif

/********************************
***  GPS  ***  GPS  ***  GPS  ***
********************************/

#define GPS_TX_PIN (A0)
#define GPS_RX_PIN (A1)
#define GPS_SYS_ON_PIN (A2)
#define GPS_ON_OFF_PIN (A3)

#define GPS_CALIBRATION_TIME (15000)

#ifdef GPS_FLOAT
#define GPS_LAT_LONG_PRECISION (6)
#define GPS_LAT_LONG_FUNCTION f_get_position
#define GPS_ALT_FUNCTION f_altitude
#define GPS_COURSE_FUNCTION f_course
#define GPS_SPEED_FUNCTION f_speed_kmph
#else
#define GPS_LAT_LONG_PRECISION (10)
#define GPS_LAT_LONG_FUNCTION get_position
#define GPS_ALT_FUNCTION altitude
#define GPS_COURSE_FUNCTION course
#define GPS_SPEED_FUNCTION speed
#endif

#define HOME_LATITUDE (47.16628)
#define HOME_LONGITUDE (7.63498)

#include <SoftwareSerial.h>
#include "TinyGPS.h"

SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
TinyGPS gps;

void gpsSetup() {

	gpsSerial.begin(9600);

	digitalWrite(GPS_ON_OFF_PIN, LOW);
	pinMode(GPS_ON_OFF_PIN, OUTPUT);
	pinMode(GPS_SYS_ON_PIN, INPUT);
	delay(100);
}

void gpsWakeUp() {

#ifndef DEBUG_DISABLE
	Serial.print("Waking up GPS module..");
#endif
	while (digitalRead(GPS_SYS_ON_PIN) == LOW) {
#ifndef DEBUG_DISABLE
		Serial.print(".");
#endif
		digitalWrite(GPS_ON_OFF_PIN, HIGH);
		delay(5);
		digitalWrite(GPS_ON_OFF_PIN, LOW);
		delay(100);
	}
#ifndef DEBUG_DISABLE
	Serial.println(" done.");

	Serial.print("Calibrating GPS module...");
#endif
	gpsDelay(GPS_CALIBRATION_TIME);
#ifndef DEBUG_DISABLE
	Serial.println(" done.");

#ifndef GPS_NO_STATS
	unsigned long encodedChars;
	unsigned short goodSentences, failedChecksums;
	gps.stats(&encodedChars, &goodSentences, &failedChecksums);

	Serial.print("GPS statistics: chars: ");
	Serial.print(encodedChars);
	Serial.print(", good sentences: ");
	Serial.print(goodSentences);
	Serial.print(", failed checksums: ");
	Serial.print(failedChecksums);
	Serial.println('.');
#endif
#endif
}

void gpsShutDown() {

#ifndef DEBUG_DISABLE
	Serial.print("Shutting down GPS module..");
#endif
	while (digitalRead(GPS_SYS_ON_PIN) == HIGH) {
#ifndef DEBUG_DISABLE
		Serial.print(".");
#endif
		digitalWrite(GPS_ON_OFF_PIN, LOW);
		delay(5);
		digitalWrite(GPS_ON_OFF_PIN, HIGH);
		delay(100);
	}
#ifndef DEBUG_DISABLE
	Serial.println(" done.");
#endif
}

void gpsDelay(const unsigned long ms) {

	const auto start = millis();
	do {
		while (gpsSerial.available())
			gps.encode(gpsSerial.read());
	} while (millis() - start < ms);
}

bool gpsTryReadData(GpsData* const data) {

	byte hundreth;
	unsigned long age1, age2;

	gps.crack_datetime(&data->year, &data->month, &data->day, &data->hour, &data->minute, &data->second, &hundreth, &age1);
	data->nofSatellites = gps.satellites();
	gps.GPS_LAT_LONG_FUNCTION(&data->latitude, &data->longitude, &age2);

#ifndef GPS_MIN_INFO
	data->altitude = gps.GPS_ALT_FUNCTION();
	data->course = gps.GPS_COURSE_FUNCTION();
	data->speed = gps.GPS_SPEED_FUNCTION();

#ifdef GPS_FLOAT
	data->fromHomeM = gps.distance_between(data->latitude, data->longitude, HOME_LATITUDE, HOME_LONGITUDE);
#endif
#endif

	return age1 != TinyGPS::GPS_INVALID_AGE
		&& data->nofSatellites != TinyGPS::GPS_INVALID_SATELLITES
		&& data->latitude != TinyGPS::GPS_INVALID_F_ANGLE
		&& data->longitude != TinyGPS::GPS_INVALID_F_ANGLE
		&& age2 != TinyGPS::GPS_INVALID_AGE
#ifndef GPS_MIN_INFO
		&& data->altitude != TinyGPS::GPS_INVALID_F_ALTITUDE
		&& data->course != TinyGPS::GPS_INVALID_F_ANGLE
		&& data->speed != TinyGPS::GPS_INVALID_F_SPEED
#endif
		;
}

#ifndef DEBUG_DISABLE
void gpsPrintData(const GpsData* const data) {

	char dateTime[20];
	sprintf(dateTime, "%02d-%02d-%02dT%02d:%02d:%02dZ", data->year, data->month, data->day, data->hour, data->minute, data->second);

	Serial.print("Date/time: ");
	Serial.print(dateTime);
	Serial.print(" # satellites: ");
	Serial.print(data->nofSatellites);
	Serial.print(" latitude: ");
	Serial.print(data->latitude, GPS_LAT_LONG_PRECISION);
	Serial.print(" longitude: ");
	Serial.print(data->longitude, GPS_LAT_LONG_PRECISION);
#ifndef GPS_MIN_INFO
	Serial.print(" altitude: ");
	Serial.print(data->altitude);
	Serial.print(" course: ");
	Serial.print(data->course);
#ifndef GPS_FLOAT
	Serial.print(" speed (kt/100): ");
	Serial.print(data->speed);
#else
	Serial.print(" speed (km/h): ");
	Serial.print(data->speed);
	Serial.print(" from home (m): ");
	Serial.print(data->fromHomeM);
#endif
#endif
	Serial.println();
}
#endif

/********************************
***  SD   ***  SD   ***  SD   ***
********************************/

#define SD_PIN (10)
#define SD_FILE ("gps.csv")

#include <SD.h>

bool sdSetup() {

	pinMode(SD_PIN, OUTPUT);
	return SD.begin(SD_PIN);
}

bool sdWriteData(const GpsData* const data) {

	const auto init = SD.exists(SD_FILE);
	auto file = SD.open(SD_FILE, FILE_WRITE);
	if (file);
	else
		return false;

	if (!init)
		file.println("Date/Time,# Satellites,Latitude,Longitude,Altitude,Course,Speed (kt/100),Speed (km/h),From Home (m)");

	char dateTime[20];
	sprintf(dateTime, "%02d-%02d-%02dT%02d:%02d:%02dZ", data->year, data->month, data->day, data->hour, data->minute, data->second);

	file.print(dateTime);
	file.print(',');
	file.print(data->nofSatellites);
	file.print(',');
	file.print(data->latitude, GPS_LAT_LONG_PRECISION);
	file.print(',');
	file.print(data->longitude, GPS_LAT_LONG_PRECISION);
	file.print(',');
#ifndef GPS_MIN_INFO
	file.print(data->altitude);
	file.print(',');
	file.print(data->course);
#ifdef GPS_FLOAT
	file.print(',');
#endif
	file.print(',');
	file.print(data->speed);
#ifndef GPS_FLOAT
	file.print(',');
#endif
#ifdef GPS_FLOAT
	file.print(',');
	file.print(data->fromHomeM);
#endif
#endif
	file.println();
	file.close();
	return true;
}

/********************************
*** WIFI  *** WIFI  *** WIFI  ***
********************************/
