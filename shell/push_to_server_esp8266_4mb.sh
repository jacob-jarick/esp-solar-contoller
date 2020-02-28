#!/bin/bash

HTMLDIR=/var/www/html/esp8266/4mb
PROJDIR=/home/mem/git/esp-solar-contoller/Arduino/esp8266_scv2
FWDIR=/home/mem/git/esp-solar-contoller/firmwares

mkdir -p $HTMLDIR/data

cp -rv $PROJDIR/data/* $HTMLDIR/data

latest=`ls -Art \`find /tmp -name esp8266_scv2.ino.bin 2>/dev/null\` | tail -n1`

cp -v $latest $HTMLDIR/firmware.bin
cp -v $latest $FWDIR/esp8266-4mb-firmware.bin

CV=`grep "#define FW_VERSION" $PROJDIR/esp8266_scv2.ino | awk '{print $3}'`

echo
echo Current Version: $CV
echo $CV > $HTMLDIR/cv.txt

echo
echo updating changelog.
git log --oneline | nl -v0 | sed 's/^ \+/&HEAD~/' | head -20 > $PROJDIR/changelog.txt

cp -rv $PROJDIR/changelog.txt $HTMLDIR

echo
echo Done.
