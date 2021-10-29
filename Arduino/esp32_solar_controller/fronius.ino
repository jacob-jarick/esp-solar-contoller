// #define jsonsize 2500

const uint8_t fronius_min_sync_seconds = 90;

const size_t jsonsize = 1024 * 4;




unsigned long fronius_last_time = 0;

bool update_p_grid()
{
  if(config.threephase)
    return update_p_grid_3phase();

  bool check = 0;
  bool fellback = 0;
  String payload = "";
  DynamicJsonDocument doc(jsonsize);
  String url;

  // main url - inverter_url
  if(timers.use_fallback < millis() && strlen(config.inverter_url))
  {
    url = String(config.inverter_url);

    bool result = get_url(url, payload);

    if(!result)
    {
      log_msg("fetch inverter_url failed");
      fellback = 1;
      timers.use_fallback = millis() + (10 * 1000);
    }
    else
    {
      check = 1;
    }
  }
  else
  {
    fellback = 1;
    Serial.println("fallback timer active." ); // or blank URL`
  }

  // push url (fallback)
  if(fellback && strlen(config.inverter_push_url))
  {
    url = String(config.inverter_push_url);
    bool result = get_url(url, payload);

    if(!result)
      log_msg(F("Error fetch inverter_push_url"));
    else
      check = 1;
  }

  if(!check)
    return 0; // failed to get datasource.

  // --------------------------------------------------------------------------
  // decode json

  DeserializationError error2 = deserializeJson(doc, payload.c_str());

  if (error2)
  {
    log_msg(String("update_p_grid JSON Decode ERROR") + error2.c_str());
    timers.use_fallback = millis() + (10 * 1000);

    return 0;
  }

  // --------------------------------------------------------------------------
  // get P_Grid value from JSON

  JsonObject root = doc.as<JsonObject>();
  JsonObject Body_Data_Site;


  // ----------------------------------------------------------------------------------------------
  // Check timestamp

  if(fellback)
  {
    String tmp = root["Head"]["Timestamp"];

    unsigned long local_s = local_secs();
    unsigned long json_secs = fronius_time_str_to_secs(tmp);
    unsigned long time_diff = mmaths.mdiff(json_secs, local_s);

    if (time_diff > fronius_min_sync_seconds)
    {
      log_msg("1p: json time " + String(time_diff) + "+ seconds out of sync");
      return 0;
    }
  }

  // check and update fronius_last_time
  {
    String tmp = root["Head"]["Timestamp"];

    unsigned long json_secs = fronius_time_str_to_secs(tmp);

    if(json_secs <= fronius_last_time)
    {
//       log_msg("1p: json not updated yet.");
      return 0;
    }
    fronius_last_time = json_secs;
//     log_msg("1p: updated.");
  }

  // ----------------------------------------------------------------------------------------------

  if(fellback)
    Body_Data_Site = root["Body"]["Site"]; // push json body location
  else
    Body_Data_Site = root["Body"]["Data"]["Site"]; // fetch (direct) json body location

  set_power(Body_Data_Site["P_Grid"]);

  if(!inverter_synced)
    inverter_synced = 1;

  timers.pgrid_last_update = millis();

  return 1;
}

bool update_p_grid_3phase()
{
  bool fellback = 0;

  String payload;
  DynamicJsonDocument doc(jsonsize);

  String url = String(config.threephase_direct_url);

  // Direct 3phase URL
  if(timers.use_fallback < millis() && strlen(config.threephase_direct_url) )
  {
    bool check = get_url(url, payload);

    if(!check)
    {
      log_msg("Fronius 3p Direct URL fetch Error");
      fellback = 1;
    }

    // decode json
    if(!fellback)
    {
      DeserializationError error2 = deserializeJson(doc, payload.c_str());

      if (error2)
      {
        log_msg(String("Fronius JSON Decode Error: ") + error2.c_str() );
        fellback = 1;
      }
    }
  }
  else
  {
    fellback = 1;
  }


  // attempt push URL
  if(fellback)
  {
    timers.use_fallback = millis() + (60 * 1000);

    if(!strlen(config.threephase_push_url))
      return 0;

    url = String(config.threephase_push_url);
    bool check = get_url(url, payload);

    if(!check)
    {
      log_msg("Fronius 3p Fallback fetch Error");
      return 0;
    }

    DeserializationError error2 = deserializeJson(doc, payload.c_str());

    if (error2)
    {
      if(flags.f3p_error1) // failed prior attempt, log message and clear flag
      {
        log_msg(String("Fronius 3p Fallback JSON Decode Error: ") + error2.c_str() );
        flags.f3p_error1 = 0;
      }
      else
      {
        flags.f3p_error1 = 1; // set error flag, if happens next time log message
      }
      return 0;
    }
  }
  flags.f3p_error1 = 0; // clear error flag as no error found

  JsonObject root = doc.as<JsonObject>();
  JsonObject Body_0;

  // --------------------------------------------------------------------------
  // check time on JSON and compare to local time.

  if(fellback)
  {
    String tmp = root["Head"]["Timestamp"];

    unsigned long local_s = local_secs();
    unsigned long json_secs = fronius_time_str_to_secs(tmp);
    unsigned long time_diff = mmaths.mdiff(json_secs, local_s);

    if (time_diff > fronius_min_sync_seconds)
    {
//       Serial.println("time check: Local: " + String(local_s) + ", JSON: " + String(json_secs) + ", Diff: " + String(time_diff) );

      log_msg("3p: json time " + String(time_diff) + "+ seconds out of sync");
      return 0;
    }
  }

  // check and update fronius_last_time
  {
    String tmp = root["Head"]["Timestamp"];

    unsigned long json_secs = fronius_time_str_to_secs(tmp);

    if(json_secs <= fronius_last_time)
    {
//       log_msg("3p: json not updated yet.");
      return 0;
    }
    fronius_last_time = json_secs;
//     log_msg("3p: updated.");
  }

  // --------------------------------------------------------------------------
  // get phase abc values from JSON

  if(fellback)
    Body_0 = root["Body"]["0"]; // push URL
  else
    Body_0 = root["Body"]["Data"]["0"]; // fetch (direct) url

  phase_a_watts = Body_0["PowerReal_P_Phase_1"];
  phase_b_watts = Body_0["PowerReal_P_Phase_2"];
  phase_c_watts = Body_0["PowerReal_P_Phase_3"];

  phase_a_voltage = Body_0["Voltage_AC_Phase_1"];
  phase_b_voltage = Body_0["Voltage_AC_Phase_2"];
  phase_c_voltage = Body_0["Voltage_AC_Phase_3"];


  if(!inverter_synced)
    inverter_synced = 1;

//   calc_energy_usage();

  timers.pgrid_last_update = millis();

  set_power(get_watts(1));

  return 1;
}


int8_t ceu_day = -1; // calc energy usage day
void calc_energy_usage()
{
  // calculate actual usage here
  // track energy consumed
  if(timers.calc_energy_usage != 0 && flags.time_synced)
  {
    float tmp_phase_sum = get_watts(3);

    float tmp_ms = millis() - timers.calc_energy_usage;

    energy_consumed += tmp_phase_sum * tmp_ms / 3600000.0 / 1000; // simplified maths, should catch decimals better too.

    int8_t tmp_day = day(now());

    if(ceu_day != tmp_day) // reset at midnight
    {
      ceu_day = tmp_day;
      energy_consumed_old = energy_consumed;
      energy_consumed = 0;
    }
  }

  timers.calc_energy_usage = millis();
}

// 0 = all summed
// 1 = only monitored
// 2 = only monitored and only if above 0
// 3 = ALL and only if above 0
float get_watts(uint8_t type)
{
  if(!config.threephase)
  {
    if(type == 0)
      return phase_sum;

    if(type == 1)
      return phase_sum;

    if(type == 3)
    {
      if(phase_sum > 0)
        return phase_sum;
      else
        return 0;
    }
  }

  float tmp = 0;

  if(type == 0)
  {
    tmp += phase_a_watts;
    tmp += phase_b_watts;
    tmp += phase_c_watts;

    return tmp;
  }
  if(type == 1)
  {
    if(config.monitor_phase_a)
      tmp += phase_a_watts;
    if(config.monitor_phase_b)
      tmp += phase_b_watts;
    if(config.monitor_phase_c)
      tmp += phase_c_watts;

    return tmp;
  }

  if(type == 2)
  {
    if(config.monitor_phase_a && phase_a_watts > 0)
      tmp += phase_a_watts;
    if(config.monitor_phase_b && phase_b_watts > 0)
      tmp += phase_b_watts;
    if(config.monitor_phase_c && phase_b_watts > 0)
      tmp += phase_c_watts;

    return tmp;
  }

  if(type == 3)
  {
    if(phase_a_watts > 0)
      tmp += phase_a_watts;
    if(phase_b_watts > 0)
      tmp += phase_b_watts;
    if(phase_c_watts > 0)
      tmp += phase_c_watts;

    return tmp;
  }

  return -1;
}

float power_array[power_array_size];
int8_t power_array_pos = -1;

void set_power(const float p)
{
  if(config.avg_phase <= 1)
  {
    phase_sum = p;
    return;
  }

  // -----------------------------------------------
  // begin average code

  // populate array on first use
  if(power_array_pos == -1)
  {
    for(uint8_t i = 0; i < power_array_size; i++)
      power_array[i] = p;
  }
  else
  {
    power_array[power_array_pos] = p;
  }

  power_array_pos++;
  if(power_array_pos >= power_array_size || power_array_pos >= config.avg_phase)
    power_array_pos = 0;

  phase_avg = 0;
  for(uint8_t i = 0; i < config.avg_phase; i++)
    phase_avg += power_array[i];

  phase_avg = phase_avg / config.avg_phase;

  phase_sum = phase_avg;
}

uint16_t fronius_time_str_to_min(String tmp)
{
  uint8_t json_d = tmp.substring(8, 10).toInt();
  uint8_t json_h = tmp.substring(11, 13).toInt();
  uint16_t json_m = tmp.substring(14, 16).toInt();

  return dhm_to_min(json_d, json_h, json_m);
}

unsigned long fronius_time_str_to_secs(String tmp)
{
  uint16_t json_y = tmp.substring(0, 4).toInt();
  uint8_t json_month = tmp.substring(5, 7).toInt();
  uint8_t json_d = tmp.substring(8, 10).toInt();

  uint8_t json_h = tmp.substring(11, 13).toInt();
  uint8_t json_min = tmp.substring(14, 16).toInt();
  uint8_t json_s = tmp.substring(17, 19).toInt();

//   Serial.println("\nF:" + String(json_y) + "." + String(json_month) + "." +  String(json_d) + "-" +  String(json_h) + ":" + String(json_min) + ":" + String(json_s) + "\n");

  return ymdhms_to_sec(json_y, json_month, json_d, json_h, json_min, json_s);
}

unsigned long ymdhms_to_sec(uint16_t YY, uint8_t MM, uint8_t DD, uint8_t HH, uint8_t mm, uint8_t ss)
{
  YY -= 2020; // 2020 is this codes epoch

  unsigned long result = 0;

  // years to months
  result = YY * 12;

  // months to days
  result += MM;
  result *= 31; // not correct but results match

  // days to hours
  result += DD;
  result *= 24;

  // hours to min
  result += HH;
  result *= 60;

  // min to sec
  result += mm;
  result *= 60;

  // add seconds
  result += ss;

//   Serial.println(result);

  return result;
}


unsigned long local_secs()
{
  time_t timetmp = now();
//   Serial.println("\nL " + String(year(timetmp)) + "-" + String(month(timetmp)) + "-" + String(day(timetmp)) + "-" + String(hour(timetmp)) + "-" + String(minute(timetmp)) + "-" + String(second(timetmp)) + "\n");
  return ymdhms_to_sec(year(timetmp), month(timetmp), day(timetmp), hour(timetmp), minute(timetmp), second(timetmp));
}



uint16_t local_minutes()
{
  time_t timetmp = now();
  return dhm_to_min(day(timetmp), hour(timetmp), minute(timetmp));
}

uint16_t dhm_to_min(uint8_t DD, uint8_t HH, uint8_t mm)
{
  return (DD * 24 * 60) + (HH * 60) + mm;
}

