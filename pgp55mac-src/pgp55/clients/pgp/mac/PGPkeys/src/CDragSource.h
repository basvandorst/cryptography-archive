#pragma once

class CKeyDragTask;

class CDragSource
{
public:
	virtual void	GetDragHiliteRgn(
						RgnHandle	ioHiliteRgn )	= 0;
						
	virtual void	AddFlavors(		
						CKeyDragTask	*dragTask )	= 0;
												
	virtual Boolean	GetTextBlock(
						ItemReference	item, 
						void			**outData,
						long			*len )		= 0;
	virtual	Boolean	GetHFS(
						ItemReference	item,
						DragReference	dragRef,
						FSSpec			*spec)		= 0;
};

