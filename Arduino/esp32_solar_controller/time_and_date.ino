// =================================================================================================================
// Time Functions
// =================================================================================================================

bool sync_time()
{
  const String ntp_prefix = "NTP: ";

  oled_clear();
  oled_set2X();
  both_println(ntp_prefix + "Sync");
  oled_set1X();

  IPAddress timeServerIP;

  WiFi.hostByName(config.ntp_server, timeServerIP);
  sendNTPpacket(timeServerIP);
  unsigned long stime = millis() + 2000;

  while (stime >= millis())
  {
    server.handleClient();
    yield();
  }

  int size = udp.parsePacket();
  if (size < NTP_PACKET_SIZE)
  {
    const String tmp = "undersized packet";
    log_error_msg(ntp_prefix + tmp);
    both_println(tmp);
    return 0;
  }

  udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

  if(packetBuffer[40] == 0 && packetBuffer[41] == 0 && packetBuffer[42] == 0 && packetBuffer[43] == 0)
  {
    const String tmp = "null packets";
    both_println(tmp);
    log_error_msg(ntp_prefix + tmp);
    return 0;
  }

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

  unsigned long secsSince1900 = highWord << 16 | lowWord; // combine the four bytes (two words) into a long integer
  unsigned long epoch = secsSince1900 - 2208988800UL;
  epoch += (60 * 60 * int_to_gmt(config.gmt));

  secsSince1900 = now(); // reuse var
  if(epoch < secsSince1900 || epoch < 1567099782)
  {
    const String tmp = "bad epoch";
    both_println(tmp);
    log_error_msg(ntp_prefix + tmp);
    return 0;
  }

  setTime(epoch);

  const String tmp = "Success";
  both_println(tmp);
  log_error_msg(ntp_prefix + tmp);

  time_synced = 1;
  return 1;
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress & address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

// used to convert int to GMT offset
float int_to_gmt(const int v)
{
  float tmp = float(v) / 2.0;
  tmp -= 12;

  return tmp;
}

String next_update_string(const byte len) // ; len = length
{
  String nu_string = "";
  long next_update = update_time - millis();
  if (next_update > 1000)
  {
    int nu_hours = (next_update / (1000 * 60 * 60)) % 24;
    int nu_minutes = (next_update / (1000 * 60)) % 60;
    int nu_seconds = (next_update / 1000) % 60;

    if (len == 0)
      nu_string = String(nu_minutes) + " minutes " + String(nu_seconds) + " seconds";
    else if (len == 1)
      nu_string = String(nu_minutes) + " min " + String(nu_seconds) + " sec";
    if (len == 2)
      nu_string = String(nu_hours) + "h "  +  String(nu_minutes) + "m " + String(nu_seconds) + "s";
  }
  else
  {
    nu_string = "NOW";
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
String datetime_str(byte type, char ds, char dts, char ts)
{
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
  night_time = 0;
  day_time = 0;

  if(config.m247)
  {
    night_time = 1;
    day_time = 1;
    return;
  }

  time_t timetmp = now();
  byte myhour = hour(timetmp);

  if (in_time_range(myhour, config.c_start_h, config.c_finish_h) ) // day
    day_time = 1;

  if (in_time_range(myhour, config.i_start_h, config.i_finish_h) ) // night
    night_time = 1;
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
