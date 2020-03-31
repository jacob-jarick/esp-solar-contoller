void download_html_from_remote()
{
  if(!config.download_html)
    return;

  if(flags.update_self) // shouldnt happen BUUUT - avoid downloading new HTML while trying to do firmware updates (should happen after reboot)
    return;

  const int asize = 21;
  String dl_array[asize] =
  {
    html_config,
    txt_system_messages,
    html_stats,
    html_mode,
    html_advance_config,
    html_battery,
    html_footer,
    html_js_header,
    css_file,
    html_issue_log,
    html_cpconfig,
    html_port_config,
    html_net_config,
    html_datasrcs,
    html_ntc10k_config,
    html_ntc10k_info,
    html_sys_info,
    html_upload_config,
    html_battery_info,
    html_timer,
    html_calibrate
  };

  if(download_index > asize-1)
  {
    config.download_html = 0;
    flags.save_config = 1;
    return;
  }

  bool ok = get_html_and_save(dl_array[download_index]);

  if(!ok)
  {
    config.download_html = 0;
    flags.save_config = 1;
    log_issue("Error fetching latest html files.");
  }

  return;
}

bool get_html_and_save(String filepath)
{
  oled_clear();
  oled_println(F("Updating\nHTML\ngetting:"));
  oled_println(filepath);

  HTTPClient http;
  http.setTimeout(httpget_timeout);

  String url = String(config.pub_url) + "/data" + filepath;

  File f = SD.open(filepath.c_str(), FILE_WRITE);
  if (f)
  {
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        http.writeToStream(&f);
        download_index++;
        return 1;
      }
      else
      {
        both_println("HTTP BAD Code:" + String(httpCode, DEC) );
//         both_println(http.errorToString().c_str());
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
  else
  {
    Serial.println("get_html_and_save: cannot open '" + filepath + "' to update.");
  }

  return 0;
}

int remote_version = 0;
String check_for_update()
{
  String message = "";
  //   flags.update_found = 0;

  if(flags.update_found == 1)
  {
    return String(F("Update Found (cached)"));
  }

//   WiFiClient client;
  HTTPClient http;
  http.setTimeout(httpget_timeout);

  String url_tmp = String(config.pub_url) + "/cv.txt";

  http.begin(url_tmp);

  int httpCode = http.GET();
  if( httpCode == 200 )
  {
    remote_version = http.getString().toInt();

    message += F("Current: ");
    message += FW_VERSION;
    message += F("\\nLatest: ");
    message += remote_version;

    if(remote_version > FW_VERSION)
      flags.update_found = 1;
  }
  else
  {
    message += F("HTTP ERROR: ");
    message += httpCode;
  }

  http.end();
  return message;
}

uint8_t update_trys = 0;
void do_update()
{
  check_for_update();
  flags.update_self = 0;

  if(!flags.update_found)
  {
    oled_clear();
    both_println(F("No update"));
    return;
  }

  update_trys++;

  const String tmp_str = "Upgrade to " + String(remote_version);
  log_issue(tmp_str);

  modeset(0);

  oled_clear();

  both_println("Update\nFirmware\n" + String(update_trys));

  String url = String(config.pub_url) + String("/firmware.bin");

  t_httpUpdate_return ret = ESPhttpUpdate.update(url);

  config.download_html = 1;
  save_config();

  if(ret == HTTP_UPDATE_FAILED)
  {
    oled_clear();
    both_println(F("ERROR"));
    config.download_html = 0;
    log_error_msg(String("Update Error (") + ESPhttpUpdate.getLastError() + String("): ") + ESPhttpUpdate.getLastErrorString().c_str());
    Serial.printf("UPDATE Error (%d): %s",  ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
    config.download_html = 0;

    if(update_trys < 20)
    {
      flags.update_self = 1;  // retry
    }
    else
    {
      config.download_html = 0;
      save_config();
      flags.update_self = 0;
    }
  }
  else if(ret == HTTP_UPDATE_NO_UPDATES)
  {
    config.download_html = 0;
    save_config();
    flags.update_self = 0;

    both_println(F("NO_UPDATES"));
  }
  else if (ret == HTTP_UPDATE_OK)
  {
    oled_clear();
    both_println(F("OK"));
  }
}

