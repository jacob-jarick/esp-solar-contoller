#include "Arduino.h"
#include <Adafruit_ADS1X15.h>
#include "Ads1115_mux.h"

Adafruit_ADS1115 _ads();
Adafruit_ADS1015 _ads2();

MCP3021 mcp3021(0x4D);
MCP3221 mcp3221(0x4D);

MCP3021 mcp3021b(0x4e);
MCP3221 mcp3221b(0x4e);

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

    for(uint8_t ii = 0; ii < ain_history_size; ii++)
      adc_val_history[i][ii] = 0;
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

  // ADS1015 or ADS1115. On new board can be either, determinted by config setting.
  // old board only ADS1115 unless jumper is wrong
  uint8_t addr = 0x48;
  if(i2c_ping(addr))
  {
    if(ads1x15type == 0)
    {
      Serial.println("ADS1015 found.");
      adctype = 0;

      _ads2.begin();
      _ads2.setGain(GAIN_ONE);
      _ads2.setDataRate(RATE_ADS1015_3300SPS); // default: RATE_ADS1015_1600SPS
    }
    else
    {
      Serial.println("ADS1115 found.");
      adctype = 1;

      _ads.begin();
      _ads.setGain(GAIN_ONE);
      _ads.setDataRate(RATE_ADS1115_860SPS); // default: RATE_ADS1115_128SPS
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
    _ads2.setDataRate(RATE_ADS1015_3300SPS); // default: RATE_ADS1015_1600SPS

    adc_found = 1;
    return;
  }

  // MCP3021 or MCP3221, determinted by config setting
  addr = 0x4D;
  if(i2c_ping(addr))
  {
    Serial.println("MCP3x21 found.");

    _mcpaddr = addr;

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

  addr = 0x4e;
  if(i2c_ping(addr))
  {
    Serial.println("MCP3x21 found at 0x4e.");

    _mcpaddr = addr;
    muxtype = 1;

    if(mcptype == 0)
    {
      adctype = 2;
      mcp3021b.init();
    }
    else
    {
      adctype = 3;
      mcp3221b.init();
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

void Ads1115_mux::adc_poll()
{
  if(!adc_found)
    return;

  polling_complete = 0;

  // ------------------------------------------------------------------------
  // CHECK 1 (reset)

  /// will reset polling loop and mark polling complete IF all inputs have been polled.
  {
    uint8_t mpcount = 16; // 16 for new boards (v4+), 8 for old boards
    if(muxtype == 0)
      mpcount = 8;

    // pol reset check 1. polled all AINs, 1-16
    if(_adc_poll_pos > mpcount-1)
    {
      _adc_mpins_set = 0;
      _adc_poll_pos = 0;
      polling_complete = 1;
      return;
    }

    // ------------------------------------------------------------------------
    // CHECK 2 (reset)

    // if all remaining AINs are disabled reset polling position and return.

    bool remaining = 0;
    for(uint8_t i = _adc_poll_pos; i< mpcount; i++)
    {
      // if any remaining are enabled set bool remaining to 1
      if
      (
        (muxtype == 0 && (adc_enable[i] || adc_enable[i+8])) || // muxtype 0, check both
        (muxtype == 1 && adc_enable[i])
      )
      {
        remaining = 1;
        break;
      }
    }

    if(!remaining)
    {
      _adc_mpins_set = 0;
      _adc_poll_pos = 0; // check 0 next round.
      polling_complete = 1;
      return;
    }

    // ------------------------------------------------------------------------
    // CHECK 3 (skip to next)

    // skip to next pol pos check
    if(!adc_enable[_adc_poll_pos])
    {
      _adc_mpins_set = 0;
      _adc_poll_pos++;
      return;
    }

  }

  // when 0, set mux pos, when 1 read mux.
  // acts as a non halting delay to get a stable voltage.
  // adc_pol gets called twice for each AIN, 1 to setup pins, 2 to read AIN.
  // Ive tried disabling this and regretted it each time...
  if(!_adc_mpins_set)
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
    else // old board
    {
      digital_write(0, value_a);
      digital_write(2, value_b);
      digital_write(3, value_c);
    }

    _adc_mpins_set = 1;
    return; // return without shifting _adc_poll_pos
  }

  // sample X times and bubble sort to get median value.
//   const uint8_t read_count = 11;
  int16_t areads[_sample_count];
  int16_t areads2[_sample_count]; // for muxtype 0

  for(uint8_t r = 0; r < _sample_count; r++)
  {
    // ADS1105
    if (adctype == 0)
    {
      areads[r] = _ads2.readADC_SingleEnded(0);

      if(muxtype == 0 && adc_enable[_adc_poll_pos+8]) // old boards mux (8-2)
        areads2[r] = _ads2.readADC_SingleEnded(1);
    }

    // ADS1115
    else if(adctype == 1)
    {
      areads[r] = _ads.readADC_SingleEnded(0); // always channel 0 on new boards

      if(muxtype == 0 && adc_enable[_adc_poll_pos+8]) // old boards mux (8-2)
        areads2[r] = _ads.readADC_SingleEnded(1); // always channel 0 on new boards
    }

    // mcp3021
    else if (adctype == 2)
    {
      if(_mcpaddr == 0x4D)
        areads[r] = mcp3021.read(); // new ADS
      else
        areads[r] = mcp3021b.read(); // new ADS
    }

    // mcp3221

    else if (adctype == 3)
    {
      if(_mcpaddr == 0x4D)
        areads[r] = mcp3221.read(); // new ADS
      else
        areads[r] = mcp3221b.read(); // new ADS
    }
  }

  bubbleSort(areads,_sample_count);
  update_adc_val(_adc_poll_pos, areads[_sample_count/2]);

  // muxtype 0 support
  if(muxtype == 0 && (adctype == 0 || adctype == 1) && adc_enable[_adc_poll_pos+8])
  {
    bubbleSort(areads2,_sample_count);
    update_adc_val(_adc_poll_pos+8, areads2[_sample_count/2]);
  }


  _adc_poll_pos++;
  _adc_mpins_set = 0;
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

void Ads1115_mux::update_adc_val(uint8_t index, uint16_t value)
{
  if(!avg_ain)
  {
    adc_val[index] = value;
    return;
  }

  // if empty (0) assume start up and prepopulate
  if(!adc_val_history[index][0])
  {
    for(uint8_t i = 0; i < ain_history_size; i++)
      adc_val_history[index][i] = value;
  }

  // shuffle array values forward
  for(uint8_t i = ain_history_size -1; i > 0; i--)
    adc_val_history[index][i] = adc_val_history[index][i-1];

  // set 0 as new value
  adc_val_history[index][0] = value;

  // calculate average
  float tmp_avg = 0;
  for(uint8_t i = 0; i < ain_history_size; i++)
    tmp_avg += adc_val_history[index][i];

  tmp_avg = tmp_avg / ain_history_size;

  // set adc_val;
  adc_val[index] = (uint16_t) tmp_avg;
}
