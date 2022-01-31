/*

Note I use esp32 doit dev kit v1.
but the arduino IDE settings for this board seem to cause issues.

so I configure the IDE to use:

* WEMOS LOLIN32
* Patition Scheme: Minimal SPIFFS, large app with OTA
* CPU 240Mhz (WiFi/BT)
* Flash Frequency 80Mhz

this seems to resolve OTA issues.

*/

#define FW_VERSION 380

// to longer timeout = esp weirdness
#define httpget_timeout 5000

// 10 min
#define check_timeout 600000

#define OPT_DISABLE -2
#define OPT_DEFAULT -1

#define stiny 32
#define ssmall 64
#define smedium 128
#define slarge 257

#define MAX_CELLS 16

// ======================================

#include <Arduino.h>
#include <SD.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h> // todo remove as WiFi is configured via SD card json config
#include <WiFiUdp.h>
#include <ESPmDNS.h>

#include <HTTPClient.h>

#include <ESP32httpUpdate.h>


#include <ArduinoJson.h>
const size_t jsonsize = 1024 * 4;

#include <WebServer.h>
WebServer server(80);

// -----------------------------------------------------------------------------------------

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

SSD1306AsciiWire oled;

// -----------------------------------------------------------------------------------------

#include <Ads1115_mux.h>
const uint8_t pin_asel1 = 26;
const uint8_t pin_asel2 = 17;
const uint8_t pin_asel3 = 14;
const uint8_t pin_asel4 = 27;

Ads1115_mux adsmux(pin_asel1, pin_asel2, pin_asel3, pin_asel4);

// -----------------------------------------------------------------------------------------
// lm75a

// note: modify library and comment out wire.begin and wire.end

#include <M2M_LM75A.h>
M2M_LM75A lm75a(0x4f);
M2M_LM75A lm75a2(0x48); // used on micro boards

uint8_t lm75a_address = 0x4f;

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
  unsigned long api = 0;
  unsigned long iapi = 0;
  unsigned long api_last_update = 0;

  unsigned long mode_check = 0;
  unsigned long pgrid_last_update = 0;
  unsigned long use_fallback = 0;

  unsigned long lv_shutdown = 0;
  unsigned long hv_shutdown = 0;

  unsigned long charger_off = 0;
  unsigned long inverter_off = 0;

  unsigned long pgrid = 0;
  unsigned long adc_poll = 0;
  unsigned long lm75a_poll = 0;

  unsigned long update_check = 0;
  unsigned long ntp_sync = 0;
  unsigned long oled = 0;

  unsigned long led = 0;

  unsigned long i2c_check = 0;

  unsigned long calc_energy_usage = 0;
};

SysTimers timers;

//------------------------------------------------------------------------------
// cell voltages

double cells_volts[MAX_CELLS*3];
double cells_volts_real[MAX_CELLS*3]; // used to track real voltage when using AIN, used as temp storage for API
float cell_volt_diff = 0;
float cell_volt_high = 0;
float cell_volt_low = 0;

float pack_total_volts = 0;

// uint8_t cell_lh[2] = {0, 0};
uint8_t low_cell = 0;
uint8_t high_cell = 0;

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
const String html_acinfo          = "/acinfo" + dothtml;
const String html_cpconfig        = "/cpconfig" + dothtml;
const String html_stats           = "/stats" + dothtml;
const String html_mode            = "/mode" + dothtml;
const String html_apiservers      = "/api" + dothtml;
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
const String txt_passwd_file      = "/passwd";
const String json_config_file     = "/config.jsn";

//------------------------------------------------------------------------------

const uint16_t size_system_msgs = 1024;
String system_msgs = "";

float board_temp = 0;
float board_temp_min = 9999999;
float board_temp_max = 0;
float board_temp_old = 0; // only used to track changes in temp to prevent message spam.

float phase_a_watts = 0;
float phase_b_watts = 0;
float phase_c_watts = 0;

float phase_a_voltage = 0;
float phase_b_voltage = 0;
float phase_c_voltage = 0;

float phase_a_voltage_high = 0;
float phase_b_voltage_high = 0;
float phase_c_voltage_high = 0;

float phase_a_voltage_low = 10000;
float phase_b_voltage_low = 10000;
float phase_c_voltage_low = 10000;


float energy_consumed_old = 0; // previous day
float energy_consumed = 0;

float phase_sum = 0;
float phase_sum_old = 0;

float phase_avg = 0;

String passwd = "";

uint8_t download_index = 0; // html page download index
byte system_mode = 0;

uint8_t i2cdevcount = 0; // used on startup and then for system i2c ping

const int8_t power_array_size = 10;

int get_url_code; // global url fetch code, eg 404, 401, 200

// =================================================================================================


//------------------------------------------------------------------------------

const uint8_t max_api_vservers = 3;
const uint8_t max_api_iservers = 1;

struct Sconfig
{
  uint16_t fwver = 0;

  float maxsystemtemp = 40;


  // API

  uint8_t api_vserver_count = 0;


  bool api_venable[max_api_vservers];
  char api_vserver_hostname[max_api_vservers][ssmall];

  bool api_ienable[max_api_iservers];
  char api_iserver_hostname[max_api_iservers][ssmall];

  bool api_lm75a = 0;
  bool api_grid = 0;

  float api_pollsecs = 1;

  // END of API

  char threephase_direct_url[smedium];
  char fronius_push_url[smedium];
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

  uint8_t i2cmaxspeed;

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

  bool avg_ain = 0;

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
  bool monitor_battery = 0;

  bool serial_off = 0;

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

  bool mcptype = 0; // 0 = MCP3021, 1 = MCP3221
  bool ads1x15type = 0; // 0 = ADS1015, 1 = ADS1115
  uint8_t muxtype = 0; // 0 = old board (2* 8-1), 1 = new boards (16-1), 2 = high precision board

  bool dumbsystem = 0; // 1 dont check fronius, turn on SSRs etc
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

  bool fronius_error = 0; // Fronius 3p Fallback JSON Decode Error:

  bool adc_config_error = 0;
  bool boot_success = 0;

  bool i2c_on = 1;

  bool cells_checked = 0;

  bool ambient_temp = 0;

  bool api_checked = 0;
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
      if(config.i2cmaxspeed >= 1)
      {
        Wire.setClock(1000000L); // fast mode plus
        i2c_test_enum1 = i2c_enum();
      }

      if(config.i2cmaxspeed == 2)
      {
        Wire.setClock(3400000L); // high speed mode (buggy on latest esp32 board)
        i2c_test_enum2 = i2c_enum();
      }
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
      {
        lm75a_address = 0x4f;
        flags.lm75a = 1;
      }
      else if(config.board_rev == 2 && i2c_ping(0x48))
      {
        lm75a_address = 0x48;
        flags.lm75a = 1;
      }

      adsmux.mcptype = config.mcptype;
      adsmux.ads1x15type = config.ads1x15type;
      adsmux.muxtype = config.muxtype;
      adsmux.avg_ain = config.avg_ain;
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
    server.on("/jsonapi", jsonapi);
    server.on("/json_cells", json_cells);

    server.on("/apiservers", apiservers);

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

    server.on("/ac_info", ac_info);

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
  }
  else
  {
    // check config, if volt monitoring enabled but no i2c dev - display alert, Force IDLE ALWAYS

    if(config.monitor_battery && !config.api_venable[0])
    {
      log_msg("monitor_battery enabled but ADC not found");
      flags.adc_config_error = 1;
    }
  }

  // --------------------------------------------------------------------------------------
  // check if FW was updated

  if(config.fwver != FW_VERSION)
  {
    log_msg("Config FW version " + String(config.fwver) + " != FW version " + String(FW_VERSION) + ". Downloading HTML.");
    flags.download_html = 1;

    config.fwver = FW_VERSION;
    save_config();
  }

  log_msg("system startup finished OK.");

  // turn off serial if set in config.

  if(config.serial_off)
  {
    Serial.end();
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

void loop()
{
  server.handleClient();

  if(check_system_triggers())
    return;

  // ----------------------------------------------------------------------

  check_system_timers();
  set_daynight();
  check_data_sources();

  if(config.dumbsystem)
  {
    return;
  }

  // ----------------------------------------------------------------------
  // ADC Error ?

  if(!config.api_venable[0] && flags.adc_config_error)
  {
    mode_reason = datetime_str(3, '/', ' ', ':') + ": Config ERROR.\nconfig requires ADC, but ADC not found.";

    modeset(0);
    return;
  }




  // ----------------------------------------------------------------------
  // waiting on API ?

  if(config.api_venable[0] && !flags.api_checked)
  {
    mode_reason = datetime_str(3, '/', ' ', ':') + ": waiting on first API check.";
    modeset(0);

    return;
  }

  // ----------------------------------------------------------------------
  // Waiting on ADC fist complete poll.

  if(config.monitor_battery && !flags.cells_checked)
  {
    mode_reason = datetime_str(3, '/', ' ', ':') + ": waiting on cells first check";
    modeset(0);

    return;
  }

  // ----------------------------------------------------------------------

  // check if cell volt is low and force update

  if(config.hv_monitor && config.monitor_battery && system_mode == 1 && cell_volt_high > config.battery_volt_max)
  {
    mode_reason = "cell over volt (" + String(cell_volt_high, 3) + "v). force IDLE.";
    log_msg(mode_reason);
    mode_reason = datetime_str(3, '/', ' ', ':') + ": " + mode_reason;

    modeset(0);

    return;
  }

  // check if cell volt is over and force update ?

  if(config.monitor_battery && system_mode == 2 && cell_volt_low <= config.battery_volt_min)
  {
    mode_reason = "Cell under volt (" + String(cell_volt_low, 3) + "v), force IDLE";
    mode_reason = datetime_str(3, '/', ' ', ':') + ": " + mode_reason;

    modeset(0);

    return;
  }


  // finish loop unless its time to update
  if (timers.mode_check > millis())
    return;

  mode_reason = datetime_str(3, '/', ' ', ':') + ": ";

  // ----------------------------------------------------------------------
  // environment to hot check
  if(flags.lm75a && board_temp > config.maxsystemtemp)
  {
    String msg = "System Overheat, Ambient Temp " + String(board_temp, 2) + "c, (Max " + String(config.maxsystemtemp, 1) + "c)";

    if(!flags.ambient_temp)
      log_issue(msg);

    flags.ambient_temp = 1;

    mode_reason += msg;

    modeset(0);
    return;
  }

  if(flags.ambient_temp && board_temp + 0.5 > config.maxsystemtemp)
  {
    String msg = "System Cooling Down, Ambient Temp " + String(board_temp, 2) + "c";

    if(board_temp_old != board_temp)
    {
      board_temp_old = board_temp;
      log_msg(msg);
    }
    mode_reason += msg;
    return;
  }

  // if flag set, turn off, log recovery
  if(flags.ambient_temp)
  {
    String msg = "System Overheat recover, Ambient Temp " + String(board_temp, 2) + "c";
    log_issue(msg);
    flags.ambient_temp = 0;
  }


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

  if(system_mode == 2 || system_mode == 3)
    finv = 1;

  if(system_mode == 1  || system_mode == 3)
    fchg = 1;

  // -------------------------------------------------------------------------
  // Night Device

  mode_reason = datetime_str(3, '/', ' ', ':') + "\n"; // made it this far, so reason will likely be multiline.

  // discharger
  if (config.i_enable && flags.night)
  {
    String new_night_reason = "";
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

    mode_reason += "Night: " + new_night_reason;
    old_night_reason = new_night_reason;
  }


  // -------------------------------------------------------------------------
  // Day Device

  // charger
  if (config.c_enable && flags.day)
  {
    String new_day_reason = "";

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
      new_day_reason += old_day_reason;
      //new_day_reason += "OLD";
    }
    else // no change from prior mode and chg oFF
    {
      new_day_reason += "idling\n";
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
    mode_reason += "Day: " + new_day_reason;
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
    log_issue("Restarting, flag set");
    modeset(0);
    delay(500);
    ESP.restart();
  }

  // WiFi disconnected trigger
  if (!flags.access_point && WiFi.status() != WL_CONNECTED)
  {
    oled_clear();
    both_println(F("WiFi ERROR"));

    log_issue("Restarting, WiFi disconnected");

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

  // Grid data source timeout trigger (time based)
  if
  (
    !config.dumbsystem &&
    !config.api_grid &&
    !flags.access_point &&
    timers.pgrid_last_update != 0 &&
    millis() - timers.pgrid_last_update > check_timeout
  )
  {
    oled_clear();
    both_print(F("CHECK\nTIMEOUT"));

    log_issue("Restarting, Grid datasource timeout");

    flags.restart = 1;
    return 1;
  }

  // API source timeout trigger (time based)
  if
    (
      !flags.access_point &&
      config.api_venable[0] &&
      timers.api_last_update != 0 &&
      millis() - timers.api_last_update > (15 * 60 * 1000)
    )
    {
      oled_clear();
      both_print(F("CHECK\nTIMEOUT"));

      String msg = "Restarting, API datasource timeout";
      Serial.println("\n\n" + msg);
      log_issue(msg);

      flags.restart = 1;
      return 1;
    }

  // AP Mode restart trigger (time based)
  if(flags.access_point && millis() > ap_time)
  {
    oled_clear();
    both_print(F("AP\nTIMEOUT"));

    log_issue("Restarting, AP Timeout");
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

    if(config.dumbsystem)
      timers.oled = millis() + 10 * 1000;
    else
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
    timers.i2c_check = millis() + (15 * 60 * 1000);
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


uint8_t adc_pcount = 0; // check data sources position count. - min poll count before running check_cells

unsigned long adc_pps_ms = millis(); // time of last complete poll
float adc_poll_time = 0;



bool check_data_sources()
{
  bool result = 0;

  // API Check

    if(config.api_ienable[0] && millis() > timers.iapi)
  {
    bool api_result = api_isync(0);

    if(api_result)
      timers.iapi = millis() + (1000 * config.api_pollsecs);
    else
      timers.iapi = millis() + (httpget_timeout * 3); // if failed, fallback to safe poll time.

    result = 1;
  }

  if(config.api_venable[0] && millis() > timers.api)
  {
    bool api_result = api_poller();

    if(api_result)
      timers.api = millis() + (1000 * config.api_pollsecs);
    else
      timers.api = millis() + (httpget_timeout * 3); // if failed, fallback to safe poll time.

    result = 1;
  }


  // fronius grid check
  if(!config.dumbsystem && !config.api_grid && millis() > timers.pgrid)
  {
    check_grid();

    timers.pgrid = millis() + 1333;
    result = 1;
  }

  // ADC poll
  if(!config.api_venable[0] && config.monitor_battery && millis() > timers.adc_poll)
  {
//     log_msg("ADC Poll");
    // wait if pins have been set, go next loop immediately otherwise (dont update timer)
    bool pins_set = adsmux.adc_poll();

    if(!pins_set)
      timers.adc_poll = millis() + 10; // be reasonable, also lets mux output settle

    // if polling complete, check cells etc
    if(adsmux.polling_complete)
    {
      unsigned long tmp_ms = millis() - adc_pps_ms;
      adc_pps_ms = millis();
      adc_poll_time = tmp_ms / 1000.0;

      // wait (adsmux.ain_history_size) full polls a little for voltages to smooth.
      if(adc_pcount > adsmux.ain_history_size)
      {
        cells_update();
        check_cells();
      }
      else
      {
        adc_pcount++;
      }
    }

    result = 1;
  }

  if(!config.api_lm75a && flags.lm75a && millis() > timers.lm75a_poll)
  {
    if(lm75a_address == 0x48)
      board_temp = lm75a2.getTemperature();
    else if(lm75a_address == 0x4f)
      board_temp = lm75a.getTemperature();

    board_temp_max = mmaths.mmax(board_temp_max, board_temp);
    board_temp_min = mmaths.mmin(board_temp_min, board_temp);

    timers.lm75a_poll = millis() + 666;

    result = 1;
  }

  return result;
}

void check_cells()
{
  flags.cells_checked = true;

  cell_volt_low = 10000;
  cell_volt_high = 0;

//   bool lv_recon_trigger = 1; // reconnect if all cells are above battery_volt_rec
  bool lv_trigger = 0;
  bool hv_trigger = 0;

  unsigned long ms = millis();

  // single cell check
  if(config.cell_count == 1 && cells_volts_real[0] < config.pack_volt_min)
    lv_trigger = 1;

  // cell checks
  pack_total_volts = 0;
  for(byte i = 0; i < config.cell_count; i++)
  {
    pack_total_volts += cells_volts[i]; // calculate pack total.

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
  }

  // after loop preform trigger checks

  // HV check
  if(config.hv_monitor && cells_volts[high_cell] >= config.battery_volt_max)
  {
    hv_trigger = 1;
    timers.hv_shutdown = ms + (config.hv_shutdown_delay * 3600000.0);
  }

  // LV Check
  if(cells_volts[low_cell] <= config.battery_volt_min)
  {
    lv_trigger = 1;
    timers.lv_shutdown = ms + (config.lv_shutdown_delay * 3600000.0);
  }

  cell_volt_diff = cell_volt_high - cell_volt_low;

  String now_str = datetime_str(0, '/', ' ', ':');

  // ----------------------------------------------------------------------
  // flags.shutdown_hvolt check

  if(config.hv_monitor)
  {
    // HV recon check
    if(!hv_trigger && flags.shutdown_hvolt && millis() > timers.hv_shutdown)
    {
      flags.shutdown_hvolt = 0;

      String tmsg = "HV Reconnect.\n";
      for(uint8_t i = 0; i < config.cell_count; i++)
      {
        String tspacer = ":\t";

        if(i == high_cell) // highlight highest cell
          tspacer = ":*\t";

        tmsg += "   " + String(i+1) + tspacer + String(cells_volts[i], 3) + "\n";
      }
      log_issue(tmsg);
    }

    // ----------------------------------------------------------------------
    // HV disconnect check
    if(hv_trigger && !flags.shutdown_hvolt)
    {
      flags.shutdown_hvolt = 1;

      String tmsg = "HV Disconnect.\n";
      for(uint8_t i = 0; i < config.cell_count; i++)
      {
        String tspacer = ":\t";

        if(cells_volts[i] >= config.battery_volt_max) // highlight cells over volt
          tspacer = ":*\t";

        tmsg += "   " + String(i+1) + tspacer + String(cells_volts[i], 3) + "\n";
      }
      log_issue(tmsg);
    }
  }

  // ----------------------------------------------------------------------
  // flags.shutdown_lvolt check
  if(lv_trigger && !flags.shutdown_lvolt)
  {
    flags.shutdown_lvolt = 1;

    String tmsg = "LV Shutdown.\n";
    for(uint8_t i = 0; i < config.cell_count; i++)
    {
      String tspacer = ":\t";

      if(cells_volts[i] <= config.battery_volt_min) // if below min volts, highlight with an *
        tspacer = ":*\t";

      tmsg += "   " + String(i+1) + tspacer + String(cells_volts[i], 3) + "\n";
    }

    log_issue(tmsg);
  }

  // ----------------------------------------------------------------------
  // reconnect check
  if(flags.shutdown_lvolt && cells_volts[low_cell] >= config.battery_volt_rec && millis() > timers.lv_shutdown)
  {
    flags.shutdown_lvolt = 0;

    String tmsg = "LV Reconnect.\n";
    for(uint8_t i = 0; i < config.cell_count; i++)
    {
      String tspacer = ":\t";

      if(i == low_cell) // highlight lowest cell
        tspacer = ":*\t";

      tmsg += "   " + String(i+1) + tspacer + String(cells_volts[i], 3) + "\n";
    }

    log_issue(tmsg);
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

  calc_energy_usage();

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

    const unsigned long ms = millis();
    unsigned long rbtime = 0;
    if(ap_time > ms)
      rbtime = ap_time - ms;

    rbtime /= 1000;
    rbtime /= 60;
    both_println(String(F("Reboot in:\n")) + String(rbtime) + String(F(" min")));
    oled_set2X();

    return;
  }

  if(config.dumbsystem)
  {
    oled_set2X();
    oled_println(config.hostn);
    oled_println(datetime_str(2, ' ', ' ', ':'));
    oled_set1X();
    oled_println("");
    oled_println(WiFi.localIP().toString());

    return;
  }

  // if over board temp display warning.
  if(flags.lm75a && board_temp > config.maxsystemtemp)
  {
    oled_set2X();
    oled_println("TOO HOT !!");
    oled_println("\n  " + String(board_temp, 2) + "c\n" );

    oled_set1X();
    oled_println("");
    oled_println(WiFi.localIP().toString());
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
  if(config.display_mode == 5) // adams display
  {
    oled_println("V " + String(phase_a_voltage, 2));
    oled_println("W " + String(phase_a_watts, 2));
    oled_println("A " + String(phase_a_watts / phase_a_voltage, 2));

    oled_set1X();
    oled_println("");
    oled_println(WiFi.localIP().toString());

    return;
  }
  if(config.display_mode == 6)
  {
    oled_println("V " + String(phase_b_voltage, 2));
    oled_println("W " + String(phase_b_watts, 2));
    oled_println("A " + String(phase_b_watts / phase_b_voltage, 2));

    oled_set1X();
    oled_println("");
    oled_println(WiFi.localIP().toString());

    return;
  }
  if(config.display_mode == 7)
  {
    oled_println("V " + String(phase_c_voltage, 2));
    oled_println("W " + String(phase_c_watts, 2));
    oled_println("A " + String(phase_c_watts / phase_c_voltage, 2));

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
    // low_cell
    oled_print("L " + String(low_cell+1) + ": " + String(cells_volts[low_cell], 2) + " ");

    // high_cell
    oled_print("H " + String(high_cell+1) + ": " + String(cells_volts[high_cell], 2));
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
