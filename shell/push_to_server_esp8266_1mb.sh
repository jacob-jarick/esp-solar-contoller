#!/bin/bash

mkdir -p /var/www/html/esp8266/1mb/data

cp -rv /home/mem/git/solar-controller/esp8266_scv2/data/* /var/www/html/esp8266/1mb/data

latest=`ls -Art \`find /tmp -name esp8266_scv2.ino.bin 2>/dev/null\` | tail -n1`

cp -v $latest /var/www/html/esp8266/1mb/firmware.bin
cp -v $latest /home/mem/git/solar-controller/esp8266-1mb-firmware.bin

CV=`grep "#define FW_VERSION"  /home/mem/git/solar-controller/esp8266_scv2/esp8266_scv2.ino | awk '{print $3}'`

echo
echo Current Version: $CV
echo $CV > /var/www/html/esp8266/1mb/cv.txt

echo
echo updating changelog.
git log --oneline | nl -v0 | sed 's/^ \+/&HEAD~/' | head -20 > changelog.txt

cp -rv /home/mem/git/solar-controller/changelog.txt /var/www/html/esp8266/1mb/

echo
echo Done.
