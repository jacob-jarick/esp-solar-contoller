// =============================================================================================================================================
// Config Files
// =============================================================================================================================================

void save_config()
{
  Serial.println("Save Conf");

  vars_sanity_check();


  JsonDocument doc;

  doc["fwver"] = FW_VERSION;

  doc["wifi_ssid1"] = config.wifi_ssid1;

  doc["wifi_pass1"] = config.wifi_pass1;
  doc["wifi_ssid2"] = config.wifi_ssid2;
  doc["wifi_pass2"] = config.wifi_pass2;

  doc["hostn"] = config.hostn;

  doc["update_host"] = config.update_host;

  doc["ntp_server"] = config.ntp_server;

  doc["description"] = config.description;

  doc["fronius_push_url"] = config.fronius_push_url;
  doc["3p_direct_url"] = config.threephase_direct_url;

  doc["pub_url"] = config.pub_url;


  for(uint8_t i = 0; i < adsmux.ain_count; i++)
  {
    doc["volt_mod" + String(i+1)] = config.battery_volt_mod[i];
  }

  // API

  for(uint8_t i = 0; i < max_api_vservers; i++)
  {
    String keyname = "api_vserver" + String(i+1);
    doc[keyname] = config.api_vserver_hostname[i];

    keyname = "api_venable" + String(i+1);
    doc[keyname] = config.api_venable[i];
  }

    for(uint8_t i = 0; i < max_api_iservers; i++)
  {
    String keyname = "api_iserver" + String(i+1);
    doc[keyname] = config.api_iserver_hostname[i];

    keyname = "api_ienable" + String(i+1);
    doc[keyname] = config.api_ienable[i];
  }

  doc["api_lm75a"] = config.api_lm75a;
  doc["api_grid"] = config.api_grid;

  // update server count
  config.api_vserver_count = 0;
  for(uint8_t i = 0; i < max_api_vservers; i++)
  {
    if(config.api_venable[i])
      config.api_vserver_count++;
  }



  // END of API

  // PINS
  // note: int8_8t not uint8_t as we use negative numbers for some defaults and pin range isnt higher than 127

  doc["pin_led"]        = (int8_t) config.pin_led;
  doc["pin_charger"]    = (int8_t) config.pin_charger;
  doc["pin_inverter"]   = (int8_t) config.pin_inverter;
  doc["pin_wd"]         = (int8_t) config.pin_wd;
  doc["pin_flash"]      = (int8_t) config.pin_flash;
  doc["pin_sda"]        = (int8_t) config.pin_sda;
  doc["pin_scl"]        = (int8_t) config.pin_scl;

  doc["api_pollsecs"] = config.api_pollsecs;


  // END PINS

  // volts

  doc["pack_volt_min"] = config.pack_volt_min;
  doc["battery_volt_min"] = config.battery_volt_min;
  doc["battery_volt_rec"] = config.battery_volt_rec;
  doc["battery_volt_max"] = config.battery_volt_max;

  doc["cpkwh"] = config.cpkwh;


  // i2c dev char addresses

  doc["oled_addr"] = (uint8_t) config.oled_addr;

  // bools
  doc["prefer_dc"] = (int)config.prefer_dc;

  doc["rotate_oled"] = (int)config.rotate_oled;
  doc["mcptype"] = (int)config.mcptype;
  doc["ads1x15type"] = (int)config.ads1x15type;
  doc["muxtype"] = (int)config.muxtype;


  doc["display_mode"] = (int)config.display_mode;



  doc["dumbsystem"] = (int)config.dumbsystem;

  doc["blink_led"] = (int)config.blink_led;
  doc["blink_led_default"] = (int)config.blink_led_default;
  doc["avg_phase"] = (int)config.avg_phase;
  doc["threephase"] = (int)config.threephase;
  doc["monitor_phase_a"] = (int)config.monitor_phase_a;
  doc["monitor_phase_b"] = (int)config.monitor_phase_b;
  doc["monitor_phase_c"] = (int)config.monitor_phase_c;
  doc["flip_ipin"] = (int)config.flip_ipin;
  doc["flip_cpin"] = (int)config.flip_cpin;
  doc["auto_update"] = (int)config.auto_update;
  doc["serial_off"] = (int)config.serial_off;
  doc["inv_idle_mode"] = (int)config.inv_idle_mode;

  doc["i_enable"] = (int)config.i_enable;
  doc["c_enable"] = (int)config.c_enable;
  doc["day_is_timer"] = (int)config.day_is_timer;

  doc["night_is_timer"] = (int)config.night_is_timer;
  doc["monitor_battery"] = (int)config.monitor_battery;

  doc["hv_monitor"] = (int)config.hv_monitor;


  doc["avg_ain"] = (int)config.avg_ain;


  doc["day_watts"] = config.day_watts;
  doc["night_watts"] = config.night_watts;
  doc["c_start_h"] = config.c_start_h;
  doc["c_finish_h"] = config.c_finish_h;
  doc["i_start_h"] = config.i_start_h;
  doc["i_finish_h"] = config.i_finish_h;

  // sortme

  doc["lv_shutdown_delay"] = config.lv_shutdown_delay;
  doc["hv_shutdown_delay"] = config.hv_shutdown_delay;

  doc["charger_oot_min"] = config.charger_oot_min;
  doc["charger_oot_sec"] = config.charger_oot_sec;
  doc["inverter_oot_min"] = config.inverter_oot_min;
  doc["inverter_oot_sec"] = config.inverter_oot_sec;


  doc["cell_count"] = config.cell_count;
  doc["gmt"] = config.gmt;
  doc["dcvoltage_offset"] = config.dcvoltage_offset;
  doc["board_rev"] = config.board_rev;
  doc["i2cmaxspeed"] = config.i2cmaxspeed;



  doc["webc_mode"] = (int)config.webc_mode;
  doc["led_status"] = (int)config.led_status;

  doc["m247"] = (int)config.m247;

  // charger off delay
  doc["charger_off_min"] = config.charger_off_min;
  doc["charger_off_sec"] = config.charger_off_sec;
  doc["c_offd"] = (int)config.c_offd;

  doc["c_amot"] = (int)config.c_amot;

  doc["maxsystemtemp"] = config.maxsystemtemp;


  if(SD.exists(json_config_file.c_str() ) )
    SD.remove(json_config_file.c_str() );

  bool save_error = 0;

  // Open file for writing
  File file = SD.open(json_config_file.c_str(), FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to create" + json_config_file);
    save_error = 1;
  }

  // Serialize JSON to file
  else if (serializeJsonPretty(doc, file) == 0)
  {
    Serial.println("Failed to write to " + json_config_file);
    save_error = 1;
  }

  if(save_error)
  {
    oled_clear();
    oled_print("SAVE ERR!");
    beep_helper(300, 100);
    beep_helper(200, 250);
  }
  // Close the file
  file.close();
}

// json doc value to char array
void docv_to_chara(JsonDocument &doc, String keyname, char arr[], uint8_t alength, String default_value)
{
    // if(doc.containsKey(keyname))
    if(doc[keyname].is<JsonVariant>())
      strlcpy(arr, doc[keyname], alength);
    else
      strlcpy(arr, default_value.c_str(), alength);
}

bool load_config()
{
//   config_set_defaults();

  if (!SD.exists(json_config_file.c_str()))
  {
    both_println("\nconf not found");
    return 0;
  }
  File f = SD.open(json_config_file.c_str());

  if (!f)
  {
    oled_clear();
    oled_println("ERROR"); // error opening config file
    oled_set1X();
    oled_println("config read"); // error opening config file

    Serial.println("load_config '" + json_config_file + "'fopen failed");

    flags.sdcard_read_error = 1;
    return 0;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, f);
  f.close();

  if (error)
  {
    Serial.println(F("Failed to read file, using default configuration"));

    return 0;
  }

  config.fwver = doc["fwver"];

  // API
  for(uint8_t i = 0; i < max_api_vservers; i++)
  {
    String keyname = "api_vserver" + String(i+1);

    docv_to_chara(doc, keyname, config.api_vserver_hostname[i], sizeof(config.api_vserver_hostname[i]), "");

    keyname = "api_venable" + String(i+1);
    config.api_venable[i] = doc[keyname];
  }

  for(uint8_t i = 0; i < max_api_iservers; i++)
  {
    String keyname = "api_iserver" + String(i+1);
    docv_to_chara(doc, keyname, config.api_iserver_hostname[i], sizeof(config.api_iserver_hostname[i]), "");

    keyname = "api_ienable" + String(i+1);
    config.api_ienable[i] = doc[keyname];
  }

  config.api_lm75a = doc["api_lm75a"];
  config.api_grid = doc["api_grid"];

  config.api_pollsecs = doc["api_pollsecs"];


  // END of API


  // NETWORK

  docv_to_chara(doc, "wifi_ssid1", config.wifi_ssid1, sizeof(config.wifi_ssid1), "");
  docv_to_chara(doc, "wifi_pass1", config.wifi_pass1, sizeof(config.wifi_pass1), "");
  docv_to_chara(doc, "wifi_ssid2", config.wifi_ssid2, sizeof(config.wifi_ssid2), "");
  docv_to_chara(doc, "wifi_pass2", config.wifi_pass2, sizeof(config.wifi_pass2), "");
  docv_to_chara(doc, "hostn", config.hostn, sizeof(config.hostn), "");
  docv_to_chara(doc, "description", config.description, sizeof(config.description), "");

  // URLS
  docv_to_chara(doc, "3p_direct_url", config.threephase_direct_url, sizeof(config.threephase_direct_url), "");

  docv_to_chara(doc, "fronius_push_url", config.fronius_push_url, sizeof(config.fronius_push_url), "");


  docv_to_chara(doc, "pub_url", config.pub_url, sizeof(config.pub_url), "");
  docv_to_chara(doc, "ntp_server", config.ntp_server, sizeof(config.ntp_server), "0.au.pool.ntp.org");
  docv_to_chara(doc, "update_host", config.update_host, sizeof(config.update_host), "");

  for(uint8_t i = 0; i < adsmux.ain_count; i++)
    config.battery_volt_mod[i] = doc["volt_mod" + String(i+1)];

  // PINS
  config.pin_led = doc["pin_led"];
  config.pin_charger = doc["pin_charger"];
  config.pin_inverter = doc["pin_inverter"];
  config.pin_wd = doc["pin_wd"];
  config.pin_flash = doc["pin_flash"];
  config.pin_sda = doc["pin_sda"];
  config.pin_scl = doc["pin_scl"];

  // cell voltage limits
  config.pack_volt_min = doc["pack_volt_min"];
  config.battery_volt_min = doc["battery_volt_min"];
  config.battery_volt_rec = doc["battery_volt_rec"];
  config.battery_volt_max = doc["battery_volt_max"];

  // VMON Calibration

  config.monitor_battery = doc["monitor_battery"];
  config.cell_count = doc["cell_count"];
  config.dcvoltage_offset = doc["dcvoltage_offset"];


  // i2c dev addresses

  config.oled_addr = (int8_t) doc["oled_addr"];

  // Fronius

  config.avg_phase = doc["avg_phase"];

  // fronius 3phase options
  config.threephase = doc["threephase"];
  config.monitor_phase_a = doc["monitor_phase_a"];
  config.monitor_phase_b = doc["monitor_phase_b"];
  config.monitor_phase_c = doc["monitor_phase_c"];

  config.cpkwh = doc["cpkwh"]; // cost per kwh

  // bools

  config.prefer_dc = doc["prefer_dc"];

  config.rotate_oled = doc["rotate_oled"];
  config.mcptype = doc["mcptype"];
  config.ads1x15type = doc["ads1x15type"];
  config.muxtype = doc["muxtype"];

  config.display_mode = doc["display_mode"];

  config.hv_monitor = doc["hv_monitor"];


  config.flip_ipin = doc["flip_ipin"];
  config.flip_cpin = doc["flip_cpin"];
  config.auto_update = doc["auto_update"];
  config.serial_off = doc["serial_off"];

  config.inv_idle_mode = doc["inv_idle_mode"];
  config.i_enable = doc["i_enable"];
  config.c_enable = doc["c_enable"];
  config.day_is_timer = doc["day_is_timer"];
  config.night_is_timer = doc["night_is_timer"];

  config.avg_ain = doc["avg_ain"];


  // ?

  // Day Device
  config.day_watts = doc["day_watts"];

  config.night_watts = doc["night_watts"];
  config.c_start_h = doc["c_start_h"];
  config.c_finish_h = doc["c_finish_h"];
  config.i_start_h = doc["i_start_h"];
  config.i_finish_h = doc["i_finish_h"];

  // sortme

  config.lv_shutdown_delay = doc["lv_shutdown_delay"];
  config.hv_shutdown_delay = doc["hv_shutdown_delay"];
  config.charger_oot_min = doc["charger_oot_min"];
  config.charger_oot_sec = doc["charger_oot_sec"];

  config.inverter_oot_min = doc["inverter_oot_min"];
  config.inverter_oot_sec = doc["inverter_oot_sec"];

  config.gmt = doc["gmt"];


  config.board_rev = doc["board_rev"];

  config.i2cmaxspeed = doc["i2cmaxspeed"];


  // FLAGS

  config.webc_mode = doc["webc_mode"];

  config.m247 = doc["m247"];


  config.dumbsystem = doc["dumbsystem"];

  // LED
  config.led_status = doc["led_status"];
  config.blink_led = doc["blink_led"];


  config.blink_led_default = doc["blink_led_default"];

  // charger off delay

  config.charger_off_min = doc["charger_off_min"];
  config.charger_off_sec = doc["charger_off_sec"];

  config.c_offd = doc["c_offd"];

  config.c_amot = doc["c_amot"];

  config.maxsystemtemp = doc["maxsystemtemp"];



  vars_sanity_check();

  return 1;
}

void save_passwd()
{

//     Serial.println(F("Save passwd"));
  File f = SD.open(txt_passwd_file, FILE_WRITE);
  if (!f)
  {

//       Serial.println("save_passwd '" + txt_passwd_file + "'fopen failed");

    return;
  }

  f.println(passwd);

  f.close();
}

void load_passwd()
{
  if (!SD.exists(txt_passwd_file))
  {

//       Serial.println(F("no password set"));
    return;
  }
  //  Serial.println(F("read config"));
  //Read File data
  File f = SD.open(txt_passwd_file);

  if (!f)
  {

//       Serial.println("load_passwd '" + txt_passwd_file + "'fopen failed");
    return;
  }

  passwd = f.readStringUntil('\n'); passwd.trim();
  f.close();
}



void vars_sanity_check()
{
  config.webc_mode = 1;

  set_led(config.led_status);

  if(config.maxsystemtemp <= 0)
    config.maxsystemtemp = 40;

  if(!config.threephase)
  {
    config.monitor_phase_a = 1;
    config.monitor_phase_b = 0;
    config.monitor_phase_c = 0;
  }

  // API

  // check if server1 hostname is blank

  for(uint8_t i = 0; i < max_api_vservers; i++)
  {
    String tmp = config.api_vserver_hostname[i];

    if(config.api_venable[i] && tmp.length() == 0)
    {
      // disable this server and all following
      for(uint8_t x = i; x < max_api_vservers; x++)
      {
        config.api_venable[x] = false;
      }

    }
  }

  for(uint8_t i = 0; i < max_api_iservers; i++)
  {
    String tmp = config.api_iserver_hostname[i];

    if(config.api_ienable[i] && tmp.length() == 0)
    {
      // disable this server and all following
      for(uint8_t x = i; x < max_api_iservers; x++)
      {
        config.api_ienable[x] = false;
      }

    }
  }
  // count servers, if server N is disabled, disabled all servers after N
  config.api_vserver_count = 0;
  for(uint8_t i = 0; i < max_api_vservers; i++)
  {
    bool exitloop = false;
    if(config.api_venable[i])
    {
      config.api_vserver_count++;
    }
    else
    {
      // disable this server and all following
      for(uint8_t x = i; x < max_api_vservers; x++)
      {
        config.api_venable[x] = false;
      }
      exitloop = true;
      break;
    }
    if(exitloop)
      break;
  }


  // if server 1 is disabled, disable extra options
  if(!config.api_venable[0])
  {
    config.api_lm75a = 0;
    config.api_grid = 0;
  }


  // limit poll time
  if(config.api_pollsecs < 1)
    config.api_pollsecs = 1;
  if(config.api_pollsecs > 900)
    config.api_pollsecs = 60;

  // END API

  if(config.hv_shutdown_delay < 0)
  {
    config.hv_shutdown_delay = 1;
    log_msg("config fix: hv_shutdown_delay");
    flags.save_config = 1;
  }

  if(config.lv_shutdown_delay < 0)
  {
    config.lv_shutdown_delay = 1;
    log_msg("config fix: lv_shutdown_delay");
    flags.save_config = 1;
  }

  //config.cell_count = constrain(config.cell_count, 1, adsmux.ain_count);

  if(config.cell_count == 1 && config.pack_volt_min != config.battery_volt_min)
  {
    config.battery_volt_min = config.pack_volt_min = config.battery_volt_min;
    log_msg("config fix: pack_volt_min != battery_volt_min");
    flags.save_config = 1;
  }


  if(config.battery_volt_min <= 0)
  {
    config.battery_volt_min = 3;
    log_msg("config fix: battery_volt_min <= 0");
    flags.save_config = 1;
  }

  if(config.battery_volt_max <= 0)
  {
    config.battery_volt_max = 4.2;
    log_msg("config fix: battery_volt_max <= 0");
    flags.save_config = 1;
  }
  if(config.battery_volt_rec >= config.battery_volt_max)
  {
    config.battery_volt_rec = config.battery_volt_min + mmaths.mdiff<float>(config.battery_volt_min, config.battery_volt_max) / 2;
    log_msg("config fix: battery_volt_rec >= battery_volt_max");
    flags.save_config = 1;
  }

  if(config.avg_phase > power_array_size)
  {
    config.avg_phase = power_array_size;
    log_msg("config fix: avg_phase > power_array_size");
    flags.save_config = 1;
  }


  // -------------------------------------
  // enable ADC channels for monitoring.

  // disable all
  for(byte i = 0; i < adsmux.ain_count; i++)
    adsmux.adc_enable[i] = 0;

  // enable battery channels
  if(config.monitor_battery)
  {
    uint8_t local_cell_count = mmaths.mmin<uint8_t>(config.cell_count, (uint8_t) MAX_CELLS);

    for(byte i = 0; i < local_cell_count; i++)
      adsmux.adc_enable[i] = 1;
  }

  // -------------------------------------

  for(uint8_t i = 0; i < adsmux.ain_count; i++)
  {
    if(config.battery_volt_mod[i] <= 0)
      config.battery_volt_mod[i] = 1;
  }

  if(config.board_rev > 2)
  {
    config.board_rev = 1;
    log_msg("config fix: board_rev > 2");
    flags.save_config = 1;
  }

  // board revisions
  if(config.board_rev == 1) // default
  {
    config.pin_sda = OPT_DEFAULT;
    config.pin_scl = OPT_DEFAULT;
    config.pin_led = 2;
    config.pin_inverter = 25;
    config.pin_charger = 33;
  }
  else if(config.board_rev == 2) // mini (i stupidly reversed the pins)
  {
    config.pin_sda = 21;
    config.pin_scl = 22;
    config.pin_led = 2;
    config.pin_inverter = 25;
    config.pin_charger = 33;
  }


  if(config.pin_inverter == OPT_DISABLE)
    config.i_enable = 0;
  if(config.pin_charger == OPT_DISABLE)
    config.c_enable = 0;

  adsmux.avg_ain = config.avg_ain;
}

void copy_config(const String target_ip)
{
  char h[32];
  strlcpy(h, config.hostn, sizeof(config.hostn));

  byte old_board_rev = config.board_rev;
  get_config_and_save(target_ip);
  load_config();

  strlcpy(config.hostn, h, sizeof(config.hostn));

  config.board_rev = old_board_rev;
  save_config();
}

void get_config_and_save(const String target_ip)
{
  oled_clear();
  oled_println(F("Copying HTML"));
  oled_print(F("from "));
  oled_println(target_ip);


  String url = http_str + target_ip + "/config_raw";
  String file_dest = json_config_file;

  bool result = get_url_and_save(url, file_dest);

  if (result)
  {
    oled_clear();
    oled_println(F("Copy OK"));

  }
  else
  {
    HTTPClient http; // used to decode error number to string.

    both_println(F("HTTP code:")); // there may be a file error as well. so dont declare this as an error
    both_println(http.errorToString(get_url_code).c_str());

    log_msg("ERROR: get_config_and_save: " + String(http.errorToString(get_url_code).c_str()));
  }
}
