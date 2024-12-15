/*____________________________________________________________________________	Copyright (C) 1999 Network Associates, Inc.	All rights reserved.	$Id: CAdvancedPanel.h,v 1.8 1999/01/31 08:28:42 wprice Exp $____________________________________________________________________________*/#pragma once #include <LListener.h>#include "CPanel.h"#include "CColumnTable.h"#include "pgpNetPrefs.h"class CAdvancedPanel;class	CIPSECTable	:	public CColumnTable{	friend				CAdvancedPanel;public:	enum { class_ID = 'ipsT' };						CIPSECTable(LStream *inStream);	virtual				~CIPSECTable();	void				FinishCreateSelf();	virtual Boolean		GetCellDrawData(								STableCell			inCell,								ResIDT				&iconID,								Int16				&indentLevel,								Str255				data,								StyleParameter		&style );	void				SetPanel( CAdvancedPanel * panel ) { mPanel = panel;}protected:	PGPNetPrefIPSECProposalEntry *	mIPSECProps;	PGPUInt32						mNumProps;private:	CAdvancedPanel *	mPanel;};class	CIKETable	:	public CColumnTable{	friend				CAdvancedPanel;public:	enum { class_ID = 'ikeT' };						CIKETable(LStream *inStream);	virtual				~CIKETable();	void				FinishCreateSelf();	virtual Boolean		GetCellDrawData(								STableCell			inCell,								ResIDT				&iconID,								Int16				&indentLevel,								Str255				data,								StyleParameter		&style );	void				SetPanel( CAdvancedPanel * panel ) { mPanel = panel;}protected:	PGPNetPrefIKEProposalEntry *	mIKEProps;	PGPUInt32						mNumProps;private:	CAdvancedPanel *	mPanel;};class CAdvancedPanel	:		public CPanel,								public LListener{public:	enum { class_ID = 'pAdv' };					CAdvancedPanel(LStream * inStream);	virtual			~CAdvancedPanel();						virtual void	ListenToMessage(MessageT inMessage, void * ioParam);									virtual void	Load(void * inParam);	virtual void	Save(void * inParam);						protected:	virtual void	FinishCreateSelf();	void			IKEProposalDialog( PGPUInt32			propIndex );	void			IPSECProposalDialog( PGPUInt32			propIndex );		CIKETable *		mIKETable;	CIPSECTable *	mIPSECTable;	CColumnTable *	mSelectedTable;	PGPBoolean		mChange;		LPushButton *	mEditButton,				*	mRemoveButton,				*	mMoveUpButton,				*	mMoveDownButton;};