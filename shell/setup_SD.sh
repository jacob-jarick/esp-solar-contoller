#!/bin/bash

CONFIG_FILE=/home/mem/config.jsn
HTML_DATA_DIR=/home/mem/git/esp-solar-contoller/Arduino/esp32_solar_controller/data/
SD_DEV=/dev/mmcblk0p1
MNT_POINT=/mnt/guest
echo $HTML_DATA_DIR

mkfs.fat /dev/mmcblk0p1

mount -o rw $SD_DEV $MNT_POINT
cp -rv $CONFIG_FILE $HTML_DATA_DIR/*htm $MNT_POINT/

umount $MNT_POINT
