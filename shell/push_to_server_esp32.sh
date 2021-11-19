#!/bin/bash

HTMLDIR=/var/www/html/esp32v4
PROJDIR=/home/mem/git/esp-solar-contoller/Arduino/esp32_solar_controller
PUBUPDIR=/home/mem/git/esp-solar-contoller/updates/esp32v4

mkdir -p $HTMLDIR/data
mkdir -p $PUBUPDIR/data

cp -rv $PROJDIR/data/* $HTMLDIR/data
cp -rv $PROJDIR/data/* $PUBUPDIR/data

latest=`ls -Art \`find /tmp -name esp32_solar_controller.ino.bin 2>/dev/null\` | tail -n1`

cp -v $latest $HTMLDIR/firmware.bin
cp -v $latest $PUBUPDIR/firmware.bin

cp -v $latest $PROJDIR/data/esp32-firmware.bin

CV=`grep "#define FW_VERSION" $PROJDIR/esp32_solar_controller.ino | awk '{print $3}'`

echo
echo Current Version: $CV
echo $CV > $HTMLDIR/cv.txt
echo $CV > $PUBUPDIR/cv.txt

cp -v $HTMLDIR/cv.txt $PUBUPDIR/data

echo
echo updating changelog.
git log --oneline | nl -v0 | sed 's/^ \+/&HEAD~/' | head -20 > $PROJDIR/changelog.txt

cp -v $PROJDIR/changelog.txt $HTMLDIR
cp -v $PROJDIR/changelog.txt $PUBUPDIR

echo
echo Done.
