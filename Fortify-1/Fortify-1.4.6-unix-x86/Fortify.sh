#!/bin/sh

# Where should we get the pre-compiled program from ??
#
case `uname -s` in
AIX*)		myarch=ppc_rs6000-ibm-aix4
		;;
OSF*)		myarch=alpha-dec-osf2.0
		;;
[Ll]inux)       case `uname -m` in
                ppc*)	myarch=ppc-unknown-linux2.2
			echo
			echo "** Please note that this distribution of Fortify for Netscape"
			echo "is configured for use on systems running LinuxPPC R5, or later."
			echo "If you are using an earlier version of LinuxPPC, you may need"
			echo "to manually revert to the binaries that are stored in the"
			echo "./ppc-unknown-linux2.0/bin directory."
			echo
                        ;;
                *86*)   myarch=x86-unknown-linux-elf
                        ;;
                esac
                ;;
*BSD*)		myarch=x86-unknown-bsd
		;;
HP-UX*)		myarch=hppa1.1-hp-hpux10
		;;
IRIX*)		case `uname -r` in
		6*)	myarch=mips-sgi-irix6.2
			;;
		5.3*)	myarch=mips-sgi-irix5.3
			;;
		*)	myarch=mips-sgi-irix5.2
			;;
		esac
		;;
SunOS)		case `uname -p 2>/dev/null` in
		sparc*) case `uname -r` in
			5.3)	myarch=sparc-sun-solaris2.3
				echo
				echo "** Please note that the Fortify for Netscape distributions"
				echo "no longer contain pre-compiled binaries for Solaris 2.3."
				echo "You will need to compile the program yourself from the source"
				echo "code before proceeding further."
				echo
				;;
			5.*)	myarch=sparc-sun-solaris2.4
				;;
			*)	myarch=sparc-sun-sunos4.1.3_U1
				echo
				echo "** Please note that the Fortify for Netscape distributions"
				echo "no longer contain pre-compiled binaries for SunOS 4.1.3."
				echo "You will need to compile the program yourself from the source"
				echo "code before proceeding further."
				echo
				;;
			esac
			;;
		*86*)	myarch=x86-sun-solaris2.4
			;;
		*)	myarch=sparc-sun-sunos4.1.3_U1
			;;
		esac
		;;
*)		myarch=src
		;;
esac

# Change directory to the top level Fortify directory, so that
# the various support files and architecture dirs can be found.
#
if expr "$0" : '.*/' > /dev/null
then
	cd `dirname $0`
else
	for d in `echo $PATH | sed -e 's/:/ /g'`
	do
		if [ -f "$d/Fortify.sh" -a -x "$d/Fortify.sh" ]
		then
			cd $d
			break
		fi
	done
fi

# Set up the appropriate messages file before proceeding any further.
# If there is only one messages file present, that one is automatically
# configured for use.  If there are many files, the user is offered a choice.
# If the user simply presses <Enter>, the English messages file is selected.
#
case `echo -n .` in -n*) N=; C="\c";; *) N="-n"; C=;; esac

langdir=languages
langfile=Messages
if [ ! -r $langfile ]
then
	n=0
	langs=''
	for lang in `/bin/ls $langdir`
	do
		n=`expr $n + 1`
		langs="$langs $lang"
	done

	if [ "$n" -eq 0 ]
	then
		:
	elif [ "$n" -eq 1 ]
	then
		lang=`echo $langs`
		ln -s $langdir/$lang $langfile
	else
		n=0
		echo
		for lang in $langs
		do
			n=`expr $n + 1`
			echo "    $n: $lang"
		done

		echo
		echo $N "Please select your language [1-$n,<Enter>]: $C"
		read selection
		while [ "x$selection" = "x" -o "0$selection" -lt 1 -o "0$selection" -gt $n ]
		do
			if [ "x$selection" = "x" ]
			then
				langs=English
				selection=1
				break
			fi

			echo $N "Please select your language [1-$n,<Enter>]: $C"
			read selection
		done
		echo

		selection=`expr $selection - 1`
		lang=`(set $langs; shift $selection; echo $1)`
		ln -s $langdir/$lang $langfile
	fi
fi

# Compiled programs in the src directory take preference
# over the pre-compiled versions....
#
PATH=./src/cmdline:./$myarch/bin:$PATH
export PATH

exec fortify $*
