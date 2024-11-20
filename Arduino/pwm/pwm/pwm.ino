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

const uint8_t pin_buzzer = 16;

void setup()
{
  const uint8_t toneinc = 45;
  uint16_t tone = 90;

// OLD
// ledcSetup(ledChannel1, freq1, resolution);
// ledcAttachPin(redLED1, ledChannel);

// NEW
// bool ledcAttach(uint8_t pin, uint32_t freq, uint8_t resolution);

  //ledcSetup(0, 1000, 8);
  //ledcAttachPin(16, 0);
  ledcAttach(pin_buzzer, 1000, 8);
  
  Serial.begin(115200);

  Serial.println("\nStart");
  delay(200);

  //sd_setup
  Serial.println("sd_setup tone: " + String(tone) );
  beep_helper(tone, 250);
  delay(250);
  Serial.println("sd_setup tone: " + String(tone) );
  tone += toneinc; beep_helper(tone, 250);

  Serial.println("sd_setup tone: " + String(tone) );
  tone += toneinc; beep_helper(tone, 250);
  delay(250);
  Serial.println("sd_setup tone: " + String(tone) );
  tone += toneinc; beep_helper(tone, 250);
  delay(2000);

  // i2c Setup
  Serial.println("i2c Setup tone: " + String(tone) );
  tone += toneinc; beep_helper(tone, 250);
  delay(500);
  
  // wifi setup
  Serial.println("wifi setup tone: " + String(tone) );
  tone += toneinc; beep_helper(tone, 250);
  delay(1000);
  
  Serial.println("last tone: " + String(tone) );
  tone += toneinc; beep_helper(tone, 250);
  delay(200);

}

void beep_helper(const uint16_t freq, const uint16_t tdelay)
{
  // uint32_t ledcWriteTone(uint8_t pin, uint32_t freq);
  ledcWriteTone(pin_buzzer, freq);
  delay(tdelay);
  ledcWriteTone(pin_buzzer, 0);
}


void loop()
{

}
