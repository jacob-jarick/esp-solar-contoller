// =============================================================================================================================================
// Config Files
// =============================================================================================================================================

#define config_json_size 8192

void save_config()
{
  Serial.println("Save Conf");

  vars_sanity_check();


  DynamicJsonDocument doc(config_json_size);

  doc["fwver"] = FW_VERSION;

  doc["wifi_ssid1"] = config.wifi_ssid1;

  doc["wifi_pass1"] = config.wifi_pass1;
  doc["wifi_ssid2"] = config.wifi_ssid2;
  doc["wifi_pass2"] = config.wifi_pass2;

  doc["hostn"] = config.hostn;

  doc["update_host"] = config.update_host;

  doc["ntp_server"] = config.ntp_server;

  doc["description"] = config.description;



  doc["inverter_url"] = config.inverter_url;
  doc["inverter_push_url"] = config.inverter_push_url;
  doc["3p_push_url"] = config.threephase_push_url;
  doc["3p_direct_url"] = config.threephase_direct_url;

  doc["pub_url"] = config.pub_url;

//   doc[""] = config.;

  for(uint8_t i = 0; i < count_ntc; i++)
  {
//     doc["temp_mod" + String(i+1)] = config.ntc_temp_mods[i];
    doc["temp_max" + String(i+1)] = config.ntc_temp_max[i];
  }

  for(uint8_t i = 0; i < count_cells; i++)
  {
    doc["volt_mod" + String(i+1)] = config.battery_volt_mod[i];
  }

  // PINS

  doc["pin_led"] = config.pin_led;
  doc["pin_charger"] = config.pin_charger;
  doc["pin_inverter"] = config.pin_inverter;
  doc["pin_wd"] = config.pin_wd;
  doc["pin_flash"] = config.pin_flash;
  doc["pin_sda"] = config.pin_sda;
  doc["pin_scl"] = config.pin_scl;
  doc["pin_buzzer"] = config.pin_buzzer;


  // END PINS

  // volts

  doc["pack_volt_min"] = config.pack_volt_min;
  doc["battery_volt_min"] = config.battery_volt_min;
  doc["battery_volt_rec"] = config.battery_volt_rec;
  doc["battery_volt_max"] = config.battery_volt_max;

  // bytes

  doc["ntc10k_count"] = config.ntc10k_count;

  // ints


  // i2c dev char addresses

  doc["oled_addr"] = config.oled_addr;

  // bools

  doc["monitor_temp"] = (int)config.monitor_temp;
  doc["rotate_oled"] = (int)config.rotate_oled;
  doc["button_timer_mode"] = (int)config.button_timer_mode;
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
  doc["i_enable"] = (int)config.i_enable;
  doc["c_enable"] = (int)config.c_enable;
  doc["day_is_timer"] = (int)config.day_is_timer;

  doc["night_is_timer"] = (int)config.night_is_timer;
  doc["cells_in_series"] = (int)config.cells_in_series;
  doc["monitor_battery"] = (int)config.monitor_battery;

  doc["hv_monitor"] = (int)config.hv_monitor;



  // ?

  doc["day_watts"] = config.day_watts;
  doc["night_watts"] = config.night_watts;
  doc["button_timer_secs"] = config.button_timer_secs;
  doc["button_timer_max"] = config.button_timer_max;
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


  doc["webc_mode"] = (int)config.webc_mode;
  doc["led_status"] = (int)config.led_status;

  doc["m247"] = (int)config.m247;
//   m247




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
  else if (serializeJson(doc, file) == 0)
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

bool load_config()
{
  if (!SD.exists(json_config_file.c_str()))
  {
    both_println(F("conf not found"));
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

  DynamicJsonDocument doc(config_json_size);
  DeserializationError error = deserializeJson(doc, f);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  f.close();

  config.fwver = doc["fwver"] | 0;

  // NETWORK
  strlcpy(config.wifi_ssid1, doc["wifi_ssid1"] | "", sizeof(config.wifi_ssid1));
  strlcpy(config.wifi_pass1, doc["wifi_pass1"] | "", sizeof(config.wifi_pass1));
  strlcpy(config.wifi_ssid2, doc["wifi_ssid2"] | "", sizeof(config.wifi_ssid2));
  strlcpy(config.wifi_pass2, doc["wifi_pass2"] | "", sizeof(config.wifi_pass2));

  strlcpy(config.hostn, doc["hostn"] | "esp32sc", sizeof(config.hostn));
  strlcpy(config.description, doc["description"] | "new", sizeof(config.description));


  // URLS
  strlcpy(config.inverter_url, doc["inverter_url"], sizeof(config.inverter_url));
  strlcpy(config.inverter_push_url, doc["inverter_push_url"] | "", sizeof(config.inverter_push_url));
  strlcpy(config.threephase_push_url, doc["3p_push_url"] | "", sizeof(config.threephase_push_url));
  strlcpy(config.threephase_direct_url, doc["3p_direct_url"] | "", sizeof(config.threephase_direct_url));

  strlcpy(config.pub_url, doc["pub_url"], sizeof(config.pub_url));

  // HOSTS
  strlcpy(config.ntp_server, doc["ntp_server"] | "0.au.pool.ntp.org", sizeof(config.ntp_server));
  strlcpy(config.update_host, doc["update_host"] | "", sizeof(config.update_host));

  // NTC
  config.monitor_temp = doc["monitor_temp"] | 0;
  config.ntc10k_count = doc["ntc10k_count"] | 0;

  for(uint8_t i = 0; i < count_ntc; i++)
  {
//     config.ntc_temp_mods[i] = doc["temp_mod" + String(i+1)] | 1;
    config.ntc_temp_max[i] = doc["temp_max" + String(i+1)] | 50;
  }

  for(int i = 0; i < count_cells; i++)
    config.battery_volt_mod[i] = doc["volt_mod" + String(i+1)];

  // PINS
  config.pin_led = doc["pin_led"] | OPT_DEFAULT;
  config.pin_charger = doc["pin_charger"] | OPT_DEFAULT;
  config.pin_inverter = doc["pin_inverter"] | OPT_DEFAULT;
  config.pin_wd = doc["pin_wd"] | OPT_DEFAULT;
  config.pin_flash = doc["pin_flash"] | OPT_DEFAULT;
  config.pin_sda = doc["pin_sda"] | OPT_DEFAULT;
  config.pin_scl = doc["pin_scl"] | OPT_DEFAULT;
  config.pin_buzzer = doc["pin_buzzer"] | OPT_DEFAULT;

  // cell voltage limits
  config.pack_volt_min = doc["pack_volt_min"] | 0;
  config.battery_volt_min = doc["battery_volt_min"] | 3;
  config.battery_volt_rec = doc["battery_volt_rec"] | 3.2;
  config.battery_volt_max = doc["battery_volt_max"] | 4.2;

  // VMON Calibration

  config.cells_in_series = doc["cells_in_series"] | 1;
  config.monitor_battery = doc["monitor_battery"] | 0;
  config.cell_count = doc["cell_count"] | 0;
  config.dcvoltage_offset = doc["dcvoltage_offset"] | 0;


  // i2c dev addresses

  config.oled_addr = doc["oled_addr"];

  // Fronius

  config.avg_phase = doc["avg_phase"] | 0;

  // fronius 3phase options
  config.threephase = doc["threephase"] | 0;
  config.monitor_phase_a = doc["monitor_phase_a"] | 0;
  config.monitor_phase_b = doc["monitor_phase_b"] | 0;
  config.monitor_phase_c = doc["monitor_phase_c"] | 0;

  // bools


  config.rotate_oled = doc["rotate_oled"] | 0;
  config.button_timer_mode = doc["button_timer_mode"] | 0;

  config.hv_monitor = doc["hv_monitor"] | 0;


  config.flip_ipin = doc["flip_ipin"] | 0;
  config.flip_cpin = doc["flip_cpin"] | 0;
  config.auto_update = doc["auto_update"] | 0;
//   config.wifi_highpower_on = doc["wifi_highpower_on"];
  config.i_enable = doc["i_enable"] | 0;
  config.c_enable = doc["c_enable"] | 0;
  config.day_is_timer = doc["day_is_timer"] | 0;
  config.night_is_timer = doc["night_is_timer"] | 0;

  // ?

  // Day Device
  config.day_watts = doc["day_watts"] | 0;

  config.night_watts = doc["night_watts"] | 0;
  config.button_timer_secs = doc["button_timer_secs"] | 5;
  config.button_timer_max = doc["button_timer_max"] | 360;
  config.c_start_h = doc["c_start_h"] | 8;
  config.c_finish_h = doc["c_finish_h"] | 17;
  config.i_start_h = doc["i_start_h"] | 18;
  config.i_finish_h = doc["i_finish_h"] | 6;

  // sortme

  config.lv_shutdown_delay = doc["lv_shutdown_delay"] | 1;
  config.hv_shutdown_delay = doc["hv_shutdown_delay"] | 1;
  config.charger_oot_min = doc["charger_oot_min"] | 5;
  config.charger_oot_sec = doc["charger_oot_sec"] | 0;

  config.inverter_oot_min = doc["inverter_oot_min"] | 0;
  config.inverter_oot_sec = doc["inverter_oot_sec"] | 30;

  config.gmt = doc["gmt"] | 0;


  config.board_rev = doc["board_rev"] | 1;

  // FLAGS

  config.webc_mode = doc["webc_mode"] | 1;

  config.m247 = doc["m247"] | 0;

  // LED
  config.led_status = doc["led_status"] | 0;
  config.blink_led = doc["blink_led"] | 1;
  config.blink_led_default = doc["blink_led_default"] | 1;

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

  if(config.hv_shutdown_delay < 0)
    config.hv_shutdown_delay = 1;

  if(config.lv_shutdown_delay < 0)
    config.lv_shutdown_delay = 1;

  config.cell_count = constrain(config.cell_count, 1, 16);

  if(config.cell_count == 1 && config.pack_volt_min != config.battery_volt_min)
    config.battery_volt_min = config.pack_volt_min = mmaths.mmax(config.battery_volt_min, config.pack_volt_min);

  // -------------------------------------
  // enable ADC channels for monitoring.

  // disable all
  for(byte i = 0; i < 32; i++)
    adsmux.adc_enable[i] = 0;

  // enable battery channels
  if(config.monitor_battery)
  {
    for(byte i = 0; i < config.cell_count; i++)
      adsmux.adc_enable[i] = 1;
  }

  // enable temp channels
  if(config.monitor_temp)
  {
    for(byte i = 0; i < config.ntc10k_count; i++)
      adsmux.adc_enable[i+16] = 1;
  }
  // -------------------------------------

  for(byte i = 0; i < count_cells; i++)
  {
    if(config.battery_volt_mod[i] <= 0)
      config.battery_volt_mod[i] = 1;
  }

  if(config.board_rev > 2)
    config.board_rev = 1;

  // board revisions
  if(config.board_rev == 1) // default
  {
    config.pin_sda = OPT_DEFAULT;
    config.pin_scl = OPT_DEFAULT;
    config.pin_buzzer = 16;
    config.pin_led = 2;
    config.pin_inverter = 25;
    config.pin_charger = 33;
  }
  else if(config.board_rev == 2) // mini (i stupidly reversed the pins)
  {
    config.pin_sda = 21;
    config.pin_scl = 22;
    config.pin_buzzer = 16;
    config.pin_led = 2;
    config.pin_inverter = 25;
    config.pin_charger = 33;
  }


  if(config.pin_inverter == OPT_DISABLE)
    config.i_enable = 0;
  if(config.pin_charger == OPT_DISABLE)
    config.c_enable = 0;
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

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(httpget_timeout * 2);

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
    both_println(F("HTTP code:")); // there may be a file error as well. so dont declare this as an error
    both_println(http.errorToString(get_url_code).c_str());
  }
}
