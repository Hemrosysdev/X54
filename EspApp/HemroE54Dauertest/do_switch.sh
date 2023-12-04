#/bin/sh

HOST_IP=$1
PORT=$2
ACTION=$3

cd eth008-python3
timeout 5 python3 -c "from eth008 import *;connect('${HOST_IP}',17494,'password');digital_command('${PORT} ${ACTION}');"
