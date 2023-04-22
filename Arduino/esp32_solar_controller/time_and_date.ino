// =================================================================================================================
// Time Functions
// =================================================================================================================

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(config.ntp_server, ntpServerIP);
  Serial.print(config.ntp_server);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + (int_to_gmt(config.gmt) * SECS_PER_HOUR);
    }
  }
  //Serial.println("No NTP Response :-(");
  log_msg("No NTP Response");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


// used to convert int to GMT offset
float int_to_gmt(const uint8_t v)
{
  float tmp = float(v) / 2.0;
  tmp -= 12;

  return tmp;
}

String next_update_string(const byte len) // ; len = length
{
  String nu_string = "";
  long next_update = timers.mode_check - millis();
  if (next_update > 1000)
  {
    int nu_hours = (next_update / (1000 * 60 * 60)) % 24;
    int nu_minutes = (next_update / (1000 * 60)) % 60;
    int nu_seconds = (next_update / 1000) % 60;

    if (len == 0)
    {
      nu_string = String(nu_minutes) + " minutes " + String(nu_seconds) + " seconds";
    }
    else if (len == 1)
    {
      nu_string = String(nu_minutes) + " min " + String(nu_seconds) + " sec";
    }
    if (len == 2)
    {
      nu_string = String(nu_hours) + "h "  +  String(nu_minutes) + "m " + String(nu_seconds) + "s";
    }
    if (len == 3) // auto
    {
      nu_string = "";

      if(nu_hours > 0)
        nu_string += String(nu_hours) + "h ";

      if(nu_minutes > 0)
        nu_string += String(nu_minutes) + "m ";

      nu_string += String(nu_seconds) + "s";
    }
  }
  else
  {
    nu_string = "NOW";
  }

  return nu_string;
}


String timer_millis_to_string(unsigned long ms) // ; len = length
{
  String nu_string = "";

  unsigned long nowms = millis();

  if(ms <= nowms)
    return "X";
  else
    ms -= nowms;

  if (ms > 1000)
  {
    int nu_hours = (ms / (1000 * 60 * 60)) % 24;
    int nu_minutes = (ms / (1000 * 60)) % 60;
    int nu_seconds = (ms / 1000) % 60;

//      nu_string = String(nu_minutes) + " minutes " + String(nu_seconds) + " seconds";

    if(nu_hours)
      nu_string += String(nu_hours) + " Hours ";

    if(nu_minutes)
      nu_string += String(nu_minutes) + " min ";

    nu_string += String(nu_seconds) + " sec";
//      nu_string = String(nu_hours) + "h "  +  String(nu_minutes) + "m " + String(nu_seconds) + "s";
  }
  else
  {
    nu_string = "<1 sec";
  }

  return nu_string;
}

// type
// 0 = date + time
// 1 = ?
// 2 = oled HH:MM
// 3 = HH:MM:SS
// ds = date seperator char: / -
// dts = date time seperator " "
// ts = time seperator :
String datetime_str(const byte type, const char ds, const char dts, const char ts)
{
  if(year() == 1970)
    return "BOOT";

  String res = "";

  // -------------------------------------
  if(type == 0)
  {
    int y = year();
    res += String(y);
    res += ds;
  }

  // -------------------------------------
  if(type == 0)
  {
    byte mm = month();
    if(mm < 10)
      res += "0";

    res += String(mm);
    res += ds;
  }

  // -------------------------------------
  if(type == 0)
  {
    byte d = day();
    if(d < 10)
      res += "0";

    res += String(d);

    res += dts;
  }

  // -------------------------------------
  if(type == 0 || type == 1 || type == 2 || type == 3)
  {
    byte h = hour();
    if(h < 10)
    {
      if(type == 2) // oled mode uses space on hour (only)
        res += " ";
      else
        res += "0";
    }

    res += String(h);


    res += ts;
  }
  // -------------------------------------
  if(type == 0 || type == 1 || type == 2 || type == 3)
  {
    byte m = minute();
    if(m < 10)
      res += "0";

    res += String(m);

    if(type != 2) // dont add time seperator
      res += ts;
  }
  // -------------------------------------
  if(type == 0 || type == 1 || type == 3)
  {
    byte s = second();
    if(s < 10)
      res += "0";
    res += String(s);
  }

  return res;
}


void set_daynight()
{
  flags.night = 0;
  flags.day = 0;

  if(config.m247)
  {
    if(config.i_enable)
      flags.night = 1;

    if(config.c_enable)
      flags.day = 1;

    return;
  }

  time_t timetmp = now();
  byte myhour = hour(timetmp);

  if (config.c_enable && in_time_range(myhour, config.c_start_h, config.c_finish_h) ) // day
    flags.day = 1;

  if (config.i_enable && in_time_range(myhour, config.i_start_h, config.i_finish_h) ) // night
    flags.night = 1;

  /*
  if(config.prefer_dc)
    flags.night = 1;
  */
}

bool in_time_range(const int h, const int t_start, const int t_end)
{
  if
    (
      t_start == t_end ||
      (t_start < t_end && h >= t_start && h < t_end) ||   // eg 7am to 5pm
      (t_start > t_end && (h >= t_start || h < t_end ) )   // eg start 10pm, finish 8am
    )
    return 1;

  return 0;
}
