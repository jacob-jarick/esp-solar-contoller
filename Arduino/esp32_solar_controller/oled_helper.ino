// =============================================================================================================================================
// oled helper
// =============================================================================================================================================

bool oled_enabled()
{
  if(!flags.i2c_on)
    return 0;

  if (config.oled_addr == 0x3C || config.oled_addr == 0x3D)
    return 1;

  return 0;
}

void both_println(const String str)
{
  Serial.println(str);

  if(oled_enabled())
    oled_print(str + "\n");
}

void both_print(const String str)
{
  Serial.println(str);

  if(oled_enabled())
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

bool i2c_ping(const char address)
{
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();

  if(error == 0)
    return 1;

  return 0;
}

char oled_detect()
{
  byte arr_size = 2;
  char arr[arr_size] = {0x3C, 0x3D};

  for (uint8_t i = 0; i < arr_size; i++ )
  {
    if (i2c_ping(arr[i]) )
    {
      Serial.print(F("oled @ "));
      Serial.println(arr[i], HEX);
      return arr[i];
    }
  }

  Serial.print(F("No Oled Detected."));

  return 0;  // negtive result = not present
}


uint8_t i2c_enum()
{
  Serial.print("i2c_enum: ");

  uint8_t icount = 0;
  for (char address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (!error)
    {
      Serial.print("+");
      icount++;
    }
    else
    {
      Serial.print(".");
    }
  }

  Serial.println();
  return icount;  // negtive result = not present
}

void oled_setup()
{
  if(!flags.i2c_on)
  {
    return;
  }

  config.oled_addr = oled_detect();

  if (!oled_enabled())
  {
    Serial.print(F("not found"));
    return;
  }

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
