// =============================================================================================================================================
// oled helper
// =============================================================================================================================================

bool oled_enabled()
{
  if (config.oled_addr == 0x3C || config.oled_addr == 0x3D)
    return 1;

  return 0;
}

void both_println(const String str)
{
  Serial.println(str);

  oled_print(str + "\n");
}

void both_print(const String str)
{
  Serial.println(str);

  oled_print(str);
}

void oled_println(const String str)
{
  oled_print(str + "\n");
}

void oled_print(String str)
{
  if (oled_enabled())
    oled.print(str);
  else
  {
//     if(serial_on)
    {
      Serial.print(F("oled: "));
      Serial.println(str);
    }
  }
}

void oled_clear()
{
  if (!oled_enabled())
    return;

  oled.clear();
}

bool i2c_ping(char address)
{
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();

  if(error == 0)
    return 1;

  return 0;
}

char i2c_detect(byte type)
{
  for (char address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0 && type == 0 && ( address == 0x3C || address == 0x3D))
    {
      Serial.print(F("oled @ "));
      Serial.println(address, HEX);
      return address;
    }
  }

  return 0;  // negtive result = not present
}

void oled_setup()
{
  config.oled_addr = i2c_detect(0);

  if (!oled_enabled())
    return;

  oled.begin(&Adafruit128x64, config.oled_addr);
  oled.setFont(Adafruit5x7);
  oled_set2X();

  oled_clear();

  if(config.rotate_oled)
    oled.displayRemap(true);

  oled_println(String(config.hostn));
}

void oled_set1X()
{
  if (!oled_enabled())
    return;

  oled.set1X();
}

void oled_set2X()
{
  if (!oled_enabled())
    return;

  oled.set2X();
}
