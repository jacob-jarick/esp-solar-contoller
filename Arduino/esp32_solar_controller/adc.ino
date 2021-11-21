void cells_update()
{
  if(!adsmux.adc_found)
    return;

  for(uint8_t x = 0; x < config.cell_count; x++)
    read_cell_volts(x);
}


// used on startup
void adc_quick_poll()
{
  if(!adsmux.adc_found)
    return;

  while(adsmux.polling_complete == 0)
  {
    oled_print(".");
    adsmux.adc_poll();
  }
}


double read_cell_volts(const byte cell)
{
  if(!adsmux.adc_found)
    return -1;

//   const double ads_mv = 0.125 / 1000;

  double v = adsmux.adc_val[cell];


  // ads1015 - 12-bit
  if(adsmux.adctype == 0)
    v = v/4096;

  // ADS1115 16-bit
  else if(adsmux.adctype == 1)
    v = v/65536;

  //mcp3021 - 10bit
  else if(adsmux.adctype == 2)
    v = v/1024;

  //mcp3221 - 12bit
  else if(adsmux.adctype == 3)
    v = v/4096;


//   v *= ads_mv;
  v *= config.battery_volt_mod[cell];
  v += config.dcvoltage_offset; // only use 1 offset

  if(cells_volts_real[cell] <= 0)
    cells_volts_real[cell] = v;
  else
    cells_volts_real[cell] = mmaths.dirty_average(cells_volts_real[cell], v, 3); // TODO user enabled / disabled

  cells_volts[cell] = cells_volts_real[cell]; // copy AFTER avg

  if(config.cells_in_series && cell > 0)
    cells_volts[cell] -= cells_volts_real[cell-1];

  return cells_volts[cell];
}

