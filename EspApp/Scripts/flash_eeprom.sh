#!/bin/sh

if [ "$#" != "2" ]
then
	echo 'syntax: ./flash_eeprom.sh <productno> <serialno>'
	echo "Flashes EEPROM of X54 via WLAN/curl interface"
	exit 0
fi

echo "Checking web server ..."

ping 192.168.4.1 -c 2

if [ ];
then
	echo "Error, web server 192.168.4.1 not available - abort"
	exit
fi

echo "Ensure, that jumper for EEPROM write enable has been set!"

echo $1 > /tmp/productno
echo $2 > /tmp/serialno
echo $2 >> serialno.list

curl -d "@/tmp/serialno" -X POST http://192.168.4.1/cmd?writeserialno
curl -d "@/tmp/productno" -X POST http://192.168.4.1/cmd?writeproductno

echo "Reboot X54 and download statistics from web site to verify successful writing."
