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
    SDERROR = 1;
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
    SDERROR = 1;
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
//     SDERROR = 1;
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
    SDERROR = 1;
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

bool ping_fs()
{

  if (SD.exists(json_config_file.c_str() ) )
  {
    return 1;
  }

  SDERROR = 1;
  return 0;
}

void setup_fs()
{
  both_println(F("FS setup"));

  if (!SD.exists(json_config_file.c_str() ) )
  {
    File f = SD.open(json_config_file.c_str());

    Serial.print(F("FS TEST: "));
    if (!f)
      Serial.println(F("FAIL"));
    else
      Serial.println(F("OK"));

    f.close();
  }
  else
  {
    Serial.println(F("OK"));
  }

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
