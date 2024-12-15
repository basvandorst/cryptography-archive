/*

mess - automagic Mail Encryption and Signing System for mail & news

Copyright 1994 Stuart Smith
May be distributed under the terms of the GNU General Public License.

*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "copy.h"
#include "header.h"

/*	Extensive #define's .. */

#define options_length	255
#define line_length	255
#define command_length	128
#define encrypt_command "pgp "
#define default_options " -waft "
#define encrypt_string	"[encrypt]"
#define sign_string	"[sign]"
#define encrypt_options	" -e "
#define sign_options	" -s +clearsig=on "

