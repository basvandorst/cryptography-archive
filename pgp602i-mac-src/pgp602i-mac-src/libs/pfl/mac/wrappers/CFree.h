/*____________________________________________________________________________	CFree.h		Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: CFree.h,v 1.3.28.1 1998/11/12 03:20:05 heller Exp $____________________________________________________________________________*/#pragma onceclass CFree{protected:	void *	mMemory;	public:			CFree( void * memory )		{ mMemory	= memory; }	virtual ~CFree()	{		/* subclass should provide destructor which frees */	}				 void	Forget( void )				{ mMemory	= nil;}};