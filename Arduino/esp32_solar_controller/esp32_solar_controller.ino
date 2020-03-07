/*
Arduino ide settings :

ESP32

*/

#define FW_VERSION 48

#define DAVG_MAGIC_NUM -12345678

// to longer timeout = esp weirdness
#define httpget_timeout 5000

// 10 min
#define check_timeout 600000


#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>

#include <HTTPClient.h>

#include <ESP32httpUpdate.h>
// <ESP32httpUpdate.cpp line 172, modify timeout, http.setTimeout(60000);

#include <ArduinoJson.h>

#include <WebServer.h>
WebServer server(80);

WiFiMulti WiFiMulti;


// -----------------------------------------------------------------------------------------

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

SSD1306AsciiWire oled;

// -----------------------------------------------------------------------------------------

#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;

const adsGain_t ads_gain = GAIN_ONE;
const float ads_mv = 0.125;

//                                                                ADS1015  ADS1115
//                                                                -------  -------
// ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
// ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
// ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
// ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
// ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
// ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

bool volt_synced = 0;

// -----------------------------------------------------------------------------------------

#include <TimeLib.h>

const unsigned int localPort = 2390;      // local port to listen for UDP packets
const byte NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP udp;

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

unsigned long timer_pgrid = 0;
unsigned long timer_voltage = 0;
unsigned long timer_ntc10k = 0;

unsigned long timer_read_button = 0;
unsigned long timer_update_check = 0;
unsigned long timer_ntp_sync = 0;
unsigned long timer_oled = 0;
unsigned long timer_serial = 0;

unsigned long timer_led = 0;

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

bool Fsave_config = 0;

//------------------------------------------------------------------------------

double cells_volts[16] = {DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM};
double cells_volts_real[16] = {DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM};
float cell_volt_diff = 0;
float cell_volt_high = 0;
float cell_volt_low = 0;

float ntc10k_sensors[16]  = {DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM, DAVG_MAGIC_NUM};

//------------------------------------------------------------------------------

const String mime_html    = "text/html";
const String mime_txt     = "text/plain";
const String http_str     = "http://";
const String denied_str   = "Denied";
const String title_str    = "title";

const String dothtml        = ".htm";
const String dottxt        = ".txt";

const String html_config          = "/config" + dothtml;
const String html_battery         = "/batconf" + dothtml;
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
const String html_ntc10k_config   = "/ntc10k" + dothtml;
const String html_ntc10k_info     = "/ntc10ki" + dothtml;
const String html_sys_info        = "/system" + dothtml;
const String html_battery_info    = "/battery" + dothtml;
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
const byte size_battery_message = 255;
String battery_message = "";

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

#define count_ntc 16
#define count_cells 16

struct Sconfig
{
  char inverter_url[smedium];
  char inverter_push_url[smedium];
  char meter_url[smedium];
  char pub_url[smedium];

  // sortme

  float lv_shutdown_delay; // float because we use fraction of hours eh 0.5h
  float hv_shutdown_delay;
  bool download_html;
  uint8_t charger_oot_min;
  uint8_t charger_oot_sec;

  uint8_t inverter_oot_min;
  uint8_t inverter_oot_sec;
  byte cell_count;
  byte gmt; // divide by 2 and minus 12
  float dcvoltage_offset;
  uint8_t board_rev;

  // wifi

  char wifi_ssid1[ssmall];
  char wifi_pass1[ssmall];
  char wifi_ssid2[ssmall];
  char wifi_pass2[ssmall];

  double ntc_temp_mods[count_ntc];

  double battery_volt_mod[count_cells];

  float ntc_temp_max[count_ntc];

  char hostn[stiny];

  int8_t pin_led;
  int8_t pin_charger;
  int8_t pin_inverter;
  int8_t pin_wd;
  int8_t pin_flash;

  int8_t pin_sda;
  int8_t pin_scl;

  int8_t pin_buzzer;


  uint8_t ntc10k_count;

  char update_host[ssmall];

  char ntp_server[ssmall];


  float pack_volt_min;
  float battery_volt_min;
  float battery_volt_rec;
  float battery_volt_idl;
  float battery_volt_max;
  float battery_volt_over;

  char description[smedium];

  // device addresses

  char oled_addr;
  char pcf857a_addr;

  // bools

  bool monitor_temp;
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
//   bool wifi_highpower_on;
  bool i_enable;
  bool c_enable;
  bool day_is_timer;
  bool night_is_timer;
  bool cells_in_series;
  bool monitor_battery;

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

uint8_t pin_sd_reset = 15;

void beep_helper(int freq, int tdelay)
{
  ledcWriteTone(0, freq);
  delay(tdelay);
  ledcWriteTone(0, 0);
}

bool SDERROR = 0;
void sd_setup(int tone)
{
  int toneinc = 45;

  Serial.println("Mounting SD Card");

  pinMode(23,INPUT_PULLUP); // SDCard pin (required)

  SD.end();

  while(1)
  {
    beep_helper(tone, 250);
    if(SD.begin())
    {
      SDERROR = 0;
      Serial.println("Card Mounted");
      tone += toneinc; beep_helper(tone, 250);
      delay(250);
      tone += toneinc; beep_helper(tone, 250);
      break;
    }

    Serial.println("Card Mount Failed");

    delay(3000);
  }

  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    //     return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);


  if(!SD.exists(txt_log_system.c_str() ) )
    log_issue("New Log.");

  tone += toneinc; beep_helper(tone, 250);
//   setup_fs();
}

void setup()
{
  int toneinc = 45;
  int tone = 90;
  ledcSetup(0, 1000, 8);
  ledcAttachPin(16, 0);

  Serial.begin(115200);

  sd_setup(tone);


  if(!load_config())
   save_config();

  vars_sanity_check();
  set_pins();

  // --------------------------------------------------------------------------------------
  tone += toneinc; beep_helper(tone, 250);

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

    Wire.setClock(400000L); // fast mode
//     Wire.setClock(1000000L); // fast mode plus
    //     Wire.setClock(3400000L); // high speed mode (buggy on latest esp32 board)
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


    ads.setGain(ads_gain);
//     ads.begin(); // not required I believe, may trigger i2c restart
  }

  // --------------------------------------------------------------------------------------

  both_println(F("Trim Log"));
  file_limit_size(txt_log_system, 4096);

  both_println(F("Set Idle"));
  modeset(0);
  set_led(config.led_status);


  // --------------------------------------------------------------------------------------

  tone += toneinc; beep_helper(tone, 250);
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
  server.on("/battery_config", battery_config);

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

  server.on("/port_info", port_info);

  server.on("/ntc10k_config", ntc10k_config);
  server.on("/ntc10k_info", ntc10k_info);

  server.on("/i2c_scan", i2c_scan);

  server.on("/sys_info", sys_info);

  server.on("/battery_info", battery_info);

  server.on("/bms_raw_info", bms_raw_info);

  server.begin();

  tone += toneinc; beep_helper(tone, 250);
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
  WiFi.mode(WIFI_STA);
//   WiFi.setSleepMode(WIFI_NONE_SLEEP);


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
  if(wifi_connect((char*)"wififallback", (char*)"password") )
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
    if (recon > 150)
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

String mode_reason = "";

unsigned long systick = 0;

void loop()
{
  for(uint8_t i = 0; i < 3; i++)
    server.handleClient();

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

  if(!volt_synced && config.monitor_battery)
  {
    // waiting on voltages to be read
    return;
  }

  // check if cell volt is low and force update

  if(config.monitor_battery && system_mode == 1 && cell_volt_high > config.battery_volt_over)
  {
    mode_reason = "cell over volt. force IDLE.";
    battery_log(mode_reason  + "\n" + String(cell_volt_high, 2) + "v" );
    mode_reason = datetime_str(0, '/', ' ', ':') + " " + mode_reason;

    modeset(0);

    return;
  }

  // check if cell volt is over and force update ?

  if(config.monitor_battery && system_mode == 2 && cell_volt_low <= config.battery_volt_min)
  {
    mode_reason = "Cell under volt, force IDLE";
    battery_log(mode_reason  + "\n");
    mode_reason = datetime_str(0, '/', ' ', ':') + " " + mode_reason;

    modeset(0);

    return;
  }


  // finish loop unless its time to update
  if (update_time > millis())
    return;

  last_update = millis();
  mode_reason = datetime_str(0, '/', ' ', ':') + " ";

  // ----------------------------------------------------------------------
  // update mode

  if(config.button_timer_mode) // if button reaches here. its on timer is up
  {
    mode_reason = "Button - IDLE.";
    modeset(0);
    return;
  }

  if(!time_synced)
  {
    mode_reason = "time unsynced";
    return;
  }

  bool finv = 0; // inverter on flag
  bool fchg = 0; // charger on flag

  if(system_mode == 2 && config.i_enable && night_time)
    finv = 1;

  if(system_mode == 1 && config.c_enable && day_time)
    fchg = 1;

  // -------------------------------------------------------------------------
  // Night Device

  // discharger
  if (config.i_enable && night_time)
  {
    // cooldown check
    if(inverter_off_time > millis())
    {
      mode_reason += "Inverter Cooldown.\n";
      finv = 0;
    }
    // timer check
    else if (config.night_is_timer)
    {
      mode_reason += "Night timer on\n";
      finv = 1;
    }

    // Regular night time logic

    // DRAIN
    else if (phase_sum > config.night_watts)
    {
      mode_reason += "phase_sum > night_watts\n";
      finv = 1;
    }
    // IDLE
    else if (phase_sum < -20) // turn off inverter
    {
      mode_reason += "night: phase_sum < -20\n";
      finv = 0;
    }
  }

  // drain monitor battery check(s)
  if (config.i_enable && night_time && config.monitor_battery && low_voltage_shutdown)
  {
    mode_reason += "Inverter - LV Shutdown\n";
    finv = 0;
  }


  // -------------------------------------------------------------------------
  // Day Device

  // charger
  if (config.c_enable && day_time)
  {
    // cooldown
    if(charger_off_time > millis()) // charger cooldown
    {
      mode_reason += "charger cooldown\n";
      fchg = 0;
    }
    // timer
    else if (config.day_is_timer)
    {
      mode_reason += "Day Timer\n";
      fchg = 1;
    }
    // IDLE
    else if (phase_sum > 20)
    {
      mode_reason += "chg, phase_sum > 20\n";
      fchg = 0;
    }
    // CHARGE
    else if (phase_sum < (config.day_watts * -1.0) )
    {
      mode_reason += "CHARGE: day_watts > phase_sum\n";
      fchg = 1;
    }
  }

  // monitoring voltage charger logic
  // all these checks disable charger if conds met
  if(config.c_enable && day_time && config.monitor_battery)
  {
    // HV Shutdown
    if(charger_high_voltage_shutdown)
    {
      mode_reason += "charger_high_voltage_shutdown\n";
      fchg = 0;
    }
    // Shutdown
    else if(cell_volt_high > config.battery_volt_max)
    {
      mode_reason += "chg battery_volt_max\n";
      fchg = 0;
    }
    // IDLE
    else if (phase_sum > 20)
    {
      mode_reason += "CHG: phase_sum > 20\n";
      fchg = 0;
    }
  }

  // -------------------------------------------------------------------------
  // Select Mode

  uint8_t tmp_mode = 0;
  if(finv)
    tmp_mode = 2;
  if(fchg)
    tmp_mode = 1;

  // -------------------------------------------------------------------------
  // IDLE over rides

  if
  (
    (!day_time && !night_time) || // not day or night
    (!config.i_enable && !config.c_enable) ||   // both devices disabled
    (!config.i_enable && night_time && !day_time) ||  // night time only and night time dev disabled
    (!config.c_enable && day_time && !night_time) ||   // day time only and day time dev disabled
    (config.monitor_temp && high_temp_shutdown)
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
  if(SDERROR || !ping_fs())
  {
    sd_setup(120);
    log_issue("SD Reconnected.");
    return 1;
  }

  if(Fsave_config)
  {
    save_config();
    Fsave_config = 0;
    return 1;
  }

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
      modeset(1);
      result = 1;
    }

    timer_read_button = millis() + 333; // every 0.3 sec
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
      timer_ntp_sync = millis() + 21600000; // 6 hours
    else
      timer_ntp_sync = millis() + 20000; // 20 s

    result = 1;
  }

  return result;
}

// ======================================================================================================================

uint8_t cds_pos = 0;
bool check_data_sources()
{
  cds_pos++;
  if(cds_pos >= 3)
    cds_pos = 0;


  bool result = 0;

  if(cds_pos == 0 && millis() > timer_pgrid)
  {
    check_grid();
    timer_pgrid = millis() + 1555;
    result = 1;
  }

  if(cds_pos == 1 && config.monitor_battery && millis() > timer_voltage)
  {
    cells_update();

    if(volt_synced)
      check_cells();

    timer_voltage = millis() + 100;
    result = 1;
  }

  if(cds_pos == 2 && config.monitor_temp && millis() > timer_ntc10k)
  {
    ntc_update();
    bool trigger_shutdown = 0;
    for(int i = 0; i < config.ntc10k_count; i++)
    {
      if(ntc10k_sensors[i] > config.ntc_temp_max[i])
      {
        trigger_shutdown = 1;
        break;
      }
    }

    high_temp_shutdown = trigger_shutdown;
    timer_ntc10k = millis() + 400;
    result = 1;
  }

  return result;
}

void check_cells()
{
  cell_volt_low = 10000;
  cell_volt_high = 0;

  bool lv_recon_trigger = 1; // reconnect if all cells are above battery_volt_rec
  bool lv_trigger = 0;
  bool hv_trigger = 0;

  unsigned long ms = millis(); // do not use millis, use update_time as disconnects take effect during mode update
  if(ms < update_time)
    ms = update_time;

  if((config.cells_in_series || config.cell_count == 1) && cells_volts_real[config.cell_count-1] < config.pack_volt_min)
    lv_trigger = 1;

  String tmp_msg = "\n"; // cell voltages string

  // cell checks
  for(byte i = 0; i < config.cell_count; i++)
  {
    // cell volt difference
    if(cells_volts[i] < cell_volt_low)
      cell_volt_low = cells_volts[i];
    if(cells_volts[i] > cell_volt_high)
      cell_volt_high = cells_volts[i];

    // HV check
    if(cells_volts[i] >= config.battery_volt_over)
    {
      hv_trigger = 1;
      hv_shutdown_time = ms + (config.hv_shutdown_delay * 3600000.0);
    }

    // LV Check
    if(cells_volts[i] <= config.battery_volt_min)
    {
      lv_trigger = 1;
      lv_shutdown_time = ms + (config.lv_shutdown_delay * 3600000.0);
    }

    // LV recon
    if(lv_trigger || cells_volts[i] < config.battery_volt_rec)
    {
      lv_recon_trigger = 0; // if cells below min voltage, keep shutdown (no night time drain)
    }

    // cell voltages string
    tmp_msg += "\t" + String(i+1) + ": " + cells_volts[i] + "v\n";

  }
  cell_volt_diff = cell_volt_high - cell_volt_low;

  String now_str = datetime_str(0, '/', ' ', ':');

  // ----------------------------------------------------------------------
  // charger_high_voltage_shutdown check

  // HV recon check
  if(!hv_trigger && charger_high_voltage_shutdown && millis() > hv_shutdown_time)
  {
    charger_high_voltage_shutdown = 0;
    battery_message = string_append_limit_size
    (
      battery_message,
      battery_message = datetime_str(0, '/', ' ', ':') + frline(25) + tmp_msg,
      size_battery_message
    );
  }

  // HV disconnect check
  if(hv_trigger && !charger_high_voltage_shutdown)
  {
    charger_high_voltage_shutdown = 1;
    battery_message = string_append_limit_size
    (
      battery_message,
     battery_message = now_str + frline(26) + tmp_msg,
      size_battery_message
    );
  }

  // ----------------------------------------------------------------------
  // low_voltage_shutdown

  // reconnect check
  if(lv_recon_trigger && low_voltage_shutdown && millis() > lv_shutdown_time)
  {
    low_voltage_shutdown = 0;
    battery_message = string_append_limit_size
    (
      battery_message,
     now_str + frline(27)  + tmp_msg,
      size_battery_message
    );
  }

  // low_voltage_shutdown check
  if(lv_trigger && !low_voltage_shutdown)
  {
    low_voltage_shutdown = 1;

    battery_message = string_append_limit_size
    (
      battery_message,
     now_str + frline(28)  + tmp_msg,
      size_battery_message
    );
  }
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

char led_cur = 255;
void set_led(char m)
{
  if(config.pin_led == OPT_DISABLE)
    return;

  if(led_cur == m && m != 3)
  {
    return;
  }
  led_cur = m;

  if(m == 0)
  {
    config.blink_led = 0;
    config.led_status = 0;
  }
  else if (m == 1)
  {
    config.blink_led = 0;
    config.led_status = 1;
  }
  else if (m == 2) // toggle
  {
    config.led_status = !config.led_status;
  }
  else if(m == 3) // toggle without changing value of config.led_status
  {
    digitalWrite(config.pin_led, !digitalRead(config.pin_led));
    return;
  }
  else if(m == 4) // led OFF without changing config.blink_led
  {
    digitalWrite(config.pin_led, 1);
    return;
  }
}

void both_print_ip()
{
  oled_set1X();
  oled_print(F("IP:    "));
  oled_println(WiFi.localIP().toString());

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

    oled_println(datetime_str(2, ' ', ' ', ':'));

    oled_print(F("KWh  "));
    oled_println(String(phase_sum / 1000, 2));
  }

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
  else if(config.monitor_temp)
  {
    oled_set2X();
    oled_println(String(oled_temp_pos + 1) + ": " +  String(ntc10k_sensors[oled_temp_pos], 1) + "c\n" );

    oled_temp_pos++;
    if(oled_temp_pos >= config.ntc10k_count)
    {
      oled_temp_pos = 0;
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

  else if(config.monitor_battery)
  {
    for(byte i=0; i< config.cell_count; i++)
    {
      oled_print(String(cells_volts[i]) + F(" "));
    }
    oled_println(F("v"));
  }
  oled_set2X();
}

void calc_next_update()
{
  int rest_s = 30;

  if(config.button_timer_mode)
  {
    if(update_time < millis())
      update_time = millis();

    if(system_mode != 0)  // when charge is enabled, use button_timer_secs. when idling use 1 sec
    {
      if(update_time - millis() < (config.button_timer_max * 60 * 1000) )
        update_time += (config.button_timer_secs * 1000);
    }
    else
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
  }

  else if (m == 2)  // drain (inverter)
  {
    i_pinmode = 1;
    c_pinmode = 0;
  }
  else if (m == 3)  // BOTH SYSTEMS ON (used for monitor_battery)
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

  if(system_mode == m)
    update_time = millis() + 1000;
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
    pcf857a_write_state(config.pcf857a_addr, 0, !c_pinmode);
    pcf857a_write_state(config.pcf857a_addr, 1, !i_pinmode);
  }
  else
  {
    if(config.pin_charger != OPT_DISABLE)
      digitalWrite(config.pin_charger, c_pinmode);

    if(config.pin_inverter != OPT_DISABLE)
      digitalWrite(config.pin_inverter, i_pinmode);
  }
}

