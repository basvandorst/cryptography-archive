/* =====================================================
  
   Eudora Extended Message Services API SDK 1.0b3 (July 12, 1996)
   This SDK supports EMSAPI version 1
   Copyright 1995, 1996 QUALCOMM Inc.
   Send comments and questions to <emsapi-info@qualcomm.com>
   
   Template for definitions supplied by translator to component manager.
   
   This file should be modified by the translator writer
   to tell the component manager a couple of things it needs
   to know about the translator itself. In general the component
   manager interface is indepedent of the translator characteristics,
   but these details are needed:
   
   * The sizeof the globals structure - because the component manager
     manages this block of storage
   
   * Which entry points are implemented - because we use the component
     manager to determine which translation API functions are implemented.
   
   This requires emsapi-mac.h (include it before you include this file)
*/
#pragma once

/* ----------------------------------------------------------------------
    Declaration of structure for global storage that is shared
    between all translator functions.  A handle to this is passed
    in as the first argument for all entry points. Allocation of this
    is taken care of by the component manager.
    
 */
typedef	struct	emsUserGlobalsS
{
	Component		self;						// Required!
	
	void *			myGlobals;
} emsUserGlobals;


/* ----------------------------------------------------------------------
   Defines indicating whether or not a translator supplies a
   function or not.  The Eudora uses these via the component manager
   to determine whether these functions should be called or not.
 */
//**** SET FLAGS FOR FUNCTIONS SUPPLIED BY YOUR TRANSLATOR HERE
// AND DELETE THIS LINE
#define EMS_HAS_PLUGIN_VERSION      true /* Required */
#define EMS_HAS_PLUGIN_INIT         true /* Required */
#define EMS_HAS_TRANSLATOR_INFO     true /* Required */
#define EMS_HAS_CAN_TRANSLATE_FILE  true 
#define EMS_HAS_CAN_TRANSLATE_BUF   false
#define EMS_HAS_TRANSLATE_FILE      true
#define EMS_HAS_TRANSLATE_BUF       false  
#define EMS_HAS_PLUGIN_FINISH       true /* Required */

