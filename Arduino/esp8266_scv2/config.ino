// =============================================================================================================================================
// Config Files
// =============================================================================================================================================

void save_config()
{
  Serial.println("Save Conf");

  vars_sanity_check();


  DynamicJsonDocument doc(2500);

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
  doc["meter_url"] = config.meter_url;
  doc["update_url"] = config.update_url;

//   doc[""] = config.;

  // PINS

  doc["pin_led"] = config.pin_led;
  doc["pin_charger"] = config.pin_charger;
  doc["pin_inverter"] = config.pin_inverter;
  doc["pin_wd"] = config.pin_wd;
  doc["pin_flash"] = config.pin_flash;
  doc["pin_sda"] = config.pin_sda;
  doc["pin_scl"] = config.pin_scl;

  // END PINS

  // floats


  // ints

  // i2c dev char addresses

  doc["oled_addr"] = config.oled_addr;
  doc["pcf857a_addr"] = config.pcf857a_addr;

  // bools

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
  doc["wifi_highpower_on"] = (int)config.wifi_highpower_on;
  doc["i_enable"] = (int)config.i_enable;
  doc["c_enable"] = (int)config.c_enable;
  doc["day_is_timer"] = (int)config.day_is_timer;

  doc["night_is_timer"] = (int)config.night_is_timer;


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

  doc["download_html"] = (int)config.download_html;
  doc["charger_oot_min"] = config.charger_oot_min;
  doc["charger_oot_sec"] = config.charger_oot_sec;
  doc["inverter_oot_min"] = config.inverter_oot_min;
  doc["inverter_oot_sec"] = config.inverter_oot_sec;
  doc["gmt"] = config.gmt;
  doc["board_rev"] = config.board_rev;

  doc["webc_mode"] = (int)config.webc_mode;
  doc["led_status"] = (int)config.led_status;




  if(SPIFFS.exists(json_config_file ) )
    SPIFFS.remove(json_config_file );

  // Open file for writing
  File file = SPIFFS.open(json_config_file, "w");
  if (!file) {
    Serial.println("Failed to create" + json_config_file);
    return;
  }

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write to " + json_config_file);
  }

  // Close the file
  file.close();
}

bool load_config()
{
  if (!SPIFFS.exists(json_config_file))
  {
    both_println(F("conf not found"));
    return 0;
  }
  File f = SPIFFS.open(json_config_file, "r");

  if (!f)
  {
    oled_clear();
    oled_println("ERROR"); // error opening config file
    oled_set1X();
    oled_println("config read"); // error opening config file

    Serial.println("load_config '" + json_config_file + "'fopen failed");


    return 0;
  }

  DynamicJsonDocument doc(2500);
  DeserializationError error = deserializeJson(doc, f);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  f.close();

  // NETWORK
  strlcpy(config.wifi_ssid1, doc["wifi_ssid1"], sizeof(config.wifi_ssid1));
  strlcpy(config.wifi_pass1, doc["wifi_pass1"], sizeof(config.wifi_pass1));
  strlcpy(config.wifi_ssid2, doc["wifi_ssid2"], sizeof(config.wifi_ssid2));
  strlcpy(config.wifi_pass2, doc["wifi_pass2"], sizeof(config.wifi_pass2));

  strlcpy(config.hostn, doc["hostn"] | "esp", sizeof(config.hostn));
  strlcpy(config.description, doc["description"] | "", sizeof(config.description));


  // URLS
  strlcpy(config.inverter_url, doc["inverter_url"] | "http://10.1.1.127/solar_api/v1/GetPowerFlowRealtimeData.fcgi", sizeof(config.inverter_url));
  strlcpy(config.inverter_push_url, doc["inverter_push_url"] | "http://10.1.1.71/inverter/currpowerflow.json", sizeof(config.inverter_push_url));
  strlcpy(config.meter_url, doc["meter_url"] | "http://10.1.1.71/inverter/realtime.json", sizeof(config.meter_url));
  strlcpy(config.update_url, doc["update_url"] | "http://10.1.1.71/esp32", sizeof(config.update_url));

  // HOSTS
  strlcpy(config.ntp_server, doc["ntp_server"] | "0.au.pool.ntp.org", sizeof(config.ntp_server));
  strlcpy(config.update_host, doc["update_host"] | "10.1.1.71", sizeof(config.update_host));

  // PINS
  config.pin_led = doc["pin_led"] | OPT_DEFAULT;
  config.pin_charger = doc["pin_charger"] | OPT_DEFAULT;
  config.pin_inverter = doc["pin_inverter"] | OPT_DEFAULT;
  config.pin_wd = doc["pin_wd"] | OPT_DEFAULT;
  config.pin_flash = doc["pin_flash"] | OPT_DEFAULT;
  config.pin_sda = doc["pin_sda"] | OPT_DEFAULT;
  config.pin_scl = doc["pin_scl"] | OPT_DEFAULT;

  // i2c dev addresses

  config.oled_addr = doc["oled_addr"] | 0x3C;
  config.pcf857a_addr = doc["pcf857a_addr"] | 0;

  // Fronius

  config.avg_phase = doc["avg_phase"] | 1;

  // fronius 3phase options
  config.threephase = doc["threephase"] | 0;
  config.monitor_phase_a = doc["monitor_phase_a"] | 1;
  config.monitor_phase_b = doc["monitor_phase_b"] | 1;
  config.monitor_phase_c = doc["monitor_phase_c"] | 1;

  // bools


  config.rotate_oled = doc["rotate_oled"] | 0;
  config.button_timer_mode = doc["button_timer_mode"] | 0;


  config.flip_ipin = doc["flip_ipin"] | 0;
  config.flip_cpin = doc["flip_cpin"] | 0;
  config.auto_update = doc["auto_update"] | 1;
  config.wifi_highpower_on = doc["wifi_highpower_on"] | 1;
  config.i_enable = doc["i_enable"] | 0;
  config.c_enable = doc["c_enable"] | 0;
  config.day_is_timer = doc["day_is_timer"] | 0;
  config.night_is_timer = doc["night_is_timer"] | 0;

  // ?

  // Day Device
  config.day_watts = doc["day_watts"] | 400;

  config.night_watts = doc["night_watts"] | 600;
  config.button_timer_secs = doc["button_timer_secs"] | 30;
  config.button_timer_max = doc["button_timer_max"] | 30;
  config.c_start_h = doc["c_start_h"] | 8;
  config.c_finish_h = doc["c_finish_h"] | 18;
  config.i_start_h = doc["i_start_h"] | 18;
  config.i_finish_h = doc["i_finish_h"] | 8;

  // sortme

  config.charger_oot_min = doc["charger_oot_min"] | 0;
  config.charger_oot_sec = doc["charger_oot_sec"] | 10;

  config.inverter_oot_min = doc["inverter_oot_min"] | 10;
  config.inverter_oot_sec = doc["inverter_oot_sec"] | 0;

  config.gmt = doc["gmt"] | 40;


  config.board_rev = doc["board_rev"] | 1;

  // FLAGS

  config.download_html = doc["download_html"] | 0;
  config.webc_mode = doc["webc_mode"] | 1;

  // LED
  config.led_status = doc["led_status"] | 1;
  config.blink_led = doc["blink_led"] | 1;
  config.blink_led_default = doc["blink_led_default"] | 1;


  vars_sanity_check();

  return 1;
}

void save_passwd()
{

//     Serial.println(F("Save passwd"));
  File f = SPIFFS.open(txt_passwd_file, "w");
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
  if (!SPIFFS.exists(txt_passwd_file))
  {

//       Serial.println(F("no password set"));
    return;
  }
  //  Serial.println(F("read config"));
  //Read File data
  File f = SPIFFS.open(txt_passwd_file, "r");

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
  set_led(config.led_status);

//     config.board_rev = 2;

  // board revisions
  if(config.board_rev == 1) // default
  {
    config.pin_flash = 0;
    config.pin_sda = OPT_DEFAULT;
    config.pin_scl = OPT_DEFAULT;
    config.pin_led = 2;
    config.pin_inverter = 12;
    config.pin_charger = 13;
  }
  if(config.board_rev == 2) // sonoff
  {
    config.pin_flash = 0;
    config.pin_sda = OPT_DISABLE;
    config.pin_scl = OPT_DISABLE;
    config.pin_led = 13;
    config.pin_inverter = OPT_DISABLE;
    config.pin_charger = 12;
  }
  if(config.board_rev == 3)  // sonoff s22 + i2c
  {
    config.pin_flash = 0;
    config.pin_sda = 3;
    config.pin_scl = 1;
    config.pin_led = 13;
    config.pin_inverter = OPT_DISABLE;
    config.pin_charger = 12;
  }

  if(config.board_rev == 4) // IoT Yunshan
  {
    config.pin_flash = OPT_DISABLE;
    config.pin_sda = OPT_DISABLE;
    config.pin_scl = OPT_DISABLE;
    config.pin_led = 2;
    config.pin_inverter = OPT_DISABLE;
    config.pin_charger = 4;
  }

  if(config.pin_inverter == OPT_DISABLE)
    config.i_enable = 0;
  if(config.pin_charger == OPT_DISABLE)
    config.c_enable = 0;

  if(config.button_timer_secs < 1)
    config.button_timer_secs = 1;

  if(config.button_timer_max < 1)
    config.button_timer_max = 1;

}

void copy_config(String target_ip)
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

void get_config_and_save(String target_ip)
{
  oled_clear();
  oled_println(F("Copying HTML"));
  oled_print(F("from "));
  oled_println(target_ip);

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(httpget_timeout * 2);

  String url = http_str + target_ip + "/config_raw";

  File f = SPIFFS.open(json_config_file, "w");
  if (f)
  {
    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        http.writeToStream(&f);
        oled_clear();
        oled_println(F("Copy OK"));

      }
    }
    else
    {
      both_println(F("HTTP GET error:"));
      both_println(http.errorToString(httpCode).c_str());
    }
    f.close();
    http.end();
  }
}
