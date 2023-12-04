#!/bin/sh

echo "generates a new project key"
echo "this is already done. Don't overwrite the key because"
echo "devices out in the field cannot decrypt!"
echo "this is listed only to understand the whole encryption process!!"

openssl rand -base64 32 > HemroX54SoftwareEncryption.key
