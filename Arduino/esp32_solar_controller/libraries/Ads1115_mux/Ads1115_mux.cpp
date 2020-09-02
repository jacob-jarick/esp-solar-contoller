#include "Arduino.h"
#include <Adafruit_ADS1015.h>
#include "Ads1115_mux.h"

Adafruit_ADS1115 _ads(0x48);
Adafruit_ADS1015 _ads2(0x49);

int8_t addr = 0;

Ads1115_mux::Ads1115_mux(uint8_t pina, uint8_t pinb, uint8_t pinc)
{
  adc_found = 1;

  _pins[0] = pina;
  _pins[1] = pinb;
  _pins[2] = pinc;

  uint8_t pins_asel[3] = {pina, pinb, pinc };
  for(uint8_t x = 0; x < 3; x++)
  {
    pinMode(pins_asel[x], OUTPUT);
    digitalWrite(pins_asel[x], LOW);
  }

  _adc_poll_pos = 8;

  for(uint8_t i = 0; i < 32; i++)
  {
    adc_val[i] = 0;
    adc_enable[i] = 0;
  }

  // setup pin
  for(uint8_t i = 0; i <3; i++)
  {
    digital_write(i, 1);  // set high to clear cache
    digital_write(i, 0);  // set low
  }

}


// setup needs i2c and serial started.
void Ads1115_mux::setup()
{
  if(i2c_ping(0x48))
  {
    addr = 0x48;
    Serial.println("ADS1115 found.");
    adctype = 1;
    _ads.setGain(GAIN_ONE);
  }
  else if(i2c_ping(0x49))
  {
    addr = 0x49;
    Serial.println("ADS1015 found.");
    adctype = 0;
    _ads2 = Adafruit_ADS1015((uint8_t)  0x49);
    _ads2.setGain(GAIN_ONE);
  }
  else
  {
    adc_found = 0;
    Serial.println("no ADC found.");
    return;
  }

}

/// cached digital write as arduinos port writes are slow.
void Ads1115_mux::digital_write(const uint8_t pin, const bool status)
{
  if(!adc_found)
    return;

  if(_pin_mode[pin] == status)
    return;

  _pin_mode[pin] = status;
  digitalWrite(_pins[pin], status);
}


void Ads1115_mux::adc_poll()
{
  if(!adc_found)
    return;

  _adc_poll_pos++;
  if(_adc_poll_pos > 7)
    _adc_poll_pos = 0;

  // if all ports for current pos disabled, return
  if
  (
    !adc_enable[_adc_poll_pos] &&
    !adc_enable[8+_adc_poll_pos] &&
    !adc_enable[16+_adc_poll_pos] &&
    !adc_enable[24+_adc_poll_pos]
  )
  {
    return;
  }


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

  digital_write(0, addr_a);
  digital_write(1, addr_b);
  digital_write(2, addr_c);

  for(uint8_t channel = 0; channel < 4; channel++)
  {
    uint8_t p = (channel*8) + _adc_poll_pos;

    if(!adc_enable[p])
      continue;

    const uint8_t read_count = 7;
    int16_t areads[read_count];

    for(uint8_t r = 0; r < read_count; r++)
    {
      if(adctype == 1)
        areads[r] = _ads.readADC_SingleEnded(channel);
      else if (adctype == 0)
        areads[r] = _ads2.readADC_SingleEnded(channel);
    }

    bubbleSort(areads,read_count);
    adc_val[p] = areads[read_count/2];
  }

  return;
}



// note on my esp32 board ntc sensors are 16-31

// borrowed NTC code from: https://github.com/OSBSS/Thermistor_v2/blob/master/Thermistor_v2.ino

float Ads1115_mux::ntc10k_read_temp(const byte sensor)
{
  if(!adc_found)
    return 0;

//   int16_t adc0 = adc_val[sensor];
  float R0 = resistance(adc_val[sensor]);
  float temperature0 = steinhart(R0);

  return temperature0;
}

// Get resistance -------------------------------------------//
float Ads1115_mux::resistance(const int16_t adc)
{
  if(!adc_found)
    return 0;


  float ADCvalue = adc*(8.192/3.3);  // Vcc = 8.192 on GAIN_ONE setting, Arduino Vcc = 3.3V in this case

//   float ADCvalue = adc*(8.192/3.3);  // Vcc = 8.192 on GAIN_ONE setting, Arduino Vcc = 3.3V in this case
  float R;

  if(adctype == 1)
    R = 10000/(65535/ADCvalue-1);  // 65535 refers to 16-bit number
  else
    R = 10000/(4095/ADCvalue-1);  // 4095 refers to 12-bit number

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
void Ads1115_mux::bubbleSort(int16_t a[], const uint8_t size)
{
  for(uint8_t i=0; i<(size-1); i++)
  {
    for(uint8_t o=0; o<(size-(i+1)); o++)
    {
      if(a[o] > a[o+1])
      {
        int16_t t = a[o];
        a[o] = a[o+1];
        a[o+1] = t;
      }
    }
  }
}

bool Ads1115_mux::i2c_ping(const char address)
{
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();

  if(error == 0)
    return 1;

  return 0;
}
