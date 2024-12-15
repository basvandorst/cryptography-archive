#!/bin/ksh
# this script will look thru the skeykeys file for
# people with sequence numbers less then LOWLIMIT=12
# and send them e-mail remindes to use keyinit soon
LOWLIMIT=12
if [ "$1" != "" ]
then
 LOWLIMIT=$1
fi
# a copy of notices will go to ADMIN
ADMIN=jsw
SUBJECT="Reminder: Run keyinit"
HOST=`/usr/bin/hostname`
# an skeykeys entry looks like
#   jsw 0076 la13079          ba20a75528de9d3a
# the sequence number is the second entry
#
for i in `/usr/bin/awk '{print $1}' /etc/skeykeys`
do
SEQ=`/usr/bin/grep "^$i" /etc/skeykeys|/usr/bin/awk '{print $2}'`
if [ $SEQ -lt $LOWLIMIT ]
then
  KEY=`/usr/bin/grep "^$i" /etc/skeykeys|/usr/bin/awk '{print $3}'`
  if [ $SEQ -lt  3 ]
  then
  SUBJECT="IMPORTANT action required"
  ADMIN=admin
  fi
  (
  print "~c jsw"
  print "You are nearing the end of your current S/Key sequence for account $i"
  print "on system $HOST."
  print "Your S/key sequence number is now $SEQ. When it reaches zero you"
  print "will no longer be able to use S/Key to login to the system.  "
  print "If you are now logged in via S/Key, use the command "
  print "   keyinit -s "
  print "on system $HOST to reinitialize your sequence number."
  print "Be sure you use a different key from the current one which is"
  print "  $KEY"
  print "You can continue to use the same secret password."
  print " "
  print "If you are logged in from work (via the secure Bellcore network),"
  print "you may use the command"
  print "  keyinit"
  print "on system $HOST. The keyinit program will automatically generate"
  print "a new key for you."
  print "Thank You."
  print "   Your S/Key System Administrators"
  ) | /usr/ucb/mail -s "$SUBJECT"  $i  $ADMIN
fi
done
