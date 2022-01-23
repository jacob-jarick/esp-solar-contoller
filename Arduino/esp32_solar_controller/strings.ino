
void string_append_limit_size(String &str, const String str_append, const uint16_t limit)
{

  if(str.length() +  str_append.length() > limit)
    str = str_append;
  else
    str += str_append;

  return;
}


void log_msg(const String msg)
{
  Serial.println(msg + "\n");
  String tmp_str = datetime_str(0, '/', ' ', ':') + " - " + msg + "\n";
  string_append_limit_size(system_msgs, tmp_str, size_system_msgs);
}
