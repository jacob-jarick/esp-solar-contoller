double dirty_average(double oldv, double newv, byte count)
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

/*

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

*/
