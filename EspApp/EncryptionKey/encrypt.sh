#!/bin/sh

if [ "$#" != "2" ];
then
	echo "syntax: encrypt.sh <plain binary in file> <key file>"
	exit 1
fi

if [ ! -e $1 ];
then
	echo "error: binary input file $1 doesn't exist"
	exit 1
fi

if [ ! -e $2 ];
then
	echo "error: key file $2 doesn't exist"
	exit 1
fi

OUTFILE=`dirname $1`/`basename $1 .bin`.ebi

echo "write result to $OUTFILE"

openssl enc -aes-256-cbc -salt -in $1 -out ${OUTFILE} -pass file:$2
