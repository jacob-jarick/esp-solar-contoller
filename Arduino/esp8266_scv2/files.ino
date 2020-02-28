// =============================================================================================================================================
// File Functions
// =============================================================================================================================================

String get_file(String fn)
{
  String contents = "";

  File f = SPIFFS.open(fn, "r");
  if (!f)
  {
    oled_clear();
    oled_println("get_file" + fn + "\nfailed.");
    Serial.println("get_file '" + fn + "' does not exist/ open failed.");

    return contents;
  }

  while (f.available())
  {
    contents += char(f.read());
  }
  f.close();

  return contents;
}

void null_file(const String fname)
{
  File f = SPIFFS.open(fname, "w");
  if (!f)
  {
    oled_println(F("fopen failed"));
    Serial.println("null_file '" + fname + "'fopen failed");

    return;
  }

  f.print("");

  f.close();
}

void file_limit_size(const String fname, int bsize)
{

  File f = SPIFFS.open(fname, "r");
  if (!f)
  {
    oled_println(F("fopen failed"));
    Serial.println("file_limit_size '" + fname + "'fopen failed");

    return;
  }

  int asize = f.size();
  f.close();

  if(asize > bsize)
    null_file(fname);
}

void log_issue(const String txt)
{
  if(txt.equals(""))
    return;

  File f = SPIFFS.open(txt_log_system, "a");
  if (!f)
  {
    oled_println(F("fopen failed"));
    Serial.println("log_issue '" + txt_log_system + "'fopen failed");

    return;
  }

  f.print(datetime_str(0, '/', ' ', ':'));
  f.print(F(" - "));
  f.println(txt);

  f.close();
}

// =============================================================================================================================================
// Config Functions
// =============================================================================================================================================

void setup_fs()
{
  both_println(F("FS setup"));

  //Initialize File System
  if (SPIFFS.begin())
    both_println(F("OK"));
  else
    both_println(F("bad"));

  if (!SPIFFS.exists(json_config_file))
  {
    both_println(F("wait 30s"));
    SPIFFS.format();
    both_println(F("OK"));

    File f = SPIFFS.open(json_config_file, "w");

    both_print(F("TEST "));
    if (!f)
      both_println(F("FAIL"));
    else
      both_println(F("OK"));
  }
  else
  {
    both_println(F("OK"));
  }
}

