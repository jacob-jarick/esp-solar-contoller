# notes

I have trouble with arduino finding the library files unless I copy or link the Ads1115_mux directory to ~/Arduino/libraries/Ads1115_mux

eg

<pre>ln -s Ads1115_mux ~/Arduino/libraries/Ads1115_mux</pre>

# Description

This library uses the Analog Mux chip XXXX with ADS1115 or ADS1015 to get 32 analog ports.

# use

## Ads1115_mux(uint8_t pina, uint8_t pinb, uint8_t pinc)

create ADS mux instance using pins (pina, pinb, pinc) to controll channel selection.

## adc_poll()
The library is designed to constantly poll the Analog channels and record their value in adc_val (int16_t array).

Only enabled channels are polled, this is controlled by the adc_enable (bool array)

## ntc10k_read_temp(const byte sensor);

treat adc channel sensor as a NTC10k temp sensor and return value in celcius

