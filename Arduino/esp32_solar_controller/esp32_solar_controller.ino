/*

Note I use esp32 doit dev kit v1.
but the arduino IDE settings for this board seem to cause issues.

so I configure the IDE to use:

* WEMOS LOLIN32
* Patition Scheme: Minimal SPIFFS, large app with OTA (testing no OTA)
* CPU 240Mhz (WiFi/BT)
* Flash Frequency 80Mhz

this seems to resolve OTA issues.

*/

#define FW_VERSION 235

// to longer timeout = esp weirdness
#define httpget_timeout 5000

// 10 min
#define check_timeout 600000

#define OPT_DISABLE -2
#define OPT_DEFAULT -1

#define stiny 32
#define ssmall 64
#define smedium 128
#define slarge 256

#define MAX_CELLS 32

// ======================================

#include <Arduino.h>
#include <SD.h>
// #include <SPI.h>

#include <WiFi.h>
// #include <WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiAP.h> // todo remove as WiFi is configured via SD card json config
#include <WiFiUdp.h>
#include <ESPmDNS.h>

#include <HTTPClient.h>

#include <ESP32httpUpdate.h>


#include <ArduinoJson.h>

#include <WebServer.h>
WebServer server(80);

// -----------------------------------------------------------------------------------------

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

SSD1306AsciiWire oled;

// -----------------------------------------------------------------------------------------
// web.ino

int get_url_code;

// -----------------------------------------------------------------------------------------


// #include <Adafruit_ADS1015.h>
#include <Ads1115_mux.h>
const uint8_t pin_asel1 = 27;
const uint8_t pin_asel2 = 14;
const uint8_t pin_asel3 = 26;

Ads1115_mux adsmux(pin_asel1, pin_asel2, pin_asel3);

// -----------------------------------------------------------------------------------------
// lm75a

// note: modify library and comment out wire.begin and wire.end

#include <M2M_LM75A.h>
M2M_LM75A lm75a(0x4f);

// -----------------------------------------------------------------------------------------

#include <Mmaths.h>

Mmaths mmaths;

// -----------------------------------------------------------------------------------------

#include <TimeLib.h>

const unsigned int localPort = 2390;      // local port to listen for UDP packets
const byte NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP udp;

// -----------------------------------------------------------------------------------------
// timers

struct SysTimers
{
  unsigned long mode_check = 0;
  unsigned long pgrid_last_update = 0;
  unsigned long use_fallback = 0;

  unsigned long lv_shutdown = 0;
  unsigned long hv_shutdown = 0;

  unsigned long charger_off = 0;
  unsigned long inverter_off = 0;

  unsigned long pgrid = 0;
  unsigned long adc_poll = 0;

  unsigned long update_check = 0;
  unsigned long ntp_sync = 0;
  unsigned long oled = 0;

  unsigned long led = 0;

  unsigned long i2c_check = 0;
};

SysTimers timers;

//------------------------------------------------------------------------------
// cell voltages

double cells_volts[MAX_CELLS];
double cells_volts_real[MAX_CELLS];
float cell_volt_diff = 0;
float cell_volt_high = 0;
float cell_volt_low = 0;

uint8_t cell_lh[2] = {0, 0};

//------------------------------------------------------------------------------

const String mime_html    = "text/html";
const String mime_txt     = "text/plain";
const String mime_json    = "application/json";
const String mime_css     = "text/css";

const String http_str     = "http://";
const String denied_str   = "Denied";
const String title_str    = "title_hostn";

const String dothtml        = ".htm";
const String dottxt        = ".txt";

const String html_header          = "/header" + dothtml;
const String html_config          = "/config" + dothtml;
const String html_battery         = "/batconf" + dothtml;
const String html_calibrate       = "/batcal" + dothtml;
const String html_timer           = "/timer" + dothtml;
const String html_3pinfo          = "/3pinfo" + dothtml;
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
const String html_battery_info    = "/battery" + dothtml;
const String html_upload_config   = "/up_conf" + dothtml;
const String html_port_config     = "/portconf" + dothtml;

const String css_file             = "/style.css";
const String txt_log_system       = "/log" + dottxt;
const String txt_system_messages  = "/msg" + dottxt;
const String txt_passwd_file      = "/passwd";
const String json_config_file     = "/config.jsn";

//------------------------------------------------------------------------------

const uint16_t size_system_msgs = 1024;
String system_msgs = "";

float board_temp = 0;

float phase_a_watts = 0;
float phase_b_watts = 0;
float phase_c_watts = 0;

float phase_a_voltage = 0;
float phase_b_voltage = 0;
float phase_c_voltage = 0;

float energy_consumed_old = 0; // previous day
float energy_consumed = 0;

float phase_sum = 0;
float phase_sum_old = 0;

float phase_avg = 0;

String passwd = "";

uint8_t download_index = 0; // html page download index
byte system_mode = 0;

uint8_t i2cdevcount = 0;

const int8_t power_array_size = 10;

// =================================================================================================


//------------------------------------------------------------------------------

struct Sconfig
{
  uint8_t fwver = 0;

  char inverter_url[smedium];
  char inverter_push_url[smedium];
  char threephase_direct_url[smedium];
  char threephase_push_url[smedium];
  char pub_url[smedium];

  // sortme

  float lv_shutdown_delay; // float because we use fraction of hours eh 0.5h
  float hv_shutdown_delay;

  uint8_t charger_oot_min;
  uint8_t charger_oot_sec;

  uint8_t charger_off_min;
  uint8_t charger_off_sec;

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

  double battery_volt_mod[MAX_CELLS];

  char hostn[stiny];

  int8_t pin_led;
  int8_t pin_charger;
  int8_t pin_inverter;
  int8_t pin_wd;
  int8_t pin_flash;

  int8_t pin_sda;
  int8_t pin_scl;

  int8_t pin_buzzer;

  char update_host[ssmall];

  char ntp_server[ssmall];

  float pack_volt_min;
  float battery_volt_min;
  float battery_volt_rec;
  float battery_volt_max;

  float cpkwh;

  char description[smedium];

  // device addresses

  char oled_addr;

  // bools

  bool rotate_oled = 0;
  bool blink_led = 0;
  bool blink_led_default = 0;

  bool threephase = 0;
  bool monitor_phase_a = 0;
  bool monitor_phase_b = 0;
  bool monitor_phase_c = 0;
  bool flip_ipin = 0;
  bool flip_cpin = 0;
  bool auto_update = 0;
  bool inv_idle_mode = 0;
  bool i_enable = 0;
  bool c_enable = 0;
  bool day_is_timer = 0;
  bool night_is_timer = 0;
  bool cells_in_series = 0;
  bool monitor_battery = 0;

  bool c_offd;
  bool c_amot;


  bool prefer_dc = 0;

  bool hv_monitor = 1;

  uint8_t display_mode = 0;

  bool m247 = 0;

  //
  int day_watts = 0;
  int night_watts = 0;

  uint8_t c_start_h = 0;
  uint8_t c_finish_h = 0;
  uint8_t i_start_h = 0;
  uint8_t i_finish_h = 0;

  uint8_t avg_phase = 0;

  bool webc_mode = 0;
  bool led_status = 0;
};

Sconfig config;



//------------------------------------------------------------------------------
// FLAGS

// group system flags into 1 structure for easy naming.
struct SysFlags
{

  bool restart = 0;
  bool access_point = 0;
  bool update_self = 0;
  bool time_synced = 0;
  bool night = 0;
  bool day = 0;
  bool shutdown_lvolt = 0;
  bool shutdown_hvolt = 0;
  bool update_found = 0;

  bool save_config = 0;

  bool sdcard_read_error = 0;

  bool download_html = 0;

  bool lm75a = 0;

  bool f3p_error1 = 0; // Fronius 3p Fallback JSON Decode Error:

  bool adc_config_error = 0;
  bool boot_success = 0;

  bool i2c_on = 1;
};

SysFlags flags;

// =================================================================================================

void setup()
{
  const uint8_t toneinc = 45;
  uint16_t tone = 90;
  ledcSetup(0, 1000, 8);
  ledcAttachPin(16, 0);

  Serial.begin(115200);

  sd_setup(tone);

  if(!load_config())
  {
    flags.download_html = 1;
    save_config();
  }

  set_pins();

  // --------------------------------------------------------------------------------------
  // i2c Setup

  tone += toneinc; beep_helper(tone, 250);

  bool i2c_requested = 0;
  if(config.pin_sda == OPT_DISABLE || config.pin_scl == OPT_DISABLE)
  {
    Serial.println("i2c Disabled.");
  }
  else if(config.pin_scl == OPT_DEFAULT || config.pin_sda == OPT_DEFAULT)
  {
    Serial.println("i2c on default pins.");
    i2c_requested = 1;

    // disable internal pullups
    // https://arduino.stackexchange.com/questions/13448/disabling-i2c-internal-pull-up-resistors
    digitalWrite(21, LOW);
    digitalWrite(22, LOW);


    Wire.begin();
  }
  else
  {
    Serial.println("i2c on custom pins.");
    i2c_requested = 1;

    digitalWrite(config.pin_scl, LOW);
    digitalWrite(config.pin_sda, LOW);

    Wire.begin(config.pin_scl, config.pin_sda);
  }

  flags.i2c_on = 1;
  if(i2c_requested)
  {
    // detect boards max speed
    uint8_t i2c_test_enum1 = 0;
    uint8_t i2c_test_enum2 = 0;

    Wire.setClock(400000L); // fast mode
    i2cdevcount = i2c_enum();

    if(i2cdevcount)
    {
      Wire.setClock(1000000L); // fast mode plus
      i2c_test_enum1 = i2c_enum();

      Wire.setClock(3400000L); // high speed mode (buggy on latest esp32 board)
      i2c_test_enum2 = i2c_enum();
    }
    else
    {
      Serial.println("no i2c devices found, disabling i2c.");
      flags.i2c_on = 0;
    }

    if(flags.i2c_on)
    {
      if(i2c_test_enum1 != i2cdevcount)
      {
        log_msg("i2c speed 400000 (fast)");
        Wire.setClock(400000L);
      }
      else if(i2c_test_enum2 != i2cdevcount)
      {
        log_msg("i2c speed 1000000 (fast plus)");
        Wire.setClock(1000000L);
      }
      else
      {
        log_msg("i2c speed 3400000 (highspeed)");
      }
    }
    else
    {
      Serial.println("i2c disabled");
    }

    // setup OLED

    oled_setup();
    oled_set1X();
    oled_print("FW: ");
    oled_println(String(FW_VERSION));

    oled_clear();

    if(flags.i2c_on)
    {
      if(i2c_ping(0x4f))
        flags.lm75a = 1;

       adsmux.setup();
    }
  }

  // --------------------------------------------------------------------------------------
  // System checks

  both_println(F("Trim Log"));
  file_limit_size(txt_log_system, 4096);

  both_println(F("Set Idle"));
  modeset(0);

  both_println(F("Set Led"));
  set_led(config.led_status);


  // --------------------------------------------------------------------------------------


  oled_clear();
  both_println(F("WiFI"));

  tone += toneinc; beep_helper(tone, 250);
  if (!wifi_start())
  {
    ap_start();
  }

  // --------------------------------------------------------------------------------------
  // Network Services

  oled_clear();
  oled_set1X();

  both_println(F("UDP"));
  udp.begin(localPort);

//   oled_clear();

  // --------------------------------------------------------------------------------------
  both_println(F("MDNS"));
  MDNS.begin(config.hostn);

//   both_println(F("MDNS http add"));
  MDNS.addService("http", "tcp", 80);

  // --------------------------------------------------------------------------------------
  // Setup HTTP Server

  both_println(F("HTTP"));
  {
    server.on("/", stats);
    server.on("/config", web_config);
    server.on("/result", web_config_submit);

    server.on("/inverter_on", inverter_on);
    server.on("/idle", idle_on);
    server.on("/charger_on", charger_on);
    server.on("/both_on", both_on);


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

    server.on("/cpconf", web_copy_config);
    server.on("/cpconf_submit", copy_config_submit);

    server.on("/upload_config", upload_config);
    server.on("/upload_config_submit", upload_config_submit);

    server.on("/port_config", port_config);
    server.on("/port_cfg_submit", port_cfg_submit);

    server.on("/network", net_config);

    server.on("/datasrcs", datasrcs);

    server.on("/forcentp", force_ntp_sync);

    server.on("/port_info", port_info);

    server.on("/i2c_scan", i2c_scan);

    server.on("/sys_info", sys_info);

    server.on("/battery_info", battery_info);

    server.on("/bms_raw_info", bms_raw_info);

    server.on("/adc_info_raw", adc_info_raw);

    server.on("/batcal", battery_calibrate);

    server.on("/3pinfo", threepase_info);

    server.on("/timers", timers_page);



    server.begin();
  }

  tone += toneinc; beep_helper(tone, 250);

  // --------------------------------------------------------------------------------------
  // Setup ADSMUX

  oled_clear();

  if(adsmux.adc_found)
  {
    oled_println("ADC 1st Poll");

    for(uint8_t i = 0; i < MAX_CELLS; i++)
    {
      cells_volts_real[i] = 0;
      cells_volts[i]      = 0;
    }

    adc_quick_poll();
  }
  else
  {
    // check config, if volt monitoring enabled but no i2c dev - display alert, Force IDLE ALWAYS

    if(config.monitor_battery)
    {
      log_msg("monitor_battery enabled but ADC not found");
      flags.adc_config_error = 1;
    }
  }

  // --------------------------------------------------------------------------------------
  // check if FW was updated

  if(config.fwver != FW_VERSION)
  {
    flags.download_html = 1;
    save_config();
  }
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
// Main Loop
// =================================================================================================================


bool inverter_synced = 0;

String mode_reason = "";

String old_day_reason = "idle";
String old_night_reason = "idle";

unsigned long systick = 0;

void loop()
{
  server.handleClient();

  if(systick > millis())
    return;

  systick = millis() + 30;

  if(check_system_triggers())
    return;

  // ----------------------------------------------------------------------

  check_system_timers();
  set_daynight();
  check_data_sources();

  // ----------------------------------------------------------------------
  // ADC Error ?

  if(flags.adc_config_error)
  {
    mode_reason = datetime_str(0, '/', ' ', ':') + " " + "config requires ADC, but ADC not found.";

    modeset(0);
    return;
  }

  // ----------------------------------------------------------------------

  // check if cell volt is low and force update

  if(config.hv_monitor && config.monitor_battery && system_mode == 1 && cell_volt_high > config.battery_volt_max)
  {
    mode_reason = "cell over volt (" + String(cell_volt_low, 3) + "v). force IDLE.";
    log_msg(mode_reason  + "\n" + String(cell_volt_high, 2) + "v" );
    mode_reason = datetime_str(0, '/', ' ', ':') + " " + mode_reason;

    modeset(0);

    return;
  }

  // check if cell volt is over and force update ?

  if(config.monitor_battery && system_mode == 2 && cell_volt_low <= config.battery_volt_min)
  {
    mode_reason = "Cell under volt (" + String(cell_volt_low, 3) + "v), force IDLE";
    log_msg(mode_reason  + "\n");
    mode_reason = datetime_str(0, '/', ' ', ':') + " " + mode_reason;

    modeset(0);

    return;
  }


  // finish loop unless its time to update
  if (timers.mode_check > millis())
    return;

  mode_reason = datetime_str(0, '/', ' ', ':') + " ";

  // ----------------------------------------------------------------------
  // update mode

  if(!flags.time_synced)
  {
    mode_reason = "time unsynced";
    return;
  }

  // -------------------------------------------------------------------------
  // IDLE checks

  if
  (
    (!flags.day && !flags.night) || // not day or night
    (!config.i_enable && !config.c_enable) ||   // both devices disabled
    (!config.i_enable && flags.night && !flags.day) ||  // night time only and night time dev disabled
    (!config.c_enable && flags.day && !flags.night)   // day time only and day time dev disabled

  )
  {
    mode_reason += F("Idle (1)\n");
    modeset(0);
    return;
  }

  // -------------------------------------------------------------------------
  // set local flags baed on current mode

  bool finv = 0; // inverter on flag
  bool fchg = 0; // charger on flag

  if((system_mode == 2 || system_mode == 3) && config.i_enable && flags.night)
    finv = 1;

  if((system_mode == 1  || system_mode == 3) && config.c_enable && flags.day)
    fchg = 1;

  // -------------------------------------------------------------------------
  // Night Device

  // discharger
  if (config.i_enable && flags.night)
  {
    String new_night_reason = "Night: ";
    // LV check
    if (config.monitor_battery && flags.shutdown_lvolt)
    {
      new_night_reason += "IDLE, LV Shutdown\n";
      finv = 0;
    }
    // cooldown check
    else if(timers.inverter_off > millis())
    {
      new_night_reason += "Idle, Inverter Cooldown.\n";
      finv = 0;
    }
    // Prefer DC (enable if !lv and !cooldown
    else if (config.prefer_dc)
    {
      new_night_reason += "Prefer DC\n";
      finv = 1;
    }
    else if (phase_sum > config.night_watts)
    {
      new_night_reason += "Drain\n";
      finv = 1;
    }
    // IDLE
    else if (config.inv_idle_mode && phase_sum < 0 ) // turn off inverter
    {
      new_night_reason += "IDLE (grid " + String(phase_sum,1) + " < 0)\n";
      finv = 0;
    }
    else if (phase_sum < config.night_watts * -1 ) // turn off inverter
    {
      new_night_reason += "IDLE (grid " + String(phase_sum,1) + " < night_watts)\n";
      finv = 0;
    }
    // timer check
    else if (config.night_is_timer)
    {
      new_night_reason += "timer\n";
      finv = 1;
    }
    else if(finv) // no change, inverter still on
    {
      new_night_reason = old_night_reason;
    }
    else // no change, inverter still off
    {
      new_night_reason = "idling\n";
    }

    mode_reason += new_night_reason;
    old_night_reason = new_night_reason;
  }


  // -------------------------------------------------------------------------
  // Day Device

  // charger
  if (config.c_enable && flags.day)
  {
    String new_day_reason = "Day: ";

    // idle after min on time check
    if(fchg == 1 && config.c_amot) // check behaviour after being on for min on time. 1 = go to off.
    {
      new_day_reason += "idle after min on time\n";
      fchg = 0;
    }

    // charger cooldown
    else if(timers.charger_off > millis())
    {
      new_day_reason += "charger cooldown\n";
      fchg = 0;
    }

    // timer
    else if (config.day_is_timer)
    {
      new_day_reason += "Timer\n";
      fchg = 1;
    }
    // CHARGE
    else if (phase_sum < (config.day_watts * -1.0) )
    {
      new_day_reason += "Charge\n";
      fchg = 1;
    }
    // IDLE
    else if (phase_sum > 50.0)
    {
      new_day_reason += "idle\n";
      fchg = 0;
    }
    else if(fchg) // no change from prior mode and chg on
    {
      new_day_reason = old_day_reason;
    }
    else // no change from prior mode and chg oFF
    {
      new_day_reason = "idling\n";
    }

    // monitoring voltage charger logic
    // all these checks disable charger if conds met
    if(config.monitor_battery && fchg)
    {
      // HV Shutdown
      if(flags.shutdown_hvolt)
      {
        new_day_reason += "Idle, HV\n";
        fchg = 0;
      }
      // Shutdown
      else if(cell_volt_high > config.battery_volt_max)
      {
        new_day_reason += "Idle, battery full\n";
        fchg = 0;
      }
    }

    mode_reason += new_day_reason;
    old_day_reason = new_day_reason;
  }

  // -------------------------------------------------------------------------
  // Select Mode

  uint8_t tmp_mode = 0;
  if(finv)
    tmp_mode = 2;
  if(fchg)
    tmp_mode = 1;

  if(config.prefer_dc && finv && fchg)
    tmp_mode = 3;

  // SET MODE
  modeset(tmp_mode);
}

// ======================================================================================================================

unsigned long boot_time = millis();

const unsigned long ap_time = 15 * 60 * 1000;

bool check_system_triggers() // returns 1 if a event was triggered
{
  // all checks should be flags

  if(flags.sdcard_read_error)
  {
    sd_setup(120);
    log_issue("SD Reconnected.");
    return 1;
  }

  if(flags.save_config)
  {
    vars_sanity_check();
    save_config();
    flags.save_config = 0;
    log_msg("Saved Config");
    return 1;
  }

  if(flags.restart)
  {
    oled_clear();
    oled_set2X();
    both_println(F("restart"));
    modeset(0);
    delay(500);
    ESP.restart();
  }

  // WiFi disconnected trigger
  if (!flags.access_point && WiFi.status() != WL_CONNECTED)
  {
    oled_clear();
    both_println(F("WiFi ERROR"));
    modeset(0);
    flags.restart = !wifi_start(); // seems to need a restart if wifi is out here.
    return 1;
  }

  // BOOT OK trigger
  if(!flags.boot_success)
  {
    oled_clear();
    oled_set1X();
    flags.boot_success = 1;
    String tmp = "FW: " + String(FW_VERSION);
    both_println(tmp);
    log_issue(tmp);

    return 1;
  }

  // update trigger
  if(flags.update_self) // ideally we would do this via timer to allow webpage to respond....
  {
    oled_clear();
    oled_set2X();
    both_println("Self\nUpdate");

    do_update();
    return 1;
  }

  // download html trigger
  if(!flags.access_point && flags.download_html)
  {
    download_html_from_remote();
    return 1;
  }



  return 0;
}

// ======================================================================================================================

bool check_system_timers()
{
  bool result = 0;

  // all if statements should be timer based

  // data source timeout trigger (time based)
  if
  (
    !flags.access_point &&
    millis() - timers.pgrid_last_update > check_timeout
  )
  {
    oled_clear();
    both_print(F("CHECK\nTIMEOUT"));

    flags.restart = 1;
    return 1;
  }

  // AP Mode restart trigger (time based)
  if(flags.access_point && millis() - boot_time > ap_time)
  {
    oled_clear();
    both_print(F("AP\nTIMEOUT"));
    flags.restart = 1;
    return 1;
  }

  if((config.blink_led || config.blink_led_default) && timers.led < millis())
  {
    timers.led = millis() + 500;
    set_led(3);
  }

  // OLED timer
  if(millis() > timers.oled)
  {
    if(timers.oled > 0)
      oled_print_info();

    result = 1;
    timers.oled = millis() + 1750;
  }

  if(flags.i2c_on && millis() > timers.i2c_check)
  {
    if(i2cdevcount != i2c_enum())
    {
      log_issue("i2c error. triggering restart.");
      flags.restart = 1;
      result = 1;
    }
    timers.i2c_check = millis() + (60 * 1000);
  }

  // auto update timer
  if(config.auto_update && millis() > timers.update_check)
  {
    if(timers.update_check > 0) // dont trigger on fist boot
    {
      oled_clear();
      oled_set2X();
      both_println("Auto\nUpdate");
      Serial.println(timers.update_check);

      do_update();
    }

    timers.update_check = millis() + 86400000; // every 24h
    result = 1;
  }

  // NTP sync timer
  if(!flags.access_point && millis() > timers.ntp_sync)
  {
    sync_time();

    if(flags.time_synced)
      timers.ntp_sync = millis() + 21600000; // 6 hours
    else
      timers.ntp_sync = millis() + 20000; // 20 s

    result = 1;
  }

  return result;
}

// ======================================================================================================================

uint8_t cds_pos = 0;

// note barrel setup. because some checks are time intensive.
bool check_data_sources()
{
  bool result = 0;

  cds_pos++;
  if(cds_pos >= 2)
    cds_pos = 0;

  // fronius grid check
  if(cds_pos == 0 && millis() > timers.pgrid)
  {
    check_grid();

    timers.pgrid = millis() + 1333;
    result = 1;
  }

  // ADC poll
  if(cds_pos == 1 && (config.monitor_battery || flags.lm75a) && millis() > timers.adc_poll)
  {
    if(config.monitor_battery)
    {
      adsmux.adc_poll();
      cells_update();
      check_cells();
    }

    if(flags.lm75a)
      board_temp = lm75a.getTemperature();

    timers.adc_poll = millis() + 50;
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

  uint8_t low_cell = 0;
  uint8_t high_cell = 0;


  unsigned long ms = millis();

  if((config.cells_in_series || config.cell_count == 1) && cells_volts_real[config.cell_count-1] < config.pack_volt_min)
    lv_trigger = 1;

  // cell checks
  for(byte i = 0; i < config.cell_count; i++)
  {
    // cell volt difference
    if(cells_volts[i] < cell_volt_low)
    {
      low_cell = i;
      cell_volt_low = cells_volts[i];
    }
    if(cells_volts[i] > cell_volt_high)
    {
      high_cell = i;
      cell_volt_high = cells_volts[i];
    }

    // HV check
    if(config.hv_monitor && cells_volts[i] >= config.battery_volt_max)
    {
      hv_trigger = 1;
      timers.hv_shutdown = ms + (config.hv_shutdown_delay * 3600000.0);
    }

    // LV Check
    if(cells_volts[i] <= config.battery_volt_min)
    {
      lv_trigger = 1;
      timers.lv_shutdown = ms + (config.lv_shutdown_delay * 3600000.0);
    }

    // LV recon
    if(lv_trigger || cells_volts[i] < config.battery_volt_rec)
    {
      lv_recon_trigger = 0; // if cells below min voltage, keep shutdown (no night time drain)
    }
  }
  cell_volt_diff = cell_volt_high - cell_volt_low;
  cell_lh[0] = low_cell;
  cell_lh[1] = high_cell;

  String now_str = datetime_str(0, '/', ' ', ':');

  // ----------------------------------------------------------------------
  // flags.shutdown_hvolt check

  if(config.hv_monitor)
  {
    // HV recon check
    if(!hv_trigger && flags.shutdown_hvolt && millis() > timers.hv_shutdown)
    {
      flags.shutdown_hvolt = 0;
      log_issue("HV recon, highest cell: " + String(high_cell+1) + " - " + String(cells_volts[high_cell]) + "v");
    }

    // HV disconnect check
    if(hv_trigger && !flags.shutdown_hvolt)
    {
      flags.shutdown_hvolt = 1;
      log_issue("HV shutdown, highest cell: " + String(high_cell+1) + " - " + String(cells_volts[high_cell]) + "v");
    }
  }

  // ----------------------------------------------------------------------
  // flags.shutdown_lvolt

  // reconnect check
  if(lv_recon_trigger && flags.shutdown_lvolt && millis() > timers.lv_shutdown)
  {
    flags.shutdown_lvolt = 0;
    log_issue("LV reconnect, lowest cell: " + String(low_cell+1) + " - " + String(cells_volts[low_cell]) + "v");
  }

  // flags.shutdown_lvolt check
  if(lv_trigger && !flags.shutdown_lvolt)
  {
    flags.shutdown_lvolt = 1;

    log_issue("LV Shutdown, lowest cell: " + String(low_cell+1) + " - " + String(cells_volts[low_cell]) + "v");
  }
}

bool check_grid()
{
  if(flags.access_point)
    return 0;

  if(update_p_grid() == 0)
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

void set_led(const char m)
{
  if(config.pin_led == OPT_DISABLE)
    return;

  if(m == 3) // toggle without changing value of config.led_status
  {
    digitalWrite(config.pin_led, !digitalRead(config.pin_led));
    return;
  }

  config.blink_led = 0;

  if(m == 0)
    config.led_status = 0;
  else if (m == 1)
    config.led_status = 1;
  else if (m == 2) // toggle
    config.led_status = !config.led_status;

  digitalWrite(config.pin_led, config.led_status);
}

void both_print_ip()
{
  oled_set1X();
  oled_print(F("IP:    "));
  oled_println(WiFi.localIP().toString());

  Serial.println("IP: " + WiFi.localIP().toString());
}

void oled_print_info()
{
  oled_clear();
  oled_set2X();

  if(flags.access_point)
  {

    both_println(F("AP Mode"));
    both_print_ip();
    both_println(F("SSID: SolarAP"));

    unsigned long rbtime = ap_time - (millis() - boot_time);
    rbtime /= 1000;
    rbtime /= 60;
    both_println(String(F("Reboot in:\n")) + String(rbtime) + String(F(" min")));
    oled_set2X();

    return;
  }

  if(config.display_mode == 1)
  {
    oled_println("A " + String(phase_a_watts, 1) + "W");
    oled_println("B " + String(phase_b_watts, 1) + "W");
    oled_println("C " + String(phase_c_watts, 1) + "W");
    oled_set1X();
    oled_println(WiFi.localIP().toString());

    return;
  }
  if(config.display_mode == 2)
  {
    oled_println("A " + String(phase_a_voltage, 1) + "v");
    oled_println("B " + String(phase_b_voltage, 1) + "v");
    oled_println("C " + String(phase_c_voltage, 1) + "v");
    oled_set1X();
    oled_println(WiFi.localIP().toString());

    return;
  }
  if(config.display_mode == 3)
  {
    oled_println("A " + String(phase_a_watts / phase_a_voltage, 2) + "A");
    oled_println("B " + String(phase_b_watts / phase_b_voltage, 2) + "A");
    oled_println("C " + String(phase_c_watts / phase_c_voltage, 2) + "A");
    oled_set1X();
    oled_println(WiFi.localIP().toString());

    return;
  }
  if(config.display_mode == 4)
  {
    float tpsum = 0;

    // NOTE: Western Power and other cunts bill ignoring exports or charging 7c for export on phase A and 29c for import on B or C.

    /*
    //tpsum = phase_a_watts + phase_b_watts + phase_c_watts;
    if(phase_a_watts > 0)
      tpsum += phase_a_watts;
    if(phase_b_watts > 0)
      tpsum += phase_b_watts;
    if(phase_c_watts > 0)
      tpsum += phase_c_watts;
    */
    tpsum = get_watts(3);

    if(tpsum >= 1000)
      oled_println("Now " + String(tpsum/1000, 2) + "kW");
    else
      oled_println("Now " + String(tpsum, 0) + "W");

    oled_println("Day " + String(energy_consumed, 1) + "kWh");

    oled_println("  $ " + String((energy_consumed) * config.cpkwh, 2) );

    oled_set1X();
    oled_println("");
    oled_println(WiFi.localIP().toString());

    return;
  }

  // ----------------------------------------------------------------------
  // oled print info


  {
    if (system_mode == 0)
      oled_print(F("IDL "));
    else if (system_mode == 1)
      oled_print(F("CHG "));
    else if (system_mode == 2)
      oled_print(F("DRN "));
    else if (system_mode == 3)
      oled_print(F("C&D "));

    oled_println(datetime_str(2, ' ', ' ', ':'));

    oled_print(F("KWh  "));
    oled_println(String(phase_sum / 1000, 2));
  }

  if(flags.lm75a)
  {
    oled_set2X();
    oled_println(String(board_temp, 2) + "c\n" );
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

  both_print_ip();

  if(config.monitor_battery)
  {
    if(config.cell_count < 4)
    {
      for(byte i=0; i< config.cell_count; i++)
      {
        oled_print(String(cells_volts[i]) + F(" "));
      }
      oled_println(F("v"));
    }
    else
    {
      float tsum = 0;
      for(byte i=0; i< config.cell_count; i++)
        tsum += cells_volts[i];

      tsum = tsum / config.cell_count;

      // low_cell
      oled_print("L " + String(cell_lh[0]) + ": " + String(cells_volts[cell_lh[0]], 2) + " ");

      // high_cell
      oled_print("H " + String(cell_lh[1]) + ": " + String(cells_volts[cell_lh[1]], 2));


    }
  }
  oled_set2X();
}

void calc_next_update()
{
  uint32_t i_sec = config.inverter_oot_sec + (config.inverter_oot_min * 60);
  uint32_t c_sec = config.charger_oot_sec + (config.charger_oot_min * 60);

  uint32_t rest_s = 30;

  if(system_mode == 0) // IDLE
  {
    rest_s = random(5, 10);
  }
  else if(system_mode == 3) // use which ever one is greater (inverter / charger)
  {
    if(i_sec < c_sec)
      rest_s = c_sec;
    else
      rest_s = i_sec;
  }
  else if(system_mode == 1) // CHARGER
  {
    rest_s = c_sec;
  }
  else if (system_mode == 2) // INVERTER
  {
    rest_s = i_sec;
  }

  timers.mode_check = millis() + (rest_s * 1000);
}

void modeset(byte m) // overload helper
{
  modeset(m, 0);
}
void modeset(byte m, bool manual)
{
  // if swapping direct from one device to another, force a n Second idle in between
  bool idle_forced = 0;
  if( (system_mode == 1 && m == 2) || (system_mode == 2 && m == 1) )
  {
    idle_forced = 1;
    m = 0;
  }

// check prior mode - set cooldown timers
  if(m != system_mode) // we care about prior mode (system_mode) which hasnt changed yet but will.
  {
    // log mode change
    String tmp = "";
    if(m == 0)
      tmp = "Idle";
    else if(m == 1)
      tmp = "Charge";
    else if(m == 2)
      tmp = "Drain";
    else if(m == 3)
      tmp = "Both";
    else
      tmp = String(m)  + "?";

    if(manual)
      tmp += " - Manual";

    log_msg(String("modeset: ") + tmp );

    // update timers IF swapping modes from inverter on OR charger on
    if
    (
      (system_mode == 1 || system_mode == 3) && // charger is on in modes 1 & 3
      ( m != 1 && m != 3)  // swapping to a non charging mode
    ) // charger turning off
    {
      if(config.c_offd) // custom off delay set
        timers.charger_off = millis() + (config.charger_off_min * 60 * 1000) + (config.charger_off_sec * 1000);
      else
        timers.charger_off = millis() + (config.charger_oot_min * 60 * 1000) + (config.charger_oot_sec * 1000);
    }
    else if
    (
      (system_mode == 2 || system_mode == 3) && // inverter is on in modes 2 & 3
      (m != 2 && m != 3) // mode swapping to a non inverter mode
    ) // inverter turning off
    {
      timers.inverter_off = millis() + (config.inverter_oot_min * 60 * 1000) + (config.inverter_oot_sec * 1000);
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
  else if (m == 3)  // BOTH SYSTEMS ON (used for prefer DC)
  {
    i_pinmode = 1;
    c_pinmode = 1;
  }

  else // BAD MODE !!!
  {
    while (1)
    {
      oled_clear();
      both_println("BAD MODE");
      delay(1000);
    }
  }

  bool same_mode = 0;

  if(system_mode == m)
    same_mode = 1;

  system_mode = m;

  if(idle_forced)
    timers.mode_check = millis() + random(1000, 5000); // 1 - 5 secs
  else if(same_mode)
    timers.mode_check = millis() + random(500, 4000); // 0.5 to x secs
  else
    calc_next_update();

  if(config.flip_cpin)
    c_pinmode = !c_pinmode;
  if(config.flip_ipin)
    i_pinmode = !i_pinmode;


  // set pin modes and return
  if(config.pin_charger != OPT_DISABLE)
    digitalWrite(config.pin_charger, c_pinmode);

  if(config.pin_inverter != OPT_DISABLE)
    digitalWrite(config.pin_inverter, i_pinmode);
}

