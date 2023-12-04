#!/bin/sh

# this is given just for test usage
# decryption is usually done within ESP32

if [ "$#" != "2" ];
then
	echo "syntax: c.sh <encrypted in file> <key file>"
	exit 1
fi

if [ ! -e $1 ];
then
	echo "error: encrypted input file $1 doesn't exist"
	exit 1
fi

if [ ! -e $2 ];
then
	echo "error: key file $2 doesn't exist"
	exit 1
fi

OUTFILE=`dirname $1`/`basename $1`.decrypted

echo "write result to $OUTFILE"

openssl enc -d -aes-256-cbc -in $1 -out ${OUTFILE} -pass file:$2
