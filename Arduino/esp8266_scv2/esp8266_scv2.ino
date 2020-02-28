/*
Esp8266 Solar Controller
by Jacob Jarick, mem.namefix@gmail.com

Arduino ide settings:

 ESP12e module
 - 160mhz
 - Flash Size: 4MB (FS 3MB, OTA ~512kb)
 - v2 higher bandwidth
 - SSL Basic
 - Exceptions Disabled

 ESP 01 / sonoff switch
 - for board use: LOLIN(WEMOS) D1 Mini Lite
 - 160mhz
 - flash size 1mb, 64kb SPIFFS
 - v2 higher bandwidth
 - SSL Basic
 - Exceptions Disabled

 */
#define FW_VERSION 7

#define DAVG_MAGIC_NUM -12345678

// to longer timeout = esp weirdness
#define httpget_timeout 5000

// 10 min
#define check_timeout 600000


#include <Arduino.h>
#include <FS.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <WiFiUdp.h>

#include <ArduinoJson.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
ESP8266WebServer server(80);

#include <ESP8266httpUpdate.h>


// -----------------------------------------------------------------------------------------

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

SSD1306AsciiWire oled;

// -----------------------------------------------------------------------------------------

#include <TimeLib.h>

const unsigned int localPort = 2390;      // local port to listen for UDP packets
const byte NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP udp;

//------------------------------------------------------------------------------

// #include "EasyBuzzer.h"

//------------------------------------------------------------------------------
// timers

unsigned long update_time = 0;
unsigned long time_since_check = 0;
unsigned long last_update = 0;
unsigned long use_fallback = 0;

unsigned long lv_shutdown_time = 0;
unsigned long hv_shutdown_time = 0;

unsigned long charger_off_time = 0;
unsigned long inverter_off_time = 0;

//------------------------------------------------------------------------------
// FLAGS

bool restart_trigger = 0;
bool ap_mode = 0;
bool self_update = 0;
bool time_synced = 0;
bool night_time = 0;
bool day_time = 0;
bool low_voltage_shutdown = 0;
bool charger_high_voltage_shutdown = 0;
bool high_temp_shutdown = 0;
bool found_update = 0;

//------------------------------------------------------------------------------

const String mime_html    = "text/html";
const String mime_txt     = "text/plain";
const String http_str     = "http://";
const String denied_str   = "Denied";
const String title_str    = "title";

const String dothtml        = ".htm";
const String dottxt        = ".txt";

const String html_config          = "/config" + dothtml;
const String html_timer           = "/timer" + dothtml;
const String html_cpconfig        = "/cpconfig" + dothtml;
const String html_stats           = "/stats" + dothtml;
const String html_mode            = "/mode" + dothtml;
const String html_advance_config  = "/advance" + dothtml;
const String html_footer          = "/footer" + dothtml;
const String html_js_header       = "/jsheader" + dothtml;
const String html_issue_log       = "/issue" + dothtml;
const String html_net_config      = "/network" + dothtml;
const String html_datasrcs        = "/datasrcs" + dothtml;
const String html_sys_info        = "/system" + dothtml;
const String html_upload_config   = "/up_conf" + dothtml;
const String html_port_config     = "/portconf" + dothtml;

const String css_file             = "/style.css";
const String txt_log_system       = "/log" + dottxt;
const String txt_system_messages  = "/msg" + dottxt;
const String txt_passwd_file      = "/passwd";
const String json_config_file     = "/config.jsn";

//------------------------------------------------------------------------------

const byte size_error_msgs = 255;
String error_msgs = "";

float phase_a_watts = 0;
float phase_b_watts = 0;
float phase_c_watts = 0;

float phase_sum = 0;
float phase_sum_old = 0;

float phase_avg = DAVG_MAGIC_NUM;

String passwd = "";

uint8_t download_index = 0; // html page download index
byte system_mode = 0;

// =================================================================================================

uint8_t pin_asel1 = 27;
uint8_t pin_asel2 = 14;
uint8_t pin_asel3 = 26;


//------------------------------------------------------------------------------

#define OPT_DISABLE -2
#define OPT_DEFAULT -1

#define stiny 32
#define ssmall 64
#define smedium 128
#define slarge 256

struct Sconfig
{
  char inverter_url[smedium];
  char inverter_push_url[smedium];
  char meter_url[smedium];
  char update_url[smedium];

  // sortme

  bool download_html;
  uint8_t charger_oot_min;
  uint8_t charger_oot_sec;

  uint8_t inverter_oot_min;
  uint8_t inverter_oot_sec;
  byte gmt; // divide by 2 and minus 12
  uint8_t board_rev;

  // wifi

  char wifi_ssid1[ssmall];
  char wifi_pass1[ssmall];
  char wifi_ssid2[ssmall];
  char wifi_pass2[ssmall];

  char hostn[stiny];

  int8_t pin_led;
  int8_t pin_charger;
  int8_t pin_inverter;
  int8_t pin_wd;
  int8_t pin_flash;

  int8_t pin_sda;
  int8_t pin_scl;

  char update_host[ssmall];

  char ntp_server[ssmall];
  char description[smedium];

  // device addresses

  char oled_addr;
  char pcf857a_addr;

  // bools

  bool rotate_oled;
  bool button_timer_mode;
  bool blink_led;
  bool blink_led_default;
  bool avg_phase;
  bool threephase;
  bool monitor_phase_a;
  bool monitor_phase_b;
  bool monitor_phase_c;
  bool flip_ipin;
  bool flip_cpin;
  bool auto_update;
  bool wifi_highpower_on;
  bool i_enable;
  bool c_enable;
  bool day_is_timer;
  bool night_is_timer;
  //
  int day_watts;
  int night_watts;

  uint8_t button_timer_secs;
  uint8_t button_timer_max;
  uint8_t c_start_h;
  uint8_t c_finish_h;
  uint8_t i_start_h;
  uint8_t i_finish_h;

  bool webc_mode;
  bool led_status;
};

Sconfig config;

// =================================================================================================

void setup()
{
  Serial.begin(115200);

  setup_fs();

  if(!load_config())
   save_config();

  vars_sanity_check();
  set_pins();

  // --------------------------------------------------------------------------------------

  bool i2c_on = 0;

  if(config.pin_sda == OPT_DISABLE || config.pin_scl == OPT_DISABLE)
  {
    Serial.println("i2c Disabled.");
  }
  else if(config.pin_scl == OPT_DEFAULT || config.pin_sda == OPT_DEFAULT)
  {
    Serial.println("i2c on default pins.");
    i2c_on = 1;
    Wire.begin();
  }
  else
  {
    Serial.println("i2c on custom pins.");
    i2c_on = 1;
    Wire.begin(config.pin_scl, config.pin_sda);
  }

  if(i2c_on)
  {
    Wire.setClock(3400000L);
    oled_setup();
    oled_set1X();
    oled_print("FW: ");
    oled_println(String(FW_VERSION));

    oled_clear();

    if(config.pcf857a_addr)
    {
      both_print("port setup 0x" + String(config.pcf857a_addr, HEX));
      pcf857a_setup(config.pcf857a_addr);
      oled_clear();
    }
  }

  // --------------------------------------------------------------------------------------

  both_println(F("Trim Log"));
  file_limit_size(txt_log_system, 4096);

  both_println(F("Set Idle"));
  modeset(0);
  set_led(config.led_status);


  // --------------------------------------------------------------------------------------

  if (!wifi_start())
  {
    ap_start();
  }

  // --------------------------------------------------------------------------------------

  oled_clear();
  oled_set1X();

  both_println(F("UDP"));
  udp.begin(localPort);

  oled_clear();

  // --------------------------------------------------------------------------------------
  both_println(F("MDNS"));
  MDNS.begin(config.hostn);

//   both_println(F("MDNS http add"));
  MDNS.addService("http", "tcp", 80);

  // --------------------------------------------------------------------------------------
  // Setup HTTP Server

  both_println(F("HTTP"));
  server.on("/", stats);
  server.on("/config", web_config);
  server.on("/result", web_config_submit);

  server.on("/inverter_on", inverter_on);
  server.on("/idle", idle_on);
  server.on("/charger_on", charger_on);

  server.on("/config_raw", config_raw);
  server.on("/whatami", whatami);
  server.on("/stats", stats);

  server.on("/force_refresh", force_refresh);

  server.on("/reset", software_reset);

  server.on("/update_menu", update_menu);
  server.on("/advance", advance_config);

  server.on("/ledon", led_on);
  server.on("/ledoff", led_off);
  server.on("/ledtoggle", led_toggle);
  server.on("/ledblink", led_blink);

  server.on("/style.css", css_raw);

  server.on("/do_update", do_update_web);

  server.on("/html_update", web_html_redownload);

  server.on("/issue_log", web_issue_log);
  server.on("/issue_submit", web_issue_submit);

  server.on("/info_raw", info_raw);

//   copy_config_submit

  server.on("/cpconf", web_copy_config);
  server.on("/cpconf_submit", copy_config_submit);

  server.on("/upload_config", upload_config);
  server.on("/upload_config_submit", upload_config_submit);

  //port_config
  server.on("/port_config", port_config);
  server.on("/port_cfg_submit", port_cfg_submit);

  server.on("/network", net_config);

  server.on("/datasrcs", datasrcs);

  server.on("/timer", web_timer_config);

  server.on("/forcentp", force_ntp_sync);

  server.on("/i2c_scan", i2c_scan);

  server.on("/sys_info", sys_info);

  server.begin();
}

void set_pins()
{
  uint8_t pins_asel[3] = {pin_asel1, pin_asel2, pin_asel3 };
  for(uint8_t x = 0; x < 3; x++)
  {
    pinMode(pins_asel[x], OUTPUT);
    digitalWrite(pins_asel[x], LOW);
  }

  if(config.pin_inverter != OPT_DISABLE)
  {
    pinMode(config.pin_inverter, OUTPUT);
    digitalWrite(config.pin_inverter, config.flip_ipin);
  }

  if(config.pin_charger != OPT_DISABLE)
  {
    pinMode(config.pin_charger, OUTPUT);
    digitalWrite(config.pin_charger, config.flip_cpin);
  }

  if(config.pin_led != OPT_DISABLE)
  {
    pinMode(config.pin_led, OUTPUT);
    digitalWrite(config.pin_led, LOW);
  }

  if(config.pin_wd != OPT_DISABLE)
  {
    pinMode(config.pin_wd, OUTPUT);
    digitalWrite(config.pin_wd, LOW);
  }

  if(config.pin_flash != OPT_DISABLE)
  {
    pinMode(config.pin_flash, INPUT);
  }
}

// =================================================================================================================
// Wifi Functions
// =================================================================================================================

bool ap_start()
{
  /* You can remove the password parameter if you want the AP to be open. */
//   WiFi.setSleepMode(WIFI_NONE_SLEEP); // always put AP in high power mode

  WiFi.softAP("SolarAP");

  ap_mode = 1;
  return 1;
}

bool wifi_start()
{
  if (ap_mode)
  {
    Serial.println("wifi_start: AP Mode");
    return 0;
  }

  if (WiFi.status() == WL_CONNECTED)
    return 1;

  WiFi.persistent( false ); // dont save wifi settings
//   if(config.wifi_highpower_on)
//   {
//     WiFi.setSleepMode(WIFI_NONE_SLEEP); // enable if wifi is buggy
//   }

  Serial.println("wifi_start:, " + String(config.wifi_ssid1) );
  if(wifi_connect(config.wifi_ssid1, config.wifi_pass1) )
    return 1;

  // fallback
  if(wifi_connect(config.wifi_ssid2, config.wifi_pass2))
    return 1;

  // hard coded fallback
  Serial.println("wifi_start: using hardcoded wifi");
  if(wifi_connect((char*)"wififallback", (char*)"xxxx") )
    return 1;

  return 0;

}

bool wifi_connect(char s[ssmall], char p[ssmall])
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
    if (recon > 400)
    {
      both_println(F("Wifi Fail"));
      return 0;
    }

    recon++;

    if(config.pin_led != OPT_DISABLE)
      set_led(3);

    Serial.print(F("."));

    if(oled_enabled())
    {
      oled_clear();
      oled_set2X();
      oled_println(F("Wifi\nConnect"));
      oled_set1X();
      both_println(String(s) + " " + String(recon) );
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

// =================================================================================================================
// Main Loop
// =================================================================================================================

bool boot_success = 0;
bool inverter_synced = 0;

unsigned long systick = 0;

String mode_reason = "";



void loop()
{
  server.handleClient();
//   EasyBuzzer.update();

  if(systick > millis())
    return;

  systick = millis() + 100;

  if(check_system_triggers())
    return;

  // ----------------------------------------------------------------------

  check_system_timers();
  set_daynight();
  check_data_sources();

  // ----------------------------------------------------------------------

  // finish loop unless its time to update

  if (update_time > millis())
    return;

  last_update = millis();
  mode_reason = datetime_str(0, '/', ' ', ':') + " ";

  // ----------------------------------------------------------------------
  // update mode

  if(config.button_timer_mode) // if button reaches here. its on timer is up
  {
//     mode_reason = frline(18);
    mode_reason = "Button - IDLE.";
    modeset(0);
    return;
  }

  if(!time_synced)
  {
//     mode_reason = frline(37);
    mode_reason = "time unsynced";
    return;
  }

  bool finv = 0; // inverter on flag
  bool fchg = 0; // charger on flag
//   bool fbms = 0; // bms on flag

  if(system_mode == 2 && config.i_enable && night_time)
    finv = 1;

  if(system_mode == 1 && config.c_enable && day_time)
    fchg = 1;

  // discharger
  if (config.i_enable && night_time)
  {
    // cooldown check
    if(inverter_off_time > millis())
    {
//       mode_reason += frline(38) + "\n";
      mode_reason += "Inverter Cooldown.\n";
      finv = 0;
    }
    // timer check
    else if (config.night_is_timer)
    {
//       mode_reason += frline(13) + "\n"; // timer turn on
      mode_reason += "Night timer on\n";
      finv = 1;
    }

    // Regular night time logic

    // DRAIN
    else if (phase_sum > config.night_watts)
    {
//       mode_reason += frline(11) + "\n";
      mode_reason += "phase_sum > night_watts\n";
      finv = 1;
    }
    // IDLE
    else if (phase_sum < -20) // turn off inverter
    {
//       mode_reason += frline(12) + "\n";
      mode_reason += "night: phase_sum < -20\n";
      finv = 0;
    }
  }

  // charger
  if (config.c_enable && day_time)
  {
    // cooldown
    if(charger_off_time > millis()) // charger cooldown
    {
//       mode_reason += frline(39) + "\n";
      mode_reason += "charger cooldown\n";
      fchg = 0;
    }
    // timer
    else if (config.day_is_timer)
    {
//       mode_reason += frline(22) + "\n";
      mode_reason += "Day Timer\n";
      fchg = 1;
    }
    // IDLE
    else if (phase_sum > 20)
    {
//       mode_reason += frline(9)  + "\n";
      mode_reason += "chg, phase_sum > 20\n";
      fchg = 0;
    }
    // CHARGE
    else if (phase_sum < (config.day_watts * -1.0) )
    {
//       mode_reason += frline(10)  + "\n";
      mode_reason += "CHARGE: day_watts > phase_sum\n";
      fchg = 1;
    }
  }

  uint8_t tmp_mode = 0;
  if(finv)
    tmp_mode = 2;
  if(fchg)
    tmp_mode = 1;

  // idle time
  if
  (
    (!day_time && !night_time) || // not day or night
    (!config.i_enable && !config.c_enable) ||   // both devices disabled
    (!config.i_enable && night_time && !day_time) ||  // night time only and night time dev disabled
    (!config.c_enable && day_time && !night_time)   // day time only and day time dev disabled
  )
  {
    mode_reason += F("idle\n");
    tmp_mode = 0;
  }


  // SET MODE
  modeset(tmp_mode);
}

// ======================================================================================================================

unsigned long boot_time = millis();

bool check_system_triggers() // returns 1 if a event was triggered
{
  if(restart_trigger)
  {
    oled_clear();
    oled_set2X();
    both_println(F("restart"));
    modeset(0);
    delay(500);
    ESP.restart();
  }

  // WiFi disconnected trigger
  if (!ap_mode && WiFi.status() != WL_CONNECTED)
  {
    oled_clear();
    both_println(F("WiFi ERROR"));
    modeset(0);
    restart_trigger = !wifi_start(); // seems to need a restart if wifi is out here.
    return 1;
  }

  // BOOT OK trigger
  if(boot_success == 0)
  {
    oled_clear();
    oled_set1X();
    boot_success = 1;
    String tmp = "Boot FW: " + String(FW_VERSION);
    both_println(tmp);
    log_issue(tmp);

    return 1;
  }

  // update trigger
  if(self_update) // ideally we would do this via timer to allow webpage to respond....
  {
    oled_clear();
    oled_set2X();
    both_println("Self\nUpdate");

    do_update();
    return 1;
  }

  // download html trigger
  if(!ap_mode && config.download_html)
  {
    download_html_from_remote();
    return 1;
  }

  // data source timeout trigger (time based)
  if
  (
    !ap_mode &&
    !config.button_timer_mode &&
    millis() - time_since_check > check_timeout
  )
  {
    oled_clear();
    both_print(F("CHECK\nTIMEOUT"));
    delay(5000);

    restart_trigger = 1;
    return 1;
  }

  // AP Mode restart trigger (time based)
  if(!config.button_timer_mode && ap_mode)
  {
    if(millis() - boot_time > 1080000) // ~20 min
    {
      restart_trigger = 1;
      return 1;
    }
  }

  return 0;
}

// ======================================================================================================================

unsigned long timer_read_button = 0;
unsigned long timer_update_check = 0;
unsigned long timer_ntp_sync = 0;
unsigned long timer_oled = 0;
unsigned long timer_serial = 0;

unsigned long timer_led = 0;

bool check_system_timers()
{
  bool result = 0;

  if((config.blink_led || config.blink_led_default) && timer_led < millis())
  {
    timer_led = millis() + 500;
    set_led(3);
  }

  // OLED timer
  if(millis() > timer_oled)
  {
    if(timer_oled > 0)
      oled_print_info();

    result = 1;
    timer_oled = millis() + 1750;
  }

  // read button timer
  if(config.button_timer_mode && millis() > timer_read_button)
  {
    if(timer_read_button > 0 && digitalRead(config.pin_flash) == LOW)
    {
      if(update_time < millis())
        update_time = millis();

      update_time += config.button_timer_secs * 1000;

      if(update_time - millis() > config.button_timer_max * 1000 * 60)
        update_time = millis() + config.button_timer_max * 1000 * 60;


      modeset(1);
      result = 1;
    }

    timer_read_button = millis() + 250; // every 0.25 sec
  }

  // auto update timer
  if(config.auto_update && millis() > timer_update_check)
  {
    if(timer_update_check > 0) // dont trigger on fist boot
    {
      oled_clear();
      oled_set2X();
      both_println("Auto\nUpdate");
      Serial.println(timer_update_check);

      do_update();
    }

    timer_update_check = millis() + 86400000; // every 24h
    result = 1;
  }

  // NTP sync timer
  if(!ap_mode && millis() > timer_ntp_sync)
  {
    sync_time();

    if(time_synced)
    {

      timer_ntp_sync = millis() + 21600000; // 6 hours
    }
    else
    {
      timer_ntp_sync = millis() + 20000; // 20 s
    }
    result = 1;

  }

  return result;
}


// ======================================================================================================================

unsigned long timer_pgrid = 0;

bool check_data_sources()
{
  bool result = 0;
  if(millis() > timer_pgrid)
  {
    check_grid();
    timer_pgrid = millis() + 1555;
    result = 1;
  }

  return result;
}

bool check_grid()
{
  if(ap_mode)
    return 0;

  if (!config.button_timer_mode && update_p_grid() == 0)
  {
    if(!inverter_synced)
    {
      oled_clear();
      oled_set2X();
      both_println(F("Waiting 4\nInverter"));
      both_print_ip();
    }

    Serial.print(F("x"));
    return 0;
  }
  Serial.print(F("."));
  return 1;
}

// ======================================================================================================================

void set_led(char m)
{
  if(config.pin_led == OPT_DISABLE)
    return;

  bool ledstat = 0;

  if(m == 0) // OFF
  {
    config.blink_led = 0;
    config.led_status = 0;

    ledstat = !config.led_status; // led pin mode is oposite of config.led_status
  }
  else if (m == 1) // ON
  {
    config.blink_led = 0;
    config.led_status = 1;

    ledstat = !config.led_status; // led pin mode is oposite of config.led_status
  }
  else if (m == 2) // toggle
  {
    config.led_status = !config.led_status;
    ledstat = !config.led_status; // led pin mode is oposite of config.led_status
  }
  else if(m == 3) // toggle without changing value of config.led_status
  {
    ledstat = !digitalRead(config.pin_led);
  }

  digitalWrite(config.pin_led, ledstat);
}

void both_print_ip()
{
  oled_set1X();
  oled_print(F("IP:    "));
  oled_print(WiFi.localIP().toString());

  Serial.println("IP: " + WiFi.localIP().toString());
}

uint8_t oled_temp_pos = 0;
void oled_print_info()
{
  oled_clear();
  oled_set2X();

  if(ap_mode && !config.button_timer_mode)
  {

    both_println(F("AP Mode"));
    both_print_ip();
    both_println(F("SSID: SolarAP"));

    unsigned long rbtime = millis() - boot_time;
    rbtime /= 1000;
    rbtime /= 60;
    both_println(String(F("Reboot in:\n")) + String(rbtime) + String(F(" min")));
    oled_set2X();

    return;
  }

  // ----------------------------------------------------------------------
  // oled print info


  if(config.button_timer_mode)
  {
    if (system_mode == 0)
      oled_print(F("OFF "));
    else
      oled_print(F("ON  "));

    oled_println(datetime_str(2, ' ', ' ', ':'));
  }
  else
  {
    if (system_mode == 0)
      oled_print(F("IDL "));
    else if (system_mode == 1)
      oled_print(F("CHG "));
    else if (system_mode == 2)
      oled_print(F("DRN "));
    else if (system_mode == 3)
      oled_print(F("CRS "));
//     else if (system_mode == 4)
//       oled_print(F("BMS "));

    oled_println(datetime_str(2, ' ', ' ', ':'));

    oled_print(F("KWh  "));
    oled_println(String(phase_sum / 1000, 2));
  }

  // ----------------------------------------------------------------------
  // oled print next update time

  if(config.button_timer_mode)
  {
    if(system_mode)
    {
      oled_println(F("Time Left"));
      oled_println(next_update_string(2));
    }
    else
    {
      oled_println("");
      oled_println("");
    }
  }
  else
  {
    oled_set1X();
    oled_print(F("Update "));
    oled_println(next_update_string(0));

  }

  // ----------------------------------------------------------------------
  // oled print system info

  oled_set1X();

  if(config.button_timer_mode)
  {
    oled_println(String(config.hostn));
  }

  both_print_ip();

  if(config.button_timer_mode)
  {
    // do nothing
  }

  oled_set2X();
}

void calc_next_update()
{
  int rest_s = 30;

  if(config.button_timer_mode)
  {
    if(update_time < millis())
    {
      update_time = millis() + 1000;
    }
    return;
  }

  else if(system_mode == 0) // IDLE
  {
    rest_s = 60;
  }
  else if(system_mode == 3) // cross over (not used atm)
  {
    rest_s = config.charger_oot_sec;
  }
  else if(system_mode == 1) // CHARGER
  {
    rest_s = config.charger_oot_sec;
    rest_s += config.charger_oot_min * 60;
  }
  else if (system_mode == 2) // INVERTER
  {
    rest_s = config.inverter_oot_sec;
    rest_s += config.inverter_oot_min * 60;

  }

  update_time = millis() + (rest_s * 1000);
}


void modeset(byte m)
{
  // check prior mode - set cooldown timers
  if(m != system_mode) // we care about prior mode (system_mode) which hasnt changed yet but will.
  {
    if(system_mode == 1) // charger turning off
    {
      charger_off_time = millis() + (config.charger_oot_min * 60 * 1000) + (config.charger_oot_sec * 1000);
    }
    else if(system_mode == 2) // inverter turning off
    {
      inverter_off_time = millis() + (config.inverter_oot_min * 60 * 1000) + (config.inverter_oot_sec * 1000);
    }
  }

  bool c_pinmode = 0;
  bool i_pinmode = 0;

  if (m == 0) // idle
  {
    i_pinmode = 0;
    c_pinmode = 0;
  }
  else if (m == 1)  // charge
  {
    i_pinmode = 0;
    c_pinmode = 1;

//     if(charger_high_voltage_shutdown) // safety check
//       c_pinmode = 0;
  }

  else if (m == 2)  // drain (inverter)
  {
    i_pinmode = 1;
    c_pinmode = 0;
  }
  else if (m == 3)  // BOTH SYSTEMS ON (used for ??)
  {
    i_pinmode = 1;
    c_pinmode = 1;
  }

  else // BAD MODE !!!
  {
    while (1)
    {
      oled_clear();
      both_println(frline(23));
      delay(1000);
    }
  }

  if(config.button_timer_mode)
  {
    system_mode = m;
  }
  else if(system_mode == m)
  {
    update_time = millis() + 1000;
  }
  else
  {
    system_mode = m;
    calc_next_update();
  }

  if(config.flip_cpin)
    c_pinmode = !c_pinmode;
  if(config.flip_ipin)
    i_pinmode = !i_pinmode;

  if(config.pcf857a_addr != 0)
  {
    port_mode(0, c_pinmode);
    port_mode(1, i_pinmode);
  }
  else
  {
    if(config.pin_charger != OPT_DISABLE)
      digitalWrite(config.pin_charger, c_pinmode);

    if(config.pin_inverter != OPT_DISABLE)
      digitalWrite(config.pin_inverter, i_pinmode);
  }
}

