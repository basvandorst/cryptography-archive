/*
	File:		ContextualMenuPlugins.h

	Contains:	<contents>

	Written by:	Guy Fullerton

	Copyright:	<copyright>

	Change History (most recent first):

         <4>     1/30/97    GBF     Adding some Apple Event keys.
         <3>     1/16/97    GBF     Adding some constants and removing some
         							obsolete stuff.
         <2>    10/15/96    GBF     Need to include Types.h
        <0+>    10/15/96    GBF     moved from assistance tree.
*/


/*
	File:		ContextualMenuPlugins.h

	Contains:	<contents>

	Written by:	Guy Fullerton

	Copyright:	<copyright>

	Change History (most recent first):

         <2>     7/28/96    GBF     first checked in
         <1>     7/15/96    GBF     first checked in
*/


#pragma once

#ifndef __CONTEXTUALMENUPLUGINS__
#define __CONTEXTUALMENUPLUGINS__

	// Mac OS Includes
#include <Types.h>


enum
{
	kExamineContextNoTimeout				=	0,
	
	kContextualMenuPluginFileType			=	'cmpi',
	
	keyContextualMenuCommandID				=	'cmcd',
	keyContextualMenuSubmenu				=	'cmsb'
};

#endif