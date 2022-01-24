void cells_update()
{
  if(!adsmux.adc_found)
    return;

  uint8_t local_cell_count = mmaths.mmin(config.cell_count, MAX_CELLS);

  for(uint8_t x = 0; x < local_cell_count; x++)
    read_cell_volts(x);


/*
  if(config.api_cellvolts)
  {
    pack_total_volts = 0;
    for(uint8_t x = 0; x < config.cell_count; x++)
      pack_total_volts += cells_volts[x];
  }
  */
  pack_total_volts = cells_volts_real[local_cell_count-1];
}

double read_cell_volts(const byte cell)
{
  if(!adsmux.adc_found)
    return -1;

  double v = adsmux.adc_val[cell];

  // avoid weirdness
  if(v<0)
    v=0;

  // ads1015 - 12-bit
  if(adsmux.adctype == 0)
    v = v/4096.0;

  // ADS1115 16-bit
  else if(adsmux.adctype == 1)
    v = v/65536.0;

  //mcp3021 - 10bit
  else if(adsmux.adctype == 2)
    v = v/1024.0;

  //mcp3221 - 12bit
  else if(adsmux.adctype == 3)
    v = v/4096.0;


  v *= config.battery_volt_mod[cell];
  v += config.dcvoltage_offset; // only use 1 offset

  cells_volts_real[cell] = cells_volts[cell] = v;

  if(cell > 0)
    cells_volts[cell] -= cells_volts_real[cell-1];

  return cells_volts[cell];
}

