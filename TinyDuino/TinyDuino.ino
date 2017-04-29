
/*/  LED  ///  LED  ///  LED  ///

void ledSetup();
void ledOn();
void ledOff();
void ledShowString(const char* s);
void ledShowChar(const char c);*/

///  GPS  ///  GPS  ///  GPS  ///

typedef struct {
  unsigned short nofSatellites;
  float latitude, longitude;
#ifndef _GPS_MIN_INFO
  float altitude, course, speedKmph, fromHome;
#endif
  int year;
  byte month, day, hour, minute, second;
} GpsData;

void gpsSetup();
void gpsWakeUp();
void gpsShutDown();
void gpsDelay(const unsigned long ms);
bool gpsTryReadData(const GpsData* data);
void gpsPrintData(const GpsData* data);

///  SD   ///  SD   ///  SD   ///

bool sdSetup();
bool sdWriteData(const char* s);

/// WIFI  /// WIFI  /// WIFI  ///

/////////////////////////////////
/// SETUP /// SETUP /// SETUP ///
/////////////////////////////////

bool error = false;

void setup() {

  Serial.begin(115200);
  //ledSetup();
  gpsSetup();
  if ((error = !sdSetup())) {
    Serial.println("Could not setup SD card!");
    return;
  }

  GpsData data;
  gpsWakeUp();
  error = !gpsTryReadData(&data);
  gpsShutDown();
  if (error) {
    Serial.println("Could not read valid GPS data!");
    return;
  }

  gpsPrintData(&data);
  Serial.println("GPS printed.");
  if ((error = !sdWriteData(&data))) {
    Serial.println("Could not write GPS data!");
    return;
  }
  Serial.println("GPS written.");
}

/////////////////////////////////
/// LOOP  /// LOOP  /// LOOP  ///
/////////////////////////////////

void loop() {

  if (error)
    return;
}

/*///////////////////////////////
///  LED  ///  LED  ///  LED  ///
/////////////////////////////////

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
}*/

/////////////////////////////////
///  GPS  ///  GPS  ///  GPS  ///
/////////////////////////////////

#define _GPS_NO_STATS
#define _GPS_MIN_INFO

#define GPS_TX_PIN (A0)
#define GPS_RX_PIN (A1)
#define GPS_SYS_ON_PIN (A2)
#define GPS_ON_OFF_PIN (A3)

#define GPS_CALIBRATION_TIME (15000)

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

  Serial.print("Waking up GPS module..");
  while (digitalRead(GPS_SYS_ON_PIN) == LOW) {
    Serial.print(".");
    digitalWrite(GPS_ON_OFF_PIN, HIGH);
    delay(5);
    digitalWrite(GPS_ON_OFF_PIN, LOW);
    delay(100);
  }
  Serial.println(" done.");

  Serial.print("Calibrating GPS module...");
  gpsDelay(GPS_CALIBRATION_TIME);
  Serial.println(" done.");

#ifndef _GPS_NO_STATS
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
}

void gpsShutDown() {

  Serial.print("Shutting down GPS module..");
  while (digitalRead(GPS_SYS_ON_PIN) == HIGH) {
    Serial.print(".");
    digitalWrite(GPS_ON_OFF_PIN, LOW);
    delay(5);
    digitalWrite(GPS_ON_OFF_PIN, HIGH);
    delay(100);
  }
  Serial.println(" done.");
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

  data->nofSatellites = gps.satellites();
  gps.f_get_position(&data->latitude, &data->longitude, &age1);
#ifndef _GPS_MIN_INFO
  data->altitude = gps.f_altitude();
  data->course = gps.f_course();
  data->speedKmph = gps.f_speed_kmph();
  data->fromHome = gps.distance_between(data->latitude, data->longitude, HOME_LATITUDE, HOME_LONGITUDE);
#endif
  gps.crack_datetime(&data->year, &data->month, &data->day, &data->hour, &data->minute, &data->second, &hundreth, &age2);

  return data->nofSatellites != TinyGPS::GPS_INVALID_SATELLITES
    && data->latitude != TinyGPS::GPS_INVALID_F_ANGLE
    && data->longitude != TinyGPS::GPS_INVALID_F_ANGLE
#ifndef _GPS_MIN_INFO
    && data->altitude != TinyGPS::GPS_INVALID_F_ALTITUDE
    && data->course != TinyGPS::GPS_INVALID_F_ANGLE
    && data->speedKmph != TinyGPS::GPS_INVALID_F_SPEED
#endif
    && age1 != TinyGPS::GPS_INVALID_AGE
    && age2 != TinyGPS::GPS_INVALID_AGE;
}

void gpsPrintData(const GpsData* const data) {

  Serial.print("GPS result: # satellites: ");
  Serial.print(data->nofSatellites);
  Serial.print(" latitude: ");
  Serial.print(data->latitude);
  Serial.print(" longitude: ");
  Serial.print(data->longitude);
#ifdef _GPS_MIN_INFO
  Serial.println();
#else
  Serial.print(" altitude: ");
  Serial.println(data->altitude);
  Serial.print("course: ");
  Serial.print(data->course);
  Serial.print(" speed (km/h): ");
  Serial.print(data->speedKmph);
  Serial.print(" from home (m): ");
  Serial.println(data->fromHome);
#endif
  Serial.print("date: ");
  Serial.print(data->year);
  Serial.print('-');
  Serial.print(data->month);
  Serial.print('-');
  Serial.print(data->day);
  Serial.print('T');
  Serial.print(data->hour);
  Serial.print(':');
  Serial.print(data->minute);
  Serial.print(':');
  Serial.print(data->second);
  Serial.println('Z');
}

/////////////////////////////////
///  SD   ///  SD   ///  SD   ///
/////////////////////////////////

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

  if (!init) {
    file.print("# Satellites,Latitude,Longitude");
#ifndef _GPS_MIN_INFO
    file.print(",Altitude,Course,Speed (km/h),From Home (m)");
#endif
    file.println(",Date/Time");
  }

  char dateTime[20];
  sprintf(dateTime, "%02d-%02d-%02dT%02d:%02d:%02dZ", data->year, data->month, data->day, data->hour, data->minute, data->second);

  file.print(data->nofSatellites);
  file.print(',');
  file.print(data->latitude);
  file.print(',');
  file.print(data->longitude);
  file.print(',');
#ifndef _GPS_MIN_INFO
  file.print(data->altitude);
  file.print(',');
  file.print(data->course);
  file.print(',');
  file.print(data->speedKmph);
  file.print(',');
  file.print(data->fromHome);
  file.print(',');
#endif
  file.println(dateTime);
  file.close();
  return true;
}

/////////////////////////////////
/// WIFI  /// WIFI  /// WIFI  ///
/////////////////////////////////

