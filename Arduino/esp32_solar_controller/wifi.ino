// =================================================================================================================
// Wifi Functions
// =================================================================================================================

bool ap_start()
{
  /* You can remove the password parameter if you want the AP to be open. */
//   WiFi.setSleepMode(WIFI_NONE_SLEEP); // always put AP in high power mode

  bool result = WiFi.softAP("SolarAP");

  if(!result)
  {
    return result;
  }

  flags.access_point = 1;

  return result;
}

bool wifi_start()
{
  if (flags.access_point)
  {
    Serial.println("IN AP Mode (this shouldnt be called)");
    return 0;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WL_CONNECTED");
    return 1;
  }

  Serial.println("wifi init");
//   WiFi.persistent( false ); // dont save wifi settings
  //WiFi.mode(WIFI_STA);

  Serial.println("wifi connect " + String(config.wifi_ssid1) );
  if(wifi_connect(config.wifi_ssid1, config.wifi_pass1) )
    return 1;

  // fallback
  Serial.println("fallback connect " + String(config.wifi_ssid2) );
  if(wifi_connect(config.wifi_ssid2, config.wifi_pass2))
    return 1;

  // hard coded fallback
  Serial.println("wifi_start: using hardcoded wifi");
  if(wifi_connect((char*)"wififallback", (char*)"password") )
    return 1;

  return 0;
}

bool wifi_connect(const char s[ssmall], const char p[ssmall])
{
  if(!strlen(s) || !strlen(p))
  {
    Serial.println("wifi_connect blank ssid/ pass");
    return 0;
  }

  Serial.println("wifi_connect SSID: " + String(s) + ", Pass: " + String(p) );

  WiFi.mode(WIFI_STA);
  WiFi.begin(s, p);
  //   WiFi.hostname(config.hostn);

  int recon = 0;

  Serial.print("Wifi Connect: " + String(s));

  while (WiFi.status() != WL_CONNECTED)
  {
    if (recon > 150)
    {
      both_println(F("Wifi Fail"));
      return 0;
    }

    recon++;

    if(config.pin_led != OPT_DISABLE)
      set_led(3);

    Serial.print(F("."));
    if(recon % 25 == 0)
    {
      Serial.println(String(s));
    }

    if(oled_enabled())
    {
      oled_clear();
      oled_set2X();
      oled_println(F("Wifi\nConnect"));
      oled_set1X();
      oled_println(String(s) + " " + String(recon) );
      oled_set2X();

      // spinner
      if (recon % 4 == 0)
        oled_println(F("\\"));
      else if (recon % 4 == 1)
        oled_println(F("|"));
      else if (recon % 4 == 2)
        oled_println(F("/"));
      else if (recon % 4 == 3)
        oled_println(F("-"));
    }
    delay(50);
  }
  oled_clear();
  both_println(F("WiFi OK"));
  both_print_ip();

  set_led(config.led_status);

  //   WiFi.hostname(config.hostn);

  return 1;
}
