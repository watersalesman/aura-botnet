#!/bin/sh

#Remove installation directory
rm -r ~/.gnupg/.seeds

#If root, disable the system service
#Otherwise disable the user service
if [ $(whoami) == 'root' ]; then
	systemctl disable --now d-bus.timer
else
	systemctl --user disable --now d-bus.timer
fi
