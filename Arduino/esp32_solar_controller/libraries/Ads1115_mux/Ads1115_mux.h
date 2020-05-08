#ifndef Ads1115_mux_h
#define Ads1115_mux_h

// the #include statment and code go here...

#include "Arduino.h"
// #include "QuickMedianLib.h"

#include <Adafruit_ADS1015.h>

class Ads1115_mux
{
  public:
    Ads1115_mux(uint8_t pina, uint8_t pinb, uint8_t pinc);

    void adc_poll();

    int16_t adc_val[32];
    bool adc_enable[32];

    float ntc10k_read_temp(const byte sensor);
    float resistance(const int16_t adc);
    float steinhart(const float R);

  private:
    void bubbleSort(int16_t a[], uint8_t size);

    uint8_t _pina;
    uint8_t _pinb;
    uint8_t _pinc;
    uint8_t _adc_poll_pos;

    Adafruit_ADS1115 _ads;

};

#endif
