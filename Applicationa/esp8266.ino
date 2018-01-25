//Including libraries
#include<Wire.h>
#include<ESP8266WiFi.h>
#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif
#ifndef max
#define max(x,y) (((x)>(y))?(x):(y))
#endif
#include <ArduinoJson.h>
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP8266.h>
//Timer part

unsigned long timeNow = 0;
unsigned long timeLast = 0;
int seconds = 0;
int minutes = 0;
bool flag;


  byte t; //Temperature
  byte h; //Humidity
  byte p; //PIR sensor
  byte s; //Soil moisture sensor
  
const char SID[]     = "Suresh"; //Network ssid
const char PASSWORD[] = "startoonlabs@2017";  //Network password
char username[] = "24779820-0102-11e8-8620-addae6ef14ff"; //User name of cayenne account
char password[] = "2d2b97f88f8e8128ccaa7a8743966302f1ca91b3"; //Password of Cayenne account
char clientID[] = "a9494d10-018d-11e8-a79c-af045a32c3da"; //Client id of Cayenne account

// Use your own API key by signing up for a free developer account.
// http://www.wunderground.com/weather/api/
#define WU_API_KEY "4a9fdc61f5681169" //API key of weather underground

#define WU_LOCATION "india/hyderabad" 


#define DELAY_NORMAL    (5*1000) // 5 seconds
#define DELAY_ERROR     (30*1000)

#define WUNDERGROUND "api.wunderground.com"

// HTTP request
const char WUNDERGROUND_REQ[] =
    "GET /api/" WU_API_KEY "/conditions/q/" WU_LOCATION ".json HTTP/1.1\r\n" //Json file link
    "User-Agent: ESP8266/0.1\r\n"
    "Accept: */*\r\n"
    "Host: " WUNDERGROUND "\r\n"
    "Connection: close\r\n"
    "\r\n";

void setup()
{
  Wire.begin();
  Serial.begin(115200);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(SID);
Cayenne.begin(username, password, clientID, SID, PASSWORD);
  //WiFi.begin(SID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

//  Serial.println();
//  Serial.println(F("WiFi connected"));
//  Serial.println(F("IP address: "));
//  Serial.println(WiFi.localIP());
  

}

static char respBuf[4096];
void loop()
{
  // Timer for every 4 min for getting info from WUNDERGROUND
  timeNow = millis()/1000; 
seconds = timeNow - timeLast;
if (seconds == 60) {
timeLast = timeNow;
minutes = minutes + 1; }
 
if (minutes == 2){
minutes = 0;
flag=true; }
  
   
   Cayenne.loop();
  int x = Wire.requestFrom(8,4); //requesting from slave
  Serial.println(x);  

  while (Wire.available()> 0) { 
    t = Wire.read();
    Serial.println(t);
    h= Wire.read();
    Serial.println(h);
    p=Wire.read();
    Serial.println(p);
    s= Wire.read();
    Serial.println(s); 
  } 
   
while(flag){
  // Open socket to WU server port 80
  Serial.print(F("Connecting to "));
  Serial.println(WUNDERGROUND);

  // Use WiFiClient class to create TCP connections
  WiFiClient httpclient;
  const int httpPort = 80;
  if (!httpclient.connect(WUNDERGROUND, httpPort)) {
    Serial.println(F("connection failed"));
    delay(DELAY_ERROR);
    return;

  }


  // This will send the http request to the server
  Serial.print(WUNDERGROUND_REQ);
  httpclient.print(WUNDERGROUND_REQ);
  httpclient.flush();

  // Collect http response headers and content from Weather Underground
  // HTTP headers are discarded.
  // The content is formatted in JSON and is left in respBuf.
  int respLen = 0;
  bool skip_headers = true;
  while (httpclient.connected() || httpclient.available()) {
    if (skip_headers) {
      String aLine = httpclient.readStringUntil('\n');
      //Serial.println(aLine);
      // Blank line denotes end of headers
      if (aLine.length() <= 1) {
        skip_headers = false;
      }
    }
    else {
      int bytesIn;
      bytesIn = httpclient.read((uint8_t *)&respBuf[respLen], sizeof(respBuf) - respLen);
      Serial.print(F("bytesIn ")); Serial.println(bytesIn);
      if (bytesIn > 0) {
        respLen += bytesIn;
        if (respLen > sizeof(respBuf)) respLen = sizeof(respBuf);
      }
      else if (bytesIn < 0) {
        Serial.print(F("read error "));
        Serial.println(bytesIn);
      }
    }
    delay(1);
  }
  httpclient.stop();

  if (respLen >= sizeof(respBuf)) {
    Serial.print(F("respBuf overflow "));
    Serial.println(respLen);
    delay(DELAY_ERROR);
    return;
  }
  // Terminate the C string
  respBuf[respLen++] = '\0';
  Serial.print(F("respLen "));
  Serial.println(respLen);
  //Serial.println(respBuf);
  
  if (showWeather(respBuf)) {
    delay(DELAY_NORMAL);
  }
  else {
    delay(DELAY_ERROR);
  }
flag=false;
  }
}

bool showWeather(char *json)
{
  StaticJsonBuffer<3*1024> jsonBuffer;

  // Skip characters until first '{' found
  // Ignore chunked length, if present
  char *jsonstart = strchr(json, '{');
  //Serial.print(F("jsonstart ")); Serial.println(jsonstart);
  if (jsonstart == NULL) {
    Serial.println(F("JSON data missing"));
    return false;
  }
  json = jsonstart;

  // Parse JSON
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println(F("jsonBuffer.parseObject() failed"));
    return false;
  }

  // Extract weather info from parsed JSON
  JsonObject& current = root["current_observation"];
  const float temp_f = current["temp_f"];
  Serial.print(temp_f, 1); Serial.print(F(" F, "));
  const float temp_c = current["temp_c"];
  Serial.print(temp_c, 1); Serial.print(F(" C, "));
  const char *humi = current[F("relative_humidity")];
  Serial.print(humi);   Serial.println(F(" RH"));
  const char *weather = current["weather"];
  Serial.println(weather);
   const char *pressure_mb = current["pressure_mb"];
  Serial.println(pressure_mb);
   const char *observation_time = current["observation_time_rfc822"];
  Serial.println(observation_time);
  //Sending the values to atmega
  Wire.beginTransmission(8);
  Wire.write(byte(temp_c));
  Wire.write(humi);
  Wire.write(weather);
  Wire.endTransmission();
  Serial.println(temp_c);
  Serial.println(humi);
  Serial.println(weather);
      
      
  // Extract local timezone fields
  const char *local_tz_short = current["local_tz_short"];
  Serial.println(local_tz_short);
  const char *local_tz_long = current["local_tz_long"];
  Serial.println(local_tz_long);
  const char *local_tz_offset = current["local_tz_offset"];
  Serial.println(local_tz_offset);

  return true;
}
CAYENNE_OUT_DEFAULT()
{
 Serial.println("cayenne call");
  //Writing on the cayenne platform (location, value)
  Cayenne.virtualWrite(1, t); // Sending temperature data on channel 1
  Cayenne.virtualWrite(2, h); // Sending humidity data on channel 2
  Cayenne.virtualWrite(3, p); // Sending pir data on channel 3
  Cayenne.virtualWrite(4, s); // Sending soil moisture data on channel 4
}

CAYENNE_IN_DEFAULT()
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}
