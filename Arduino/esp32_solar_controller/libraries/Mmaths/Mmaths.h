#ifndef Mmaths_h
#define Mmaths_h

#include "Arduino.h"

class Mmaths
{
  public:
    Mmaths();

    int16_t magic_num;

    double dirty_average(const double oldv, const double newv, uint8_t count);


    uint8_t mmax(const uint8_t a, const uint8_t b);
    uint16_t mmax(const uint16_t a, const uint16_t b);
    unsigned long mmax(const unsigned long a, const unsigned long b);
    float mmax(const float a, const float b);
    double mmax(const double a, const double b);

    uint8_t mmin(const uint8_t a, const uint8_t b);
    uint16_t mmin(const uint16_t a, const uint16_t b);
    unsigned long mmin(const unsigned long  a, const unsigned long  b);
    float mmin(const float a, const float b);
    double mmin(const double a, const double b);

    uint8_t mdiff(const uint8_t a, const uint8_t b);
    uint16_t mdiff(const uint16_t a, const uint16_t b);
    unsigned long mdiff(const unsigned long a, const unsigned long b);
    float mdiff(const float a, const float b);

    unsigned long ymdhms_to_sec(uint16_t YY, uint8_t MM, uint8_t DD, uint8_t HH, uint8_t mm, uint8_t ss);


  private:
};

#endif
