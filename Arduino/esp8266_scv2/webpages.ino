// =================================================================================================================
// JS Helpers
// =================================================================================================================

// todo make this a lib

String js_header()
{
  return get_file(html_js_header);
}

String js_helper_innerhtml(const String n, const String v)
{
  //document.getElementById("title").innerHTML = "s";
//   String tmp = "document.getElementById(\"" + n + "\").innerHTML = `" + v + "`;\n";
  return "shtml(`" + n  + "`, `" + v + "`);\n";
}

String js_helper(const String n, const String v)
{
//   String tmp = "document.getElementById(\"" + n + "\").setAttribute('value','" + v + "');\n";
  return "sv('" + n + "', `" + v + "`);\n";
}

// add_input("html_id", "input_id", "input_length", "input_value", "input_note");
String html_create_input(const String html_id, const String input_id, const String input_length, const String input_value, const String input_note)
{
  //   String tmp = "document.getElementById(\"" + n + "\").setAttribute('value','" + v + "');\n";
  return "add_input('" + html_id + "', '" + input_id + "', '" + input_length + "', '" + input_value + "', '" + input_note + "');\n";
//   return "sv('" + n + "', `" + v + "`);\n";
}

String js_radio_helper(const String n1, const String n2, const bool v)
{
//   String tmp = "document.getElementById(\"";
//   if (v)
//     tmp += n1;
//   else
//     tmp += n2;
//
//   tmp +=  + "\").checked = true;\n";

  return "rsv('" + n1 + "', '" + n2 + "', '" + v + "');\n";
}

String js_select_helper(const String n, const String v)
{
  return String("ssv(\"" + n + "\", " + v + ");\n");
}

String js_table_add_row(const String id, const String cell1, const String cell2, const String cell3)
{
  return String("table_add_row(\"" + id + "\", \"" + cell1 + "\", \"" + cell2 + "\", \"" + cell3 + "\");\n");
}


// =================================================================================================================
// HTML Pages
// =================================================================================================================

// reget html trigger page config.download_html

void force_ntp_sync()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, F("Force NTP Sync"));

  webpage += web_footer();

  server.send(200, mime_html, webpage);
  timer_ntp_sync = 0;
}


void web_html_redownload()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, F("Force Updating HTML"));

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  config.download_html = 1;
  download_index = 0;
}

// config page
void web_config()
{
  String webpage;
  webpage =  get_file(html_config);

  webpage += js_header();

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn));

  webpage += js_helper(F("day_watts"), String(config.day_watts));
  webpage += js_select_helper(F("c_start_h"), String(config.c_start_h));
  webpage += js_select_helper(F("c_finish_h"), String(config.c_finish_h));

  webpage += js_select_helper(F("charger_oot_min"), String(config.charger_oot_min));
  webpage += js_select_helper(F("inverter_oot_min"), String(config.inverter_oot_min));
  webpage += js_select_helper(F("charger_oot_sec"), String(config.charger_oot_sec));
  webpage += js_select_helper(F("inverter_oot_sec"), String(config.inverter_oot_sec));

  webpage += js_helper(F("night_watts"), String(config.night_watts));
  webpage += js_select_helper(F("i_start_h"), String(config.i_start_h));
  webpage += js_select_helper(F("i_finish_h"), String(config.i_finish_h));

  webpage += js_radio_helper(F("c_enable"), F("c_disable"), config.c_enable);
  webpage += js_radio_helper(F("i_enable"), F("i_disable"), config.i_enable);

  webpage += js_radio_helper(F("timer_enable"), F("timer_disable"), config.day_is_timer);
  webpage += js_radio_helper(F("ntimer_enable"), F("ntimer_disable"), config.night_is_timer);

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


// network config
void datasrcs()
{
  String webpage;
  webpage =  get_file(html_datasrcs);

  webpage += js_header();

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn));

  webpage += js_helper(F("inverter_url"), String(config.inverter_url));
  webpage += js_helper(F("inverter_push_url"), String(config.inverter_push_url));
  webpage += js_helper(F("update_url"), String(config.update_url));
  webpage += js_helper(F("meter_url"), String(config.meter_url));
  // 3phase
  webpage += js_radio_helper(F("threephase1"), F("threephase0"), config.threephase);

  webpage += js_radio_helper(F("monitor_phase_a1"), F("monitor_phase_a0"), config.monitor_phase_a);
  webpage += js_radio_helper(F("monitor_phase_b1"), F("monitor_phase_b0"), config.monitor_phase_b);
  webpage += js_radio_helper(F("monitor_phase_c1"), F("monitor_phase_c0"), config.monitor_phase_c);


  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


// network config
void net_config()
{
  String webpage;
  webpage =  get_file(html_net_config);

  webpage += js_header();

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn));
  webpage += js_radio_helper(F("wifi_highpower_on"), F("wifi_highpower_off"), config.wifi_highpower_on);

  webpage += js_helper(F("description"), String(config.description));

  webpage += js_helper(F("wifi_ssid1"), config.wifi_ssid1);
  webpage += js_helper(F("wifi_pass1"), config.wifi_pass1);
  webpage += js_helper(F("wifi_ssid2"), config.wifi_ssid2);
  webpage += js_helper(F("wifi_pass2"), config.wifi_pass2);

  webpage += js_helper(F("hostn"), String(config.hostn));

  webpage += js_helper(F("inverter_url"), String(config.inverter_url));


  // NTP
  webpage += js_helper(F("ntp_server"), String(config.ntp_server));
  webpage += js_select_helper(F("gmt"), String(config.gmt));

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


// advance config page
void advance_config()
{
  String webpage;
  webpage =  get_file(html_advance_config);

  webpage += js_header();

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn));

  webpage += js_radio_helper(F("webc_mode_1"), F("webc_mode_0"), config.webc_mode);

  webpage += js_radio_helper(F("auto_update_on"), F("auto_update_off"), config.auto_update);

  webpage += js_radio_helper(F("avg_phase1"), F("avg_phase0"), config.avg_phase);

  webpage += js_radio_helper(F("rotate_oled1"), F("rotate_oled0"), config.rotate_oled);

  // BOARD Revision
  webpage += js_select_helper(F("board_rev"), String(config.board_rev));

  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
}

void web_timer_config()
{
  // ups mode stuff

  String webpage;
  webpage =  get_file(html_timer);

  webpage += js_header();

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn));
  webpage += js_radio_helper(F("button_timer_mode1"), F("button_timer_mode0"), config.button_timer_mode);
  webpage += js_helper(F("button_timer_secs"), String(config.button_timer_secs));

  //
  webpage += js_helper(F("button_timer_max"), String(config.button_timer_max));

  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
}

void web_config_submit()
{
  String password_sent = "";
  String newpasswd = "";
  String confnewpasswd = "";
  bool passchange = 0;

  String webpage = get_file(html_mode);

  if (server.args() > 0 )
  {
    for ( uint8_t i = 0; i < server.args(); i++ )
    {
      Serial.println(server.argName(i) + " = '" + server.arg(i) + "'");

      if (server.argName(i) == F("board_rev"))
        config.board_rev = server.arg(i).toInt();

      else if (server.argName(i) == F("c_enable"))
        config.c_enable = server.arg(i).toInt();

      else if (server.argName(i) == F("i_enable"))
        config.i_enable = server.arg(i).toInt();

      else if (server.argName(i) == F("timer_enable"))
        config.day_is_timer = server.arg(i).toInt();

      else if (server.argName(i) == F("wifi_highpower_on"))
        config.wifi_highpower_on = server.arg(i).toInt();

      else if (server.argName(i) == F("ntimer_enable"))
        config.night_is_timer = server.arg(i).toInt();

      else if (server.argName(i) == F("auto_update"))
        config.auto_update = server.arg(i).toInt();

      else if (server.argName(i) == F("avg_phase"))
        config.avg_phase = server.arg(i).toInt();


      else if (server.argName(i) == F("rotate_oled"))
        config.rotate_oled = server.arg(i).toInt();

      else if (server.argName(i) == F("button_timer_mode"))
        config.button_timer_mode = server.arg(i).toInt();

      else if (server.argName(i) == F("button_timer_secs"))
        config.button_timer_secs = server.arg(i).toInt();

      else if (server.argName(i) == F("button_timer_max"))
        config.button_timer_max = server.arg(i).toInt();

      else if (server.argName(i) == F("web_mode"))
        config.webc_mode = server.arg(i).toInt();

      // BOARD
      else if (server.argName(i) == F("ntp_server"))
        strlcpy(config.ntp_server, server.arg(i).c_str(), sizeof(config.ntp_server));

      else if (server.argName(i) == F("hostn"))
        strlcpy(config.hostn, server.arg(i).c_str(), sizeof(config.hostn));

      else if (server.argName(i) == F("description"))
        strlcpy(config.description, server.arg(i).c_str(), sizeof(config.description));

      // WIFI
      else if (server.argName(i) == F("wifi_ssid1"))
        strlcpy(config.wifi_ssid1, server.arg(i).c_str(), sizeof(config.wifi_ssid1));

      else if (server.argName(i) == F("wifi_pass1"))
        strlcpy(config.wifi_pass1, server.arg(i).c_str(), sizeof(config.wifi_pass1));

      else if (server.argName(i) == F("wifi_ssid2"))
        strlcpy(config.wifi_ssid2, server.arg(i).c_str(), sizeof(config.wifi_ssid2));

      else if (server.argName(i) == F("wifi_pass2"))
        strlcpy(config.wifi_pass2, server.arg(i).c_str(), sizeof(config.wifi_pass2));

      // TIME


      else if (server.argName(i) == F("gmt"))
        config.gmt = server.arg(i).toInt();

      // DAY Device

      else if (server.argName(i) == F("inverter_url"))
        strlcpy(config.inverter_url, server.arg(i).c_str(), sizeof(config.inverter_url));

      else if (server.argName(i) == F("inverter_push_url"))
        strlcpy(config.inverter_push_url, server.arg(i).c_str(), sizeof(config.inverter_push_url));

      else if (server.argName(i) == F("update_url"))
        strlcpy(config.update_url, server.arg(i).c_str(), sizeof(config.update_url));

      else if (server.argName(i) == F("c_start_h"))
        config.c_start_h = server.arg(i).toInt();

      else if (server.argName(i) == F("c_finish_h"))
        config.c_finish_h = server.arg(i).toInt();

      else if (server.argName(i) == F("charger_oot_min"))
        config.charger_oot_min = server.arg(i).toInt();

      else if (server.argName(i) == F("charger_oot_sec"))
        config.charger_oot_sec = server.arg(i).toInt();

      else if (server.argName(i) == F("day_watts"))
        config.day_watts = server.arg(i).toInt();

      // Night Device

      else if (server.argName(i) == F("inverter_oot_min"))
        config.inverter_oot_min = server.arg(i).toInt();

      else if (server.argName(i) == F("inverter_oot_sec"))
        config.inverter_oot_sec = server.arg(i).toInt();


      else if (server.argName(i) == F("i_start_h"))
        config.i_start_h = server.arg(i).toInt();

      else if (server.argName(i) == F("i_finish_h"))
        config.i_finish_h = server.arg(i).toInt();

      else if (server.argName(i) == F("night_watts"))
        config.night_watts = server.arg(i).toInt();


      // 3 phase options
      else if (server.argName(i) == F("threephase"))
        config.threephase = server.arg(i).toInt();
      else if (server.argName(i) == F("monitor_phase_a"))
        config.monitor_phase_a = server.arg(i).toInt();
      else if (server.argName(i) == F("monitor_phase_b"))
        config.monitor_phase_b = server.arg(i).toInt();
      else if (server.argName(i) == F("monitor_phase_c"))
        config.monitor_phase_c = server.arg(i).toInt();

      else if (server.argName(i) == F("meter_url"))
        strlcpy(config.meter_url, server.arg(i).c_str(), sizeof(config.meter_url));

      else if (server.argName(i) == F("pcf857a_addr"))
        config.pcf857a_addr = server.arg(i).toInt();

      // latest

      // System Passwords

      else if (server.argName(i) == F("passwd"))
        password_sent = server.arg(i);

      else if (server.argName(i) == F("newpasswd"))
      {
        newpasswd = server.arg(i);
        passchange = 1;
      }
      else if (server.argName(i) == F("confnewpasswd"))
      {
        confnewpasswd = server.arg(i);
        passchange = 1;
      }

      // Unknown Argument
      else
      {
        webpage += F("<!-- Unknown ARG '");
        webpage += server.argName(i);
        webpage += F("' = '");
        webpage += server.arg(i);
        webpage += F("'--> \n");
      }
    }
  }

  webpage += js_header();

  if(passwd != "")
  {
    if(passwd != password_sent)
    {
      load_config();
      webpage += js_helper_innerhtml(title_str, F("bad pass"));
      webpage += web_footer();
      server.send(200, mime_html, webpage);
      return;
    }
  }

  if(passchange)
  {
    if(newpasswd == confnewpasswd)
    {
      passwd = newpasswd;
      save_passwd();
    }
    else
    {
      load_config();
      webpage += js_helper_innerhtml(title_str, F("pass miss match"));
      webpage += web_footer();
      server.send(200, mime_html, webpage);
      return;
    }
  }

  webpage += js_helper_innerhtml(title_str, F("Config Saved"));
  webpage += web_footer();

  server.send(200, mime_html, webpage);

  save_config();
}

void css_raw()
{
  String webpage = get_file(css_file);

  server.send(200, mime_txt, webpage);
}

void whatami()
{
  String webpage;
  webpage += "IP: " + WiFi.localIP().toString() + ", host: " + String(config.hostn) + ", Dev Type: ";

  if(config.board_rev == 1)
    webpage += "esp32 solar controller";

  if(config.board_rev == 0)
    webpage += "esp32 solar custom";

  webpage += ", Description: " + String(config.description) + ", SSID: ";
  String tmps = WiFi.SSID();
  tmps.trim();
  webpage += tmps + ", FW Version: " + String(FW_VERSION) + ", SPIFFS Card Size: ";
  webpage += fssize();
  webpage += "\n";

  server.send(200, mime_txt, webpage);
}

void config_raw()
{
  String webpage;
  if(config.webc_mode)
    webpage += get_file(json_config_file);

  server.send(200, mime_txt, webpage);
}

void info_raw()
{
  String webpage = datetime_str(0, '-', 'T', ':');
  webpage += ",";
  webpage += phase_sum;
  webpage += "\n";


  server.send(200, mime_txt, webpage);
}

// returns FLASH size
// returns FLASH size
String fssize()
{
  uint32_t realSize = ESP.getFlashChipRealSize();

  return String(realSize/1024) + " Kb";
}


String fsfree()
{
  FSInfo fs_info;
  SPIFFS.info(fs_info);

  uint32_t fspace = fs_info.totalBytes - fs_info.usedBytes;

  String result = "Size: ";

  result += String(fs_info.totalBytes/1024) + "Kb";

  result += ", Free: " + String(fspace/1024) + " Kb";

  return result;
}


void stats()
{
  String tmps;

  String mymode = "<pre>";

  if (system_mode == 0)
    mymode += F("IDLE");
  else if (system_mode == 1)
    mymode += F("CHARGE");
  else if (system_mode == 2)
    mymode += F("DRAIN");
  else if (system_mode == 3)
    mymode += F("Cross Over");
//   else if (system_mode == 4)
//     mymode += F("BMS");

  mymode += F(" - ");

  if (!day_time && !night_time)
    mymode += F("ZZZ");
  if (day_time && night_time)
    mymode += F("DAY + NIGHT");
  if (!day_time && night_time)
    mymode += F("NIGHT");
  if (day_time && !night_time)
    mymode += F("DAY");

  mymode += F("\n");


  mymode += "\n" + mode_reason;
  mymode += "</pre>";

  String nu_string = next_update_string(1);

  // ----------------------------------------------------------------------

  String webpage =  get_file(html_stats);
  webpage += js_header();
  webpage += js_helper_innerhtml(title_str, String(config.hostn) + " Info");

  webpage += js_helper_innerhtml(F("mode"), mymode);
  String extra_info = "";
  if(config.threephase)
  {
    extra_info += "<pre>Phase A: " + String(phase_a_watts) + " watts\\n";
    extra_info += "Phase B: " + String(phase_b_watts) + " watts\\n";
    extra_info += "Phase C: " + String(phase_c_watts) + " watts\\n";
    extra_info += "Monitored Sum: " + String(phase_sum) + " watts\\n";
    extra_info += "</pre>";
  }
  webpage += js_helper_innerhtml(F("watts"), String(phase_sum) + " Watts" + extra_info);

  webpage += js_helper_innerhtml(F("error_msgs"), "<pre>" + error_msgs + "</pre>");

  webpage += js_helper_innerhtml(F("next_update"), nu_string);


  webpage += web_footer();

  server.send(200, mime_html, webpage);
}

void sys_info()
{
  String tmps;

  //   description

  // ---------------------------------------------------------------------------------------------
  // calc next update time for stats display

  long millisecs = millis();
  const int days = int((millisecs / (1000 * 60 * 60 * 24)) % 365);
  const int hours = int((millisecs / (1000 * 60 * 60)) % 24);
  const int minutes = int((millisecs / (1000 * 60)) % 60);

  // ----------------------------------------------------------------------

  String mytime = "";
  time_t timetmp = now();

  if (hour(timetmp) < 10)
    mytime += "0";
  mytime += hour(timetmp);
  mytime += ":";
  if (minute(timetmp) < 10)
    mytime += "0";
  mytime += minute(timetmp);

  // ----------------------------------------------------------------------

  String built = String(__DATE__) + " " + String(__TIME__);
  String myuptime = String(days) + " days, " + String(hours) + " hours, " + String(minutes) + " min";

  String nu_string = next_update_string(1);

  // ----------------------------------------------------------------------

  String webpage =  get_file(html_sys_info);
  webpage += js_header();
  webpage += js_helper_innerhtml(title_str, String(config.hostn) + " Sys Info");
  webpage += js_helper_innerhtml(F("description"), String(config.description));
  webpage += js_helper_innerhtml(F("time"), mytime);
  webpage += js_helper_innerhtml(F("uptime"), myuptime);

  webpage += js_helper_innerhtml(F("flash_size"), fssize());
  webpage += js_helper_innerhtml(F("fsfree"), fsfree());


  if(config.wifi_highpower_on)
    webpage += js_helper_innerhtml(F("wifi_mode"), F("High Power") );
  else
    webpage += js_helper_innerhtml(F("wifi_mode"), F("Low Power") );

  webpage += js_helper_innerhtml(F("wifi_sigs"), String(WiFi.RSSI()) + " dBm" );

  tmps = WiFi.SSID(); tmps.trim();
  webpage += js_helper_innerhtml(F("wifi_ssid"), tmps);
  webpage += js_helper_innerhtml(F("mac"), WiFi.macAddress() );
  webpage += js_helper_innerhtml(F("ip"), WiFi.localIP().toString() );


  webpage += js_helper_innerhtml(F("built"), built);
  webpage += js_helper_innerhtml(F("fwver"), String(FW_VERSION));

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


void led_on()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, frline(7));
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
    set_led(1);
}

void led_off()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, frline(6) );
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
    set_led(0);
}

void led_toggle()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, frline(5) );
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
    set_led(2);
}

void led_blink()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, frline(4) );
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
    config.blink_led = 1;
}

void inverter_on()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  if(config.webc_mode)
  {
    mode_reason = frline(1);
    webpage += js_helper_innerhtml(title_str, mode_reason);

  }
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
  {
//     mode_reason = frline(1);
    low_voltage_shutdown = 0;
    inverter_off_time = 0;
    modeset(2);
  }
}

String web_footer()
{
  String webpage = F("</script>");

  webpage += get_file(html_footer);

  webpage += F("</body>");
  webpage += F("</html>");

  return webpage;
}

void idle_on()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  if(config.webc_mode)
  {
    mode_reason = frline(2);
    webpage += js_helper_innerhtml(title_str, mode_reason);
  }
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
  {

    modeset(0);
  }
}

void charger_on()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  if(config.webc_mode)
  {
    mode_reason = frline(3);
    webpage += js_helper_innerhtml(title_str, mode_reason);
  }
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
  {
    charger_high_voltage_shutdown = 0;
    charger_off_time = 0;
    modeset(1);
  }
}

void force_refresh()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, F("Forcing Refresh"));
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
  {
    update_time = 0;
    timer_pgrid = 0;
    sync_time();
  }
}

void software_reset()
{
  String webpage = get_file(html_mode);
  webpage += js_header();

  // avoid restarts being triggered after boot.
  time_t timetmp = now();
  if (!time_synced ||  (hour(timetmp) < 1 && minute(timetmp) < 2))
  {
    webpage += js_helper_innerhtml(title_str, F("System Just Booted"));
    server.send(200, mime_html, webpage);
    return;
  }

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, F("Software Reboot"));
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();
  server.send(200, mime_html, webpage);

  // https://github.com/esp8266/Arduino/issues/1722#issuecomment-192624783
  // note first restart after flashing by serial will hang.
  oled_clear();
  oled_println(F("Software\nRESTART"));

  restart_trigger = 1;
}


// =================================================================================================================
// Port Configuration Pages
// =================================================================================================================


void port_config()
{
  String webpage;
  webpage =  get_file(html_port_config);

  webpage += js_header();

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn));

  webpage += js_select_helper(F("lport"), String(config.pin_led));

  // config.pin_charger
  webpage += js_select_helper(F("cport"), String(config.pin_charger));
  webpage += js_select_helper(F("dport"), String(config.pin_inverter));

  //config.pin_flash
  webpage += js_select_helper(F("fuport"), String(config.pin_flash));

  // i2c ports

  webpage += js_select_helper(F("sclport"), String(config.pin_scl));
  webpage += js_select_helper(F("sdaport"), String(config.pin_sda));

  webpage += js_radio_helper(F("flip_cpin1"), F("flip_cpin0"), config.flip_cpin);
  webpage += js_radio_helper(F("flip_ipin1"), F("flip_ipin0"), config.flip_ipin);

  // LED

  webpage += js_radio_helper(F("blink_led_on"), F("blink_led_off"), config.blink_led_default);
  webpage += js_radio_helper(F("led_status_on"), F("led_status_off"), config.led_status);

  // port module
  webpage += js_select_helper(F("port_module_addr"), String(config.pcf857a_addr, DEC));

  webpage += web_footer();

  server.send(200, mime_html, webpage);

}

void port_cfg_submit()
{
  String webpage = get_file(html_mode);
  webpage += js_header();

  if (server.args() > 0 )
  {
    for ( uint8_t i = 0; i < server.args(); i++ )
    {
      if (server.argName(i) == F("lport"))
        config.pin_led = server.arg(i).toInt();
      else if (server.argName(i) == F("cport"))
        config.pin_charger = server.arg(i).toInt();
      else if (server.argName(i) == F("dport"))
        config.pin_inverter = server.arg(i).toInt();
      else if (server.argName(i) == F("fuport"))
        config.pin_flash = server.arg(i).toInt();
      else if (server.argName(i) == F("sdaport"))
        config.pin_sda = server.arg(i).toInt();
      else if (server.argName(i) == F("sclport"))
        config.pin_scl = server.arg(i).toInt();
      else if (server.argName(i) == F("flip_cpin"))
        config.flip_cpin = server.arg(i).toInt();
      else if (server.argName(i) == F("flip_ipin"))
        config.flip_ipin = server.arg(i).toInt();
      else if (server.argName(i) == F("led_status"))
        config.led_status = server.arg(i).toInt();
      else if (server.argName(i) == F("blink_led"))
        config.blink_led_default = server.arg(i).toInt();
      else if (server.argName(i) == F("port_module_addr"))
        config.pcf857a_addr = server.arg(i).toInt();
//       else
//       {
//         if(serial_on)
//           Serial.println(F("Unknown arg"));
//       }
    }

    save_config();
    restart_trigger = 1;
  }


  webpage += web_footer();

  server.send(200, mime_html, webpage);
}

void web_copy_config()
{
  String webpage = get_file(html_cpconfig);

  webpage += js_header();

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}

void copy_config_submit()
{
  String webpage = get_file(html_mode);
  webpage += js_header();
  webpage += js_helper_innerhtml(title_str, F("Copying Config"));
  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if (server.args() > 0 )
  {
    for ( uint8_t i = 0; i < server.args(); i++ )
    {
      if (server.argName(i) == F("IP"))
      {
        copy_config(server.arg(i));
      }
    }
  }
}


void update_menu()
{
  String webpage = get_file(html_mode);
  String pre;
  String link;

  webpage += js_header();

  pre = check_for_update();
  link = F("<a href=/do_update>Upgrade Now</a>");

  webpage += js_helper_innerhtml(title_str, F("Update Check"));
  webpage += js_helper_innerhtml(F("pre"), pre);
  webpage += js_helper_innerhtml(F("link"), link);
  webpage += js_helper_innerhtml(F("link2"), "<a href=" + String(config.update_url) + "/changelog.txt>Change Log</a>");

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


void do_update_web()
{
  String pre = check_for_update();

  if(found_update)
  {
    pre += String("\\n\\nSTARTING UPDATE.\\nDEVICE WILL RESTART SOON.");
  }

  String webpage = get_file(html_mode);
  String link = "";

  webpage += js_header();
  webpage += js_helper_innerhtml(title_str, F("Updating"));
  webpage += js_helper_innerhtml(F("pre"), pre);
  webpage += js_helper_innerhtml(F("link"), link);
  webpage += web_footer();

  server.send(200, mime_html, webpage);

  self_update = 1;

  return;
}


void upload_config()
{
  String webpage = get_file(html_upload_config);

  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, F("Upload Config"));
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


void upload_config_submit()
{
  String webpage = get_file(html_mode);
  webpage += js_header();
  webpage += js_helper_innerhtml(title_str, F("Saving Uploaded Config"));
  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if (server.args() > 0 )
  {
    for ( uint8_t i = 0; i < server.args(); i++ )
    {
      if (server.argName(i) == F("config"))
      {
        File f = SPIFFS.open(json_config_file, "w");
        f.print(server.arg(i));
        f.close();
      }
    }
  }
  restart_trigger = 1;
}

void i2c_scan()
{
  String webpage = "i2c Scan Results:\n\n";
  for (char address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0)
    {
      if(address == 0x3C || address == 0x3D)
      {
        webpage += "OLED:\t\t0x" + String(address, HEX)  + "\n";
      }
      else if (address == 0x60 || address == 0x61 ||  address == 0x62 || address == 0x63)
      {
        webpage += "MCP4725:\t0x" + String(address, HEX) + "\n";
      }
      else if (address == 0x48 || address == 0x49 ||  address == 0x4a || address == 0x4b)
      {
        webpage += "ADS1115:\t0x" + String(address, HEX) + "\n";
      }
      else if (address >= 0x38 && address <= 0x3F)
      {
        webpage += "PCF8574a:\t0x" + String(address, HEX) + "\n";
      }
      else if (address >= 0x20 && address <= 0x27)
      {
        webpage += "PCF8574:\t0x" + String(address, HEX) + "\n";
      }
      else
        webpage += "Unknown:\t0x" + String(address, HEX) + "\n";
    }
  }

  server.send(200, mime_txt, webpage);
}


void web_issue_submit()
{
  String webpage = get_file(html_mode);
  webpage += js_header();



  if (server.args() > 0 )
  {
    for ( uint8_t i = 0; i < server.args(); i++ )
    {
      if (server.argName(i) == F("issue"))
      {
        webpage += js_helper_innerhtml(F("title"), F("Logged Message"));
        log_issue("user: " + server.arg(i));
      }
      else if (server.argName(i) == F("clear"))
      {
        if(server.arg(i).toInt())
        {
          webpage += js_helper_innerhtml(F("title"), F("Cleared Log"));
          null_file(txt_log_system);
        }
      }
      else
      {
//         if(serial_on)
//           Serial.println(F("Unknown arg"));
      }
    }
  }


  webpage += web_footer();

  server.send(200, mime_html, webpage);
}

void web_issue_log()
{
  String webpage = get_file(html_issue_log);

  webpage += js_header();

  webpage += js_helper_innerhtml(F("title_hostn"), F("SysLog"));
  webpage += js_helper_innerhtml(F("log"), get_file(txt_log_system));

  webpage += js_radio_helper(F("clear1"), F("clear0"), 0);

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


