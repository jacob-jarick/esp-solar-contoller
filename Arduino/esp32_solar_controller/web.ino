

bool get_url(String &url, String &payload)
{
//   if(url.equals(""))
//   {
//     Serial.println("get_url: blank URL.");
//     return 0;
//   }

  bool result = 0;

  HTTPClient http;
  http.setTimeout(httpget_timeout);

  http.begin(url);
  get_url_code = http.GET();
  if (get_url_code == HTTP_CODE_OK)
  {
    payload = http.getString();
    http.end();

    result = 1;
  }
  else
  {
    payload = "";
    Serial.println("HTTP BAD Code:" + String(get_url_code, DEC) );
  }

  http.end();

  return result;
}

bool get_url_and_save(String &url, String &filepath)
{
  if(url.equals(""))
  {
    Serial.println("get_url_and_save: blank URL.");
    return 0;
  }

  bool result = 0;

  HTTPClient http;
  http.setTimeout(httpget_timeout);

  File f = SD.open(filepath.c_str(), FILE_WRITE);

  if (f)
  {
    http.begin(url);
    get_url_code = http.GET();
    if (get_url_code == HTTP_CODE_OK)
    {
      Serial.println("read file from HTTP OK");
      http.writeToStream(&f);
      result = 1;
    }
    else
    {
      Serial.println("get_url_and_save: HTTP BAD Code:" + String(get_url_code, DEC) );
    }
  }
  else
  {
    flags.sdcard_read_error = 1;
    Serial.println("get_url_and_save: cannot open '" + filepath);
  }

  f.close();
  http.end();
  return result;
}

