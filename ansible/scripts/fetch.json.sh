#!/bin/bash

mkdir -p /var/www/html/esp32_json/

# office
curl -s http://10.1.1.227/jsonapi > /var/www/html/esp32_json/office.api.json
curl -s http://10.1.1.227/json_cells > /var/www/html/esp32_json/office.cells.json

# network

curl -s http://10.1.1.212/jsonapi > /var/www/html/esp32_json/network.api.json
curl -s http://10.1.1.212/json_cells > /var/www/html/esp32_json/network.cells.json


# lounge
curl -s http://10.1.1.191/jsonapi > /var/www/html/esp32_json/lounge.api.json
