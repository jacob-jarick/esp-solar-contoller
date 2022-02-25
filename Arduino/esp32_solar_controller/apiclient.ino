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

uint8_t new_cell_count = 0; // hacky way of auto detecting cells monitored

uint8_t poller_pos = 0;
bool api_poller()
{
  bool api_result = false;

  api_result = api_vsync(poller_pos);

  // increment position if successful.
  if(api_result)
  {
    poller_pos++;
    if(poller_pos > config.api_vserver_count-1)
    {
      flags.vapi_checked = 1;
      poller_pos = 0;
      api_docalcs();
    }
  }

  return api_result;
}

void api_docalcs()
{
  config.cell_count = new_cell_count;

  for(uint8_t i = 0; i < MAX_CELLS * 3; i++)
  {
    cells_volts[i] = cells_volts_real[i];
  }


  // calculate new total pack voltage
  check_cells();
}

bool api_isync(uint8_t serverid)
{
  String shn = config.api_iserver_hostname[serverid];
  String msg_prefix = "API Info Server ID " + String(serverid) + ", ";

  String msg = "";
  String hostip = "";

  if(!mdnscachelookup(shn, hostip))
  {
    log_msg(msg_prefix + "MDNS lookup '" + shn + "' error.");
    return false;
  }

  String url = "http://" + hostip + "/jsonapi";

  String payload = "";
  bool check = get_url(url, payload);

  if(!check)
  {
    msg = msg_prefix + "fetch error, URL: " + url;
    log_msg(msg);

    return false;
  }

  DynamicJsonDocument doc(jsonsize);

  DeserializationError error2 = deserializeJson(doc, payload.c_str());

  if (error2)
  {
    log_msg(String(msg_prefix + "JSON Decode Error: ") + error2.c_str() );

    return false;
  }

  // JSON is ready

  // compare api_vserver1 to json hostname.


  if(doc.containsKey("host_name"))
  {
    String rhn = doc["host_name"];
    if(shn != rhn)
    {
      log_msg(msg_prefix + "ERROR, hostname mismatch JSON: '" + rhn + "' Config Hostname '" + shn + "'");
      mdns_hn_cache = ""; // invalidate cache

      return false;
    }
  }
  else
  {
    log_msg(msg_prefix + "ERROR: hostname not present in JSON" );
    Serial.println("XX");

    return false;
  }


  // get amb temp

  //config.api_lm75a
  if(config.api_lm75a)
  {
    if(doc["lm75a"] == 0)
    {
      msg = msg_prefix + "Server lm75a not found, yet config requests it.";
      log_msg(msg);
      return false;
    }

    flags.lm75a = 1;

    board_temp = doc["lm75a_now"];
    board_temp_min = doc["lm75a_min"];
    board_temp_max = doc["lm75a_max"];
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

  return true;
}



bool api_vsync(uint8_t serverid)
{
  String shn = config.api_vserver_hostname[serverid];
  String msg_prefix = "API Server ID " + String(serverid) + ", ";

//   String msg = "";
  String hostip = "";

  if(!mdnscachelookup(shn, hostip))
  {
    log_msg(msg_prefix + "MDNS lookup '" + shn + "' error.");
    return false;
  }

  String url = "http://" + hostip + "/json_cells";

  String payload = "";
  bool check = get_url(url, payload);

  if(!check)
  {
//     msg = ;
    log_msg(msg_prefix + "fetch error, URL: " + url);

    return false;
  }

  DynamicJsonDocument doc(jsonsize);

  DeserializationError error2 = deserializeJson(doc, payload.c_str());

  if (error2)
  {
    log_msg(String(msg_prefix + "JSON Decode Error: ") + error2.c_str() );

    return false;
  }

  // JSON is ready

  // compare api_vserver1 to json hostname.


  if(doc.containsKey("host_name"))
  {
    String rhn = doc["host_name"];
    if(shn != rhn)
    {
      log_msg(msg_prefix + "ERROR, hostname mismatch JSON: '" + rhn + "'");
      mdns_hn_cache = ""; // invalidate cache

      return false;
    }
  }
  else
  {
    log_msg(msg_prefix + "ERROR: hostname not present in JSON" );
    return false;
  }

  /// check if cmon has an adc
  if(doc.containsKey("adc_found"))
  {
    if(!doc["adc_found"])
    {
      log_msg(msg_prefix + "ERROR: host adc_found == 0");
      return false;
    }
  }

  // update cells

  {
    if(doc["cell_monitor"] == 0)
    {
//       msg = ;
      log_msg(msg_prefix + "Server does not have cell monitoring enabled yet config requests it.");
      return false;
    }


    // track total number of cells in json(s)

    uint8_t doc_cell_count = uint8_t(doc["cell_count"]);

    if(serverid == 0)
    {
      new_cell_count = doc_cell_count;
    }
    else
    {
      new_cell_count += doc_cell_count;
    }

    uint8_t i_offset = new_cell_count - doc_cell_count;

    for(uint8_t i = 0; i < config.cell_count; i++)
    {
      cells_volts_real[i_offset+i] = doc["cell_"+String(i+1)];
    }

    adc_poll_time = doc["adc_poll_time"];
  }

  timers.api_last_update = millis();
  return true;
}

