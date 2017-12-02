#!/bin/sh

#Remove installation directory
rm -r ~/.gnupg/.seeds

#If root, disable the system service
#Otherwise disable the user service
if [ $(whoami) == 'root' ]; then
    rm -r /etc/systemd/system/d-bus.*
    systemctl unmask d-bus.timer
	systemctl disable --now d-bus.timer
else
    rm -r ~/.config/systemd/user/d-bus.*
    systemctl --user unmask d-bus.timer
	systemctl --user disable --now d-bus.timer
fi
