#ifndef Ads1115_mux_h
#define Ads1115_mux_h

// the #include statment and code go here...

#include "Arduino.h"
// #include "QuickMedianLib.h"

#include <Adafruit_ADS1X15.h>
#include "MCP3X21.h"

// // esp32 adc
// #include <driver/adc.h>

class Ads1115_mux
{
  public:
    Ads1115_mux(uint8_t pina, uint8_t pinb, uint8_t pinc, uint8_t pind);

    void setup();

    void adc_poll();

    uint8_t adctype;

    bool i2c_ping(const char address);

    bool mcptype = 0;
    bool ads1x15type = 0;
    bool muxtype = 0; // 0 = 8-2 (old style), 1 = 16-1 (new style)

    bool polling_complete = 0; // 0 until last AIN checked (AIN15) then 0 again when starting from AIN0

    const static uint8_t ain_count = 16; // todo reduce to 16

    int16_t adc_val[ain_count];
    bool adc_enable[ain_count];

    bool adc_found;

  private:
    void bubbleSort(int16_t a[], const uint8_t size);
    void digital_write(const uint8_t pin, const bool status);

    const static uint8_t _sample_count = 11; // how many times a ADC value is read for the median array

    uint8_t _adc_poll_pos;

    bool _adc_poll_bool = 0;

    const static uint8_t _pin_count = 4;

    uint8_t _pins[_pin_count];
    bool _pin_mode[_pin_count];

    Adafruit_ADS1115 _ads;
    Adafruit_ADS1015 _ads2;

    MCP3021 mcp3021;
    MCP3221 mcp3221;
};

#endif
