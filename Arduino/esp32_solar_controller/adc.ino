void cells_update()
{
  for(uint8_t x = 0; x < config.cell_count; x++)
    read_cell_volts(x);
}


void ntc_update()
{
  for(uint8_t sensor = 0; sensor < config.ntc10k_count; sensor++)
  {
    ntc10k_sensors[sensor] = dirty_average(ntc10k_sensors[sensor], adsmux.ntc10k_read_temp(sensor+16), 3);
  }

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
    adsmux.adc_poll();
  }
}


double read_cell_volts(const byte cell)
{
  const double ads_mv = 0.125 / 1000;

  double v = adsmux.adc_val[cell];

  v *= ads_mv;
  v += config.dcvoltage_offset; // only use 1 offset
  v *= config.battery_volt_mod[cell];

  cells_volts_real[cell] = dirty_average(cells_volts_real[cell], v, 4); // TODO user enabled / disabled

  cells_volts[cell] = cells_volts_real[cell]; // copy AFTER avg

  if(config.cells_in_series && cell > 0)
    cells_volts[cell] -= cells_volts_real[cell-1];

  return cells_volts[cell];
}

