#define jsonsize 2500

bool update_p_grid()
{
  bool check = 0;
  bool fellback = 0;
  String payload = "";
  DynamicJsonDocument doc(jsonsize);

  if(config.threephase)
  {
    check = update_p_grid_3phase();

    if(check)
      set_power(phase_sum);

    return check;
  }

  // main url - inverter_url
  String url = String(config.inverter_url);
  if(timers.use_fallback < millis())
  {
    bool result = get_url(url, payload);

    if(!result)
    {
      log_msg("update_p_grid fetch failed");
    }
    else
    {
      check = 1;
    }
  }
  else
  {
    Serial.println("fallback timer active." );
  }

  if(!check)
    timers.use_fallback = millis() + (10 * 1000);

  // push url (fallback)
  if(!check && strlen(config.inverter_push_url))
  {
    fellback = 1;
    String url = String(config.inverter_push_url);
    bool result = get_url(url, payload);

    if(!result)
    {
      Serial.println("update_p_grid fetch fallback: '" + String(config.inverter_url) + "' failed" );
      log_msg(F("Fronius Fallback failed"));
    }
    else
    {
      check = 1;
    }
  }

  if(!check)
    return 0; // failed to get datasource.

  // --------------------------------------------------------------------------
  // decode json

  DeserializationError error2 = deserializeJson(doc, payload.c_str());

  if (error2)
  {
    Serial.println(String("JSON Decode ERROR") + error2.c_str());
    Serial.println(payload);

    timers.use_fallback = millis() + (5 * 1000);

    log_msg(String("update_p_grid JSON Decode ERROR") + error2.c_str());

    return 0;
  }

  // --------------------------------------------------------------------------
  // get P_Grid value from JSON

  JsonObject root = doc.as<JsonObject>();
  JsonObject Body_Data_Site;

  if(fellback)
  {
    Body_Data_Site = root["Body"]["Site"];
  }
  else
  {
    JsonObject Body_Data = root["Body"]["Data"];
    Body_Data_Site = Body_Data["Site"];
  }
  set_power(Body_Data_Site["P_Grid"]);

  if(!inverter_synced)
    inverter_synced = 1;

  return 1;
}

bool update_p_grid_3phase()
{
  if(!strlen(config.meter_url))
  {
    return 0;
  }

  String payload;
  DynamicJsonDocument doc(jsonsize);

  String url = String(config.meter_url);
  bool check = get_url(url, payload);

  if(!check)
  {
    oled_clear();
    log_msg("Fronius 3phase URL fetch Error");
    both_println(F("1 HTTP Fetch\nERROR"));
    oled_set2X();
    both_println(WiFi.localIP().toString());

    return 0;
  }

//   check = json_decode_payload();
  // --------------------------------------------------------------------------
  // decode json
  check = 1;
  DeserializationError error2 = deserializeJson(doc, payload.c_str());

  if (error2)
  {
    Serial.println(String("JSON Decode ERROR") + error2.c_str());
    Serial.println(payload);

    log_msg(String("Fronius JSON Decode Error: ") + error2.c_str() );

//     payload = "";
//     check = 0;
    return 0;
  }

  // --------------------------------------------------------------------------
  // get phase abc values from JSON

  JsonObject root = doc.as<JsonObject>();
  JsonObject Body_0 = root["Body"]["0"];

  phase_a_watts = Body_0["PowerReal_P_Phase_1"]; // 671.6
  phase_b_watts = Body_0["PowerReal_P_Phase_2"]; // 0
  phase_c_watts = Body_0["PowerReal_P_Phase_3"]; // 0

  phase_sum = 0;

  if(config.monitor_phase_a)
    phase_sum += phase_a_watts;
  if(config.monitor_phase_b)
    phase_sum += phase_b_watts;
  if(config.monitor_phase_c)
    phase_sum += phase_c_watts;



  // --------------------------------------------------------------------------
  // check time on JSON and compare to local time.

  time_t timetmp = now();
  String tmp = root["Head"]["Timestamp"];

  // must be same hour
  if (hour(timetmp) != tmp.substring(11, 13).toInt())
  {
    log_msg("3p: hour does not match");
    return 0;
  }

  // must be  +/- 1min
  int8_t m = tmp.substring(14, 16).toInt();
  if (minute(timetmp) < m-1 || minute(timetmp) > m+1)
  {
    log_msg("3p: minute does not match");
    return 0;
  }

  if(!inverter_synced)
    inverter_synced = 1;

  return 1;
}

void set_power(const float p)
{
  phase_sum = p;
  phase_avg = dirty_average(phase_avg, phase_sum, 3);

  if(config.avg_phase)
    phase_sum = phase_avg;
}
