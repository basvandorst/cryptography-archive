#
# initialize modem
#
output atz\13
input 10 OK\n
#
# set modem to indicate DCD
#
output at&c1
input 10 OK\n
#
# send phone number
#
output atdt242284\13
#
# my other number
#
#output atdt241644\13
#
# now we are connected.
#
input 30 CONNECT
#
#  wait till it's safe to send because some modem's hang up
#  if you transmit during the connection phase
#
wait 30 dcd
#
# now prod the terminal server
#
output \13
#
#  wait for the username prompt
#
input 30 username:
username Enter your username
output \u\13
#
# and the password
#
input 30 password:
password Enter your password
output \p\13
#
# we are now logged in
#
input 30 >
#
# see who on for informational reasons.
#
output who\13
input 30 >
#
# jump into slip mode
#
output slip\13
#
# wait for the address string
#
input 30 Your address is
#
# parse address
#
address 30
input 30 \n
#
# we are now connected, logged in and in slip mode.
#
display \n
display Connected.  Your IP address is \i.\n
#
#  ping a well known host locally...  our slip server won't work
#  for a while
#
exec pingw 131.217.10.1
#
# now we are finished.
#

