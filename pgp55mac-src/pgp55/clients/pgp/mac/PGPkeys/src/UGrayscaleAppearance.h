//
//	UGrayscaleAppearance.h
//
//	Utilities for supporting the Apple Grayscale Appearance for System 7.5
//
//	Note: The Attachments can't paint to the edges of dialogs.
//	For dialogs, use ResEdit to set the Content Color of the WIND's wctb 
//	to ga_Background = { 56797, 56797, 56797 }
//
//	Note: Most references to ga_2 or ga_Background might be ignored since
//	the background will usually have been erased.
// 
//	Copyright (C) 1996 by James Jennings. All rights reserved.
//

#pragma once

class UGrayscaleAppearance {
	// really just a namespace for our basic utilities.
public:
	typedef enum {
			ga_White 	= 0xF,
			ga_1		= 0xE,
			ga_2		= 0xD,
			ga_3		= 0xC,
			ga_4		= 0xB,
			ga_5		= 0xA,
			ga_6		= 0x9,
			ga_7		= 0x8,
			ga_8		= 0x7,
			ga_9		= 0x6,
			ga_10		= 0x5,
			ga_11		= 0x4,
			ga_12		= 0x2,
			ga_Black	= 0x0,
			ga_Hilite	= ga_White,
			ga_Background = ga_2,
			ga_Shadow	= ga_6
			} EGAColor;
	static void		GetColor( EGAColor in, RGBColor & out )
					{ out.red = out.green = out.blue = in * 0x1111; }
	static void		SetForeColor( EGAColor in )
					{ RGBColor c; GetColor( in, c ); ::RGBForeColor( &c ); }
	static void		SetBackColor( EGAColor in )
					{ RGBColor c; GetColor( in, c ); ::RGBBackColor( &c ); }
	
	static void		PaintRect( Rect &inFrame, EGAColor inFill )
					{ SetForeColor( inFill ); ::PaintRect( &inFrame ); }
	static void		EraseRect( Rect &inFrame, EGAColor inFill = ga_Background )
					{ SetBackColor( inFill ); ::EraseRect( &inFrame ); }
	static void		FrameRect( const Rect &inFrame, 
						EGAColor inTopLeft, EGAColor inBottomRight,
						EGAColor inCorners);
};

