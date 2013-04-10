/*
  Esplora LED Show
 
 Makes the RGB LED bright and glow as the joystick or the
 slider are moved.
 
 Created on 22 november 2012
 By Enrico Gueli <enrico.gueli@gmail.com>
 Modified 24 Nov 2012
 by Tom Igoe
 */
#include <Esplora.h>

int red = 0;
int green = 0;
int blue = 0;

void setup() {
  // initialize the serial communication:
  Serial.begin(9600);
}

void loop() {
  int slider = Esplora.readSlider()/4;
  if(Esplora.readButton(SWITCH_LEFT) == LOW) red=slider;
  if(Esplora.readButton(SWITCH_UP) == LOW) green=slider;
  if(Esplora.readButton(SWITCH_RIGHT) == LOW) blue=slider;
  if(red>255) red=255;
  if(green>255) green=255;
  if(blue>255) blue=255;
  Esplora.writeRGB(red, green,blue);
  // add a delay to keep the LED from flickering:  
  delay(100);
}

