/*
	File:		ContextTypeCMPlugin.cp

	Contains:	<contents>

	Written by:	Guy Fullerton

	Copyright:	<copyright>

	Change History (most recent first):

         <4>     1/16/97    GBF     Updating for Direct to SOM. Adding fragment initializer.
         <3>     4/12/96    GBF     The parameters for ExamineContext had changed.
         <2>     3/22/96    GBF     Added outNeedMoreTime parameter to ExamineContext
         <1>     3/14/96    GBF     first checked in
*/


// Class Header
#include "ContextTypeCMPlugin.h"

// Mac OS Includes
#include <AERegistry.h>
#include <CodeFragments.h>
#include <ContextualMenuPlugins.h>



// Function declarations
extern pascal OSErr __initialize(CFragInitBlockPtr); // metrowerks's default initializer
pascal OSErr ContextTypeCMPluginInitialize(CFragInitBlockPtr init); // our initializer
OSStatus AddCommandToAEDescList(ConstStr255Param inCommandString,
	SInt32 inCommandID, AEDescList* ioCommandList);



/*******************************************************************************

	ContextTypeCMPluginInitialize
	
		All plugin SOM object must somehow register themselves so clients
		can instantiate them by name.  The best place that I've found to
		do it is in the fragment initializer.  Using direct to SOM means
		that we can't explicitly register our class name, so we have to
		implicitly register it by instantiating an object.

*******************************************************************************/

pascal OSErr ContextTypeCMPluginInitialize(CFragInitBlockPtr init)
{
#pragma unused (init)

	OSErr theError = __initialize(init);
	if (theError == noErr)
	{
		ContextTypeCMPlugin* thePlugin = new ContextTypeCMPlugin;
		if (thePlugin != NULL)
			delete thePlugin;
		else
			theError = memFullErr;
	}

	return theError;
	
} // ContextTypeCMPluginInitialize



/*******************************************************************************

	ContextTypeCMPlugin::Initialize

*******************************************************************************/

OSStatus  ContextTypeCMPlugin::Initialize(
	Environment*,
	FSSpec* inFileSpec)
{
#pragma unused (inFileSpec)

	// we don't need to do anything special here

    return noErr;
    
} // ContextTypeCMPlugin::Initialize



/*******************************************************************************

	ContextTypeCMPlugin::ExamineContext

*******************************************************************************/

OSStatus  ContextTypeCMPlugin::ExamineContext(
	Environment*,
	AEDesc *inContextDescriptor,
	SInt32 inTimeOutInTicks,
	AEDescList* ioCommands,
	Boolean* outNeedMoreTime)
{
#pragma unused(inTimeOutInTicks)
	
	// this is a quick sample that looks for text in the context descriptor
	
	// make sure the descriptor isn't null
	if (inContextDescriptor != NULL)
	{
		// tell the raw type of the descriptor
		Str15 theDescriptorType;
		theDescriptorType[0] = 4;
		*(DescType*)(&theDescriptorType[1]) = inContextDescriptor->descriptorType;
		::AddCommandToAEDescList(theDescriptorType, 1, ioCommands);
		
		// try to get text out of the context descriptor; make sure to
		// coerce it, cuz the app may have passed an object specifier or
		// styled text, etc.
		AEDesc theTextDesc = { typeNull, NULL };
		if (::AECoerceDesc(inContextDescriptor, typeChar, &theTextDesc) == noErr)
		{
			// add a text only command to our command list
			::AddCommandToAEDescList("\pWe got text!", 2, ioCommands);
		}
		::AEDisposeDesc(&theTextDesc);
	}
	else
	{
		// we have a null descriptor
		::AddCommandToAEDescList("\pNULL Descriptor", 3, ioCommands);
	}
	
	*outNeedMoreTime = false;
	
	return noErr;
	
} // ContextTypeCMPlugin::ExamineContext



/*******************************************************************************

	ContextTypeCMPlugin::HandleSelection

*******************************************************************************/

OSStatus ContextTypeCMPlugin::HandleSelection(
	Environment*,
	AEDesc *inContextDescriptor,
	SInt32 inCommandID)
{
#pragma unused (inContextDescriptor, inCommandID)

	// here is where you would actually carry out the action that the user
	// requested.
	
    return noErr;
    
} // ContextTypeCMPlugin::HandleSelection



/*******************************************************************************

	ContextTypeCMPlugin::PostMenuCleanup

*******************************************************************************/

OSStatus ContextTypeCMPlugin::PostMenuCleanup(
	Environment*)
{
	// we didn't allocate any buffers or cache any data in ExamineContext,
	// so we don't need to clean anything up here
	
    return noErr;
    
} // ContextTypeCMPlugin::PostMenuCleanup


/*******************************************************************************

	AddCommandToAEDescList

*******************************************************************************/

OSStatus AddCommandToAEDescList(
	ConstStr255Param inCommandString,
	SInt32 inCommandID,
	AEDescList* ioCommandList)
{
	OSStatus theError = noErr;
	
	AERecord theCommandRecord = { typeNull, NULL };
	
	do
	{
		// create an apple event record for our command
		theError = ::AECreateList(NULL, 0, true, &theCommandRecord);
		if (theError != noErr) break;
		
		// stick the command text into the aerecord
		theError = ::AEPutKeyPtr(&theCommandRecord, keyAEName, typeChar,
			&inCommandString[1], inCommandString[0]);
		if (theError != noErr) break;
			
		// stick the command ID into the AERecord
		theError = ::AEPutKeyPtr(&theCommandRecord, keyContextualMenuCommandID, typeLongInteger,
			&inCommandID, sizeof (inCommandID));
		if (theError != noErr) break;
		
		// stick this record into the list of commands that we are passing back to CMM
		theError = ::AEPutDesc(ioCommandList, // the list we're putting our command into
						0, // stick this command onto the end of our list
						&theCommandRecord); // the command I'm putting into the list
		
	} while (false);
	
	// clean up after ourself; dispose of the AERecord
	AEDisposeDesc(&theCommandRecord);

    return theError;
    
} // ContextTypeCMPlugin::PostMenuCleanup


