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
    void bubbleSort(int16_t a[], const uint8_t size);
    void digital_write(const uint8_t pin, const bool status);

    uint8_t _adc_poll_pos;

    uint8_t _pins[3];
    bool _pin_mode[3];

    Adafruit_ADS1115 _ads;
};

#endif
