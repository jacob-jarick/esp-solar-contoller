// =============================================================================================================================================
// File Functions
// =============================================================================================================================================

String get_file(const String fn)
{
  String contents = "";

  File f = SD.open(fn.c_str() );
  if (!f)
  {
    oled_clear();
    oled_println("get_file" + fn + "\nfailed.");
    Serial.println("get_file '" + fn + "' does not exist/ open failed.");
    flags.sdcard_read_error = 1;
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
  File f = SD.open(fname.c_str(), FILE_WRITE);
  if (!f)
  {
    oled_println(F("fopen failed"));
    Serial.println("null_file '" + fname + "'fopen failed");
    flags.sdcard_read_error = 1;
    return;
  }

  f.print("");

  f.close();
}

void file_limit_size(const String fname, const int bsize)
{

  File f = SD.open(fname.c_str());
  if (!f)
  {
    oled_println(F("fopen failed"));
    Serial.println("file_limit_size '" + fname + "'fopen failed");
//     flags.sdcard_read_error = 1;
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

  File f = SD.open(txt_log_system.c_str(), FILE_APPEND);
  if (!f)
  {
    oled_println(F("fopen failed"));
    Serial.println("log_issue '" + txt_log_system + "'fopen failed");
    flags.sdcard_read_error = 1;
    return;
  }

  String tmp = datetime_str(0, '/', ' ', ':') + " - " + txt;
  log_msg(tmp); // all issues get copied to syslog
  f.println(tmp);

  f.close();
}

// =============================================================================================================================================
// Config Functions
// =============================================================================================================================================

bool ping_fs()
{

  if (SD.exists(json_config_file.c_str() ) )
  {
    return 1;
  }

  flags.sdcard_read_error = 1;
  return 0;
}

void setup_fs()
{
  both_println(F("FS setup"));

  if(!ping_fs())
      Serial.println(F("FAIL"));
  else
    Serial.println(F("OK"));

  listDir();
}

void listDir()
{
  const char *dirname = "/";
  Serial.printf("Listing directory: %s\n", dirname);

  File root = SD.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file)
  {
    if(file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}


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
      flags.sdcard_read_error = 0;
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
