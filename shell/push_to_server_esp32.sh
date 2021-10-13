#!/bin/bash

HTMLDIR=/var/www/html/esp32
PROJDIR=/home/mem/git/esp-solar-contoller/Arduino/esp32_solar_controller
FWDIR=/home/mem/git/esp-solar-contoller/firmwares

mkdir -p $HTMLDIR/data
mkdir -p $FWDIR

cp -rv $PROJDIR/data/* $HTMLDIR/data

latest=`ls -Art \`find /tmp -name esp32_solar_controller.ino.bin 2>/dev/null\` | tail -n1`

cp -v $latest $HTMLDIR/firmware.bin
cp -v $latest $FWDIR/esp32-firmware.bin

cp -v $PROJDIR/data/esp32-firmware.bin

CV=`grep "#define FW_VERSION" $PROJDIR/esp32_solar_controller.ino | awk '{print $3}'`

echo
echo Current Version: $CV
echo $CV > $HTMLDIR/cv.txt

cp -v $HTMLDIR/cv.txt $PROJDIR/data

echo
echo updating changelog.
git log --oneline | nl -v0 | sed 's/^ \+/&HEAD~/' | head -20 > $PROJDIR/changelog.txt

cp -rv $PROJDIR/changelog.txt $HTMLDIR

echo
echo Done.
