#!/bin/sh
#
# Installation script for the ACE/Agent for Netscape Plugin.
#
#********************************************************************************
#* This software, proprietary and confidential to Security Dynamics Technologies*
#* Inc., is furnished under a license, and may be used and copied only in       *
#* accordance with the terms of such license and with the inclusion of the above*
#* copyright notice.  This software or any copies thereof may not be provided or*
#* otherwise made available to any other person. No title to or ownership of the*
#* software is hereby transferred.  Any unauthorized use or reproduction of this*
#* software may be subject to civil or criminal liability.                      *
#*                                                                              *
#* This software is subject to change without notice and should not be construed*
#* as a commitment by Security Dynamics Technologies Inc.                       *
#********************************************************************************
#

############################################################
# Set default values and commands for the shell script. 
############################################################
UNAME=`uname`
UNAMER=`uname -r`
HOSTNAME=`hostname`

PWD_EXEC="/bin/pwd"
LINE_EXEC="/bin/line"
DEFVARACE="/var/ace/data"
OLD_PLUG="/tmp/sdiplug"
OLD_DOC="/tmp/sdidocs"
NS_PLUGINS=""
NS_DOCS=""
NS_CONFIG=""
ACE_DOCS=""
ACE_PLUGINS=""
PATH_ROOT=""
SOURCE_DIR=""

PLUG_IS_INSTLD=FALSE
ADMIN_IS_INSTLD=FALSE
BACKUP_CRTD=FALSE
PREV_INSTL=FALSE

echo_no_nl_mode="unknown"

##############################################################################
# echo_no_nl()
#       Echo a string with no carriage return (if possible).  Must set
#       $echo_no_nl to "unknown" before using for the first time.
##############################################################################
echo_no_nl()
{
  if [ "$echo_no_nl_mode" = "unknown" ] ; then
     echo_test=`echo \\\c`
     if [ "$echo_test" = "\c" ] ; then
        echo_no_nl_mode="-n"
     else
        echo_no_nl_mode="\c"
     fi
  fi

  if [ "$echo_no_nl_mode" = "\c" ] ; then
     echo $* \\c
  else
     echo -n $*
  fi
}

##############################################################################
# getyesorno()
# Gets either a "yes" or a "no" in the traditional (y/n) pattern or
# "quit" to abort installation.
#               $1      The default value if user hits <Enter> (TRUE/FALSE)
#               $2      The string to print to prompt the user
# The variable $YESORNO is set in accordance to what the user entered.
##############################################################################
getyesorno()
{
#####################################################
# Set up line parameters as $1 and $2 are overwritten
#####################################################
  yesornodef=$1
  yesornoprompt=$2

  YESORNO=""
  until [ -n "$YESORNO" ] ;
  do
    echo ""
    echo_no_nl $yesornoprompt
    YESORNO=`$LINE_EXEC`

    case "$YESORNO"
    in
      'y' )  YESORNO=TRUE;;
      'n' )  YESORNO=FALSE;;
      'q' )  abort_installation;;
      ''  )  YESORNO=$yesornodef;;
      *   )  echo ""
             echo "Please enter 'y', 'n' or 'q'."
             YESORNO="";;
    esac
  done

}

##############################################################################
# cleanup_installation()
# This subroutine removes files that were recently installed and restores
# the previous installation files, if they existed.
##############################################################################
cleanup_installation()
{

if [ "$BACKUP_CRTD" = TRUE ] ; then
    cp -pf $NS_CONFIG/sdiobj.sav $OBJ_FILE
    rm -f $NS_CONFIG/sdiobj.sav 
fi

if [ "$PLUG_IS_INSTLD" = TRUE ] ; then
    rm -rf "$ACE_PLUGINS"
fi

if [ "$ADMIN_IS_INSTLD" = TRUE ] ; then
    rm -rf "$ACE_DOCS"
fi

if [ "$PREV_INSTL" = TRUE ] ; then
    cp -Rpf $OLD_PLUG $ACE_PLUGINS
    cp -Rpf $OLDDOC $ACE_DOCS
    rm -rf $OLDPLUG $OLDDOC
fi

}

##############################################################################
# abort_installation()
# Abort the installation - this calls cleanup_installation() - however, it
# also prints on the screen that the installation was aborted.
##############################################################################
abort_installation()
{
  echo ""
  echo "Aborting Installation..."

  cleanup_installation
  exit 1
}

##############################################################################
# check_startup()
# Checks the "essentials" of startup - what the current working
# directory is, whether the user is root or file owner.
##############################################################################
check_startup()
{

############################################
# Try to determine current working directory
############################################

  if [ -x "$PWD_EXEC" ] ; then
    SOURCE_DIR=`"$PWD_EXEC"`
  elif [ -n "$PWD" ] ; then
    SOURCE_DIR=$PWD
  elif [ -n "$cwd" ] ; then
    SOURCE_DIR=$cwd
  else
    echo ""
    echo "Unable to determine the current working directory."
    until [ -n "$SOURCE_DIR" ] ;
    do
      echo ""
      echo_no_nl "Please enter the current working directory:"
      SOURCE_DIR=`$LINE_EXEC`
    done
  fi
}

############################################################
# Display the License and Copyright files. 
############################################################
startup_screen()
{
   if [ -f ./acedocs/license.txt ] ; then
# Get number of columns from stty command, use sed to strip off trailing
# semi colon that appears on Solaris.
      COLS=`stty -a | grep columns | awk '{print $6}'  | sed -e s/\;//`
#     if [ $UNAME = AIX ]; then COLS=w$COLS
#     fi
      fold -s -w$COLS ./acedocs/license.txt | more
    else
      echo "The License Agreement text file could not be found in the current directory."
      echo "Installation is aborting..."
      abort_installation
    fi

    create=""
    until [ -n "$create" ] ;
    do
        echo "Do you accept the License Terms and Conditions stated above? (y/n):"
        create=`$LINE_EXEC`
        case "$create"
        in
          'y' ) continue;;    # OK to proceed with the installation
          'n' ) abort_installation;;
           *  ) echo ""
                create=""
                echo "Please enter either Yes or No.";;
        esac
    done
    echo ""
    echo "
********************************************************************************
* This software, proprietary and confidential to Security Dynamics Technologies*
* Inc., is furnished under a license, and may be used and copied only in       *
* accordance with the terms of such license and with the inclusion of the above*
* copyright notice.  This software or any copies thereof may not be provided or*
* otherwise made available to any other person. No title to or ownership of the*
* software is hereby transferred.  Any unauthorized use or reproduction of this*
* software may be subject to civil or criminal liability.                      *
*                                                                              *
* This software is subject to change without notice and should not be construed*
* as a commitment by Security Dynamics Technologies Inc.                       *
********************************************************************************
"
}

#######################################################
# Ask the user if he wants to continue the installation
# after encountering a non-supported platform.
#######################################################
asktocontinue()
{
  getyesorno FALSE "Should the installation continue? (y/n) [n]:"
  if [ "$YESORNO" = FALSE ] ; then
    abort_installation
  fi
}

############################################################
# Check to see if this is a supported platform. 
############################################################
check_platform()
{
case $UNAME 
   in
     'SunOS'  )  SUN_VERS=`echo $UNAMER | awk '{print substr($1, 1, 1)}'`
                case "$SUN_VERS" in
                  '4' ) OS_NAME="SunOS"
                        PLATFORM="sun"
                        echo ""
                        echo "Sorry we currently only support the Solaris Platform."
                        asktocontinue;;

                  '5' ) SOL_VERS=`echo $UNAMER | awk '{print substr($1, 3, 1)}'` 
                        if [ "$SOL_VERS" -ge 6 ] ; then 
                            OS_NAME="Solaris"
                            PLATFORM="sol"
			elif [ "$SOL_VERS" -eq 5 ] ; then
			    SOL_VERS=`echo $UNAMER | awk '{print substr($1, 5, 1)}'`
			    if [ "$SOL_VERS" -ge 1 ] ; then
				OS_NAME="Solaris"
				PLATFORM="sol"
			    else
				echo "Sorry we currently only support Solaris 5.5.1 or greater."
				asktocontinue
			    fi
                        else
                            echo "Sorry we currently only support Solaris 5.5.1 or greater."
                            asktocontinue
                        fi
                        ;;
                  *   ) PLATFORM="";;
                esac;;
     'HP-UX' )  ;;
     'AIX' ) AIX_VERS=`uname -vr | awk '{print $2 * 1000 + $1}'`
		if [ AIX_VERS -lt 4001 ]; then
		    echo "Sorry we currently only support AIX 4.1 or greater."
		    asktocontinue
		fi
		;;
     *   )  echo ""
            echo "Sorry, $UNAME is currently not supported."
            echo ""
            asktocontinue ;;
esac
}

############################################################
# Ask the user for the Netscape Serever root and setup 
# other installation information. 
############################################################
setup_paths()
{
echo "
Before installing ACE/Agent for Netscape be sure to stop the 
web server or servers running on this machine."
echo "
The installation script needs to know the path of the Netscape Server 
root and VAR_ACE directories.  Please answer the following questions.
"

#
# Get the Netscape server root directory path
#
NS_SERVER_ROOT=""
until [ -n "$NS_SERVER_ROOT" ] ;
do
    echo ""
    echo "Please enter the path for the Netscape Server root directory: [/usr/ns-home]"
    read answer
    if [ -n "$answer" ] ; then
      if [ "$answer" = "/" ] ; then
        NS_SERVER_ROOT="" 
      else
        NS_SERVER_ROOT=$answer
      fi
    else
      NS_SERVER_ROOT="/usr/ns-home"
    fi
    if [ ! \( -d "$NS_SERVER_ROOT" -a -w "$NS_SERVER_ROOT" \) ] ; then
       NS_SERVER_ROOT="" 
       echo ""
       echo "The specified Netscape Server root directory does not exist or you do"
       echo "not have write permissions.  Please enter a valid directory path." 
    fi
done

#
# Now find the obj.conf file.
#

# This is our best guess at the directory.
CONFIG_TMP=""
if [ ! -f $NS_SERVER_ROOT/https-$HOSTNAME/config/obj.conf ] ; then
    if [ -f $NS_SERVER_ROOT/httpd-$HOSTNAME/config/obj.conf ] ; then
       CONFIG_TMP=$NS_SERVER_ROOT/httpd-$HOSTNAME/config 
   fi
else
   CONFIG_TMP=$NS_SERVER_ROOT/https-$HOSTNAME/config
fi

# Now confirm.

until [ -n "$NS_CONFIG" ] ;
do
    echo ""
    echo "Please enter the path for the Netscape configuration directory: [$CONFIG_TMP]"
    read answer
    if [ -n "$answer" ] ; then
      if [ "$answer" = "/" ] ; then
        NS_CONFIG="" 
      else
        NS_CONFIG=$answer
      fi
    else
      NS_CONFIG="$CONFIG_TMP"
    fi
    if [ ! \( -d "$NS_CONFIG" -a -w "$NS_CONFIG" \) ] ; then
       NS_CONFIG="" 
       echo ""
       echo "The specified Netscape Server root directory does not exist or you do"
       echo "not have write permissions.  Please enter a valid directory path." 
    fi
done

if [ ! -w "$NS_CONFIG/obj.conf" ] ; then
    echo ""
    echo "We do not have write permissions for the $NS_CONFIG/obj.conf file."
    echo "The installation cannot continue."
    abort_installation
else
    OBJ_FILE="$NS_CONFIG/obj.conf"
    OBJ_TMP="$NS_CONFIG/obj.tmp"
fi

#
# Setup the path to the plugins directory
#
if [ ! -d $NS_SERVER_ROOT/plugins ] ; then
   answer=""
   until [ -d "$answer" -a -w "$answer" ] ;
   do
     echo ""
     echo ""
     echo "ACE/Agent for netscape MUST be installed in the Netscape server plugins directory."
     echo "Please enter the correct path  [ $NS_SERVER_ROOT/plugins ]"
     read answer
     if [ -n "$answer" ] ; then
       NS_PLUGINS="$answer"
     else
       NS_PLUGINS="$NS_SERVER_ROOT/plugins"
     fi
   done
else
     NS_PLUGINS="$NS_SERVER_ROOT/plugins"
fi

ACE_PLUGINS="$NS_PLUGINS/aceagent"
if [ ! -d $ACE_PLUGINS ] ; then
    mkdir $ACE_PLUGINS
fi
echo ""
echo "The ACE/Agent plugin will be installed to the $ACE_PLUGINS directory."

#
# If the docs directory is not in the obj.conf file then something is wrong. 
# Abort the install and inform the user.
#
FIELD2=`grep document-root $NS_CONFIG/obj.conf | awk '{ print $2 }'`
FIELD3=`grep document-root $NS_CONFIG/obj.conf | awk '{ print $3 }'`
TMP=`echo "$FIELD2" | cut -c1-2` 
if [ "$TMP" != "fn" ] ; then
    NS_DOCS=`echo "$FIELD2" | sed 's/root=//'`
else
    NS_DOCS=`echo "$FIELD3" | sed 's/root=//'`
fi

if [ ! -d $NS_DOCS ] ; then
    NS_NOQUOTE=`echo $NS_DOCS | awk -F\" '{ print $2 }'`
    if [ ! -d $NS_NOQUOTE ] ; then
        echo ""
        echo "The Netscape Server root documentation directory cannot be found and"
        echo "is required for installation.  Please check your Netscape Server Installation"
        echo "and try again"
        echo ""
        abort_installation
    else
        NS_DOCS=$NS_NOQUOTE
    fi
fi

ACE_DOCS="$NS_DOCS/aceagent" 
if [ ! -d "$ACE_DOCS" ] ; then
    mkdir $ACE_DOCS
fi

echo ""
echo "The ACE/Agent Administration program will be installed in the $ACE_DOCS directory"
echo "" 

#
# Now set up the ACE_DATA path
#
if [ ! -z "$VAR_ACE" ] ; then
    DEFVARACE="$VAR_ACE"
fi
echo "Enter the path to the directory that will serve as the ACE data"
echo "directory. The username under which the Netscape Web server runs must"
echo "have \"write\" permissions on this directory. [default=$DEFVARACE]:"
read answer
if [ $answer ]; then
  DEFVARACE=$answer
fi

if [ ! -d $DEFVARACE ] ; then
    mkdir -p $DEFVARACE
fi

if [ ! -w $DEFVARACE ] ; then
    echo "Cannot write to the $DEFVARACE directory."
    echo "Please check your permissions and try again."
    abort_installation
fi

echo "Please enter the username of the tokenholder who is authorized to administer ACE/Agent."
if [ $answer ]; then 
  ADMINUSER=$answer
fi

read answer
if [ $answer ]; then 
  ADMINUSER=$answer
fi

}

############################################################
# Edit the obj.conf file to complete the plugin install. 
############################################################
edit_obj()
{
#
# Construct the new obj.conf entries and get line numbers.
#
AGENT_STR="Init fn=load-modules shlib=$ACE_PLUGINS/aceagent.so \
   funcs=\"securidinit,securidnametrans,securidauthform,securidauthorize,securidadmin\""

CFG_STR="Init fn=securidinit fname=\"$ACE_PLUGINS/aceagent.cfg\""

OBJ_STR="NameTrans fn=securidnametrans"

SRVC_STR="Service fn=\"securidauthorize\" type=\"magnus-internal/securidauth\""

FORM_STR="Service fn=\"securidauthform\" type=\"magnus-internal/securidauthform\""

ADMIN_STR="Service fn=\"securidadmin\" type=\"magnus-internal/securidadmin\""
#
# Save a copy of the original obj.conf file.
#
if cp -p $OBJ_FILE $NS_CONFIG/sdiobj.sav; then
   echo "Backup of $OBJ_FILE copied to $NS_CONFIG/sdiobj.sav";
else
   echo "Error backing up $OBJ_FILE"
   abort_installation;
fi
     
BACKUP_CRTD=TRUE

#
# Now perform the necessary obj.conf edits
#
if [ -n "`grep aceagent.so $OBJ_FILE`" ] ; then
    echo ""
    echo "It appears that the ACE/Agent has already been installed."
    echo "If we continue, the current obj.conf data will be changed."
    echo "A backup of the original will be saved."
    getyesorno FALSE "Should the installation continue? (y/n) [n]:"
    if [ "$YESORNO" = FALSE ] ; then
       abort_installation
    fi
    PREV_INSTL=TRUE
    echo "/securid/d" > .sdised
    sed -f .sdised $OBJ_FILE > $OBJ_TMP
    if cp -f $OBJ_TMP $OBJ_FILE 
    then :
    else
       echo "Error modifying obj.conf"
       abort_installation;
    fi
fi
echo "Now installing the ACE/Agent for Netscape Server files."
echo "This may take several minutes."
echo ""

echo "1i\\
$AGENT_STR
" > .sdised
    sed -f .sdised $OBJ_FILE > $OBJ_TMP
    if cp -f $OBJ_TMP $OBJ_FILE 
    then :
    else
       echo "Error modifying obj.conf"
       abort_installation
    fi

    echo "/aceagent.so/a\\
$CFG_STR" > .sdised
    sed -f .sdised $OBJ_FILE > $OBJ_TMP
    if cp -f $OBJ_TMP $OBJ_FILE 
    then :
    else
       echo "Error modifying obj.conf"
       abort_installation
    fi

    echo "/Object name=\"\{0,1\}default/a\\
$OBJ_STR " > .sdised
    sed -f .sdised $OBJ_FILE > $OBJ_TMP
    if cp -f $OBJ_TMP $OBJ_FILE 
    then :
    else
       echo "Error modifying obj.conf"
       abort_installation
    fi

    echo "/Object name=\"\{0,1\}default/,/^Service /{
/^Service /i\\
$SRVC_STR
}" > .sdised
    sed -f .sdised $OBJ_FILE > $OBJ_TMP
    if cp -f $OBJ_TMP $OBJ_FILE 
    then :
    else
       echo "Error modifying obj.conf"
       abort_installation
    fi

    echo "/Object name=\"\{0,1\}default/,/^Service /{
/^Service /a\\
$FORM_STR
}" > .sdised
    sed -f .sdised $OBJ_FILE > $OBJ_TMP
    if cp -f $OBJ_TMP $OBJ_FILE 
    then :
    else
       echo "Error modifying obj.conf"
       abort_installation
    fi

    echo "/Object name=\"\{0,1\}default/,/^Service /{
/^Service /a\\
$ADMIN_STR
}" > .sdised
    sed -f .sdised $OBJ_FILE > $OBJ_TMP
    if cp -f $OBJ_TMP $OBJ_FILE 
    then :
    else
       echo "Error modifying obj.conf"
       abort_installation
    fi

    echo "/Object name=\"\{0,1\}cgi/,/\/Object/{
/force-type/a\\
$SRVC_STR
}" > .sdised
    sed -f .sdised $OBJ_FILE > $OBJ_TMP
    if cp -f $OBJ_TMP $OBJ_FILE 
    then :
    else
       echo "Error modifying obj.conf"
       abort_installation
    fi

    echo "/Object name=\"\{0,1\}cgi/,/\/Object/{
/force-type/a\\
$FORM_STR
}" > .sdised
    sed -f .sdised $OBJ_FILE > $OBJ_TMP
    if cp -f $OBJ_TMP $OBJ_FILE 
    then :
    else
       echo "Error modifying obj.conf"
       abort_installation
    fi

    echo "/Object name=\"\{0,1\}cgi/,/\/Object/{
/force-type/a\\
$ADMIN_STR
}" > .sdised
    sed -f .sdised $OBJ_FILE > $OBJ_TMP
    if cp -f $OBJ_TMP $OBJ_FILE 
    then :
    else
       echo "Error modifying obj.conf"
       abort_installation
    fi

}

############################################################
# Install the Plugin library and configuration. 
############################################################
install_plugin()
{
#
# Install the plugin and the config file to the ACE_PLUGINS directory.
#
if [ ! \( -f "aceagent/aceagent.so" -o -f "aceagent/aceagent.cfg" \) ] ; then
    echo ""
    echo "The ageagent.so and the aceagent.cfg files must be"
    echo "in the \"aceagent\" installation directory.  Please make sure that"
    echo "the files are there and try again."
    echo ""
    abort_installation
fi
if cp -R $ACE_PLUGINS $OLD_PLUG
then :
else
    echo "Error copying $ACE_PLUGINS to $OLD_PLUG"
    abort_installation
fi

if cp -Rf aceagent/* $ACE_PLUGINS
then :
else 
    echo "Error copying plugins files to $ACE_PLUGINS"
    abort_installation
fi

if chmod 644 $ACE_PLUGINS/aceagent.cfg
then :
else 
    echo "Error changing permissions on $ACE_PLUGINS/aceagent.cfg"
    abort_installation
fi
 
#
# Now customize the configuration file. 
#
echo "/TemplatesPath/c\\
TemplatesPath=$ACE_PLUGINS/templates" >.sdised
    sed -f .sdised $ACE_PLUGINS/aceagent.cfg > .cfgtmp 
    if cp -f .cfgtmp $ACE_PLUGINS/aceagent.cfg 
    then :
    else 
        echo "Error modifying $ACE_PLUGINS/aceagent.cfg"
        abort_installation
    fi
echo "/var_ace/c\\
var_ace=$DEFVARACE" >.sdised
    sed -f .sdised $ACE_PLUGINS/aceagent.cfg > .cfgtmp 
    if cp -f .cfgtmp $ACE_PLUGINS/aceagent.cfg 
    then :
    else 
        echo "Error modifying $ACE_PLUGINS/aceagent.cfg"
        abort_installation
    fi

if [ $ADMINUSER ]; then 
    echo "/AdminList/a\\
0=$ADMINUSER" >.sdised
    sed -f .sdised $ACE_PLUGINS/aceagent.cfg > .cfgtmp 
    if cp -f .cfgtmp $ACE_PLUGINS/aceagent.cfg 
    then :
    else 
       echo "Error modifying $ACE_PLUGINS/aceagent.cfg"
       abort_installation
    fi
fi

if [ "$PREV_INSTL" = TRUE ]; then
    getyesorno TRUE "Should the existing aceagent.cfg file be overwritten? (y/n) [y]:"
    if [ "$YESORNO" = FALSE ] ; then
# restore the original cfg file
        cp -pf $OLD_PLUG/aceagent.cfg $ACE_PLUGINS 
    else
# just save it as a backup
        cp -pf $OLD_PLUG/aceagent.cfg $ACE_PLUGINS/aceagent.cfg.bak 
    fi
fi
PLUG_IS_INSTLD=TRUE

}

############################################################
# Install the Admin Applet and doc files. 
############################################################
install_admin()
{

if [ ! -d aceadmin ] ; then
    echo ""
    echo "The 'aceadmin' directory must be in the 'aceagent' "
    echo "installation directory.  Please make sure that"
    echo "the directory exists and try again."
    echo ""
    abort_installation
fi
if cp -Rp $ACE_DOCS $OLD_DOC
then :
else 
   echo "Error writing to $OLD_DOC"
   abort_installation
fi

if cp -Rf aceadmin/* $ACE_DOCS
then :
else
   echo "Error writing to $ACE_DOCS"
   abort_installation
fi

if [ -d "acedocs" ] ; then
    cp -Rf acedocs/* $ACE_DOCS
fi
ADMIN_IS_INSTLD=TRUE
}

############################################################
# All done now cleanup after yourself. 
############################################################
sdi_done()
{
rm -rf $OLD_PLUG $OLD_DOC $OBJ_TMP .sdised 

if [ ! -f "$DEFVARACE/sdconf.rec" ] ; then 
    echo ""
    echo "IMPORTANT:"
    echo "The sdconf.rec file could not be found.  Make sure that you"
    echo "copy this file to the \"$DEFVARACE\" directory before"
    echo "you attempt to use the ACE/Agent for Netscape Plugin."
    echo ""
fi

echo ""
echo "ACE/Agent for Netscape has been successfully installed. "
echo ""
echo "A Netscape server configuration file was modified. For"
echo "the changes to take effect in the Netscape Admin server,"
echo "run the Netscape Administration utility from your web"
echo "browser and click the Apply button in the upper right hand"
echo "corner."
echo ""
echo "Before you authenticate through the ACE/Agent for the first time,"
echo "make sure the username under which the Netscape Web server is running"
echo "has \"write\" permissions on $DEFVARACE"
echo "and $ACE_PLUGINS/aceagent.cfg"
echo "directories. If this is not done you may experience ACE/Server node"
echo "verification failures and trouble using the ACE/Agent administration utility."
}

############################################################
# MAIN 
############################################################

check_platform
startup_screen
setup_paths
edit_obj
install_plugin
install_admin
sdi_done
