#!/bin/bash

# it is possible to open several windows of gedit (ctrl+N) and use script many times

NAME=$"file_new_125"
if [[ -e ~/$NAME ]]; then
	rm ~/$NAME
fi

WID=$(xdotool search --name --desktop 0 "gedit" | head -n 1)
xdotool windowactivate --sync $WID

#test: write text, save and cat
SENTENCE=$"Test sentense to simulate manually printing a text."
xdotool type --delay 10 "$SENTENCE"
xdotool key "ctrl+s"
xdotool type $NAME
xdotool key "alt+s"
sleep 1
xdotool key "ctrl+w"

FILE=$(cat ~/$NAME)
echo "$FILE"
if [[ $FILE = $SENTENCE ]]; then
	echo OK
fi

#test: open, add, save, check with cat
xdotool key "ctrl+o"
sleep 1
xdotool type $NAME
xdotool key "Tab+Return"
sleep 1
xdotool key Return
sleep 1
xdotool key End
A=$"aaaa"
xdotool type $A
xdotool key "ctrl+s"
xdotool key "ctrl+w"
xdotool key "alt+F4"
FILENEW=$(cat ~/$NAME)
if [[ $SENTENCE$A = $FILENEW ]]; then
	echo OK
fi
