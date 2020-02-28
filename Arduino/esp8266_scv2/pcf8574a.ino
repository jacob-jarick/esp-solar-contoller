// #include <IoAbstraction.h>
// #include <IoAbstractionWire.h>


// #include "Arduino.h"
#include "PCF8574.h"

// unsigned long cell_cooldown[8] = {0, 0, 0, 0, 0, 0, 0, 0};


PCF8574 pcf8574_1(0x38);
PCF8574 pcf8574_2(0x39);
PCF8574 pcf8574_3(0x3A);
PCF8574 pcf8574_4(0x3B);
PCF8574 pcf8574_5(0x3C);
PCF8574 pcf8574_6(0x3D);
PCF8574 pcf8574_7(0x3E);
PCF8574 pcf8574_8(0x3F);

PCF8574 pcf8574_9(0x20);
PCF8574 pcf8574_10(0x21);
PCF8574 pcf8574_11(0x22);
PCF8574 pcf8574_12(0x23);
PCF8574 pcf8574_13(0x24);
PCF8574 pcf8574_14(0x25);
PCF8574 pcf8574_15(0x26);
PCF8574 pcf8574_16(0x27);


void pcf857a_setup(char p_addr)
{
  for (byte i = 0; i < 8; i++)
  {
    if(p_addr == 0x38)
      pcf8574_1.pinMode(i, OUTPUT);
    else if(p_addr == 0x39)
      pcf8574_2.pinMode(i, OUTPUT);
    else if(p_addr == 0x3A)
      pcf8574_3.pinMode(i, OUTPUT);
    else if(p_addr == 0x3B)
      pcf8574_4.pinMode(i, OUTPUT);
    else if(p_addr == 0x3C)
      pcf8574_5.pinMode(i, OUTPUT);
    else if(p_addr == 0x3D)
      pcf8574_6.pinMode(i, OUTPUT);
    else if(p_addr == 0x3E)
      pcf8574_7.pinMode(i, OUTPUT);
    else if(p_addr == 0x3F)
      pcf8574_8.pinMode(i, OUTPUT);

    else if(p_addr == 0x20)
      pcf8574_9.pinMode(i, OUTPUT);
    else if(p_addr == 0x21)
      pcf8574_10.pinMode(i, OUTPUT);
    else if(p_addr == 0x22)
      pcf8574_11.pinMode(i, OUTPUT);
    else if(p_addr == 0x23)
      pcf8574_12.pinMode(i, OUTPUT);
    else if(p_addr == 0x24)
      pcf8574_13.pinMode(i, OUTPUT);
    else if(p_addr == 0x25)
      pcf8574_14.pinMode(i, OUTPUT);
    else if(p_addr == 0x26)
      pcf8574_15.pinMode(i, OUTPUT);
    else if(p_addr == 0x27)
      pcf8574_16.pinMode(i, OUTPUT);
  }

  // do not use pcf8574.begin() - not needed and messes with i2c pins.
}

void ports_off()
{
  for (uint8_t i = 0; i < 8; i++)
    pcf857a_write_state(config.pcf857a_addr, i, 1);
}


void pcf857a_write_state(char addr, byte pin, bool state)
{
  if(!addr)
    return;

  if(addr == 0x38)
    pcf8574_1.digitalWrite(pin, state);
  else if(addr == 0x39)
    pcf8574_2.digitalWrite(pin, state);
  else if(addr == 0x3A)
    pcf8574_3.digitalWrite(pin, state);
  else if(addr == 0x3B)
    pcf8574_4.digitalWrite(pin, state);
  else if(addr == 0x3C)
    pcf8574_5.digitalWrite(pin, state);
  else if(addr == 0x3D)
    pcf8574_6.digitalWrite(pin, state);
  else if(addr == 0x3E)
    pcf8574_7.digitalWrite(pin, state);
  else if(addr == 0x3F)
    pcf8574_8.digitalWrite(pin, state);

  else if(addr == 0x20)
    pcf8574_9.digitalWrite(pin, state);
  else if(addr == 0x21)
    pcf8574_10.digitalWrite(pin, state);
  else if(addr == 0x22)
    pcf8574_11.digitalWrite(pin, state);
  else if(addr == 0x23)
    pcf8574_12.digitalWrite(pin, state);
  else if(addr == 0x24)
    pcf8574_13.digitalWrite(pin, state);
  else if(addr == 0x25)
    pcf8574_14.digitalWrite(pin, state);
  else if(addr == 0x26)
    pcf8574_15.digitalWrite(pin, state);
  else if(addr == 0x27)
    pcf8574_16.digitalWrite(pin, state);
}

// 1 = port on
// 0 = port off
void port_mode(byte p, bool m)
{
  pcf857a_write_state(config.pcf857a_addr, p, !m);
}

