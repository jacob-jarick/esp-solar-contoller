#include <TimeLib.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char ssid[] = "xxxx";  //  your network SSID (name)
const char pass[] = "xxxx";       // your network password

// NTP Servers:
static const char ntpServerName[] = "10.1.1.1";
int timeZone = 8;     // GMT offset

WiFiUDP Udp;

void setup()
{
  Serial.begin(115200);
  delay(250);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Starting UDP");
  Udp.begin(8888);
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
}

time_t lastTime = 0; // when the digital clock was displayed

void loop()
{
  if (timeStatus() == timeNotSet) 
  {
    Serial.println("Time not synced.");
  }
  else
  {
    if (now() != lastTime) { //update the display only if time has changed
      lastTime = now();
      printTime();
    }    
  }
}

void printTime()
{
  int H = hour();
  int M = minute();
  int S = second();

  if (H < 10)
    Serial.print('0');

  Serial.print(String(H) + ":");

  if (M < 10)
    Serial.print('0');

  Serial.print(String(M) + ":");

  if (S < 10)
    Serial.print('0');

  Serial.print(String(S) + " ");

  Serial.print(String(day()) + "/" + String(month()) + "/" + String(year()) + "\n" );
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + (timeZone * SECS_PER_HOUR);
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
