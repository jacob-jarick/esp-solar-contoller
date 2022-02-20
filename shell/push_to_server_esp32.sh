#!/bin/bash

HTMLDIR=/var/www/html/esp32v4
PROJDIR=/home/mem/git/esp-solar-contoller/Arduino/esp32_solar_controller
PUBUPDIR=/home/mem/git/esp-solar-contoller/updates/esp32v4
HTML_HOST=jacob@10.1.1.207

# mkdir -p $HTMLDIR/data
mkdir -p $PUBUPDIR/data

scp $PROJDIR/data/* $HTML_HOST:$HTMLDIR/data
cp -rv $PROJDIR/data/* $PUBUPDIR/data

latest=`ls -Art \`find /tmp -name esp32_solar_controller.ino.bin 2>/dev/null\` | tail -n1`

scp $latest $HTML_HOST:$HTMLDIR/firmware.bin

cp -v $latest $PUBUPDIR/firmware.bin
cp -v $latest $PROJDIR/data/esp32-firmware.bin

CV=`grep "#define FW_VERSION" $PROJDIR/esp32_solar_controller.ino | awk '{print $3}'`

echo
echo Current Version: $CV
# echo $CV > $HTMLDIR/cv.txt
echo $CV > $PUBUPDIR/cv.txt
echo $CV > $PUBUPDIR/data/cv.txt

scp $PUBUPDIR/cv.txt $HTML_HOST:$HTMLDIR/


echo
echo updating changelog.
git log --oneline | nl -v0 | sed 's/^ \+/&HEAD~/' | head -20 > $PROJDIR/changelog.txt

scp $PROJDIR/changelog.txt $HTML_HOST:$HTMLDIR/
cp -v $PROJDIR/changelog.txt $PUBUPDIR

echo
echo Done.
