/* ======================================================================	Functions to aid manipulation and parsing of MIME RFC1847 objects.	Filename:			RFC1847.h	Last Edited:		March 7, 1997	Authors:			Scott Manjourides, Bob Fronabarger	Copyright:			1995, 1996 QUALCOMM Inc.	Technical support:	<emsapi-info@qualcomm.com>*/#ifndef RFC1847_H#define RFC1847_H#include "buftype.h"/* Internally used values */typedef enum {	CSE_Start,	CSE_DoHeader,	CSE_DoBoundary,	CSE_DoPart,	CSE_DoOutput,	CSE_Done,	CSE_Fail} CreateStageEnum;/* Internally used values */typedef enum {	PSE_Start,	PSE_SkipHeader,	PSE_ParseHeader,	PSE_SkipFirst,	PSE_PrePart,	PSE_OutPart,	PSE_Done,	PSE_Fail,	PSE_KeepUntil,	PSE_IgnoreUntil,	PSE_OutputUntil,	PSE_DoOutput} ParseStageEnum;/* Create-state structure */typedef struct createState {	Boolean			initialized;	CreateStageEnum stage, next_stage, afterout_stage;	unsigned long	current_part;	BufTypeHandle	hTmpBuf, hOutputBuf;	Str255			boundaryStr;} createState, *createStatePtr, **createStateHandle;/* Parse-state structure */typedef struct parseState {	Boolean			initialized;	ParseStageEnum	stage, next_stage, afterout_stage;	unsigned long	current_part, outputCount, afteroutSkip, leftSpanCount;	BufTypeHandle	hOutputBuf, hSearchBuf, hPrevBuf, hBoundaryBuf,					hDblNewlineBuf, hNewlineBuf;} parseState, *parseStatePtr, **parseStateHandle;enum { RFC1847_FAIL, RFC1847_COMPLETED, RFC1847_BUFFERFULL };createStateHandle	NewCreateState();void				DeleteCreateState(createStateHandle pState);parseStateHandle	NewParseState();void				DeleteParseState(parseStateHandle pState);long	RFC1847_Create(emsMIMEtypeH mimeHdl, BufTypeHandle pOutBuf,			BufTypeHandle pInPart1, BufTypeHandle pInPart2, createStateHandle pState);long	RFC1847_Parse(emsMIMEtypeH *mimeHdl, BufTypeHandle pOutPart1,			BufTypeHandle pOutPart2, BufTypeHandle pInBuf, parseStateHandle pState);#endif /* RFC1847_H */