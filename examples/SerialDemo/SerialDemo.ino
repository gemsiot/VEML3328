/******************************************************************************
SerialDemo.ino
A demo interface to the VEML3328 ambient light (and color) sensor 
Bobby Schulz @ GEMS IoT
12/14/2021

Displays the relative color intensities as well as the absolute chromatic intensity once every update period
Serial baud rate = 115200 bits/s

"Colors are the deeds and sufferings of light."
-Johann Wolfgang von Goethe

Distributed as-is; no warranty is given.
******************************************************************************/
#include <VEML3328.h>

VEML3328 ALS; //Instatiate Ambient Light Sensor

void setup() {
  pinMode(13, OUTPUT); //DEBUG!
  digitalWrite(13, HIGH); //DEBUG!
	Serial.begin(115200);
	while(!Serial); //Wait for serial to start
	Serial.println("They Should Have Sent a Poet...");
	ALS.begin();
}

void loop() {
	ALS.AutoRange(); //Run auto-range on each update 
	Serial.print("Green: ");
	Serial.println(ALS.GetValue(VEML3328::Channel::Clear), 6);
	Serial.print("Ambient: "); 
	Serial.println(ALS.GetLux(), 6);
	delay(5000);
}