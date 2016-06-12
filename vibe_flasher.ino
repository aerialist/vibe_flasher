// Detect movement and turn neopixel on for a while at different colors
//
// Copyright (c) 2016 Shunya Sato
// Author: Shunya Sato
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <avr/sleep.h>
#include <Adafruit_NeoPixel.h>

#define BUTTON_PIN   2    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.
#define PIXEL_PIN    4    // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 1

#define DELTA_PLUS 1
#define DELTA_MINUS 1

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

volatile int wheelVal = 0;
volatile uint8_t brightness = 255;
volatile unsigned long isrMillis = 0;

void vib_isr(){
  if (wheelVal < 255 - DELTA_PLUS){
    wheelVal += DELTA_PLUS;
  } else {
    wheelVal = 0;
  }
  brightness = 255;
  isrMillis = millis();
}

void vib_decrease(){
  if (brightness > DELTA_MINUS){
    brightness -= DELTA_MINUS;
  } else {
    brightness = 0;
  }
}

void sleep_isr(){
  // isr to wake up from sleep
  sleep_disable();
  detachInterrupt(0);
  vib_isr();
}

void go_sleep(){
  detachInterrupt(0);
  noInterrupts(); // disable interrupts i.e. cli();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //Serial.println("Going to sleep!");
  sleep_enable(); // Set the SE (sleep enable) bit.
  attachInterrupt(0, sleep_isr, LOW);
  //sleep_bod_disable();
  interrupts(); // enable interrupts i.e. sei();
  sleep_cpu();  // Put the device into sleep mode. The SE bit must be set beforehand, and it is recommended to clear it afterwards.

  /* wake up here */
  sleep_disable();
  //Serial.println("I'm awake!");
  interrupts(); // end of some_condition
  attachInterrupt(0, vib_isr, FALLING);
}

void setup() {
  //Serial.begin(115200);
  //Serial.println("Starting vibe flasher!");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  attachInterrupt(0, vib_isr, FALLING);
}

void loop() {
  if (brightness <= 0){
    strip.setPixelColor(0, strip.Color(0,0,0));
    strip.show();
    go_sleep();
  }

  unsigned long currentMillis = millis();
  unsigned long elapsedTime = currentMillis - isrMillis; // elapsed time since last isr

  //strip.setPixelColor(0, strip.Color(255,0,0));
  strip.setPixelColor(0, Wheel(wheelVal));
  strip.setBrightness(brightness);
  strip.show();
  vib_decrease();
  noInterrupts();
  delay(200);
  interrupts();

}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
