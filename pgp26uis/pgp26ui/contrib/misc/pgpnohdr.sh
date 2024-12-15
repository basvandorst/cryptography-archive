Subject: Re: PGP with mail/news
Date: Fri, 8 Jan 1993 05:14:09 GMT
From: res@colnet.cmhnet.org (Rob Stampfli)
Lines: 50

>}one feature i discovered that would be nice to have in PGP: a switch
>}that tells it to sign only the body of a message, instead of the whole
>}thing.

This is really very easy to accomplish external to pgp.  Here is an example
of a shell script that sits between the mailer program and /bin/mail, which
does exactly what you want.  It was written before the implemenation of
+clearsig, but it should work just fine with it.
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# This script is invoked by adding the line "set sendmail=pgpmail" to your
# .mailrc file.  mailx then invokes pgpmail instead of /bin/mail to deliver
# email.  This script checks whether outbound encryption is specified, and
# automatically performs it as required.

trap "" 1 2 3				# req'd since this can run in bg
exec 2>/dev/tty				# can be "exec 2>/dev/null"

nl="" en="" sg=""
for i					# for each argument...
do
  case "$i" in				# look for encryption specifier...
#  *encrypt=*)	en="`sed 's/.*=//'`"	# this line req'd if SHELL=sh
  *encrypt=*)	en="${i#*=}";;		# this line is faster if SHELL=ksh
  *sig=*)	sg="${i#*=}";;		# a pgp signature specification...
  *)		nl="$nl $i";;		# a real mail address...
  esac
done

[ X = "X$en" ] && exec /bin/rmail "$@"	# encryption was not requested

(					# encryption *was* specified
	OIFS="$IFS"			# needed to preserve tabs in header
	IFS='
'
	while read x		 	# read and process header intact
	do
	print - "$x"
	[ "" = "$x" ] && break
	done
	IFS="$OIFS"			# reset field separators
	if [ X = "X$sg" ]; then		# no signature specified:
	  pgp -feat "$en"		#   feed the rest to encryption pgm
	else				# a signature was specified:
	  pgp -feast "$en" -u "$sg"	#   feed pgp...
	fi
	echo "Encryption phase completed" 1>&2
) | /bin/rmail $nl
-- 
Rob Stampfli  rob@colnet.cmhnet.org      The neat thing about standards:
614-864-9377  HAM RADIO: kd8wk@n8jyv.oh  There are so many to choose from.

