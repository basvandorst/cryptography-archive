/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//: TreeListVer.h	- version history of TreeList control
//
//	$Id: TreeListVer.h,v 1.7 1997/06/03 14:54:12 pbj Exp $
//
//
// 970131	024	begin version history tracking
//
// 970202   025	-move hMemDC creation/deletion from TLPaint to TLInitPaint
//				 to increase performance.
//				-changed bar background to pattern brush.
//				-fixed hilight bar when iFirstColumnWidth < xoffset.
//				-fixed vertical line bug (when prev item was out of window).
//				-fixed font handling and line/header spacing
//
// 970203	026	-fixed bug where focus was incorrect following autoscroll
//				-fixed one pixel row of garbage at upper right on resize
//				-fixed pattern of pattern brush
//				-added code to allow solid brush on hicolor displays
//
// 970207	027 -fixed bug that caused strings of 2 chars not being shown
//
// 970212	028	-added support for "item click" header notifications
//
// 970224	029	-changed deletetree message to support non-freeing of items
//				-change TLInsertItem to allow reinserting items
//
// 970227	030 -changed handling of keyboard messages to support DEL (and
//				 other non-character) keys
//
// 970303	031 -fixed bug that could cause hanging focus and firstselect
//				 pointers after item deletion
//				
// 970304	032	-added code to paint out-of-range bars in hatched brush
//				 (for axiomatic keys)
//
// 970306	033	-fixed first chance exception by creating TLPrePaint routine
//
// 970310	034 -changed line and focus line to dotted line (using LineDDA)
//
// 970312	035	-fixed string allocation bug in TLSetTreeItem
//				-added call to TLSelectAutoScroll when collapsing entire tree
//
// 970314	036	-rebuilt with static runtime linkage
//
// 970325	037	-added keyboard support for home/end
//				-added keyboard support for keypad +/-
//				-added support for shift-up/dn and shift-home/end
//				-added keyboard autoscroll support to emulate Windows standard
//
// 970327	038	-fixed bug that was causing PointSearch to make mistakes
//				-fixed bug that could cause crash after tree deletion
//
// 970331	039	-allowed non-alphanumeric keystrokes when autoscrolling
//				-fixed autoscroll on expand/collapse
//
// 970401	040 -added TLIF_PREVHANDLE for retrieving handle of previous item
//				-added flag to TreeList_Select to not deselect
//				-added TLM_ENSUREVISIBLE message handling
//
// 970408	041	-fixed bug where focused item following delete could not be
//				 selected with single mouse click
//
// 970416	042	-fixed bug where right clicking w/control or shift caused
//				 loss of selection
//
// 970418	043	-fixed minor bugs with selection/focus/keyboard
//
// 970425	044	-fixed bug causing lockup when enter key pressed
//
// 970505	045	-fixed memory leak on reinsertion
//				-fixed bug where focus was not getting removed when
//				 clicking on "nothing"
//				-added promiscuous selection
//				-changed logic of IterateSelected to traverse entire tree
//				-reversed order of IterateSelected to obviate problems
//				 in PGPkeys
//
// 970507	046	-fixed bug where notification was not being sent when
//				 clicking on "nothing"
//
// 970508	047	-refinement of fix introduced in 046
//
// 970519	048	-removed all global variables
//
// 970603		-fixed bug causing lockup on keyboard input when userid or
//				 cert is focused
//
