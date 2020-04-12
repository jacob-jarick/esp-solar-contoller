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

  // set board mux channel
  digitalWrite(pin_asel1, addr_a);
  digitalWrite(pin_asel2, addr_b);
  digitalWrite(pin_asel3, addr_c);

  for(uint8_t channel = 0; channel < 4; channel++)
  {
    uint8_t p = (channel*8) + adc_poll_pos;

    if(!adc_enable[p])
      continue;

    // do 2 throw aways
    ads.readADC_SingleEnded(channel);
    ads.readADC_SingleEnded(channel);
    adc_val[p] = ads.readADC_SingleEnded(channel);
  }


  adc_poll_pos++;
  if(adc_poll_pos > 7)
    adc_poll_pos = 0;

  return;
}


double read_cell_volts(const byte cell)
{
  double v = adc_val[cell];

  v *= ads_mv;
  v += config.dcvoltage_offset; // only use 1 offset
  v *= config.battery_volt_mod[cell];

  cells_volts_real[cell] = dirty_average(cells_volts_real[cell], v, 4); // TODO user enabled / disabled

  cells_volts[cell] = cells_volts_real[cell]; // copy AFTER avg

  if(config.cells_in_series && cell > 0)
    cells_volts[cell] -= cells_volts_real[cell-1];

  return cells_volts[cell];
}


float ntc10k_read_temp(const byte sensor)
{
  int16_t adc0 = adc_val[sensor + 16];

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
