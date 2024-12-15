//////////////////////////////////////////////////////////////////////////////
// CPGPdiskInterfaceHooks.h
//
// Declarations for CPGPdiskInterfaceHooks.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskInterfaceHooks.h,v 1.3 1998/12/14 18:59:33 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskInterfaceHooks_h	// [
#define Included_CPGPdiskInterfaceHooks_h


////////////
// Constants
////////////

#define MOUSE_LEFT_BUTTON_DOWN	 0x0001  // Left Button changed to down.
#define MOUSE_LEFT_BUTTON_UP	 0x0002  // Left Button changed to up.
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004  // Right Button changed to down.
#define MOUSE_RIGHT_BUTTON_UP	 0x0008  // Right Button changed to up.
#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010  // Middle Button changed to down.
#define MOUSE_MIDDLE_BUTTON_UP	 0x0020  // Middle Button changed to up.

#define MOUSE_BUTTON_1_DOWN 	MOUSE_LEFT_BUTTON_DOWN
#define MOUSE_BUTTON_1_UP		MOUSE_LEFT_BUTTON_UP
#define MOUSE_BUTTON_2_DOWN 	MOUSE_RIGHT_BUTTON_DOWN
#define MOUSE_BUTTON_2_UP		MOUSE_RIGHT_BUTTON_UP
#define MOUSE_BUTTON_3_DOWN 	MOUSE_MIDDLE_BUTTON_DOWN
#define MOUSE_BUTTON_3_UP		MOUSE_MIDDLE_BUTTON_UP

#define MOUSE_BUTTON_4_DOWN 	0x0040
#define MOUSE_BUTTON_4_UP		0x0080
#define MOUSE_BUTTON_5_DOWN 	0x0100
#define MOUSE_BUTTON_5_UP		0x0200

#define MOUSE_WHEEL 			0x0400


////////
// Types
////////

typedef struct _MOUSE_INPUT_DATA {

	USHORT	UnitId;
	USHORT	Flags;

	union 
	{
		ULONG Buttons;

		struct 
		{
			USHORT	ButtonFlags;
			USHORT	ButtonData;
		};
	};

	ULONG	RawButtons;
	LONG	LastX;
	LONG	LastY;
	ULONG	ExtraInformation;

} MOUSE_INPUT_DATA, *PMOUSE_INPUT_DATA;


/////////////////////////////
// Class KeyboardFilterDevice
/////////////////////////////

class KeyboardFilterDevice : public KFilterDevice
{
public:
	DualErr mInitErr;

						KeyboardFilterDevice();
						~KeyboardFilterDevice();

	virtual NTSTATUS	Read(KIrp I);
	virtual NTSTATUS	OnIrpComplete(KIrp I, PVOID Context);

private:
	PGPUInt32			mNumOutstandingIrps;	// # of IRPs outstanding
	PIO_STACK_LOCATION	mCurrentStackLocation;	// current stack location

	SAFE_DESTRUCTORS
};


//////////////////////////
// Class MouseFilterDevice
//////////////////////////

class MouseFilterDevice : public KFilterDevice
{
public:
	DualErr mInitErr;

						MouseFilterDevice();
						~MouseFilterDevice();

	virtual NTSTATUS	Read(KIrp I);
	virtual NTSTATUS	OnIrpComplete(KIrp I, PVOID Context);

private:
	PGPUInt32			mNumOutstandingIrps;	// # of IRPs outstanding
	PIO_STACK_LOCATION	mCurrentStackLocation;	// current stack location

	SAFE_DESTRUCTORS
};

#endif	// ] Included_CPGPdiskInterfaceHooks_h
