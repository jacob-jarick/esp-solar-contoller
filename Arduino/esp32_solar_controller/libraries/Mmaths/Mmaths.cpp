#include "Arduino.h"
#include "Mmaths.h"

Mmaths::Mmaths()
{
  magic_num = int16_t(-123);
}

double Mmaths::dirty_average(const double oldv, const double newv, uint8_t count)
{
  if(oldv == magic_num) // if uninitalized (ie set to magic_num then dont avg).
    return newv;

  // must be at least 2
  if(count < 2)
    count = 2;

  double result = 0;
  for (uint8_t i = 0; i < count - 1; i++)
  {
    result += oldv;
  }
  result += newv;

  result /= count;

  return result;
}


// max

// if I understand my C++ right, you can define function names multiple times and it will send to the function matching the arguments...
double Mmaths::mmax(const double a, const double b)
{
  if(a > b)
    return a;

  return b;
}

float Mmaths::mmax(const float a, const float b)
{
  if(a > b)
    return a;

  return b;
}

uint16_t Mmaths::mmax(const uint16_t a, const uint16_t b)
{
  if(a > b)
    return a;

  return b;
}

uint8_t Mmaths::mmax(const uint8_t a, const uint8_t b)
{
  if(a > b)
    return a;

  return b;
}

// Min

uint8_t Mmaths::mmin(const uint8_t a, const uint8_t b)
{
  if(a < b)
    return a;

  return b;
}

uint16_t Mmaths::mmin(const uint16_t a, const uint16_t b)
{
  if(a < b)
    return a;

  return b;
}

// difference

uint8_t Mmaths::mdiff(const uint8_t a, const uint8_t b)
{
  return mmax(a, b) - mmin(a, b);
}


uint16_t Mmaths::mdiff(const uint16_t a, const uint16_t b)
{
  return mmax(a, b) - mmin(a, b);
}
