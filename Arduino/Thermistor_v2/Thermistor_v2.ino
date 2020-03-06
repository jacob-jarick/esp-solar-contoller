// 8/12/2014 - OSBSS 16-bit thermisor 
// Temperature from Vishay thermistor using ADS1115 16-bit ADC
// Using Original extended Steinhard equation and coefficients for number 9 thermistor (B = 3977)
// Reference resistance is 10K-ohm
// Can change decimal accuracy to reflect 16-bit precision

#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;


void setup()
{
  Serial.begin(19200);
  ads.begin();
  ads.setGain(GAIN_ONE);
}


void loop()
{
  // get ADC values --------------------------------------------//
  int16_t adc0 = ads.readADC_SingleEnded(0);
  int16_t adc0_average = samples(0);
  
  // get resistance --------------------------------------------//
  float R0 = resistance(adc0);
  float R0_avg = resistance(adc0_average);
  
  // get temperature --------------------------------------------//
  float temperature0 = steinhart(R0);
  float temperature0_avg = steinhart(R0_avg);
  
  // print everything -------------------------------------------//
  Serial.print("Temperature 0: ");
  Serial.println(temperature0, 3);
  
  Serial.print("Temperature 0 (average): ");
  Serial.println(temperature0_avg, 3);
  
  Serial.println("---------------------------------------------");
  Serial.println();
  delay(2000);
}

// Get resistance -------------------------------------------//
float resistance(int16_t adc)
{
  float ADCvalue = adc*(8.192/3.3);  // Vcc = 8.192 on GAIN_ONE setting, Arduino Vcc = 3.3V in this case
  float R = 10000/(65535/ADCvalue-1);  // 65535 refers to 16-bit number
  return R;
}

// Get temperature from Steinhart equation -------------------------------------------//
float steinhart(float R)
{
  float Rref = 10000.0;
  float A = 0.003354016;
  float B = 0.0002569850;
  float C = 0.000002620131;
  float D = 0.00000006383091;
  float E = log(R/Rref);
  
  float T = 1/(A + (B*E) + (C*(E*E)) + (D*(E*E*E)));
  return T-273.15;
}

// Perform multiple iterations to get higher accuracy ADC values (reduce noise) --------------------------------//
int16_t samples(int pin)
{
  int n=5;  // number of iterations to perform
  int32_t sum=0;  //store sum as a 32-bit number
  for(int i=0;i<n;i++)
  {
    int16_t value = ads.readADC_SingleEnded(pin);
    sum = sum + value;
    delay(10);
  }
  int32_t average = sum/n;   //store average as a 32-bit number
  return average;
}
