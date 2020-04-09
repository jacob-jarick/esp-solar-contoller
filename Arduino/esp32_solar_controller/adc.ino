void cells_update()
{
  for(uint8_t cells_update_pos = 0; cells_update_pos < config.cell_count; cells_update_pos++)
    read_cell_volts(cells_update_pos);
}


void ntc_update()
{
  for(uint8_t ntc_update_pos = 0; ntc_update_pos < config.ntc10k_count; ntc_update_pos++)
    ntc10k_read_temp(ntc_update_pos);

  bool trigger_shutdown = 0;
  for(int i = 0; i < config.ntc10k_count; i++)
  {
    if(ntc10k_sensors[i] > config.ntc_temp_max[i])
    {
      trigger_shutdown = 1;
      break;
    }
  }

  flags.shutdown_htemp = trigger_shutdown;

}


int16_t adc_read(const uint8_t p)
{
  return adc_val[p];
}

// // ADC ports 0-35
// int16_t adc_read_old(const uint8_t p)
// {
//   uint8_t channel = 0;
//   uint8_t vp = p; // virtual port
//   bool addr_a = 0;
//   bool addr_b = 0;
//   bool addr_c = 0;
//
//   channel = p / 8;
//   vp = p % 8;
//
//   if(vp == 0)
//   {
//     addr_a = 0;
//     addr_b = 0;
//     addr_c = 0;
//   }
//   else if(vp == 1)
//   {
//     addr_a = 0;
//     addr_b = 0;
//     addr_c = 1;
//   }
//   else if(vp == 2)
//   {
//     addr_a = 0;
//     addr_b = 1;
//     addr_c = 0;
//   }
//   else if(vp == 3)
//   {
//     addr_a = 0;
//     addr_b = 1;
//     addr_c = 1;
//   }
//   else if(vp == 4)
//   {
//     addr_a = 1;
//     addr_b = 0;
//     addr_c = 0;
//   }
//   else if(vp == 5)
//   {
//     addr_a = 1;
//     addr_b = 0;
//     addr_c = 1;
//   }
//   else if(vp == 6)
//   {
//     addr_a = 1;
//     addr_b = 1;
//     addr_c = 0;
//   }
//   else if(vp == 7)
//   {
//     addr_a = 1;
//     addr_b = 1;
//     addr_c = 1;
//   }
//
//   // set ads1115 mux channel
//   ads.readADC_SingleEnded(channel);
//
//   // set board mux channel
//   digitalWrite(pin_asel1, addr_a);
//   digitalWrite(pin_asel2, addr_b);
//   digitalWrite(pin_asel3, addr_c);
//   delay(3);
//
//   int16_t value = ads.readADC_SingleEnded(channel);
//
//   if(value < 0) // we are not measuring negative voltages
//     return 0;
//
//   return value;
// }

// used on startup
void adc_quick_poll()
{
  for(uint8_t i = 0; i < 8; i++)
  {
    oled_print(".");
    adc_poll();
  }
}

// ADC ports 0-35
uint8_t adc_poll_pos = 0;
void adc_poll()
{
  bool addr_a = 0;
  bool addr_b = 0;
  bool addr_c = 0;

  if(adc_poll_pos == 0)
  {
    addr_a = 0;
    addr_b = 0;
    addr_c = 0;
  }
  else if(adc_poll_pos == 1)
  {
    addr_a = 0;
    addr_b = 0;
    addr_c = 1;
  }
  else if(adc_poll_pos == 2)
  {
    addr_a = 0;
    addr_b = 1;
    addr_c = 0;
  }
  else if(adc_poll_pos == 3)
  {
    addr_a = 0;
    addr_b = 1;
    addr_c = 1;
  }
  else if(adc_poll_pos == 4)
  {
    addr_a = 1;
    addr_b = 0;
    addr_c = 0;
  }
  else if(adc_poll_pos == 5)
  {
    addr_a = 1;
    addr_b = 0;
    addr_c = 1;
  }
  else if(adc_poll_pos == 6)
  {
    addr_a = 1;
    addr_b = 1;
    addr_c = 0;
  }
  else if(adc_poll_pos == 7)
  {
    addr_a = 1;
    addr_b = 1;
    addr_c = 1;
  }

//   // set ads1115 mux channel
//   ads.readADC_SingleEnded(channel);

  // set board mux channel
  digitalWrite(pin_asel1, addr_a);
  digitalWrite(pin_asel2, addr_b);
  digitalWrite(pin_asel3, addr_c);

  for(uint8_t channel = 0; channel < 4; channel++)
  {
    // do 2 throw aways
    ads.readADC_SingleEnded(channel);
    ads.readADC_SingleEnded(channel);

    uint8_t p = (channel*8) + adc_poll_pos;
    adc_val[p] = ads.readADC_SingleEnded(channel);
  }


  adc_poll_pos++;
  if(adc_poll_pos > 7)
    adc_poll_pos = 0;

  return;
}


double read_cell_volts(const byte cell)
{
  double v = adc_read(cell);

  v *= ads_mv;
  v += config.dcvoltage_offset; // only use 1 offset
  v *= config.battery_volt_mod[cell];

  if(config.battery_volt_mod[cell] == 1) // if 1 we must be calibrating manually.
    cells_volts_real[cell] = v;
  else
    cells_volts_real[cell] = dirty_average(cells_volts_real[cell], v, 3); // TODO user enabled / disabled

  cells_volts[cell] = cells_volts_real[cell]; // copy AFTER avg

  if(config.cells_in_series && cell > 0)
    cells_volts[cell] -= cells_volts_real[cell-1];

  return cells_volts[cell];
}


float ntc10k_read_temp(const byte sensor)
{
  int virtual_pin = sensor + 16;

  int16_t adc0 = adc_read(virtual_pin);

  // get resistance --------------------------------------------//
  float R0 = resistance(adc0);

  // get temperature --------------------------------------------//
  float temperature0 = steinhart(R0);

  temperature0 *= config.ntc_temp_mods[sensor];

  ntc10k_sensors[sensor] = dirty_average(ntc10k_sensors[sensor], temperature0, 3);

  return ntc10k_sensors[sensor];
}

// borrowed this code from: https://github.com/OSBSS/Thermistor_v2/blob/master/Thermistor_v2.ino

// Get resistance -------------------------------------------//
float resistance(const int16_t adc)
{
  float ADCvalue = adc*(8.192/3.3);  // Vcc = 8.192 on GAIN_ONE setting, Arduino Vcc = 3.3V in this case
  float R = 10000/(65535/ADCvalue-1);  // 65535 refers to 16-bit number
  return R;
}

// Get temperature from Steinhart equation -------------------------------------------//
float steinhart(const float R)
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
