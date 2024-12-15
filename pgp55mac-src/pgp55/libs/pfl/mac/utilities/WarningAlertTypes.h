/*
 * Copyright (C) 1994-1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 *
 */
#pragma once

typedef enum WarningAlertType
{
	kWAInvalidAlertType	= 0,
	kWANoteAlertType,
	kWACautionAlertType,
	kWAStopAlertType,
	kWACustomAlertType

} WarningAlertType;

typedef enum WarningAlertStyle
{
	kWAInvalidAlertStyle	= 0,
	kWAOKStyle,
	kWACancelStyle,
	kWAOKCancelStyle,
	kWACancelOKStyle,
	kWAYesNoStyle,
	kWANoYesStyle,
	kWACustomAlertStyle

} WarningAlertStyle;

