// #define jsonsize 2500

const size_t jsonsize = 1024 * 3;

int8_t fronius_day = -1;

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

    // TODO check timestamp

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

  // Check timestamp
  if(fellback)
  {
    String tmp = root["Head"]["Timestamp"];
    uint16_t json_minutes = fronius_time_str_to_min(tmp);
    uint16_t local_m = local_minutes();
    uint16_t time_diff = mmaths.mdiff(json_minutes, local_m);

    if(time_diff > 2)
      log_msg("json time " + String(time_diff) + "+ min out of sync");

    return 0;
  }

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
//     time_t timetmp = now();
    String tmp = root["Head"]["Timestamp"];

    /*
//  2020-06-25T21:04:11+08:00
    uint8_t json_d = tmp.substring(8, 10).toInt();
    uint8_t json_h = tmp.substring(11, 13).toInt();
    uint16_t json_m = tmp.substring(14, 16).toInt();
  */
    uint16_t json_minutes = fronius_time_str_to_min(tmp);
    uint16_t local_m = local_minutes();
    uint16_t time_diff = mmaths.mdiff(json_minutes, local_m);


//     uint8_t local_d = day(timetmp);
//     uint16_t local_h = hour(timetmp);
//     uint16_t local_m = (local_d * 24 * 60) + (local_h * 60) + minute(timetmp);



    if (time_diff > 2)
    {
      log_msg("3p: json time " + String(time_diff) + "+ minutes out of sync");
      return 0;
    }
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


//   energy_consumed = Body_0["EnergyReal_WAC_Sum_Consumed"];
//   energy_consumed =  energy_consumed / 1000;


  phase_sum = 0;

  if(config.monitor_phase_a)
    phase_sum += phase_a_watts;
  if(config.monitor_phase_b)
    phase_sum += phase_b_watts;
  if(config.monitor_phase_c)
    phase_sum += phase_c_watts;

  if(!inverter_synced)
    inverter_synced = 1;

  // calculate actual usage here

  if(timers.pgrid_last_update != 0 && flags.time_synced)
  {
    float tmp_phase_sum = 0;

    if(phase_a_watts > 0)
      tmp_phase_sum += phase_a_watts;
    if(phase_b_watts > 0)
      tmp_phase_sum += phase_b_watts;
    if(phase_c_watts > 0)
      tmp_phase_sum += phase_c_watts;

    float tmp_ms = millis() - timers.pgrid_last_update;
//     energy_consumed += (tmp_phase_sum/1000.0) * (tmp_ms/3600000.0);
    energy_consumed += tmp_phase_sum * tmp_ms / 3600000.0 / 1000; // simplified maths, should catch decimals better too.

//     time_t timetmp = now();
//     uint16_t local_h = hour(timetmp);
//     uint16_t local_m = minute(timetmp);
    int8_t local_d = day(now());

    if(fronius_day != local_d) // reset at midnight
    {
      fronius_day = local_d;
      energy_consumed_old = energy_consumed;
      energy_consumed = 0;
    }
  }

  // END calculate actual usage here

  timers.pgrid_last_update = millis();

  set_power(phase_sum);

  return 1;
}

void set_power(const float p)
{
  phase_sum = p;
  phase_avg = mmaths.dirty_average(phase_avg, phase_sum, 3);

  if(config.avg_phase)
    phase_sum = phase_avg;
}

uint16_t fronius_time_str_to_min(String tmp)
{
  uint8_t json_d = tmp.substring(8, 10).toInt();
  uint8_t json_h = tmp.substring(11, 13).toInt();
  uint16_t json_m = tmp.substring(14, 16).toInt();

  return dhm_to_min(json_d, json_h, json_m);
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

