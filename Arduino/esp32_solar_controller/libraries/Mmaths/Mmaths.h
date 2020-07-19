#ifndef Mmaths_h
#define Mmaths_h

#include "Arduino.h"

class Mmaths
{
  public:
    Mmaths();

    int16_t magic_num;

    double dirty_average(const double oldv, const double newv, uint8_t count);
    double mmax(const double a, const double b);
    float mmax(const float a, const float b);
    uint16_t mmax(const uint16_t a, const uint16_t b);
    uint8_t mmax(const uint8_t a, const uint8_t b);
    uint8_t mmin(const uint8_t a, const uint8_t b);
    uint16_t mmin(const uint16_t a, const uint16_t b);
    uint8_t mdiff(const uint8_t a, const uint8_t b);
    uint16_t mdiff(const uint16_t a, const uint16_t b);


  private:
};

#endif
