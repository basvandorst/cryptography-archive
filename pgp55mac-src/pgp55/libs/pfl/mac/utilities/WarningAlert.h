/*
 * Copyright (C) 1994-1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 *
 */

#pragma once

#include "WarningAlertTypes.h"

			
short	WarningAlert( WarningAlertType alertType, WarningAlertStyle alertStyle,
			ConstStr255Param message, ConstStr255Param str1 = NULL,
			ConstStr255Param str2 = NULL, ConstStr255Param str3 = NULL);

			
short	WarningAlert( WarningAlertType alertType, WarningAlertStyle alertStyle,
			ResID stringListResID, short stringListIndex,
			ConstStr255Param str1 = NULL,
			ConstStr255Param str2 = NULL, ConstStr255Param str3 = NULL);


