
// max

// if I understand my C++ right, you can define function names multiple times and it will send to the function matching the arguments...
double m_max(const double a, const double b)
{
  if(a > b)
    return a;

  return b;
}

float m_max(const float a, const float b)
{
  if(a > b)
    return a;

  return b;
}

uint16_t m_max(const uint16_t a, const uint16_t b)
{
  if(a > b)
    return a;

  return b;
}

uint8_t m_max(const uint8_t a, const uint8_t b)
{
  if(a > b)
    return a;

  return b;
}

// Min

uint8_t m_min(const uint8_t a, const uint8_t b)
{
  if(a < b)
    return a;

  return b;
}

uint16_t m_min(const uint16_t a, const uint16_t b)
{
  if(a < b)
    return a;

  return b;
}

// difference

uint8_t m_diff(const uint8_t a, const uint8_t b)
{
  return m_max(a, b) - m_min(a, b);
}


uint16_t m_diff(const uint16_t a, const uint16_t b)
{
  return m_max(a, b) - m_min(a, b);
}
