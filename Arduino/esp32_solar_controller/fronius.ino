#define jsonsize 2500

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
      log_msg("Fronius 3phase Direct URL fetch Error");
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
      log_msg("Fronius 3phase Fallback URL fetch Error");
      return 0;
    }

    DeserializationError error2 = deserializeJson(doc, payload.c_str());

    if (error2)
    {
      log_msg(String("Fronius 3p Fallback JSON Decode Error: ") + error2.c_str() );
      return 0;
    }

  }

  JsonObject root = doc.as<JsonObject>();
  JsonObject Body_0;

  // --------------------------------------------------------------------------
  // check time on JSON and compare to local time.

  if(fellback)
  {
    time_t timetmp = now();
    String tmp = root["Head"]["Timestamp"];

    // must be same hour

    uint8_t json_h = tmp.substring(11, 13).toInt() % 23;
    uint16_t json_m = tmp.substring(14, 16).toInt();

    json_m += (json_h * 60);

    uint16_t local_m = ((hour(timetmp) % 23) * 60) + minute(timetmp);

    uint16_t time_diff = mmaths.mdiff(json_m, local_m);

    if (time_diff > 2)
    {
      log_msg("3p: json timestamp " + String(time_diff) + "+ minutes out of sync");
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


  phase_sum = 0;

  if(config.monitor_phase_a)
    phase_sum += phase_a_watts;
  if(config.monitor_phase_b)
    phase_sum += phase_b_watts;
  if(config.monitor_phase_c)
    phase_sum += phase_c_watts;

  if(!inverter_synced)
    inverter_synced = 1;

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
