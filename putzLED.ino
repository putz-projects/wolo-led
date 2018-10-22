#include <TimerOne.h>
#include "LPD6803.h"

//Example to control LPD6803-based RGB LED Modules in a strand
// Original code by Bliptronics.com Ben Moyes 2009
//Use this as you wish, but please give credit, or at least buy some of my LEDs!

// Code cleaned up and Object-ified by ladyada, should be a bit easier to use

/*****************************************************************************/

// Choose which 2 pins you will use for output.
// Can be any valid output pins.
int dataPin = 2;       // 'yellow' wire
int clockPin = 3;      // 'green' wire
// Don't forget to connect 'blue' to ground and 'red' to +5V

// Timer 1 is also used by the strip to send pixel clocks

// Set the first variable to the NUMBER of pixels. 20 = 20 pixels in a row
LPD6803 strip = LPD6803(180, dataPin, clockPin);

void colorWipe(uint16_t c, uint8_t wait);
unsigned int Color(byte r, byte g, byte b);
unsigned int Color(byte r, byte g, byte b, byte i); // includes an intensity scaling factor
unsigned int Wheel(byte);
unsigned int Wheel(byte, byte);
void off();

//Serial incoming data
int inputByte = 0;

void setup() {
  
  // The Arduino needs to clock out the data to the pixels
  // this happens in interrupt timer 1, we can change how often
  // to call the interrupt. setting CPUmax to 100 will take nearly all all the
  // time to do the pixel updates and a nicer/faster display, 
  // especially with strands of over 100 dots.
  // (Note that the max is 'pessimistic', its probably 10% or 20% less in reality)
  
  strip.setCPUmax(100);  // start with 50% CPU usage. up this if the strand flickers or is slow
  
  // Start up the LED counter
  strip.begin();

  // Update the strip, to start they are all 'off'
  strip.show();

  // Setup Serial Connection
  Serial.begin(9600);
}

uint32_t wait = 200;
const uint8_t intensity1 = 32;
const uint8_t intensity2 = 120;
uint8_t intensity = intensity2;
uint32_t last = 0;
uint16_t seed = 37;
uint32_t last_command = 0;

void newseed() {
  uint8_t parity = seed >> 15;
  parity += seed >> 13;
  parity += seed >> 12;
  parity += seed >> 10;
  seed <<= 1;
  seed += parity & 1;
}

void loop() {
  int i, j;
   
  for (j=0; j < 96; j++) {     // 1 cycle of all 96 colors in the wheel
    if (Serial.available()) {
      switch (Serial.read()) {
      case 'X':
        colorWipe(Color(31, 31, 31), 25);
        colorWipe(Color(0, 0, 0), 25);
        last_command = millis();
        break;
      case 'Y':
        colorWipe(Color(31, 31, 31), 25);
        colorWipe(Color(0, 0, 0), 25);
        colorWipe(Color(31, 31, 31), 25);
        colorWipe(Color(0, 0, 0), 75);
        last_command = millis();
        break;
      case 'Z':
        colorWipe(Color(31, 31, 31), 25);
        colorWipe(Color(24, 24, 24), 25);
        colorWipe(Color(16, 16, 16), 25);
        colorWipe(Color(0, 0, 0), 75);
        last_command = millis();
        break;
      case '0':
        intensity = 0;
        break;
      case '1':
        intensity = intensity1;
        break;
      case '2':
        intensity = intensity2;
        break;
      }
    }
    else {
      uint8_t lo, med, hi;
      if (millis() - last_command < wait) {
        lo = uint32_t(intensity) * (millis() - last_command) / wait;
      } else {
        lo = intensity;
      }
      med = lo * 3/2;
      hi = lo * 2;
      for (i=0; i < strip.numPixels(); i++) {
        if (seed % 4)
          strip.setPixelColor(i, Wheel((i+j) % 96, lo));
        else if (seed % 8)
          strip.setPixelColor(i, Wheel((i+j) % 96, med));
        else
          strip.setPixelColor(i, Wheel((i+j) % 96, hi));
        newseed();
      }
    }  
    strip.show();   // write all the pixels out
  }
}

// fill the dots one after the other with said color
void colorWipe(uint16_t c, uint8_t wait) {
  uint32_t time = millis() + wait;
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
  while (millis() < time);
}

/* Helper functions */

// Create a 15 bit color value from R,G,B
unsigned int Color(byte r, byte g, byte b)
{
  //Take the lowest 5 bits of each value and append them end to end
  return( ((unsigned int)g & 0x1F )<<10 | ((unsigned int)b & 0x1F)<<5 | (unsigned int)r & 0x1F);
}

unsigned int Color(byte r, byte g, byte b, byte i)
{
  r = ((uint16_t(r) * i) >> 8);
  g = ((uint16_t(g) * i) >> 8);
  b = ((uint16_t(b) * i) >> 8);
  //Take the lowest 5 bits of each value and append them end to end
  return( ((unsigned int)g & 0x1F )<<10 | ((unsigned int)b & 0x1F)<<5 | (unsigned int)r & 0x1F);
}


//Input a value 0 to 127 to get a color value.
//The colours are a transition r - g -b - back to r
unsigned int Wheel(byte WheelPos)
{
  byte r,g,b;
  switch(WheelPos >> 5)
  {
    case 0:
      r=31- WheelPos % 32;   //Red down
      g=WheelPos % 32;      // Green up
      b=0;                  //blue off
      break; 
    case 1:
      g=31- WheelPos % 32;  //green down
      b=WheelPos % 32;      //blue up
      r=0;                  //red off
      break; 
    case 2:
      b=31- WheelPos % 32;  //blue down 
      r=WheelPos % 32;      //red up
      g=0;                  //green off
      break; 
  }
  return(Color(r,g,b));
}
unsigned int Wheel(byte WheelPos, byte intensity)
{
  byte r,g,b;
  switch(WheelPos >> 5)
  {
    case 0:
      r=31- WheelPos % 32;   //Red down
      g=WheelPos % 32;      // Green up
      b=0;                  //blue off
      break; 
    case 1:
      g=31- WheelPos % 32;  //green down
      b=WheelPos % 32;      //blue up
      r=0;                  //red off
      break; 
    case 2:
      b=31- WheelPos % 32;  //blue down 
      r=WheelPos % 32;      //red up
      g=0;                  //green off
      break; 
  }
  return(Color(r,g,b,intensity));
}
