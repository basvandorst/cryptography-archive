#ifndef _H_VPNDBG_
#define _H_VPNDBG_

#define DBG_ERROR_ON         0x0001L     // Display DBG_ERROR messages
#define DBG_WARNING_ON       0x0002L     // Display DBG_WARNING messages
#define DBG_NOTICE_ON        0x0004L     // Display DBG_NOTICE messages
#define DBG_TRACE_ON         0x0008L     // Display ENTER/TRACE/LEAVE messages
#define DBG_BREAK_ON		 0x0010L     // Enable breakpoint
#define DBG_PRINT_ON         0x0020L     // Enable print out

#define DBG_DEBUG_ALL		0xFFFFFFFFL  // Enable all debug, etc.
#define DBG_DEBUG_NONE		0x0L		 // Disable all debug, etc.

#if ( defined(DBG) && (DBG != 0)) || ( defined(DBG_MESSAGE) )

extern unsigned long VpnDbgMask;

#define STATIC

#define DBG_FUNC(F)		static const char __FUNC__[] = F;


#define DBG_ENTER()		if (VpnDbgMask & DBG_TRACE_ON) \
							{ DbgPrint("PGPMAC: >>>>> %s \n", __FUNC__); }

#define DBG_LEAVE(R)	if (VpnDbgMask & DBG_TRACE_ON) {  \
							DbgPrint("PGPMAC: <<<<< %s returning (0x%08X) \n", __FUNC__, (R));  \
						}

#define DBG_BREAK()		if (VpnDbgMask & DBG_BREAK_ON) { \
							  DbgPrint ("PGPMAC: BreakPoint in %s at line %d \n", __FUNC__, __LINE__); \
							  DbgBreakPoint(); \
						}

#define DBG_PRINT(S)	if (VpnDbgMask & DBG_PRINT_ON) { \
							DbgPrint("PGPMAC: %s: ", __FUNC__); DbgPrint S	\
						}

#define DBG_DISPLAY(S)	if (VpnDbgMask & DBG_PRINT_ON) { \
							DbgPrint("DBG TRACE: "); DbgPrint S \
						}

#ifdef ASSERT
#undef ASSERT
#endif

#define ASSERT(C)     if (!(C)) { \
                          DbgPrint("!---%s: ASSERT(%s) FAILED!\n%s #%d\n", \
                                         __FUNC__, #C, __FILE__, __LINE__); \
                          DbgBreakPoint(); \
                      }

#else	// ( defined(DBG) && (DBG != 0) )

#define STATIC           static
#define DBG_FUNC(F)
#define DBG_ENTER()
#define DBG_LEAVE(R)
#define DBG_BREAK()
#define DBG_PRINT(S)
#define DBG_LOCAL(V, P)


// End #if ( defined(DBG) && (DBG != 0) )
#endif

#ifdef CHICAGO
#ifndef DEBUG
#ifdef DBG_MESSAGE
void __cdecl DbgPrint();
#else
//#define DBG_PRINTF(A) DbgPrint A
#endif
#endif


#endif
#endif // _H_VPNDBG_