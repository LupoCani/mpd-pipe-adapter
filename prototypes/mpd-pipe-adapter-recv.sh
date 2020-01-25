#!/bin/bash

port="6671"
port_data="6672"
port_volume="6673"


while true; do

echo "Wating for connection."
params=$(echo "Remote accepted." | /bin/nc -l $port | grep -m 1 "open_pipe")

echo "Starting: $params"

params=$(echo "$params" | grep -om 3 "[0-9f]\+")

echo "Raw parameters: $params"

rate=$(echo "$params" | sed -n 1p)
form=$(echo "$params" | sed -n 2p)
cnum=$(echo "$params" | sed -n 3p)

form="S${form}_LE"

case $form in
8)
	form="S8"
	;;
f)
	form="FLOAT_LE"
	;;
16 | 24 | 32)
	form="S${form}_LE"
	;;
esac

echo "MPD pipe adapter: Detected audio paramters f:${form} r:${rate} c:${cnum}, forwarding to aplay."

(
while true; do
	vol=$(/bin/nc -l $port_volume | grep "volume" | grep -o -m 1 "[0-9]\+") || continue
	echo "Forwarding volume level ${vol}% to alsa."
	amixer -q set Master "${vol}%"
done
) &

echo "Volume pipe active."

echo "Remote done." | nc -l $port_data | aplay -D converter -f $form -r $((rate * 2)) -c $cnum -v

done
