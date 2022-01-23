void download_html_from_remote()
{
  if(!flags.download_html)
    return;

  if(flags.update_self) // shouldnt happen BUUUT - avoid downloading new HTML while trying to do firmware updates (should happen after reboot)
    return;

  const int asize = 20;
  String dl_array[asize] =
  {
    html_header,
    html_stats,
    html_mode,
    html_apiservers,
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
    html_acinfo,
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
uint16_t check_for_update()
{
  if(flags.update_found)
    return remote_version;

  if(get_remote_version() > FW_VERSION)
    flags.update_found = 1;

  return remote_version;
}

uint16_t get_remote_version()
{
  if(remote_version > FW_VERSION)
    return remote_version;

  String url_tmp = String(config.pub_url) + "/cv.txt";
  String result = "";

  if(get_url(url_tmp, result))
    remote_version = result.toInt();

  return remote_version;
}

uint8_t update_trys = 0;
void do_update()
{
  flags.update_self = 0;

  if(get_remote_version() <= FW_VERSION)
  {
    oled_clear();
    both_println(F("No update"));
    return;
  }

  oled_clear();
  both_println("Update\nFirmware\ntry " + String(update_trys));
  log_issue(String("Upgrade to ") + String(remote_version));

  modeset(0);
  update_trys++;

  t_httpUpdate_return ret = ESPhttpUpdate.update(String(config.pub_url) + "/firmware.bin" );

//   save_config();

  if(ret == HTTP_UPDATE_FAILED)
  {
    oled_clear();
    both_println(F("UPDATE\nERROR"));
    log_msg(String("OTA Update Error (") + ESPhttpUpdate.getLastError() + String("): ") + ESPhttpUpdate.getLastErrorString().c_str());

    if(update_trys < 10)
      flags.update_self = 1;  // retry
  }
  else if(ret == HTTP_UPDATE_NO_UPDATES)
  {
    oled_clear();
    both_println(F("NO_UPDATES"));
  }
  else if (ret == HTTP_UPDATE_OK)
  {
    oled_clear();
    both_println(F("OK"));
  }
}

