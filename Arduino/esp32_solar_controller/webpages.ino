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
  return "shtml(`" + n  + "`, `" + v + "`);\n";
}

String js_helper(const String n, const String v)
{
  return "sv('" + n + "', `" + v + "`);\n";
}

// html_create_input("hvrd", "config.hv_shutdown_delay", "3", "10", "...")
// add_input("html_id", "input_id", "input_length", "input_value", "input_note");
String html_create_input(const String html_id, const String input_id, const String input_length, const String input_value, const String input_note)
{
  return "add_input('" + html_id + "', '" + input_id + "', '" + input_length + "', '" + input_value + "', '" + input_note + "');\n";
}

String js_radio_helper(const String n1, const String n2, const bool v)
{
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

// reget html trigger page flags.download_html

void force_ntp_sync()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, F("Force NTP Sync"));

  webpage += web_footer();

  server.send(200, mime_html, webpage);
  timers.ntp_sync = 0;
}


void web_html_redownload()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, F("Force Updating HTML"));

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  flags.download_html = 1;
  download_index = 0;
}

// config page
void web_config()
{
  String webpage;
  webpage =  get_file(html_config);

  webpage += js_header();

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn));

  webpage += js_radio_helper(F("m247-1"), F("m247-0"), config.m247);

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
  webpage += js_helper(F("pub_url"), String(config.pub_url));
  webpage += js_helper(F("threephase_push_url"), String(config.threephase_push_url));

  webpage += js_helper(F("threephase_direct_url"), String(config.threephase_direct_url));

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
//   webpage += js_radio_helper(F("wifi_highpower_on"), F("wifi_highpower_off"), config.wifi_highpower_on);

  webpage += js_helper(F("description"), String(config.description));

  webpage += js_helper(F("wifi_ssid1"), config.wifi_ssid1);
  webpage += js_helper(F("wifi_pass1"), config.wifi_pass1);
  webpage += js_helper(F("hostn"), String(config.hostn));

  webpage += js_helper(F("wifi_ssid2"), config.wifi_ssid2);
  webpage += js_helper(F("wifi_pass2"), config.wifi_pass2);

  // NTP
  webpage += js_helper(F("name_input"), String(config.ntp_server));
  webpage += js_select_helper(F("gmt"), String(config.gmt));

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


// ntc10k_config page
void ntc10k_config()
{
  String webpage =  get_file(html_ntc10k_config);

  webpage += js_header();

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn) + " ntc10k config");

  for(int i = 0; i < count_ntc; i++)
  {
    String input_name = "ntc_temp_max";
    String id_name = "ntc_temp_max" + String(i+1);
    input_name += String(i + 1);

    webpage += html_create_input(id_name, input_name, "20", String(config.ntc_temp_max[i]), ".");
  }

  webpage += js_radio_helper(F("monitor_temp_1"), F("monitor_temp_0"), config.monitor_temp);

  // address
  webpage += js_select_helper(F("ntc10k_count"), String(config.ntc10k_count));

  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
}


// ntc10k_config page
void ntc10k_info()
{
  String webpage =  get_file(html_ntc10k_info);

  webpage += js_header();

  for(uint8_t i = 0; i < config.ntc10k_count; i++)
  {
    webpage += js_table_add_row("temp_table", String(i+1), "", ntc10k_sensors[i] + String("c") );
  }

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn) + " ntc10k Info");

  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
}

void battery_info()
{
  String webpage =  get_file(html_battery_info);

  webpage += js_header();

//   float vsum = 0;

  for(uint8_t i = 0; i < config.cell_count; i++)
  {
//     vsum += cells_volts[i];

    String tmp = "";


    if(cells_volts[i] < config.battery_volt_min)
      tmp += " UNDERVOLT";

    if(cells_volts[i] > config.battery_volt_max)
      tmp += " OVERVOLT";

    webpage += js_table_add_row("battery_table", String(i+1), String(cells_volts[i], 4),  tmp);
  }

  webpage += js_table_add_row("battery_table", String("*"), String(cell_volt_diff, 4),  "Cell Difference");

  if(config.cells_in_series)
  {
    webpage += js_table_add_row("battery_table", String("*"), String(cells_volts_real[config.cell_count-1], 4),  "SUM");
  }

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn) + " Battery Info");

  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
}


void battery_calibrate()
{
  String webpage =  get_file(html_calibrate);

  webpage += js_header();

  //   float vsum = 0;

  for(uint8_t i = 0; i < config.cell_count; i++)
  {
    String tmp = "";
//     function batcal_add_row(cell, voltage, modifier)
    webpage += "batcal_add_row(" + String(i+1) + ", " + String(cells_volts_real[i], 7) + ", " + String(config.battery_volt_mod[i], 7) + ");\n";
  }

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn) + " Battery Info");

  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
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

//   webpage += js_radio_helper(F("display_phases1"), F("display_phases0"), config.display_phases);
  webpage += js_select_helper(F("display_mode"), String(config.display_mode));

  // BOARD Revision
  webpage += js_select_helper(F("board_rev"), String(config.board_rev));

  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
}

void battery_config()
{
  // ups mode stuff

  String webpage =  get_file(html_battery);

  webpage += js_header();

  webpage += js_helper_innerhtml(F("title_hostn"), String(config.hostn) + String(F("Battery Config")) );
  webpage += js_radio_helper(F("monitor_battery1"), F("monitor_battery0"), config.monitor_battery);


  webpage += js_radio_helper(F("hv_monitor1"), F("hv_monitor0"), config.hv_monitor);

  webpage += html_create_input(F("idcc"), F("cell_count"), "3", String(config.cell_count), "1-16");

  webpage += html_create_input(F("bvmin"), F("battery_volt_min"), "10", String(config.battery_volt_min, 4), "float");

  webpage += html_create_input(F("bvrec"), F("battery_volt_rec"), "10", String(config.battery_volt_rec, 4), "float");
  webpage += html_create_input(F("bvmax"), F("battery_volt_max"), "10", String(config.battery_volt_max, 4), "float");
  webpage += html_create_input(F("pvm"), F("pack_volt_min"), "10", String(config.pack_volt_min, 4), "float");
  webpage += html_create_input(F("voff"), F("ups_volt_ofs"), "10", String(config.dcvoltage_offset, 4), "float");

  for(int i = 0; i < count_cells; i++)
  {
    String input_name = "battery_volt_mod";
    String id_name = "bvm" + String(i+1);
    input_name += String(i + 1);

    webpage += html_create_input(id_name, input_name, "20", String(config.battery_volt_mod[i], 7), ".");
  }

  webpage += js_select_helper(F("cells_in_series"), String(config.cells_in_series) );

  webpage += html_create_input(F("lvrd"), F("lv_shutdown_delay"), "3", String(config.lv_shutdown_delay, 2), "float");
  webpage += html_create_input(F("hvrd"), F("hv_shutdown_delay"), "3", String(config.hv_shutdown_delay, 2), "float");


//   webpage += F("</script>"); // page string to big, footer not attached

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
      bool skip2next = 0;
//       Serial.println(server.argName(i) + " = '" + server.arg(i) + "'");

      if (server.argName(i) == F("board_rev"))
        config.board_rev = server.arg(i).toInt();


      else if (server.argName(i) == F("m247"))
        config.m247 = server.arg(i).toInt();

      else if (server.argName(i) == F("c_enable"))
        config.c_enable = server.arg(i).toInt();

      else if (server.argName(i) == F("i_enable"))
        config.i_enable = server.arg(i).toInt();

      else if (server.argName(i) == F("timer_enable"))
        config.day_is_timer = server.arg(i).toInt();

//       else if (server.argName(i) == F("wifi_highpower_on"))
//         config.wifi_highpower_on = server.arg(i).toInt();

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

      else if (server.argName(i) == F("display_mode"))
        config.display_mode = server.arg(i).toInt();

      // BOARD
      else if (server.argName(i) == F("name_input"))
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

      else if (server.argName(i) == F("pub_url"))
        strlcpy(config.pub_url, server.arg(i).c_str(), sizeof(config.pub_url));

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

      else if (server.argName(i) == F("threephase_push_url"))
        strlcpy(config.threephase_push_url, server.arg(i).c_str(), sizeof(config.threephase_push_url));
      else if (server.argName(i) == F("threephase_direct_url"))
        strlcpy(config.threephase_direct_url, server.arg(i).c_str(), sizeof(config.threephase_direct_url));



      // ups mode options
      else if (server.argName(i) == F("monitor_battery"))
        config.monitor_battery = server.arg(i).toInt();

      else if (server.argName(i) == F("monitor_battery"))
        config.monitor_battery = server.arg(i).toInt();

      else if (server.argName(i) == F("hv_monitor"))
        config.hv_monitor = server.arg(i).toInt();


      else if (server.argName(i) == F("cells_in_series"))
        config.cells_in_series = server.arg(i).toInt();

      else if (server.argName(i) == F("battery_volt_min"))
        config.battery_volt_min = server.arg(i).toFloat();

      else if (server.argName(i) == F("battery_volt_max"))
        config.battery_volt_max = server.arg(i).toFloat();

      else if (server.argName(i) == F("cell_count"))
        config.cell_count = server.arg(i).toInt();

      else if (server.argName(i) == F("battery_volt_rec"))
        config.battery_volt_rec = server.arg(i).toFloat();

      else if (server.argName(i) == F("pack_volt_min"))
        config.pack_volt_min = server.arg(i).toFloat();


      else if (server.argName(i) == F("monitor_temp"))
        config.monitor_temp = server.arg(i).toInt();


      else if(server.argName(i).startsWith("ntc_temp"))
      {
        for(int x = 0; x < count_ntc; x++)
        {
          /*
          if (server.argName(i) == String("ntc_temp_mod") + String(x+1) )
          {
            config.ntc_temp_mods[x] = server.arg(i).toFloat();
            skip2next = 1;
            break;
          }
          else */
          if (server.argName(i) == String("ntc_temp_max") + String(x+1) )
          {
            config.ntc_temp_max[x] = server.arg(i).toInt();
            skip2next = 1;
            break;
          }
        }
        if(skip2next)
          continue;
      }

      else if(server.argName(i).startsWith("battery_volt_mod"))
      {
        for(uint8_t x = 0; x < count_cells; x++)
        {
          if (server.argName(i) == String("battery_volt_mod") + String(x+1))
          {
            config.battery_volt_mod[x] = server.arg(i).toFloat();
            skip2next = 1;
            break;
          }
        }
        if(skip2next)
          continue;
      }

      else if (server.argName(i) == F("lv_shutdown_delay"))
        config.lv_shutdown_delay = server.arg(i).toFloat();

      else if (server.argName(i) == F("hv_shutdown_delay"))
        config.hv_shutdown_delay = server.arg(i).toFloat();

      // ntc10k options

      else if (server.argName(i) == F("ntc10k_count"))
        config.ntc10k_count = server.arg(i).toInt();

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

  flags.save_config = 1;
}

void css_raw()
{
  String webpage = get_file(css_file);

  server.send(200, mime_txt, webpage);
}

void whatami()
{
  String webpage;
  webpage += "IP: " + WiFi.localIP().toString() + ", host: " + String(config.hostn) + ", Dev Type: esp32";

  webpage += ", Description: " + String(config.description) + ", SSID: ";
  String tmps = WiFi.SSID();
  tmps.trim();
  webpage += tmps + ", FW Version: " + String(FW_VERSION) + ", SD Card Size: ";
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
String fssize()
{
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  String tmp = String(int(cardSize)) + "MB";
  return tmp;
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

  if (!flags.day && !flags.night)
    mymode += F("ZZZ");
  else if (config.m247)
    mymode += F("24/7");
  else if (flags.day && flags.night)
    mymode += F("DAY + NIGHT");
  else if (!flags.day && flags.night)
    mymode += F("NIGHT");
  else if (flags.day && !flags.night)
    mymode += F("DAY");
  else
    mymode += "???";

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
    extra_info += "<pre>Phase A: " + String(phase_a_watts) + " watts, " + String(phase_a_voltage) + " volts\\n";
    extra_info += "Phase B: " + String(phase_b_watts) + " watts, " + String(phase_b_voltage) + " volts\\n";
    extra_info += "Phase C: " + String(phase_c_watts) + " watts, " + String(phase_c_voltage) + " volts\\n";
    extra_info += "Monitored Sum: " + String(phase_sum) + " watts\\n";
    extra_info += "</pre>";
  }
  webpage += js_helper_innerhtml(F("watts"), String(phase_sum) + " Watts" + extra_info);

  if(config.monitor_battery)
  {
    String cell_string = "";

    // multi cells
    if(config.cell_count > 1)
    {
      cell_string = "<pre>";
      for(byte i = 0; i < config.cell_count; i++)
      {
        cell_string += String(i+1);
        cell_string += ": ";
        cell_string += String(cells_volts[i]);
        cell_string += "v\n";
      }
      if(config.cells_in_series)
      {
        cell_string += "Diff: " + String(cell_volt_diff, 4) + "v</pre>\n";
        cell_string += "Pack Total: " + String(cells_volts_real[config.cell_count-1]) + "v\n";
      }
    }
    // single cell
    else
    {
      cell_string = String(cells_volts_real[config.cell_count-1]) + "v";
    }

    webpage += js_helper_innerhtml(F("battery_voltage"), cell_string );
  }

  webpage += js_helper_innerhtml(F("system_msgs"), "<pre>" + system_msgs + "</pre>");

  webpage += js_helper_innerhtml(F("next_update"), nu_string);


  webpage += web_footer();

  server.send(200, mime_html, webpage);
}

void threepase_info()
{
  String webpage;
  webpage =  get_file(html_3pinfo);

  if(!config.threephase)
  {
    webpage += "three phase mode is disabled</pre>";
    webpage += js_header();
    webpage += web_footer();

    server.send(200, mime_html, webpage);
    return;
  }


  webpage += "Phase A: " + String(phase_a_watts) + " watts, " + String(phase_a_voltage) + " volts\n";
  webpage += "Phase B: " + String(phase_b_watts) + " watts, " + String(phase_b_voltage) + " volts\n";
  webpage += "Phase C: " + String(phase_c_watts) + " watts, " + String(phase_c_voltage) + " volts\n\n";
  webpage += "Phase Sum: " + String((phase_a_watts + phase_b_watts + phase_c_watts) ) + " watts\n\n";
  webpage += "Todays Usage: " + String(energy_consumed - energy_consumed_old, 1) + " Kwh\n";
  webpage += "</pre>";


  webpage += js_header();
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

  if(flags.lm75a)
    webpage += js_helper_innerhtml(F("board_temp"), String(board_temp, 2) + "C");
  else
    webpage += js_helper_innerhtml(F("board_temp"), "LM75a not present");

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
    webpage += js_helper_innerhtml(title_str, "LED ON");
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
    webpage += js_helper_innerhtml(title_str, "LED OFF" );
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
    webpage += js_helper_innerhtml(title_str, "LED TOGGLE" );
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
    webpage += js_helper_innerhtml(title_str, "LED BLINK" );
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
    mode_reason = "Manual Inverter On";
    webpage += js_helper_innerhtml(title_str, mode_reason);

  }
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
  {
    flags.shutdown_lvolt = 0;
    timers.inverter_off = 0;
    modeset(2);
  }
}

String web_footer()
{
  String webpage = F("</script>");

  webpage += get_file(html_footer);

  webpage += F("</body>\n</html>");

  return webpage;
}

void idle_on()
{
  String webpage = get_file(html_mode);

  webpage += js_header();

  if(config.webc_mode)
  {
    mode_reason = "Manual Idle";
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
    mode_reason = "Manual Charger ON";
    webpage += js_helper_innerhtml(title_str, mode_reason);
  }
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
  {
    flags.shutdown_hvolt = 0;
    timers.charger_off = 0;
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
    timers.mode_check = 0;
    timers.pgrid = 0;
    timers.adc_poll = 0;
    sync_time();
  }
}

void software_reset()
{
  String webpage = get_file(html_mode);
  webpage += js_header();

  // avoid restarts being triggered after boot.
  time_t timetmp = now();
  if (!flags.time_synced ||  (hour(timetmp) < 1 && minute(timetmp) < 2))
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

  flags.restart = 1;
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
  webpage += js_select_helper(F("pin_buzzer"), String(config.pin_buzzer));

  webpage += js_radio_helper(F("flip_cpin1"), F("flip_cpin0"), config.flip_cpin);
  webpage += js_radio_helper(F("flip_ipin1"), F("flip_ipin0"), config.flip_ipin);

  // LED

  webpage += js_radio_helper(F("blink_led_on"), F("blink_led_off"), config.blink_led_default);
  webpage += js_radio_helper(F("led_status_on"), F("led_status_off"), config.led_status);

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
      else if (server.argName(i) == F("pin_buzzer"))
        config.pin_buzzer = server.arg(i).toInt();
      else if (server.argName(i) == F("flip_cpin"))
        config.flip_cpin = server.arg(i).toInt();
      else if (server.argName(i) == F("flip_ipin"))
        config.flip_ipin = server.arg(i).toInt();
      else if (server.argName(i) == F("led_status"))
        config.led_status = server.arg(i).toInt();
      else if (server.argName(i) == F("blink_led"))
        config.blink_led_default = server.arg(i).toInt();
    }

    save_config();
    flags.restart = 1;
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
  webpage += js_helper_innerhtml(F("link2"), "<a href=" + String(config.pub_url) + "/changelog.txt>Change Log</a>");

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


void do_update_web()
{
  String pre = check_for_update();

  if(flags.update_found)
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

  flags.update_self = 1;

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
        File f = SD.open(json_config_file, FILE_WRITE);
        f.print(server.arg(i));
        f.close();
      }
    }
  }
  flags.restart = 1;
}

void bms_raw_info()
{
  String webpage = datetime_str(0, '-', 'T', ':') + String(",");

  for(byte i = 0; i< config.cell_count; i++)
    webpage += String(cells_volts[i], 6) + ",";

  server.send(200, mime_txt, webpage);
}


void port_info()
{
  String webpage = "system_mode: " + String(system_mode) + "\n";

  if(config.monitor_battery)
  {
    webpage += "cell_volt_low - " + String(cell_volt_low, 4) + "v\n";
    webpage += "cell_volt_diff - " + String(cell_volt_diff, 4) + "v\n";

//     webpage += "\nlow volt shutdown: " + String(flags.shutdown_lvolt ) + "\n";
//     webpage += "high volt shutdown: " + String(flags.shutdown_hvolt ) + "\n\n";

    webpage += "Cell Series Volts:\n";
    for(byte i = 0; i< config.cell_count; i++)
      webpage += String(i+1) + ") " + String(cells_volts_real[i], 6) + "v\n";

    webpage += "\n\n";

    webpage += "Cell Volts:\n\n";
    for(byte i = 0; i< config.cell_count; i++)
      webpage += String(i+1) + ") " + String(cells_volts[i], 6) + "v\n";

    webpage += "\n\n";

    webpage += "LV Shutdown: ";
    if(millis() > timers.lv_shutdown)
      webpage += "False";
    else
      webpage += "True";

    webpage += "\nHV Shutdown: ";
    if(millis() > timers.hv_shutdown)
      webpage += "False";
    else
      webpage += "True";

    webpage += "\n";
  }

  server.send(200, mime_txt, webpage);
}

void adc_info_raw()
{
  String webpage = "#\tEN\tVal\n";

  for(byte i = 0; i< 32; i++)
    webpage += String(i) + "\t" + String(adsmux.adc_enable[i]) + "\t" + String(adsmux.adc_val[i]) + "\n";


  server.send(200, mime_txt, webpage);
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
      else if (address == 0x48)
      {
        webpage += "ADS1115:\t0x" + String(address, HEX) + "\n";
      }
      else if (address == 0x49)
      {
        webpage += "ADS1015:\t0x" + String(address, HEX) + "\n";
      }
      else if (address == 0x4f)
      {
        webpage += "LM75a:\t\t0x" + String(address, HEX) + "\n";
      }
      else if (address == 0x20)
      {
        webpage += "PCA9555PW:\t0x" + String(address, HEX) + "\n";
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


