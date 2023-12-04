#/bin/sh

while IFS='$\n' read -r line; do
	echo -n "$line" > $1
done


