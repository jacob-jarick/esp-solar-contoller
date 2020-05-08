#include "Arduino.h"
// #include "QuickMedianLib.h"
#include <Adafruit_ADS1015.h>
#include "Ads1115_mux.h"

uint8_t _pina;
uint8_t _pinb;
uint8_t _pinc;
uint8_t _adc_poll_pos;
Adafruit_ADS1115 _ads;


Ads1115_mux::Ads1115_mux(uint8_t pina, uint8_t pinb, uint8_t pinc)
{
  _pina = pina;
  _pinb = pinb;
  _pinc = pinc;

  uint8_t pins_asel[3] = {pina, pinb, pinc };
  for(uint8_t x = 0; x < 3; x++)
  {
    pinMode(pins_asel[x], OUTPUT);
    digitalWrite(pins_asel[x], LOW);
  }

  _adc_poll_pos = 0;

  for(uint8_t i = 0; i < 32; i++)
  {
    adc_val[i] = 0;
    adc_enable[i] = 0;
  }

  const adsGain_t ads_gain = GAIN_ONE;

  _ads.setGain(ads_gain);
}



void Ads1115_mux::adc_poll()
{
  bool addr_a = 0;
  bool addr_b = 0;
  bool addr_c = 0;

  if(_adc_poll_pos == 0)
  {
    addr_a = 0;
    addr_b = 0;
    addr_c = 0;
  }
  else if(_adc_poll_pos == 1)
  {
    addr_a = 0;
    addr_b = 0;
    addr_c = 1;
  }
  else if(_adc_poll_pos == 2)
  {
    addr_a = 0;
    addr_b = 1;
    addr_c = 0;
  }
  else if(_adc_poll_pos == 3)
  {
    addr_a = 0;
    addr_b = 1;
    addr_c = 1;
  }
  else if(_adc_poll_pos == 4)
  {
    addr_a = 1;
    addr_b = 0;
    addr_c = 0;
  }
  else if(_adc_poll_pos == 5)
  {
    addr_a = 1;
    addr_b = 0;
    addr_c = 1;
  }
  else if(_adc_poll_pos == 6)
  {
    addr_a = 1;
    addr_b = 1;
    addr_c = 0;
  }
  else if(_adc_poll_pos == 7)
  {
    addr_a = 1;
    addr_b = 1;
    addr_c = 1;
  }

  // set board mux channel
  digitalWrite(_pina, addr_a);
  digitalWrite(_pinb, addr_b);
  digitalWrite(_pinc, addr_c);

  for(uint8_t channel = 0; channel < 4; channel++)
  {
    uint8_t p = (channel*8) + _adc_poll_pos;

    if(!adc_enable[p])
      continue;

    // do X reads
    const uint8_t read_count = 7;
    int16_t areads[read_count];

    for(uint8_t r = 0; r < read_count; r++)
      areads[r] = _ads.readADC_SingleEnded(channel);

    bubbleSort(areads,read_count);

//     adc_val[p] = QuickMedian<int16_t>::GetMedian(areads, sizeof(areads) / sizeof(int16_t));
    adc_val[p] = areads[read_count/2];
  }


  _adc_poll_pos++;
  if(_adc_poll_pos > 7)
    _adc_poll_pos = 0;

  return;
}



// note on my esp32 board ntc sensors are 16-31
float Ads1115_mux::ntc10k_read_temp(const byte sensor)
{
  int16_t adc0 = adc_val[sensor];
  float R0 = resistance(adc0);
  float temperature0 = steinhart(R0);

  return temperature0;
}

// borrowed this code from: https://github.com/OSBSS/Thermistor_v2/blob/master/Thermistor_v2.ino

// Get resistance -------------------------------------------//
float Ads1115_mux::resistance(const int16_t adc)
{
  float ADCvalue = adc*(8.192/3.3);  // Vcc = 8.192 on GAIN_ONE setting, Arduino Vcc = 3.3V in this case
  float R = 10000/(65535/ADCvalue-1);  // 65535 refers to 16-bit number
  return R;
}

// Get temperature from Steinhart equation -------------------------------------------//
float Ads1115_mux::steinhart(const float R)
{
  float Rref = 10000.0;
  float A = 0.003354016;
  float B = 0.0002569850;
  float C = 0.000002620131;
  float D = 0.00000006383091;
  float E = log(R/Rref);

  float T = 1/(A + (B*E) + (C*(E*E)) + (D*(E*E*E)));
  return T-273.15;
}


// bubble sort
// https://www.femtech.dk/teaching-day-2017-at-diku/02-bubble-sort-algorithm/
void Ads1115_mux::bubbleSort(int16_t a[], uint8_t size) {
  for(uint8_t i=0; i<(size-1); i++) {
    for(uint8_t o=0; o<(size-(i+1)); o++) {
      if(a[o] > a[o+1]) {
        int16_t t = a[o];
        a[o] = a[o+1];
        a[o+1] = t;
      }
    }
  }
}
