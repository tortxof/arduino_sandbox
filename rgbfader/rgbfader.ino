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
int maxIntensity = 255;
int red = maxIntensity;
int green = 0;
int blue = 0;

void setup() {
  // initialize the serial communication:
  Serial.begin(9600);
}

void loop() {
  // minus red plus green
  for(int i = 0; i < maxIntensity+1; i++) {
    green = i;
    red = maxIntensity - i;
    Esplora.writeRGB(red,green,blue);
    delay(10);
  }
  // minus green plus blue
  for(int i = 0; i < maxIntensity+1; i++) {
    blue = i;
    green = maxIntensity - i;
    Esplora.writeRGB(red,green,blue);
    delay(10);
  }
  // minus blue plus red
  for(int i = 0; i < maxIntensity+1; i++) {
    red = i;
    blue = maxIntensity - i;
    Esplora.writeRGB(red,green,blue);
    delay(10);
  }


}



