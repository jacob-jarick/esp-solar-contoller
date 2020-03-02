#define jsonsize 2500

String get_payload(const String url)
{
  if(url.equals(""))
  {
    Serial.println("get_payload: blank URL.");
    return "";
  }

  String payload;
//   bool v = 0;

//   WiFiClient client;
  HTTPClient http;

  http.setTimeout(httpget_timeout);

  http.begin(url); //HTTP
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      time_since_check = millis();
      payload = http.getString();

//       Serial.print("get OK: ");
//       Serial.println(url);
    }
    else
    {
      Serial.print("\nHTTP ERROR: '" + url + "'" + String(httpCode));

      String tmp_str = datetime_str(0, '/', ' ', ':') + " - " + String(httpCode) + F(": ") + url;
      error_msgs = string_append_limit_size(error_msgs, tmp_str, size_error_msgs);

      payload = "";
    }
  }
  else
  {

    Serial.print("\nHTTP ERROR: '" + url + "'" + String(httpCode));
    Serial.println(http.errorToString(httpCode).c_str());

    payload = "";

    String tmp_str = datetime_str(0, '/', ' ', ':') + " - " + http.errorToString(httpCode).c_str() + String(": ") + url + String("\n");

    error_msgs = string_append_limit_size(error_msgs, tmp_str, size_error_msgs);

  }
  http.end();
  return payload;
}

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
  if(use_fallback < millis())
  {
    payload = get_payload(String(config.inverter_url));

    if(payload.equals(""))
    {
//       Serial.println("update_p_grid fetch: '" + String(config.inverter_url) + "' failed" );
      error_msgs = string_append_limit_size(error_msgs, datetime_str(0, '/', ' ', ':') + F(" - update_p_grid fetch failed\n"), size_error_msgs);
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
    use_fallback = millis() + (10 * 1000);

  // push url (fallback)
  if(!check && strlen(config.inverter_push_url))
  {
    fellback = 1;
    payload = get_payload(String(config.inverter_push_url));

    if(payload.equals(""))
    {
      Serial.println("update_p_grid fetch fallback: '" + String(config.inverter_url) + "' failed" );
      error_msgs = string_append_limit_size(error_msgs, datetime_str(0, '/', ' ', ':') + F(" - Fallback failed\n"), size_error_msgs);
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

    String tmp_str = datetime_str(0, '/', ' ', ':') +  String(F(" - JSON: ")) + error2.c_str() + String("\n");
    error_msgs = string_append_limit_size(error_msgs, tmp_str, size_error_msgs);

    use_fallback = millis() + (5 * 1000);
    error_msgs = string_append_limit_size(error_msgs, datetime_str(0, '/', ' ', ':') + F(" - JSON Decode Error\n"), size_error_msgs);

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

  bool check = get_payload(String(config.meter_url));

  if(!check)
  {
    oled_clear();
    error_msgs = string_append_limit_size(error_msgs, datetime_str(0, '/', ' ', ':') + F(" - 3p fetch Error\n"), size_error_msgs);
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

    String tmp_str = datetime_str(0, '/', ' ', ':') +  String(F(" - JSON: ")) + error2.c_str() + String("\n");
    error_msgs = string_append_limit_size(error_msgs, tmp_str, size_error_msgs);

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
    error_msgs = string_append_limit_size(error_msgs, datetime_str(0, '/', ' ', ':') + F(" - 3p: hour does not match\n"), size_error_msgs);
    return 0;
  }

  // must be  +/- 1min
  int8_t m = tmp.substring(14, 16).toInt();
  if (minute(timetmp) < m-1 || minute(timetmp) > m+1)
  {
    error_msgs = string_append_limit_size(error_msgs, datetime_str(0, '/', ' ', ':') + F(" - 3p: min does not match\n"), size_error_msgs);
    return 0;
  }

  if(!inverter_synced)
    inverter_synced = 1;

  return 1;
}

void set_power(float p)
{
  phase_sum = p;
  phase_avg = dirty_average(phase_avg, phase_sum, 3);

  if(config.avg_phase)
    phase_sum = phase_avg;
}
