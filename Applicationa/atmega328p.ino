
#include<Wire.h> //I2C
#include <TFT.h>  // Arduino LCD library
#include <SPI.h>
#include <dht.h> //DHT library

//Initializing the pins 

#define DHT11_PIN 3
int sensor=2;
int sensor_value;
int sensor_pin= A0;
int output;
byte t;
byte h;
dht DHT;
// pin definition for the Uno
#define cs   6
#define dc   9
#define rst  7


// create an instance of the library
TFT TFTscreen = TFT(cs, dc, rst);

// char array to print to the screen
char sensorPrintout[4];

void setup() {

  // Put this line at the beginning of every sketch that uses the GLCD:
  TFTscreen.begin();

  // clear the screen with a black background
  TFTscreen.background(0, 0, 0);

  // write the static text to the screen
  // set the font color to white
  TFTscreen.stroke(255, 255, 255);
  // set the font size
  TFTscreen.setTextSize(1.5);
  // write the text to the top left corner of the screen
  TFTscreen.text("Temperature(C):\n ", 0, 1);
  TFTscreen.text("Feels like", 0, 24);
  TFTscreen.text("Humidity is :\n", 0,44); 
Wire.begin(8); //address pin 8
Wire.onReceive(receiveEvent);
Wire.onRequest(requestEvent);
Serial.begin(115200);
pinMode(sensor, INPUT);
}
byte i;
void loop() {
  int chk = DHT.read11(DHT11_PIN);
   t= DHT.temperature;
   
  Serial.print("Temperature = ");
  Serial.println(t); //Temperature
  Serial.print("Humidity = "); 
  Serial.println(h); //Humidity
  sensor_value= digitalRead(sensor); //PIR sensor
  Serial.println(sensor_value);  
 output= analogRead(sensor_pin); //Soil moisture sensor
 output= 1020-output;
 Serial.println(output);
}
//character array to string
char c;
void receiveEvent(int howMany) {

 float t= Wire.read();
 Serial.println(t);
    
 char str[15] = {'\0'};
int i;
while(Wire.available()){
  
    str[i] = Wire.read();
   Serial.print(str[i]);
 i++; 
 
}

char th[10]= {'\0'};
int j=0;
int k=0;
for(j=0; j<3; j++) {
  th[k]= str[j];
  k++;

}
  Serial.println(th);
int h=3;
int l = 0;
char wt[15]={'\0'};
for(h=3; h<10;h++) {
wt[l]=str[h];
l++; 
}
Serial.println(wt);
 String sensorVal = String(t); 
 sensorVal.toCharArray(sensorPrintout, 6);
 TFTscreen.stroke(255, 255, 255); //Setting the color to write
 //Writing the values on screen 
 TFTscreen.text(sensorPrintout, 10, 14);
 TFTscreen.text(wt, 10, 34);
 TFTscreen.text(th, 10, 59);
 delay(250);
 //Refreshing the values
 TFTscreen.stroke(0, 0, 0);
 TFTscreen.stroke(255, 255, 255);
 TFTscreen.text(sensorPrintout, 10, 14);
 TFTscreen.text(wt, 10, 34);
 TFTscreen.text(th, 10, 59);
}
void requestEvent() {
 
  Wire.write(t);//write temperature value
  Wire.write(h); //write humidity value
  Wire.write(sensor_value); //Write PIR sensor value
  Wire.write(output); //write soil moisture sensorr value
}
