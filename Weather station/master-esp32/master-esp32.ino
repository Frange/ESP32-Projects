#if !( defined(ESP32) )
  #error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

#include <WiFi.h>

#include "ESPAsyncWebServer.h" // Local zip - https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip
#include "DHT.h" // https://github.com/adafruit/DHT-sensor-library
//#include <TM1637.h> // https://github.com/AKJ7/TM1637
#include <EasyBuzzer.h> //https://evert-arias.github.io/EasyBuzzer/
#include "pitches.h"

//Delay
#define DELAY_ONE_SECOND        1000
#define DELAY_SECONDS_ALIVE     10 // Seconds
#define DELAY_SECONDS_RAINDROP  10 // Seconds
#define DELAY_SECONDS_SLEEP     90 // Seconds

#define DELAY_ESP32_ONE_SECOND_SLEEP 1000000ULL  /* Conversion factor for micro seconds to seconds */

#define NUM_SLAVES      1
#define LED             2

// DHT22 Sensor
#define PIN_DHT_22          4    
#define TYPE_DHT         DHT22   

// TM1637 - Led display
//#define PIN_CLK             16
//#define PIN_DIO             0

// Battery level
#define PIN_BATTERY_STATUS     19

// Buzzer 
#define PIN_BUZZER      21 

//Raindrop sensor
#define PIN_RAINDROP_ANALOG      35
#define PIN_RAINDROP_DIGITAL     34

//Parameters
String nom =            "Master";

//Buzzer config
/*
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

int melody[] = {
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5,

  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5,

  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5
};

// note durations: 4 = quarter note, 8 = eighth note, etc, also called tempo:
int noteDurations[] = {
  8, 8, 4,
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 8, 8,
  8, 8, 8, 16, 16,
  8, 8, 8, 8,
  4, 4,

  8, 8, 4,
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 8, 8,
  8, 8, 8, 16, 16,
  8, 8, 8, 8,
  4, 4,
  
  8, 8, 4,
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 8, 8,
  8, 8, 8, 16, 16,
  8, 8, 8, 8,
  4, 4
};
*/

unsigned int frequency = 1000;  
unsigned int onDuration = 50;
unsigned int offDuration = 100;
unsigned int beeps = 2;
unsigned int pauseDuration = 500;
unsigned int cycles = 10;

//const char* ssid = "ESP32-Access-Point";
const char* ssid = "ESP32-Master";
const char* password = "testtest1"; //Password must have 9 characters.

//Variables
bool sendCmd = false;
bool isSleeping = false;

String slaveCmd = "0";
String slaveState = "0";

int battAnalogVal = 0;
int rainAnalogVal = 0;
int rainDigitalVal = 0;

float h = 0;
float t = 0;
float f = 0;

float batteryLevel = 0;

//Wifi
AsyncWebServer server(80);

IPAddress ip(192, 168, 1, 60);
IPAddress gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);

DHT dht(PIN_DHT_22, TYPE_DHT);
//TM1637 tm(PIN_CLK, PIN_DIO);

void setup() {
  //Init serial USB
  //Serial.begin(9600);
  Serial.begin(115200);

  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  
  //Clean serial monitor
  Serial.write(12);  
  Serial.println(F("\n\n\n\n\nDHT22 test!\n\n"));

  initWifi();

  //Init buzzer
  EasyBuzzer.setPin(PIN_BUZZER);

  //Init battery level pinout
  pinMode(PIN_BATTERY_STATUS,INPUT);

  //Init raindrop sensor
  pinMode(PIN_RAINDROP_DIGITAL,INPUT);

  //Init temperature sensor (DHT22)
  dht.begin();

  //Init Led display (TM1637)
  //tm.begin();
  //tm.setBrightness(10);
}

void loop() {
  if (isSleeping) {
    wakeUp();
  }
  
  //buzzer();

  battAnalogVal = analogRead(PIN_BATTERY_STATUS);
  batteryLevel = map(battAnalogVal, 0.0f, 4095.0f, 0, 100);

  rainAnalogVal = analogRead(PIN_RAINDROP_ANALOG);
  rainDigitalVal = digitalRead(PIN_RAINDROP_DIGITAL);
  
  h = dht.readHumidity();
  t = dht.readTemperature();
  f = dht.readTemperature(true);

  // Prints for raindrop
  Serial.println(F("\nDatos de lluvia: "));
  Serial.println(F("------------------------------"));
  Serial.print(F("Analogico: "));
  Serial.println(rainAnalogVal);

  if (rainDigitalVal == 0) {
     Serial.println(F("ESTA LLOVIENDO"));
     //tm.display("AGUA");
     delay(DELAY_SECONDS_RAINDROP * DELAY_ONE_SECOND);
  } else {
     Serial.println(F("No llueve"));
     //tm.display(t);
  }

  //Battery level status
  Serial.print(F("\nNivel de bateria: "));
  Serial.print(batteryLevel);
  Serial.print(" - Analog: ");
  Serial.println(battAnalogVal);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  } else {
 
    float hif = dht.computeHeatIndex(f, h);
    float hic = dht.computeHeatIndex(t, h, false);
    
    // Prints for temperature
    Serial.println(F("\nDatos de temperatura: "));
    Serial.println(F("------------------------------"));
    Serial.print(F("Temperatura: "));
    Serial.print(t);
    Serial.print(F(" C"));
    Serial.print(F("\nHumedad: "));
    Serial.print(h);
    Serial.print(F("%"));
    //Serial.print(F("\nIndice de calor: "));
    //Serial.print(hic);

    Serial.println(F("\n"));

    //tm.display(t);
  }

  delay(DELAY_SECONDS_ALIVE * DELAY_ONE_SECOND);

  goToSleep();
}

void buzzer() {
  /*
  EasyBuzzer.beep(
      frequency,		      // Frequency in hertz(HZ). 
      onDuration, 	      // On Duration in milliseconds(ms).
      offDuration, 	      // Off Duration in milliseconds(ms).
      beeps, 			        // The number of beeps per cycle.
      pauseDuration, 	    // Pause duration.
      cycles, 		        // The number of cycle.
      buzzerDone			    // [Optional] Callback. A function to call when the sequence ends.
    );
    
  EasyBuzzer.update();
  */

  /*for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(PIN_BUZZER, melody[thisNote], noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(PIN_BUZZER);
  }
*/
  int size = sizeof(noteDurations) / sizeof(int);

  for (int thisNote = 0; thisNote < size; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(PIN_BUZZER, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(PIN_BUZZER);
  }
}

void buzzerDone() {

}

void initWifi() {

  //Init wifi
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(ip, gateway, subnet);  
  delay(100);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request-> send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request-> send_P(200, "text/plain", String(h).c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request-> send_P(200, "text/plain", String(f).c_str());
  });
  server.on("/battery", HTTP_GET, [](AsyncWebServerRequest *request){
    request-> send_P(200, "text/plain", String(batteryLevel).c_str());
  });
  
  server.begin();
}

void goToSleep() {
  // ESP32 wakes up every X seconds
  esp_sleep_enable_timer_wakeup(DELAY_SECONDS_SLEEP * DELAY_ESP32_ONE_SECOND_SLEEP);

  isSleeping = true;

  Serial.println("**********************************************************");
  Serial.println("                 Going to light-sleep now");
  Serial.println("**********************************************************");
  
  Serial.print(F("\n\n\n\n\n"));
  Serial.flush(); 

  esp_light_sleep_start();
} 

void wakeUp() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  Serial.println("==========================================================");
  Serial.print("                 ");
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }

  Serial.println("==========================================================");
  isSleeping = false;

  //initWifi();
  ESP.restart();
}
