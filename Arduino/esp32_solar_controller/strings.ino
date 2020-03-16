
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
