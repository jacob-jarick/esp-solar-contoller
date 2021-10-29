void download_html_from_remote()
{
  if(!flags.download_html)
    return;

  if(flags.update_self) // shouldnt happen BUUUT - avoid downloading new HTML while trying to do firmware updates (should happen after reboot)
    return;

  const int asize = 19;
  String dl_array[asize] =
  {
    html_header,
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
    html_sys_info,
    html_upload_config,
    html_battery_info,
    html_calibrate,
    html_3pinfo,
    html_config
  };

  if(download_index > asize-1)
  {
    flags.download_html = 0;
    flags.save_config = 1;
    return;
  }

  bool ok = get_html_and_save(dl_array[download_index]);

  if(!ok)
  {
    flags.download_html = 0;
    flags.save_config = 1;
    log_issue("Error fetching latest html files.");
  }
  else
  {
    download_index++;
  }

  return;
}


bool get_html_and_save(String filepath)
{
  oled_clear();
  oled_println(F("Updating\nHTML\ngetting:"));
  oled_println(filepath);

  String url = String(config.pub_url) + "/data" + filepath;

  return get_url_and_save(url, filepath);
}

uint16_t remote_version = 0;
String check_for_update()
{
  if(flags.update_found == 1)
    return String(F("Update Found (cached)"));

  get_remote_version();

  if(remote_version <= FW_VERSION)
    return String("No Updates");

  String message = "";
  message += F("Current: ");
  message += FW_VERSION;
  message += F("\\nLatest: ");
  message += remote_version;

  return message;
}

void get_remote_version()
{
  if(flags.update_found == 1)
    return;

  String url_tmp = String(config.pub_url) + "/cv.txt";
  String result = "";

  if(get_url(url_tmp, result))
  {
    remote_version = result.toInt();

    if(remote_version > FW_VERSION)
      flags.update_found = 1;
  }
  return;
}

uint8_t update_trys = 0;
void do_update()
{
  if(!flags.update_found)
    get_remote_version();

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

  const String url = String(config.pub_url) + String("/firmware.bin");

  t_httpUpdate_return ret = ESPhttpUpdate.update(url);

//   flags.download_html = 1;
  save_config();

  if(ret == HTTP_UPDATE_FAILED)
  {
    oled_clear();
    both_println(F("ERROR"));
//     flags.download_html = 0;
    log_msg(String("Update Error (") + ESPhttpUpdate.getLastError() + String("): ") + ESPhttpUpdate.getLastErrorString().c_str());
//     Serial.printf("UPDATE Error (%d): %s",  ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
//     flags.download_html = 0;

    if(update_trys < 10)
    {
      flags.update_self = 1;  // retry
    }
    else
    {
      flags.update_self = 0;
//       flags.download_html = 0;

//       save_config();
    }
  }
  else if(ret == HTTP_UPDATE_NO_UPDATES)
  {
//     flags.download_html = 0;
    flags.update_self = 0;

//     save_config();
    both_println(F("NO_UPDATES"));
  }
  else if (ret == HTTP_UPDATE_OK)
  {
    oled_clear();
    both_println(F("OK"));
  }
}

