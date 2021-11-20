#include "Arduino.h"
#include <Adafruit_ADS1X15.h>
#include "Ads1115_mux.h"

Adafruit_ADS1115 _ads();
Adafruit_ADS1015 _ads2();

MCP3021 mcp3021(0x4D);
MCP3221 mcp3221(0x4D);

// make most of this part of setup function.
// so pins can be set based on config settings.
// even so....easier to defunct old board design because of different mux to ADC pins setup (16-1 pin vs 2x8-2).
Ads1115_mux::Ads1115_mux(uint8_t pina, uint8_t pinb, uint8_t pinc, uint8_t pind)
{
  _pins[0] = pina;
  _pins[1] = pinb;
  _pins[2] = pinc;
  _pins[3] = pind;

  _adc_poll_pos = 0;

  for(uint8_t i = 0; i < ain_count; i++)
  {
    adc_val[i] = 0;
    adc_enable[i] = 0;
  }

  // setup pin
  for(uint8_t i = 0; i <_pin_count; i++)
  {
    pinMode(_pins[i], OUTPUT);

    digital_write(i, 1);  // set high to clear cache
    digital_write(i, 0);  // set low
  }
}


// setup needs i2c and serial started.
void Ads1115_mux::setup()
{
  adc_found = 0;

  uint8_t addr = 0x48;
  if(i2c_ping(addr))
  {
    if(ads1x15type == 0)
    {
      Serial.println("ADS1015 found.");
      adctype = 0;

      _ads2.begin();
      _ads2.setGain(GAIN_ONE);
    }
    else
    {
      Serial.println("ADS1115 found.");
      adctype = 1;

      _ads.begin();
      _ads.setGain(GAIN_ONE);
    }

    adc_found = 1;
    return;
  }

  // ADS1015 on address 0x49 (old board)
  addr = 0x49;
  if(i2c_ping(addr))
  {
    Serial.println("ADS1015 found.");
    adctype = 0;

    _ads2.begin();
    _ads2.setGain(GAIN_ONE);

    adc_found = 1;
    return;
  }

  addr = 0x4D;
  if(i2c_ping(addr))
  {
    Serial.println("MCP3021 found.");

    muxtype = 1;

    if(mcptype == 0)
    {
      adctype = 2;
      mcp3021.init();
    }
    else
    {
      adctype = 3;
      mcp3221.init();
    }

    adc_found = 1;
    return;
  }

  adc_found = 0;
  Serial.println("no ADC found.");
  return;

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

// _adc_poll_bool = 0;
void Ads1115_mux::adc_poll()
{
  if(!adc_found)
    return;

  // _adc_poll_pos wrap to zero check
  if
  (

    (muxtype == 0 && _adc_poll_pos > 7) || // 7 for old boards (8-2)
    (_adc_poll_pos > 15) // 16 is mux's max inputs. (16-1)
  )
  {
    _adc_poll_pos = 0;
  }

  // is this last poll for this round ?
  polling_complete = 0;
  if
  (
    (muxtype == 0 && _adc_poll_pos == 7) || // 7 for old boards (8-2)
    (_adc_poll_pos == 15) // 16 is mux's max inputs. (16-1)
  )
  {
    polling_complete = 1;
  }

  if(!adc_enable[_adc_poll_pos])
  {
    _adc_poll_pos++;
    return;
  }


  // when 0, set mux pos, when 1 read mux.
  // acts as a non halting delay to get a stable voltage.
  // adc_pol gets called twice for each AIN, 1 to setup pins, 2 to read AIN.
  if(!_adc_poll_bool)
  {
    const bool value_a = bitRead(_adc_poll_pos, 0);
    const bool value_b = bitRead(_adc_poll_pos, 1);
    const bool value_c = bitRead(_adc_poll_pos, 2);
    const bool value_d = bitRead(_adc_poll_pos, 3);

    if(muxtype == 1) // new board
    {
      digital_write(0, value_a);
      digital_write(1, value_b);
      digital_write(2, value_c);
      digital_write(3, value_d);
    }
    else
    {
      digital_write(0, value_a);
      digital_write(2, value_b);
      digital_write(3, value_c);

      // 3 0 2 - 0 correct, 1s reading goes to 2
      // 3 2 0 - 0 correct, 1s reading goes to 4
      // 0 2 3 -
    }

    _adc_poll_bool = 1;
    return; // return without shifting _adc_poll_pos
  }

  // sample X times and bubble sort to get median value.
  const uint8_t read_count = 16;
  int16_t areads[read_count];
  int16_t areads2[read_count]; // for muxtype 0

  for(uint8_t r = 0; r < read_count; r++)
  {
    // ADS1105
    if (adctype == 0)
    {
      areads[r] = _ads2.readADC_SingleEnded(0);

      if(muxtype == 0) // old boards mux (8-2)
        areads2[r] = _ads2.readADC_SingleEnded(1);
    }

    // ADS1115
    else if(adctype == 1)
    {
      areads[r] = _ads.readADC_SingleEnded(0); // always channel 0 on new boards

      if(muxtype == 0) // old boards mux (8-2)
        areads2[r] = _ads.readADC_SingleEnded(1); // always channel 0 on new boards
    }

    // mcp3021
    else if (adctype == 2)
      areads[r] = mcp3021.read(); // new ADS

    // mcp3221
    else if (adctype == 3)
      areads[r] = mcp3221.read(); // new ADS
  }

  bubbleSort(areads,read_count);
  adc_val[_adc_poll_pos] = areads[read_count/2];

  if((adctype == 0 || adctype == 1) && muxtype == 0)
  {
    bubbleSort(areads2,read_count);
    adc_val[_adc_poll_pos+8] = areads2[read_count/2];
  }


  _adc_poll_pos++;
  _adc_poll_bool = 0;
  return;
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
