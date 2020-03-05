
String string_append_limit_size(const String str, const String str_append, const uint limit)
{
  String result = str + str_append;
  if(result.length() > limit)
    result = str_append;

  return result;
}


void log_error_msg(const String msg)
{
  String tmp_str = datetime_str(0, '/', ' ', ':') + " - " + msg + "\n";
  error_msgs = string_append_limit_size(error_msgs, tmp_str, size_error_msgs);
}

String frline(const int l)
{
  if (!SD.exists(txt_system_messages.c_str() ) )
  {
//     both_println(F("conf not found"));
    return "";
  }
  //  Serial.println(F("read config"));
  //Read File data
  File f = SD.open(txt_system_messages.c_str());

  if (!f)
  {
    both_println(F("fopen failed"));
    //     Serial.println("load_config '" + config_file + "'fopen failed");

    return "";
  }

  String tmp = "";
  for(int i = 0; i < l; i++)
    tmp = f.readStringUntil('\n');

  f.close();
  return tmp;
}
