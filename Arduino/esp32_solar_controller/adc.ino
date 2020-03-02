//Todo swap to ADS1115


uint8_t cells_update_pos = 0;
void cells_update()
{
  read_cell_volts(cells_update_pos);

  uint8_t pos_plus_8 = cells_update_pos + 8;

  if(pos_plus_8 < config.cell_count)
    read_cell_volts(pos_plus_8);

  // update total voltage when we hit last cell
  if(config.cells_in_series && cells_update_pos == config.cell_count-1)
  {
    battery_voltage = cells_volts_real[config.cell_count];
  }

  cells_update_pos++;
  if(cells_update_pos >= 8 || cells_update_pos >= config.cell_count)
    cells_update_pos = 0;
}


// ADC ports 0-35
int16_t adc_read(uint8_t p)
{
  uint8_t channel = 0;
  uint8_t vp = p; // virtual port
  bool addr_a = 0;
  bool addr_b = 0;
  bool addr_c = 0;

  channel = p / 8;
  vp = p % 8;

//   if(p>7)
//   {
//     channel = 1;
//     vp = p - 8;
//   }

  if(vp == 0)
  {
    addr_a = 0;
    addr_b = 0;
    addr_c = 0;
  }
  else if(vp == 1)
  {
    addr_a = 0;
    addr_b = 0;
    addr_c = 1;
  }
  else if(vp == 2)
  {
    addr_a = 0;
    addr_b = 1;
    addr_c = 0;
  }
  else if(vp == 3)
  {
    addr_a = 0;
    addr_b = 1;
    addr_c = 1;
  }
  else if(vp == 4)
  {
    addr_a = 1;
    addr_b = 0;
    addr_c = 0;
  }
  else if(vp == 5)
  {
    addr_a = 1;
    addr_b = 0;
    addr_c = 1;
  }
  else if(vp == 6)
  {
    addr_a = 1;
    addr_b = 1;
    addr_c = 0;
  }
  else if(vp == 7)
  {
    addr_a = 1;
    addr_b = 1;
    addr_c = 1;
  }

  // set ads1115 mux channel
  ads.readADC_SingleEnded(channel);

  // set board mux channel
  digitalWrite(pin_asel1, addr_a);
  digitalWrite(pin_asel2, addr_b);
  digitalWrite(pin_asel3, addr_c);
  delay(5);

  int16_t value = ads.readADC_SingleEnded(channel);

  if(value < 0) // we are not measuring negative voltages
    return 0;

  return value;

//   const uint8_t sample_size = 8;
//   uint16_t tmp[sample_size];

//   for(uint8_t i = 0; i< sample_size; i++)
//     tmp[i] = analogRead(pin);

//   bubbleSort(tmp, sample_size);
//
//   uint16_t sample = tmp[sample_size/2];
//   return sample;
}

double read_cell_volts(byte cell)
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


float ntc10k_read_temp(byte sensor)
{
  float average = adc_read(sensor);

  const float THERMISTORNOMINAL = 10000;
  // temp. for nominal resistance (almost always 25 C)
  const float TEMPERATURENOMINAL = 25;
  // how many samples to take and average, more takes longer
  // but is more 'smooth'
  const float BCOEFFICIENT = 3950;
  // the value of the 'other' resistor
  const float SERIESRESISTOR = 10000;

  // convert the value to resistance
  //   average = 32767.0 / average - 1;
  //   https://forums.adafruit.com/viewtopic.php?f=25&t=135742
  //   float range  = 65535.0;
  //   float range = 32767.0; // 5v
  //   float range = 17599.46; // 3.3v

  if(average <= 0)    // shouldnt happen but just incase (ie absolute zero)
    average = 1;

  if(average > 4095) // again shouldnt happen
    average =  4095;



  average = average / 4095;

  average = SERIESRESISTOR / average;

  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C

  //   Serial.print("Temperature ");
  //   Serial.print(steinhart);
  //   Serial.println(" *C");

  return steinhart * config.ntc_temp_mods[sensor];
}
