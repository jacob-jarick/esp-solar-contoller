// =============================================================================================================================================
// Config Files
// =============================================================================================================================================

// #define config_json_size 8192

const size_t config_json_size = 1024 * 5;

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
    doc["mt" + String(i+1)] = config.ntc_temp_max[i];
  }

  for(uint8_t i = 0; i < MAX_CELLS; i++)
  {
    doc["volt_mod" + String(i+1)] = config.battery_volt_mod[i];
  }

  doc["otsdh"] = config.otsdh;


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

  doc["cpkwh"] = config.cpkwh;


  // bytes

  doc["ntc10k_count"] = config.ntc10k_count;

  // ints


  // i2c dev char addresses

  doc["oled_addr"] = config.oled_addr;

  // bools
  doc["prefer_dc"] = (int)config.prefer_dc;

  doc["monitor_temp"] = (int)config.monitor_temp;
  doc["rotate_oled"] = (int)config.rotate_oled;
  doc["display_mode"] = (int)config.display_mode;

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
  doc["inv_idle_mode"] = (int)config.inv_idle_mode;

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

  // charger off delay
  doc["charger_off_min"] = config.charger_off_min;
  doc["charger_off_sec"] = config.charger_off_sec;
  doc["c_offd"] = (int)config.c_offd;

  doc["c_amot"] = (int)config.c_amot;

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

  DynamicJsonDocument doc(config_json_size);
  DeserializationError error = deserializeJson(doc, f);
  f.close();

  if (error)
  {
    Serial.println(F("Failed to read file, using default configuration"));

    return 0;
  }

  config.fwver = doc["fwver"];

  // NETWORK
  strlcpy(config.wifi_ssid1, doc["wifi_ssid1"], sizeof(config.wifi_ssid1));
  strlcpy(config.wifi_pass1, doc["wifi_pass1"], sizeof(config.wifi_pass1));
  strlcpy(config.wifi_ssid2, doc["wifi_ssid2"], sizeof(config.wifi_ssid2));
  strlcpy(config.wifi_pass2, doc["wifi_pass2"], sizeof(config.wifi_pass2));

  strlcpy(config.hostn, doc["hostn"], sizeof(config.hostn));
  strlcpy(config.description, doc["description"], sizeof(config.description));


  // URLS
  strlcpy(config.inverter_url, doc["inverter_url"], sizeof(config.inverter_url));
  strlcpy(config.inverter_push_url, doc["inverter_push_url"], sizeof(config.inverter_push_url));
  strlcpy(config.threephase_push_url, doc["3p_push_url"], sizeof(config.threephase_push_url));
  strlcpy(config.threephase_direct_url, doc["3p_direct_url"], sizeof(config.threephase_direct_url));

  strlcpy(config.pub_url, doc["pub_url"], sizeof(config.pub_url));

  // HOSTS
  strlcpy(config.ntp_server, doc["ntp_server"], sizeof(config.ntp_server));
  strlcpy(config.update_host, doc["update_host"], sizeof(config.update_host));

  // NTC
  config.monitor_temp = doc["monitor_temp"];
  config.ntc10k_count = doc["ntc10k_count"];

  for(uint8_t i = 0; i < count_ntc; i++)
  {
    config.ntc_temp_max[i] = doc["mt" + String(i+1)]; // mt = max temp

    if(!config.ntc_temp_max[i])
      config.ntc_temp_max[i] = 50;
  }

  for(uint8_t i = 0; i < MAX_CELLS; i++)
    config.battery_volt_mod[i] = doc["volt_mod" + String(i+1)];

  config.otsdh = doc["otsdh"]; // overtemp shutdown hours

  // PINS
  config.pin_led = doc["pin_led"];
  config.pin_charger = doc["pin_charger"];
  config.pin_inverter = doc["pin_inverter"];
  config.pin_wd = doc["pin_wd"];
  config.pin_flash = doc["pin_flash"];
  config.pin_sda = doc["pin_sda"];
  config.pin_scl = doc["pin_scl"];
  config.pin_buzzer = doc["pin_buzzer"];

  // cell voltage limits
  config.pack_volt_min = doc["pack_volt_min"];
  config.battery_volt_min = doc["battery_volt_min"];
  config.battery_volt_rec = doc["battery_volt_rec"];
  config.battery_volt_max = doc["battery_volt_max"];

  // VMON Calibration

  config.cells_in_series = doc["cells_in_series"];
  config.monitor_battery = doc["monitor_battery"];
  config.cell_count = doc["cell_count"];
  config.dcvoltage_offset = doc["dcvoltage_offset"];


  // i2c dev addresses

  config.oled_addr = doc["oled_addr"];

  // Fronius

  config.avg_phase = doc["avg_phase"];

  // fronius 3phase options
  config.threephase = doc["threephase"];
  config.monitor_phase_a = doc["monitor_phase_a"];
  config.monitor_phase_b = doc["monitor_phase_b"];
  config.monitor_phase_c = doc["monitor_phase_c"];

  config.cpkwh = doc["cpkwh"];

  // bools

  config.prefer_dc = doc["prefer_dc"];

  config.rotate_oled = doc["rotate_oled"];
  config.display_mode = doc["display_mode"];

  config.hv_monitor = doc["hv_monitor"];


  config.flip_ipin = doc["flip_ipin"];
  config.flip_cpin = doc["flip_cpin"];
  config.auto_update = doc["auto_update"];
  config.inv_idle_mode = doc["inv_idle_mode"];
  config.i_enable = doc["i_enable"];
  config.c_enable = doc["c_enable"];
  config.day_is_timer = doc["day_is_timer"];
  config.night_is_timer = doc["night_is_timer"];

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

  // FLAGS

  config.webc_mode = doc["webc_mode"];

  config.m247 = doc["m247"];

  // LED
  config.led_status = doc["led_status"];
  config.blink_led = doc["blink_led"];
  config.blink_led_default = doc["blink_led_default"];

  // charger off delay

  config.charger_off_min = doc["charger_off_min"];
  config.charger_off_sec = doc["charger_off_sec"];

  config.c_offd = doc["c_offd"];

  config.c_amot = doc["c_amot"];

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

  if(config.otsdh < 0.1)
  {
    config.otsdh = 0.1;
    log_msg("config fix: otsdh");
    flags.save_config = 1;
  }

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

  config.cell_count = constrain(config.cell_count, 1, MAX_CELLS);

  if(config.cell_count == 1 && config.pack_volt_min != config.battery_volt_min)
  {
    config.battery_volt_min = config.pack_volt_min = mmaths.mmax(config.battery_volt_min, config.pack_volt_min);
    log_msg("config fix: pack_volt_min != battery_volt_min");
    flags.save_config = 1;
  }

  if(!config.battery_volt_min)
  {
    config.battery_volt_min = 3;
    log_msg("config fix: !battery_volt_min");
    flags.save_config = 1;
  }

  if(!config.battery_volt_rec || config.battery_volt_rec <= config.battery_volt_min)
  {
    config.battery_volt_rec = config.battery_volt_min * 1.05;
    log_msg("config fix: battery_volt_rec");
    flags.save_config = 1;
  }

  if(!config.battery_volt_max || config.battery_volt_max <= config.battery_volt_rec)
  {
    config.battery_volt_max = 4.2;
    log_msg("config fix: battery_volt_max");
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

  for(uint8_t i = 0; i < MAX_CELLS; i++)
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

    log_msg("ERROR: get_config_and_save: " + String(http.errorToString(get_url_code).c_str()));
  }
}
