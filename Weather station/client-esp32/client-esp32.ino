#if !( defined(ESP32) )
  #error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

#include <WiFi.h>
#include <HTTPClient.h>
#include <TM1637.h> // https://github.com/AKJ7/TM1637
//#include <EasyBuzzer.h> //https://evert-arias.github.io/EasyBuzzer/

//Delay
#define DELAY_ONE_SECOND        1000
#define DELAY_SECONDS_ALIVE     10
#define DELAY_SECONDS_SLEEP     50
#define DELAY_ESP32_ONE_SECOND_SLEEP 1000000ULL  /* Conversion factor for micro seconds to seconds */

// TM1637 - Led display
#define PIN_CLK             16
#define PIN_DIO             0

// Buzzer 
#define PIN_BUZZER      9 

const char* ssid = "ESP32-Master";
const char* password = "testtest1";

const char* serverIp = "192.168.1.60"; 

const char* urlTemperature = "http://192.168.1.60/temperature";
const char* urlHumidity = "http://192.168.1.60/humidity";
const char* urlPressure = "http://192.168.1.60/pressure";
const char* urlBattery = "http://192.168.1.60/battery";

String temperature = "";
String humidity = "";
String pressure = "";
String batteryLevel = "";
String payload = "----"; 

unsigned long previousMillis = 0;
const long interval = 3000; 

boolean isConnected = false;

TM1637 tm(PIN_CLK, PIN_DIO);

void setup() {
  Serial.begin(115200);

  //Init Led display (TM1637)
  tm.begin();
  tm.setBrightness(10);

  tm.display("HOLA");
  
  tryToConnectToMaster();
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    
    if (WiFi.status() == WL_CONNECTED ) { 

      isConnected = true;
      
      temperature = httpGETRequest(urlTemperature);
      humidity = httpGETRequest(urlHumidity);
      pressure = httpGETRequest(urlPressure);
      batteryLevel = httpGETRequest(urlBattery);
      
      myprint("Temperature: ");
      myprintln(String(temperature).c_str());
      //myprintln("Temperature: " + temperature + " *C - Humidity: " + humidity + " % - Pressure: " + pressure + " hPa  - Battery level: "+ batteryLevel );
      //myprintln("Temperature: " + temperature + " *C - Humidity: " + humidity + " % - Pressure: " + pressure + " hPa  - Battery level: "+ batteryLevel );

      if (temperature != payload) {
        tm.display(temperature,4);
      }
      
      previousMillis = currentMillis;
      
    } else {
      previousMillis = 0;
      if (isConnected){
        myprintln("WiFi Disconnected");
      }
      //tm.display("----");
      isConnected = false;

      tryToConnectToMaster();
    }
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, serverName);
 
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    //myprint("HTTP Response code: ");
    //myprintln(httpResponseCode);
    payload = http.getString();
  } else {
    myprint("Error code: ");
    myprintln(String(httpResponseCode).c_str());
    //tm.display("ER-"+httpResponseCode);
  }
  http.end();

  return payload;
}

void tryToConnectToMaster() {
  WiFi.begin(ssid, password);
  myprint("Connecting to ");
  myprintln(ssid);

  while(WiFi.status() != WL_CONNECTED) { 
    delay(DELAY_SECONDS_ALIVE * DELAY_ONE_SECOND);
    myprint(".");
  }
  
  //tm.display("-OK-");
  myprintln("");
  myprint("IP Address: ");
  myprintln(String(WiFi.localIP()).c_str());
}

void myprint(const char* string) {
  while (*string != '\0') {
    Serial.print(*string);
    ++string;
  }
}

void myprintln(const char* string) {
  while (*string != '\0') {
    Serial.print(*string);
    ++string;
  }
  Serial.print("\n");
}
