double dirty_average(const double oldv, const double newv, const byte count)
{
  if(oldv == DAVG_MAGIC_NUM) // if uninitalized (ie set to DAVG_MAGIC_NUM then dont avg).
    return newv;

  double result = 0;
  for (byte i = 0; i < count - 1; i++)
  {
    result += oldv;
  }
  result += newv;

  result /= count;

  return result;
}

void bubbleSort(uint16_t a[], uint16_t size) {
  for(uint16_t i=0; i<(size-1); i++) {
    for(uint16_t o=0; o<(size-(i+1)); o++) {
      if(a[o] > a[o+1]) {
        uint16_t t = a[o];
        a[o] = a[o+1];
        a[o+1] = t;
      }
    }
  }
}


// max

// if I understand my C++ right, you can define function names multiple times and it will send to the function matching the arguments...
double m_max(double a, double b)
{
  if(a > b)
    return a;

  return b;
}

float m_max(float a, float b)
{
  if(a > b)
    return a;

  return b;
}

uint8_t m_max(uint8_t a, uint8_t b)
{
  if(a > b)
    return a;

  return b;
}
