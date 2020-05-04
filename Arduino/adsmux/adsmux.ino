#include "Arduino.h"
#include "QuickMedianLib.h"

#include <Adafruit_ADS1015.h>


#include <Ads1115_mux.h>
const uint8_t pin_asel1 = 27;
const uint8_t pin_asel2 = 14;
const uint8_t pin_asel3 = 26;

const adsGain_t ads_gain = GAIN_ONE;
const float ads_mv = 0.125 / 1000;

Ads1115_mux adsmux(pin_asel1, pin_asel2, pin_asel3);

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
