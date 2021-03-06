void download_html_from_remote()
{

  const int asize = 16;
  String dl_array[asize] =
  {
    html_config,
    txt_system_messages,
    html_stats,
    html_mode,
    html_advance_config,
    html_footer,
    html_js_header,
    css_file,
    html_issue_log,
    html_cpconfig,
    html_port_config,
    html_net_config,
    html_datasrcs,
    html_sys_info,
    html_upload_config,
    html_timer
  };

  if(download_index > asize-1)
  {
    config.download_html = 0;
    save_config();
    return;
  }

  if(!config.download_html)
    return;

  get_html_and_save(dl_array[download_index]);
  return;

}

void get_html_and_save(String filepath)
{
  oled_clear();
  oled_println(F("Updating\nHTML\ngetting:"));
  oled_println(filepath);

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(httpget_timeout);

  String url = String(config.update_url) + "/data" + filepath;

  File f = SPIFFS.open(filepath.c_str(), "w");
  if (f)
  {
    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        http.writeToStream(&f);
        download_index++;
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
}

int remote_version = 0;
String check_for_update()
{
  String message = "";
  //   found_update = 0;

  if(found_update == 1)
  {
    return String(F("Update Found (cached)"));
  }

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(httpget_timeout);

  String url_tmp = String(config.update_url) + "/cv.txt";

  http.begin(client, url_tmp);

  int httpCode = http.GET();
  if( httpCode == 200 )
  {
    remote_version = http.getString().toInt();

    message += F("Current: ");
    message += FW_VERSION;
    message += F("\\nLatest: ");
    message += remote_version;

    if(remote_version > FW_VERSION)
      found_update = 1;
  }
  else
  {
    message += F("HTTP ERROR: ");
    message += httpCode;
  }

  http.end();
  return message;
}

void do_update()
{
  check_for_update();
  self_update = 0;

  if(!found_update)
  {
    oled_clear();
    both_println(F("No update"));
    return;
  }

  const String tmp_str = "Upgrade to " + String(remote_version);
  log_issue(tmp_str);

  modeset(0);
  ports_off();

  oled_clear();

  config.download_html = 1;
  save_config();

  oled_clear();
  both_println(F("Update\nFirmware"));

  String fwbin = "/firmware.bin";
  String url = String(config.update_url) + fwbin;

  WiFiClient client;
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

  t_httpUpdate_return ret = ESPhttpUpdate.update(client, url );

  switch(ret)
  {
    case HTTP_UPDATE_FAILED:
      oled_clear();
      both_println(F("ERROR"));
      config.download_html = 0;
      log_error_msg(ESPhttpUpdate.getLastError() + ESPhttpUpdate.getLastErrorString().c_str());
      //       if(serial_on)
      //         Serial.printf("UPDATE Error (%d): %s",  ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      //       if(serial_on)
      //         Serial.println(F("NO_UPDATES"));
      break;

    case HTTP_UPDATE_OK:
      oled_clear();
      both_println(F("OK"));
      break;
  }
}

/*
void do_update_old()
{
  check_for_update();
  self_update = 0;

  if(!found_update)
  {
    oled_clear();
    both_println(F("No update"));
    return;
  }

  const String tmp_str = "Upgrade to " + String(remote_version);
  log_issue(tmp_str);

  modeset(0);
  ports_off();

  oled_clear();

  config.download_html = 1;
  save_config();

  oled_clear();
  both_println(F("Update\nFirmware"));

  String fwbin = "/firmware.bin";
  String url = String(config.update_url) + fwbin;

  WiFiClient client;

  t_httpUpdate_return ret = httpUpdate.update(client, url );

  switch(ret)
  {
    case HTTP_UPDATE_FAILED:
      oled_clear();
      both_println(F("ERROR"));
      config.download_html = 0;
      log_error_msg(httpUpdate.getLastError() + httpUpdate.getLastErrorString().c_str());
      Serial.printf("UPDATE Error (%d): %s",  httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println(F("NO_UPDATES"));
      break;

    case HTTP_UPDATE_OK:
      oled_clear();
      both_println(F("OK"));
      break;
  }
}

*/

