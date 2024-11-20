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

String js_hide(const String n)
{
  return "hide('" + n  + "');\n";
}

String js_helper(const String n, const String v)
{
  return "sv('" + n + "', `" + v + "`);\n";
}

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

void web_html_redownload()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, F("Force Updating HTML"));
  webpage += "\nredir(\"/\", \"15\");\n";

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  flags.download_html = 1;
  download_index = 0;
}

// config page
void web_config()
{
  String webpage = get_file(html_header);
  webpage += get_file(html_config);
  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, String(config.hostn));

  webpage += js_radio_helper(F("m247-1"), F("m247-0"), config.m247);

  webpage += js_helper(F("day_watts"), String(config.day_watts));
  webpage += js_select_helper(F("c_start_h"), String(config.c_start_h));
  webpage += js_select_helper(F("c_finish_h"), String(config.c_finish_h));

  webpage += js_radio_helper(F("prefer_dc1"), F("prefer_dc0"), config.prefer_dc);

  webpage += js_select_helper(F("charger_oot_min"), String(config.charger_oot_min));
  webpage += js_select_helper(F("inverter_oot_min"), String(config.inverter_oot_min));
  webpage += js_select_helper(F("charger_oot_sec"), String(config.charger_oot_sec));
  webpage += js_select_helper(F("inverter_oot_sec"), String(config.inverter_oot_sec));

  webpage += js_select_helper(F("charger_off_min"), String(config.charger_off_min));
  webpage += js_select_helper(F("charger_off_sec"), String(config.charger_off_sec));
  webpage += js_radio_helper(F("c_offd_enable"), F("c_offd_disable"), config.c_offd);
  webpage += js_radio_helper(F("c_amot_enable"), F("c_amot_disable"), config.c_amot);


  webpage += js_helper(F("night_watts"), String(config.night_watts));
  webpage += js_select_helper(F("i_start_h"), String(config.i_start_h));
  webpage += js_select_helper(F("i_finish_h"), String(config.i_finish_h));

  webpage += js_radio_helper(F("c_enable"), F("c_disable"), config.c_enable);
  webpage += js_radio_helper(F("i_enable"), F("i_disable"), config.i_enable);

  webpage += js_radio_helper(F("timer_enable"), F("timer_disable"), config.day_is_timer);
  webpage += js_radio_helper(F("ntimer_enable"), F("ntimer_disable"), config.night_is_timer);

  webpage += js_radio_helper(F("inv_idle_mode_on"), F("inv_idle_mode_off"), config.inv_idle_mode);

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


// network config
void datasrcs()
{
  String webpage = get_file(html_header);
  webpage += get_file(html_datasrcs);
  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + " configure data sources");

  webpage += js_helper(F("cpkwh"), String(config.cpkwh, 3));

//   webpage += js_helper(F("inverter_url"), String(config.inverter_url));
//   webpage += js_helper(F("inverter_push_url"), String(config.inverter_push_url));
  webpage += js_helper(F("pub_url"), String(config.pub_url));
  webpage += js_helper(F("fronius_push_url"), String(config.fronius_push_url));

  webpage += js_helper(F("threephase_direct_url"), String(config.threephase_direct_url));

  // 3phase
  webpage += js_radio_helper(F("threephase1"), F("threephase0"), config.threephase);

  webpage += js_radio_helper(F("monitor_phase_a1"), F("monitor_phase_a0"), config.monitor_phase_a);
  webpage += js_radio_helper(F("monitor_phase_b1"), F("monitor_phase_b0"), config.monitor_phase_b);
  webpage += js_radio_helper(F("monitor_phase_c1"), F("monitor_phase_c0"), config.monitor_phase_c);

  webpage += html_create_input(F("avgp"), F("avg_phase"), "10", String(config.avg_phase), "0 - "+ String(power_array_size) + " (0-1 = disabled)");

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


// network config
void net_config()
{
  String webpage = get_file(html_header);
  webpage += get_file(html_net_config);
  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + " Net Config");

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

void battery_info()
{
  String webpage = get_file(html_header);
  webpage += get_file(html_battery_info);
  webpage += js_header();

  if(!config.monitor_battery)
  {
    webpage += js_table_add_row("battery_table", "x", "x",  "disabled");
  }
  else
  {
    for(uint8_t i = 0; i < config.cell_count; i++)
    {
      String tmp = "";

      if(i == low_cell)
        tmp += " Lowest";

      if(i == high_cell)
        tmp += " Highest";

      if(cells_volts[i] < config.battery_volt_min)
        tmp += " UNDERVOLT";

      if(cells_volts[i] > config.battery_volt_max)
        tmp += " OVERVOLT";

      webpage += js_table_add_row("battery_table", String(i+1), String(cells_volts[i], 4),  tmp);
    }

    webpage += js_table_add_row("battery_table", String("*"), String(cell_volt_diff, 4),  "Cell Difference");

    // add complete adc poll time
    webpage += js_table_add_row("battery_table", String("*"), String(adc_poll_time, 3) + String(" sec"),  "ADC Complete poll time duration");

    webpage += js_table_add_row("battery_table", String("*"), String(pack_total_volts, 4),  "SUM");
  }

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + " Battery Info");

  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
}


void battery_calibrate()
{
  String webpage = get_file(html_header);
  webpage += get_file(html_calibrate);
  webpage += js_header();

  webpage += "muxtype = " + String(config.muxtype) + ";\n";

  uint8_t local_cell_count = mmaths.mmin<uint8_t>(config.cell_count, (uint8_t) MAX_CELLS);

  for(uint8_t i = 0; i < local_cell_count; i++)
  {
    String tmp = "";
    webpage += "batcal_add_row(" + String(i+1) + ", " + String(cells_volts_real[i], 24) + ", " + String(config.battery_volt_mod[i], 24) + ");\n";
  }

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + " Battery Info");

  webpage += js_helper(F("ccount"), String(local_cell_count));

  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
}

// advance config page
void advance_config()
{
  String webpage = get_file(html_header);
  webpage += get_file(html_advance_config);
  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + " advance config");

  webpage += js_radio_helper(F("webc_mode_1"), F("webc_mode_0"), config.webc_mode);

  webpage += js_radio_helper(F("auto_update_on"), F("auto_update_off"), config.auto_update);

  webpage += js_radio_helper(F("serial_off1"), F("serial_off0"), config.serial_off);

  webpage += html_create_input(F("maxsystemtemp"), F("maxsystemtemp"), "3", String(config.maxsystemtemp, 1), "(min 0, float)");

  webpage += js_radio_helper(F("rotate_oled1"), F("rotate_oled0"), config.rotate_oled);

  webpage += js_radio_helper(F("mcptype1"), F("mcptype0"), config.mcptype);
  webpage += js_radio_helper(F("ads1x15type1"), F("ads1x15type0"), config.ads1x15type);
  webpage += js_select_helper(F("muxtype"), String(config.muxtype));

  webpage += js_radio_helper(F("dumbsystem1"), F("dumbsystem0"), config.dumbsystem);


  webpage += js_select_helper(F("display_mode"), String(config.display_mode));

  // BOARD Revision
  webpage += js_select_helper(F("board_rev"), String(config.board_rev));

  webpage += js_select_helper(F("i2cmaxspeed"), String(config.i2cmaxspeed));



  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
}

void battery_config()
{
  // ups mode stuff
  String webpage = get_file(html_header);
  webpage += get_file(html_battery);
  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + String(F(" Battery Config")) );
  webpage += js_radio_helper(F("monitor_battery1"), F("monitor_battery0"), config.monitor_battery);


  webpage += js_radio_helper(F("hv_monitor1"), F("hv_monitor0"), config.hv_monitor);

  webpage += js_radio_helper(F("avg_ain1"), F("avg_ain0"), config.avg_ain);

  webpage += html_create_input(F("idcc"), F("cell_count"), "3", String(config.cell_count), "1-16");

  webpage += html_create_input(F("bvmin"), F("battery_volt_min"), "10", String(config.battery_volt_min, 4), "float");

  webpage += html_create_input(F("bvrec"), F("battery_volt_rec"), "10", String(config.battery_volt_rec, 4), "float");
  webpage += html_create_input(F("bvmax"), F("battery_volt_max"), "10", String(config.battery_volt_max, 4), "float");
  webpage += html_create_input(F("pvm"), F("pack_volt_min"), "10", String(config.pack_volt_min, 4), "float");
  webpage += html_create_input(F("voff"), F("ups_volt_ofs"), "10", String(config.dcvoltage_offset, 4), "float");

  webpage += html_create_input(F("lvrd"), F("lv_shutdown_delay"), "3", String(config.lv_shutdown_delay, 2), "float");
  webpage += html_create_input(F("hvrd"), F("hv_shutdown_delay"), "3", String(config.hv_shutdown_delay, 2), "float");

  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
}

void web_config_submit()
{
  String password_sent = "";
  String newpasswd = "";
  String confnewpasswd = "";
  bool passchange = 0;

  String webpage = get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  if (server.args() > 0 )
  {
    for ( uint8_t i = 0; i < server.args(); i++ )
    {
      bool skip2next = 0;
//       Serial.println(server.argName(i) + " = '" + server.arg(i) + "'");


      if (server.argName(i) == F("board_rev"))
        config.board_rev = server.arg(i).toInt();

      else if (server.argName(i) == F("i2cmaxspeed"))
        config.i2cmaxspeed = server.arg(i).toInt();

      // API stuff


      else if(server.argName(i).startsWith("api_venable"))
      {
        for(uint8_t x = 0; x < max_api_vservers; x++)
        {
          if(server.argName(i) == "api_venable" + String(x+1))
          {
            config.api_venable[x] = server.arg(i).toInt();
          }
        }
      }
      else if(server.argName(i).startsWith("api_ienable"))
      {
        for(uint8_t x = 0; x < max_api_vservers; x++)
        {
          if(server.argName(i) == "api_ienable" + String(x+1))
          {
            config.api_ienable[x] = server.arg(i).toInt();
          }
        }
      }
      else if(server.argName(i).startsWith("api_vserver_hostname"))
      {
        for(uint8_t x = 0; x < max_api_vservers; x++)
        {
          if(server.argName(i) == "api_vserver_hostname" + String(x+1))
          {
            strlcpy(config.api_vserver_hostname[x], server.arg(i).c_str(), sizeof(config.api_vserver_hostname[x]));
          }
        }
      }
      else if(server.argName(i).startsWith("api_iserver_hostname"))
      {
        for(uint8_t x = 0; x < max_api_vservers; x++)
        {
          if(server.argName(i) == "api_iserver_hostname" + String(x+1))
          {
            strlcpy(config.api_iserver_hostname[x], server.arg(i).c_str(), sizeof(config.api_iserver_hostname[x]));
          }
        }
      }

      else if (server.argName(i) == F("api_lm75a"))
        config.api_lm75a = server.arg(i).toInt();

      else if (server.argName(i) == F("api_grid"))
        config.api_grid = server.arg(i).toInt();

      else if (server.argName(i) == F("api_pollsecs"))
        config.api_pollsecs = server.arg(i).toFloat();

      // end API stuff

      else if (server.argName(i) == F("m247"))
        config.m247 = server.arg(i).toInt();

      else if (server.argName(i) == F("c_enable"))
        config.c_enable = server.arg(i).toInt();

      else if (server.argName(i) == F("c_offd"))
        config.c_offd = server.arg(i).toInt();

      else if (server.argName(i) == F("c_amot"))
        config.c_amot = server.arg(i).toInt();



      else if (server.argName(i) == F("prefer_dc"))
        config.prefer_dc = server.arg(i).toInt();

      else if (server.argName(i) == F("i_enable"))
        config.i_enable = server.arg(i).toInt();

      else if (server.argName(i) == F("timer_enable"))
        config.day_is_timer = server.arg(i).toInt();

      else if (server.argName(i) == F("ntimer_enable"))
        config.night_is_timer = server.arg(i).toInt();

      else if (server.argName(i) == F("auto_update"))
        config.auto_update = server.arg(i).toInt();

      else if (server.argName(i) == F("serial_off"))
        config.serial_off = server.arg(i).toInt();

      else if (server.argName(i) == F("avg_phase"))
        config.avg_phase = server.arg(i).toInt();

      else if (server.argName(i) == F("rotate_oled"))
        config.rotate_oled = server.arg(i).toInt();

      else if (server.argName(i) == F("mcptype"))
        config.mcptype = server.arg(i).toInt();

      else if (server.argName(i) == F("maxsystemtemp"))
        config.maxsystemtemp = server.arg(i).toFloat();


      else if (server.argName(i) == F("ads1x15type"))
        config.ads1x15type = server.arg(i).toInt();

      else if (server.argName(i) == F("muxtype"))
        config.muxtype = server.arg(i).toInt();

      else if (server.argName(i) == F("dumbsystem"))
        config.dumbsystem = server.arg(i).toInt();


      else if (server.argName(i) == F("web_mode"))
        config.webc_mode = server.arg(i).toInt();

      else if (server.argName(i) == F("inv_idle_mode"))
        config.inv_idle_mode = server.arg(i).toInt();

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


      else if (server.argName(i) == F("cpkwh"))
        config.cpkwh = server.arg(i).toFloat();

      else if (server.argName(i) == F("pub_url"))
        strlcpy(config.pub_url, server.arg(i).c_str(), sizeof(config.pub_url));

      // Day Device

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

      else if (server.argName(i) == F("charger_off_min"))
        config.charger_off_min = server.arg(i).toInt();

      else if (server.argName(i) == F("charger_off_sec"))
        config.charger_off_sec = server.arg(i).toInt();

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

      else if (server.argName(i) == F("fronius_push_url"))
        strlcpy(config.fronius_push_url, server.arg(i).c_str(), sizeof(config.fronius_push_url));
      else if (server.argName(i) == F("threephase_direct_url"))
        strlcpy(config.threephase_direct_url, server.arg(i).c_str(), sizeof(config.threephase_direct_url));



      // ups mode options
      else if (server.argName(i) == F("monitor_battery"))
        config.monitor_battery = server.arg(i).toInt();

      else if (server.argName(i) == F("monitor_battery"))
        config.monitor_battery = server.arg(i).toInt();

      else if (server.argName(i) == F("hv_monitor"))
        config.hv_monitor = server.arg(i).toInt();

      else if (server.argName(i) == F("avg_ain"))
        config.avg_ain = server.arg(i).toInt();

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

      else if(server.argName(i).startsWith("battery_volt_mod"))
      {
        for(uint8_t x = 0; x < adsmux.ain_count; x++)
        {
          if (server.argName(i) == String("battery_volt_mod") + String(x+1))
          {
            config.battery_volt_mod[x] = server.arg(i).toDouble();
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

  if(passwd != "")
  {
    if(passwd != password_sent)
    {
      load_config();
      webpage += js_helper_innerhtml(title_str, F(" bad pass"));
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
      webpage += js_helper_innerhtml(title_str, F(" pass miss match"));
      webpage += "\nredir(\"/\", \"60\");\n";
      webpage += web_footer();
      server.send(200, mime_html, webpage);
      return;
    }
  }

  webpage += js_helper_innerhtml(title_str, String(config.hostn) +  F(" Config Saved"));
  webpage += "\nredir(\"/\", \"15\");\n";
  webpage += web_footer();

  server.send(200, mime_html, webpage);

  adsmux.avg_ain = config.avg_ain;
  flags.save_config = 1;
}

void css_raw()
{
  String webpage = get_file(css_file);

  server.send(200, mime_css, webpage);
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

  server.send(200, mime_json, webpage);
}

void json_cells()
{
  String webpage = "";

  DynamicJsonDocument doc(config_json_size);

  doc["host_name"] = String(config.hostn);

  doc["api_version"] = 1;

  doc["adc_found"] = adsmux.adc_found;

  doc["cell_monitor"] = config.monitor_battery;

  if(config.monitor_battery)
  {
    doc["cell_count"] = config.cell_count;

    doc["cell_low"] = low_cell;
    doc["cell_high"] = high_cell;

    doc["adc_poll_time"] = adc_poll_time;

    for(uint8_t i = 0; i < config.cell_count; i++)
    {
      doc["cell_"+String(i+1)] = cells_volts[i];
    }
    doc["cell_diff"] = cell_volt_diff;

    doc["cell_total"] = pack_total_volts;
  }

  serializeJsonPretty(doc, webpage);

  server.send(200, mime_json, webpage);
}


void jsonapi()
{
  String webpage = "";

  DynamicJsonDocument doc(config_json_size);

  doc["host_name"] = String(config.hostn);

  doc["api_version"] = 1;

  doc["system_mode"] = system_mode;
  doc["system_time"] = datetime_str(0, '/', ' ', ':');

  doc["lm75a"] = flags.lm75a;
  doc["lm75a_now"] = board_temp;
  doc["lm75a_min"] = board_temp_min;
  doc["lm75a_max"] = board_temp_max;

  // grid info

  doc["phase_a_watts"] = phase_a_watts;
  doc["phase_b_watts"] = phase_b_watts;
  doc["phase_c_watts"] = phase_c_watts;

  doc["phase_a_voltage"] = phase_a_voltage;
  doc["phase_b_voltage"] = phase_b_voltage;
  doc["phase_c_voltage"] = phase_c_voltage;

  doc["phase_a_voltage_low"] = phase_a_voltage_low;
  doc["phase_b_voltage_low"] = phase_b_voltage_low;
  doc["phase_c_voltage_low"] = phase_c_voltage_low;

  doc["phase_a_voltage_high"] = phase_a_voltage_high;
  doc["phase_b_voltage_high"] = phase_b_voltage_high;
  doc["phase_c_voltage_high"] = phase_c_voltage_high;

  doc["c_pinmode"] = flags.c_pinmode;
  doc["i_pinmode"] = flags.i_pinmode;

  serializeJsonPretty(doc, webpage);

  server.send(200, mime_json, webpage);
}

void apiservers()
{
  // ups mode stuff
  String webpage = get_file(html_header);
  webpage += get_file(html_apiservers);
  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + String(F(" API Servers")) );

  // create table rows first
  for(uint8_t i = 0; i < max_api_vservers; i++)
  {
    webpage += "table_add(" + String(i+1) + ");\n";
  }

  for(uint8_t i = 0; i < max_api_vservers; i++)
  {
    webpage += js_helper("api_vserver_hostname" + String(i+1), String(config.api_vserver_hostname[i]));

    webpage += js_radio_helper("api_venable1" + String(i+1), "api_venable0" + String(i+1), config.api_venable[i]);
  }
  for(uint8_t i = 0; i < max_api_iservers; i++)
  {
    webpage += js_helper("api_iserver_hostname" + String(i+1), String(config.api_iserver_hostname[i]));

    webpage += js_radio_helper("api_ienable1" + String(i+1), "api_ienable0" + String(i+1), config.api_ienable[i]);
  }

  webpage += js_radio_helper(F("api_lm75a1"), F("api_lm75a0"), config.api_lm75a);
  webpage += js_radio_helper(F("api_grid1"), F("api_grid0"), config.api_grid);
  webpage += js_helper(F("api_pollsecs"), String(config.api_pollsecs, 1));

  webpage += "table_add_submit('apitable');\n";

  webpage += web_footer();

  server.send(200, mime_html, webpage); // Send response
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
  String webpage =  get_file(html_header);
  webpage +=  get_file(html_stats);
  webpage += js_header();

  webpage += "\nrefresh(\"60\");\n";

  String tmps;

  String mymode = "";

  if (system_mode == 0)
    mymode += F("IDLE");
  else if (system_mode == 1)
    mymode += F("CHARGE");
  else if (system_mode == 2)
    mymode += F("DRAIN");
  else if (system_mode == 3)
    mymode += F("Both");

  webpage += js_helper_innerhtml(F("mode"), mymode);

  String daynight = "";
  if (config.m247)
  {
    if(!config.i_enable && !config.c_enable)
      daynight += F("NEITHER - 24/7 Mode");
    else if(config.i_enable && config.c_enable)
      daynight += F("BOTH - 24/7 Mode");
    else if(config.i_enable)
      daynight += F("NIGHT - 24/7 Mode");
    else if(config.c_enable)
      daynight += F("DAY - 24/7 Mode");
  }
  else if (!flags.day && !flags.night)
    daynight += F("NEITHER");

  else if (flags.day && flags.night)
    daynight += F("BOTH - overlap period");
  else if (flags.night)
    daynight += F("NIGHT");
  else if (flags.day)
    daynight += F("DAY");
  else
    daynight += "???";

  webpage += js_helper_innerhtml(F("daynight"), daynight);

  webpage += js_helper_innerhtml(F("modereason"), "<pre>" + mode_reason + "</pre>");

  String nu_string = next_update_string(3);

  // ----------------------------------------------------------------------

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + " Info");
  webpage += js_helper_innerhtml("pdesc", String(config.description));

  if(config.threephase)
  {
    String extra_info = "";

    extra_info += "<pre>";

    if(config.monitor_phase_a)
      extra_info += "* ";
    else
      extra_info += "  ";

    extra_info += "Phase A: " + String(phase_a_watts) + " watts, " + String(phase_a_voltage) + " volts\\n";

    if(config.monitor_phase_b)
      extra_info += "* ";
    else
      extra_info += "  ";

    extra_info += "Phase B: " + String(phase_b_watts) + " watts, " + String(phase_b_voltage) + " volts\\n";

    if(config.monitor_phase_c)
      extra_info += "* ";
    else
      extra_info += "  ";
    extra_info += "Phase C: " + String(phase_c_watts) + " watts, " + String(phase_c_voltage) + " volts";

    extra_info += "</pre>";

    webpage += js_helper_innerhtml(F("tphased"), extra_info);
  }
  else
  {
    webpage += js_hide("td3pd");
  }

  webpage += js_helper_innerhtml(F("watts"), String(phase_sum) + " Watts");

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

      cell_string += "Diff: " + String(cell_volt_diff, 4) + "v</pre>\n";
      cell_string += "Pack Total: " + String(pack_total_volts) + "v\n";
    }
    // single cell
    else if(config.cell_count == 1)
    {
      cell_string = String(cells_volts[0]) + "v";
    }

    webpage += js_helper_innerhtml(F("battery_voltage"), cell_string );
  }
  else
  {
    webpage+= js_hide("bvtr");
  }

  if(flags.lm75a)
  {
    webpage += js_helper_innerhtml(F("systemp"), "Now: " + String(board_temp,2) + "c, Min: " +  String(board_temp_min, 2) + "c, Max: " + String(board_temp_max,2));
  }
  else
  {
    webpage+= js_hide("sttr");
  }

  webpage += js_helper_innerhtml(F("next_update"), nu_string);

  webpage += web_footer();
  server.send(200, mime_html, webpage);
}

void ac_info()
{
  String webpage = get_file(html_header);
  webpage += get_file(html_acinfo);

  if(!config.threephase)
  {
    webpage += String(phase_a_watts ,1) + " Watts\n" + String(phase_a_voltage ,1) + " Volts\n"+ String(phase_a_watts/phase_a_voltage, 2) + " Amps\n\n";
  }
  else
  {
    webpage += "Phase A: " + String(phase_a_watts ,1) + " Watts, " + String(phase_a_voltage ,1) + " Volts, "+ String(phase_a_watts/phase_a_voltage, 2) + " Amps \n";
    webpage += "Phase B: " + String(phase_b_watts ,1) + " Watts, " + String(phase_b_voltage ,1) + " Volts, "+ String(phase_b_watts/phase_b_voltage, 2) + " Amps \n";
    webpage += "Phase C: " + String(phase_c_watts ,1) + " Watts, " + String(phase_c_voltage ,1) + " Volts, "+ String(phase_c_watts/phase_c_voltage, 2) + " Amps \n\n";

    webpage += "Phase Sum: " + String((phase_a_watts + phase_b_watts + phase_c_watts) ) + " Watts\n\n";
  }

  webpage += "Todays Usage: " + String(energy_consumed, 1) + " Kwh, $" + String(energy_consumed * config.cpkwh, 2) + "\n";
  webpage += "Yesterdays Usage: " + String(energy_consumed_old, 1) + " Kwh, $" + String(energy_consumed_old * config.cpkwh, 2) + "\n\n";

  // --------------------------
  // voltage high low tracking

  webpage += "Voltage Min and Max since boot.\n\n";
  webpage += "Phase A) Min: " + String(phase_a_voltage_low ,1) + "v Max: " + String(phase_a_voltage_high ,1) + "v Diff: " + String(phase_a_voltage_high - phase_a_voltage_low ,1) +"v\n";
  if(config.threephase)
  {
    webpage += "Phase B) Min: " + String(phase_b_voltage_low ,1) + "v Max: " + String(phase_b_voltage_high ,1) + "v Diff: " + String(phase_b_voltage_high - phase_b_voltage_low ,1) +"v\n";
    webpage += "Phase C) Min: " + String(phase_c_voltage_low ,1) + "v Max: " + String(phase_c_voltage_high ,1) + "v Diff: " + String(phase_c_voltage_high - phase_c_voltage_low ,1) +"v\n";
  }
  // --------------------------


  webpage += "\n</pre></center>\n";

  webpage += js_header();
  webpage += "\nrefresh(\"5\");\n";

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + " AC Info");

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}

void sys_info()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_sys_info);
  webpage += js_header();

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
  String webpage = get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, "LED ON");
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += "\nredir(\"/\", \"15\");\n";

  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
    set_led(1);
}

void led_off()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, "LED OFF" );
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += "\nredir(\"/\", \"15\");\n";
  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
    set_led(0);
}

void led_toggle()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, "LED TOGGLE" );
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += "\nredir(\"/\", \"15\");\n";
  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
    set_led(2);
}

void led_blink()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, "LED BLINK" );
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += "\nredir(\"/\", \"15\");\n";
  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
    config.blink_led = 1;
}

// mode time
void modet()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_modet);
  webpage += js_header();

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}

void mode_set()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  uint8_t mode_temp = 0;  // default to idle
  uint16_t manual_duration = 0;

  if (server.args() > 0 )
  {
    for ( uint8_t i = 0; i < server.args(); i++ )
    {
      if (server.argName(i) == F("mode"))
        mode_temp = server.arg(i).toInt();
      else if (server.argName(i) == F("manual_duration"))
        manual_duration = server.arg(i).toInt();
    }
  }

  if(config.webc_mode)
  {
    String mode_type;

    if(mode_temp == 0)
    {
      old_night_reason = mode_reason;
      old_day_reason = mode_reason;

      mode_type = "Idle";
    }
    else if(mode_temp == 1)
    {
      old_day_reason = mode_reason;
      mode_type = "Charge";

      flags.shutdown_hvolt = 0;
      flags.shutdown_lvolt = 0;
      timers.charger_off = 0;
    }
    else if(mode_temp == 2)
    {
      old_night_reason = mode_reason;
      mode_type = "Drain";

      flags.shutdown_hvolt = 0;
      flags.shutdown_lvolt = 0;
      timers.inverter_off = 0;
    }
    else if(mode_temp == 3)
    {
      old_day_reason = mode_reason;
      old_night_reason = mode_reason;
      mode_type = "Both";

      flags.shutdown_hvolt = 0;
      flags.shutdown_lvolt = 0;
      timers.inverter_off = 0;
      timers.charger_off = 0;
    }
    else
    {
      mode_type = "Idle";
      mode_temp = 0;
    }

    mode_reason = "Manual "+ mode_type +" On";

    webpage += js_helper_innerhtml(title_str, mode_reason);

    modeset(mode_temp, 1, manual_duration);
  }
  else
  {
    webpage += js_helper_innerhtml(title_str, denied_str);
  }

  webpage += "\nredir(\"/\", \"15\");\n";
  webpage += web_footer();

  server.send(200, mime_html, webpage);
}

String web_footer()
{
  String webpage;

  webpage += F("</script>");

  webpage += get_file(html_footer);

  webpage += js_helper_innerhtml(F("fsystem_msgs"), system_msgs);

  webpage += F("</script>\n");

  for(uint8_t i = 0; i < 16; i++)
    webpage += F("<br>\n");

  webpage += F("</body>\n</html>");

  return webpage;
}



void force_refresh()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, F("Forcing Refresh"));
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += "\nredir(\"/\", \"15\");\n";
  webpage += web_footer();

  server.send(200, mime_html, webpage);

  if(config.webc_mode)
  {
    timers.mode_check = 0;
    timers.pgrid = 0;
    timers.lv_shutdown = 0;
    timers.hv_shutdown = 0;

    timers.charger_off = 0;
    timers.inverter_off = 0;


    // sync_time();
  }
}

void software_reset()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();


  // avoid restarts being triggered after boot.
  time_t timetmp = now();
  if (timeStatus() == timeNotSet ||  (hour(timetmp) < 1 && minute(timetmp) < 2))
  {
    webpage += js_helper_innerhtml(title_str, F("System Just Booted"));
    server.send(200, mime_html, webpage);
    return;
  }

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, F("Software Reboot"));
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += "\nredir(\"/\", \"5\");\n";
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
  String webpage = get_file(html_header);
  webpage += get_file(html_port_config);
  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, String(config.hostn));

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

  webpage += "\nredir(\"/\", \"10\");\n";
  webpage += web_footer();

  server.send(200, mime_html, webpage);

}

void port_cfg_submit()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_mode);
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
    }

    save_config();
    flags.restart = 1;
  }


  webpage += "\nredir(\"/\", \"10\");\n";
  webpage += web_footer();

  server.send(200, mime_html, webpage);
}

void web_copy_config()
{
  String webpage = get_file(html_header);
  webpage += get_file(html_cpconfig);
  webpage += js_header();

  webpage += "\nredir(\"/\", \"10\");\n";
  webpage += web_footer();

  server.send(200, mime_html, webpage);
}

void copy_config_submit()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, F("Copying Config"));

  webpage += "\nredir(\"/\", \"10\");\n";
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
  String webpage = get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  String pre = "";
  check_for_update();

  pre += F("Current: ");
  pre += FW_VERSION;
  pre += F("\\nLatest: ");
  pre += remote_version;

  String link_html = "";

  if(remote_version <= FW_VERSION)
    link_html = F("No update found.");
  else
    link_html = F("<a href=/do_update>Upgrade Now</a>");

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + F(" Update Check"));
  webpage += js_helper_innerhtml(F("pre"), pre);
  webpage += js_helper_innerhtml(F("link"), link_html);
  webpage += js_helper_innerhtml(F("link2"), "<a href=" + String(config.pub_url) + "/changelog.txt>Change Log</a>");

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


void do_update_web()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  String pre;

  if(!flags.update_found)
  {
    pre = String("No Update Found...");
  }
  else
  {
    pre = "Updating to " + String(remote_version);
    pre += String("\\nDEVICE WILL RESTART SOON.");
  }

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + F(" Updating"));
  webpage += js_helper_innerhtml(F("pre"), pre);

  webpage += "\nredir(\"/\", \"45\");\n";
  webpage += web_footer();

  server.send(200, mime_html, webpage);

  flags.update_self = 1;

  return;
}


void upload_config()
{
  String webpage = get_file(html_header);
  webpage += get_file(html_upload_config);
  webpage += js_header();

  if(config.webc_mode)
    webpage += js_helper_innerhtml(title_str, String(config.hostn) + F(" Upload Config"));
  else
    webpage += js_helper_innerhtml(title_str, denied_str);

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


void upload_config_submit()
{
  String webpage =  get_file(html_header);
  webpage += get_file(html_mode);
  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, F("Saving Uploaded Config"));

  webpage += "\nredir(\"/\", \"10\");\n";
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

  uint8_t local_cell_count = mmaths.mmin<uint8_t>(config.cell_count, (uint8_t) MAX_CELLS);

  for(byte i = 0; i< local_cell_count; i++)
    webpage += String(cells_volts[i], 6) + ",";

  server.send(200, mime_txt, webpage);
}

void timers_page()
{
  String webpage = datetime_str(0, '-', 'T', ':') + String("\n\n");

  webpage += String("mode_check:\t") + timer_millis_to_string(timers.mode_check) + "\n";
  webpage += String("lv_shutdown:\t") + timer_millis_to_string(timers.lv_shutdown) + "\n";
  webpage += String("hv_shutdown:\t") + timer_millis_to_string(timers.hv_shutdown) + "\n";
  webpage += String("charger_off:\t") + timer_millis_to_string(timers.charger_off) + "\n";
  webpage += String("inverter_off:\t") + timer_millis_to_string(timers.inverter_off) + "\n";
  webpage += String("pgrid:\t\t") + timer_millis_to_string(timers.pgrid) + "\n";
  webpage += String("update_check:\t") + timer_millis_to_string(timers.update_check) + "\n";
  webpage += String("ntp_sync:\t") + timer_millis_to_string(timers.ntp_sync) + "\n";
  webpage += String("use_fallback:\t") + timer_millis_to_string(timers.use_fallback) + "\n";

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

    uint8_t local_cell_count = mmaths.mmin<uint8_t>(config.cell_count, (uint8_t) MAX_CELLS);

    webpage += "Cell Series Volts:\n";
    for(byte i = 0; i< local_cell_count; i++)
      webpage += String(i+1) + ") " + String(cells_volts_real[i], 6) + "v\n";

    webpage += "\n\n";

    webpage += "Cell Volts:\n\n";
    for(byte i = 0; i< local_cell_count; i++)
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


    webpage += "\n\nDay Flag: " + String(flags.day) + "\n";
    webpage += "Night Flag: " + String(flags.night) + "\n";

    webpage += "\n";
  }

  server.send(200, mime_txt, webpage);
}

void adc_info_raw()
{
  String webpage = "#\tEN\tVal\n";

  for(byte i = 0; i< adsmux.ain_count; i++)
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
        if(config.board_rev == 2)
          webpage += "LM75a:\t\t0x" + String(address, HEX) + "\n";
        else
          webpage += "ADS1115:\t0x" + String(address, HEX) + " (or ADS1015 if v3 board)\n";
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
      else if (address == 0x4D || address == 0x4e)
      {
        webpage += "MCP3021:\t0x" + String(address, HEX) + " (or MCP3221, check board text)\n";
      }
      else
        webpage += "Unknown:\t0x" + String(address, HEX) + "\n";
    }
  }

  server.send(200, mime_txt, webpage);
}


void web_issue_submit()
{
  String webpage = get_file(html_header);
  webpage += get_file(html_mode);
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
//         Serial.println(F("Unknown arg"));
      }
    }
  }

  webpage += "\nredir(\"/\", \"10\");\n";
  webpage += web_footer();

  server.send(200, mime_html, webpage);
}

void web_issue_log()
{
  String webpage = get_file(html_header);
  webpage += get_file(html_issue_log);
  webpage += js_header();

  webpage += js_helper_innerhtml(title_str, String(config.hostn) + " SysLog");
  webpage += js_helper_innerhtml(F("log"), get_file(txt_log_system));

  webpage += js_radio_helper(F("clear1"), F("clear0"), 0);

  webpage += web_footer();

  server.send(200, mime_html, webpage);
}


