#!/bin/sh

# Author: John Van Enk
# Date: Fall, 2007
# Description: Find all the machines in the lab that have SSH running and
#              will allow you to log in without a password. (You need to 
#              have SSH Key Authentication working.
# Note: This will put your host at the top of the list which makes your
#       machine process 0 when running OpenMPI.

IPRANGE=153.106.116.60-95
SSHPORT=22
CONNECTTIMEOUT=4

SSHOPTS="-o ConnectTimeout=4 -o PasswordAuthentication=no"

hostname > .afile

nmap -oG - -p $SSHPORT $IPRANGE -TAggressive | grep "open" | grep -v "^#" > .tfile

for i in `grep -v "^#" .tfile  | cut -f 2 -d " "`; do
	echo $RANDOM `ssh $SSHOPTS $i hostname 2> /dev/null` >> .afile ;
done;

grep "[a-zA-Z]$" .afile | cut -f 2 -d " " | uniq

