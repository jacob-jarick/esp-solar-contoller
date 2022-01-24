String mdns_hn_cache = "";
String mdns_ip_cache = "";
unsigned long mdns_cache_timer = 0;
bool mdnscachelookup(String hn, String &ip)
{
  if(hn == "")
  {
    log_msg("MDNS lookup, null hostname");
    return false;
  }

  if(hn == mdns_hn_cache && mdns_cache_timer > millis())
  {
    ip = mdns_ip_cache;
    return 1;
  }

  IPAddress serverIp = MDNS.queryHost(hn);
  if(serverIp.toString() == "0.0.0.0")
  {
    String msg = "API: MDNS address resolution failed for host: " + hn;
    log_msg(msg);

    return false;
  }

  mdns_hn_cache = hn;
  mdns_ip_cache = ip = serverIp.toString();
  mdns_cache_timer = millis() + (15 * 60 * 1000);

  return true;
}


bool api_sync(uint8_t serverid)
{
  String shn = "";
  if(serverid == 1)
  {
    shn = String(config.api_server1);
  }
  else if(serverid == 2)
  {
    shn = String(config.api_server2);
  }
  else
  {
    log_msg("api_sync ERROR unknown server id: " + String(serverid) );
  }


  String msg = "";
  String hostip = "";

  if(!mdnscachelookup(String(config.api_server1), hostip))
  {
    log_msg("MDNS lookup '" + String(config.api_server1) + "' error.");
    return false;
  }

  String url = "http://" + hostip + "/jsonapi";

  String payload = "";
  bool check = get_url(url, payload);

  if(!check)
  {
    msg = "API fetch error, URL: " + url;
    log_msg(msg);

    return false;
  }

  DynamicJsonDocument doc(jsonsize);

  DeserializationError error2 = deserializeJson(doc, payload.c_str());

  if (error2)
  {
    log_msg(String("API Decode Error: ") + error2.c_str() );

    return false;
  }

  // JSON is ready

  // compare api_server1 to json hostname.


  if(doc.containsKey("host_name"))
  {
    String rhn = doc["host_name"];
    if(shn != rhn)
    {
      log_msg("API ERROR: wrong hostname '" + rhn + "' in JSON.");
      mdns_hn_cache = ""; // invalidate cache

      return false;
    }
  }
  else
  {
    log_msg("API ERROR: hostname not present in JSON.");
    Serial.println("XX");

    return false;
  }


  // get amb temp

  //config.api_lm75a
  if(config.api_lm75a)
  {
    if(doc["lm75a"] == 0)
    {
      msg = "API Server lm75a not found, yet config requests it.";
      log_msg(msg);
      return false;
    }

    flags.lm75a = 1;

    board_temp = doc["lm75a_now"];
    board_temp_min = doc["lm75a_min"];
    board_temp_max = doc["lm75a_max"];
  }

  // update cells
  if(config.api_cellvolts)
  {
    if(doc["cell_monitor"] == 0)
    {
      msg = "API Server does not have cell monitoring enabled yet config requests it.";
      log_msg(msg);
      return false;
    }

    flags.cells_checked = 1;

    config.cell_count = doc["cell_count"];

    for(uint8_t i = 0; i < config.cell_count; i++)
    {
      cells_volts[i] = doc["cell_"+String(i+1)];
    }

    cell_volt_diff = doc["cell_diff"];

    pack_total_volts = doc["cell_total"];

    adc_poll_time = doc["adc_poll_time"];

    low_cell = doc["cell_low"];
    high_cell = doc["cell_high"];

  }

  // update grid info
  if(config.api_grid)
  {
    phase_a_watts = doc["phase_a_watts"];
    phase_b_watts = doc["phase_b_watts"];
    phase_c_watts = doc["phase_c_watts"];

    phase_a_voltage = doc["phase_a_voltage"];
    phase_b_voltage = doc["phase_b_voltage"];
    phase_c_voltage = doc["phase_c_voltage"];

    phase_a_voltage_low = doc["phase_a_voltage_low"];
    phase_b_voltage_low = doc["phase_b_voltage_low"];
    phase_c_voltage_low = doc["phase_c_voltage_low"];

    phase_a_voltage_high = doc["phase_a_voltage_high"];
    phase_b_voltage_high = doc["phase_b_voltage_high"];
    phase_c_voltage_high = doc["phase_c_voltage_high"];

    set_power(get_watts(1));
  }

  flags.api_checked = 1;
  timers.api_last_update = millis();
  return true;
}

