
///  LED  ///  LED  ///  LED  ///

void ledSetup();
void ledOn();
void ledOff();
void ledShowString(char* s);
void ledShowChar(char c);

///  GPS  ///  GPS  ///  GPS  ///

///  SD   ///  SD   ///  SD   ///

/// WIFI  /// WIFI  /// WIFI  ///

/////////////////////////////////
/// SETUP /// SETUP /// SETUP ///
/////////////////////////////////

void setup() {

  Serial.begin(115200);
  ledSetup();

  ledShowString("running");
}

/////////////////////////////////
/// LOOP  /// LOOP  /// LOOP  ///
/////////////////////////////////

void loop() {

}

/////////////////////////////////
///  LED  ///  LED  ///  LED  ///
/////////////////////////////////

const int32_t morseSpeed = 100;
const int32_t charMin = 'a';
const int32_t charMax = 'z';
const int32_t morse[26][4] = {
  /*A*/ { 1, 3 },       /*B*/ { 3, 1, 1, 1 }, /*C*/ { 3, 1, 3, 1 }, /*D*/ { 3, 1, 1 },    /*E*/ { 1 },
  /*F*/ { 1, 1, 3, 1 }, /*G*/ { 3, 3, 1 },    /*H*/ { 1, 1, 1, 1 }, /*I*/ { 1, 1 },       /*J*/ { 1, 3, 3, 3 },
  /*K*/ { 3, 1, 3 },    /*L*/ { 1, 3, 1, 1 }, /*M*/ { 3, 3 },       /*N*/ { 3, 1 },       /*O*/ { 3, 3, 3 },
  /*P*/ { 1, 3, 3, 1 }, /*Q*/ { 3, 3, 1, 3 }, /*R*/ { 1, 3, 1 },    /*S*/ { 1, 1, 1 },    /*T*/ { 3 },
  /*U*/ { 1, 1, 3 },    /*V*/ { 1, 1, 1, 3 }, /*W*/ { 1, 3, 3 },    /*X*/ { 3, 1, 1, 3 }, /*Y*/ { 3, 1, 3, 3 },
  /*Z*/ { 3, 3, 1, 1 }
};

void ledSetup() { pinMode(LED_BUILTIN, OUTPUT); }

void ledOn() {  digitalWrite(LED_BUILTIN, HIGH); }
void ledOff() { digitalWrite(LED_BUILTIN, LOW); }

void ledShowString(const char *s) {

  for (auto i = 0; s[i] != 0; i++) {
    if (s[i] == ' ')
      delay(morseSpeed * (7 - 3)); //exclude the next three units
    else {
      if (i > 0) delay(morseSpeed * 3);
      ledShowChar(s[i]);
    }
  }
}

void ledShowChar(const char c) {

  if (charMin > c || c > charMax)
    return;

  auto m = morse[c - charMin];
  for (auto i = 0; i < 4 && m[i] != 0; i++) {
    if (i > 0) delay(morseSpeed);
    ledOn();
    delay(morseSpeed * m[i]);
    ledOff();
  }
}

/////////////////////////////////
///  GPS  ///  GPS  ///  GPS  ///
/////////////////////////////////

/////////////////////////////////
///  SD   ///  SD   ///  SD   ///
/////////////////////////////////

/////////////////////////////////
/// WIFI  /// WIFI  /// WIFI  ///
/////////////////////////////////

