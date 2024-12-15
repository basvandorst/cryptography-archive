/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: SearchFilter.c,v 1.28 1998/08/11 15:20:18 pbj Exp $
____________________________________________________________________________*/
#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include <assert.h>
#include <commctrl.h>

#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpBuildFlags.h"

#include "Search.h"
#include "SearchFilter.h"
#include "PGPcl.h"
#include "PGPWerr.h"
#include "resource.h"

#define	IS					0
#define IS_NOT				1
#define IS_ON_OR_BEFORE		1
#define IS_AT_LEAST			1
#define IS_NOT_FROM			1
#define CONTAINS			2
#define IS_ON_OR_AFTER		2
#define IS_AT_MOST			2
#define DOES_NOT_CONTAIN	3
#define IS_SIGNED_BY		4
#define IS_NOT_SIGNED_BY	5

#define DH_KEY_TYPE			0
#define RSA_KEY_TYPE		1

#define REVOKED				0
#define DISABLED			1

extern PGPContextRef	g_Context; 
extern HINSTANCE		g_hInst;

PGPError KeyIdFilterFromKeySet(	PGPKeySetRef keySet,
								PGPFilterRef* filter);

PGPError CreateFilter(HWND hwnd, PGPFilterRef* filter, int* action)
{
	PGPError error = kPGPError_NoErr;
	HWND hwndAttribute, hwndVerb;
	HWND hwndEditSpecifier, hwndComboSpecifier, hwndComboListSpecifier;
	HWND hwndTime;
	int AttributeSelection = 0;
	int VerbSelection = 0;
	int SpecifierSelection = 0;
	char* SpecifierBuffer = NULL;
	DWORD SpecifierLength = 0;
	int Month = 1;
	int Day = 1;
	int Year = 1970;
	BOOL bNegate = FALSE;
	SYSTEMTIME st;

	assert(hwnd);
	assert(filter);
	assert(action);

	// set default action
	*action = ACTION_INTERSECT;

	// find all our window handles
	hwndAttribute			= GetDlgItem(hwnd, IDC_ATTRIBUTE);
	hwndVerb				= GetDlgItem(hwnd, IDC_VERB);
	hwndEditSpecifier		= GetDlgItem(hwnd, IDC_SPECIFIER_EDIT);
	hwndComboSpecifier		= GetDlgItem(hwnd, IDC_SPECIFIER_COMBO);
	hwndComboListSpecifier	= GetDlgItem(hwnd, IDC_SPECIFIER_COMBO_LIST);
	hwndTime				= GetProp(hwnd, "hwndTime");
		
	// find what the user has chosen to search on
	AttributeSelection	= ComboBox_GetCurSel(hwndAttribute);
	VerbSelection		= ComboBox_GetCurSel(hwndVerb);
	SpecifierSelection	= ComboBox_GetCurSel(hwndComboSpecifier);
	if (SpecifierSelection < 0) 
		SpecifierSelection	= ComboBox_GetCurSel(hwndComboListSpecifier);

	// get date currently in time/date picker control
	SendMessage (hwndTime, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
	// struct tm represents months 0-11  so we have to subtract 1
	Month = st.wMonth - 1; 
	// struct tm represents days 1-31  we have don't subtract 1
	Day =	st.wDay;
	// struct tm represents years as offset from 1900
	Year =	st.wYear - 1900;

	// combo is zero based and the string list is not... adjust Selection
	switch( AttributeSelection + IDS_SEARCH_STRINGTABLE_BASE + 1 ) 
	{
		case IDS_ATTRIBUTE_1: // user id
		{
			SpecifierLength = Edit_GetTextLength(hwndEditSpecifier) + 1;
			SpecifierBuffer = (char*) malloc(SpecifierLength);

			if( SpecifierBuffer )
			{
				Edit_GetText(	hwndEditSpecifier, 
								SpecifierBuffer, 
								SpecifierLength);

				// MessageBox(NULL, SpecifierBuffer, "user id", MB_OK);

				switch( VerbSelection )
				{
					case IS_NOT:
						bNegate = TRUE;
					case IS:
					{
						error = PGPNewUserIDStringFilter(
										g_Context, 
										SpecifierBuffer, 
										kPGPMatchEqual,
										filter );

						if( bNegate && IsntPGPError(error) )
						{
							PGPFilterRef NegatedFilter = kPGPInvalidRef;

							error = PGPNegateFilter(*filter, 
													&NegatedFilter);

							if( IsntPGPError(error) )
							{
								*filter = NegatedFilter;
							}
						}

						break;
					}

					case DOES_NOT_CONTAIN:
						bNegate = TRUE;
					case CONTAINS:
					{
						
						error = PGPNewUserIDStringFilter(
										g_Context, 
										SpecifierBuffer, 
										kPGPMatchSubString,
										filter );

						if( bNegate && IsntPGPError(error) )
						{
							PGPFilterRef NegatedFilter = kPGPInvalidRef;

							error = PGPNegateFilter(*filter, 
													&NegatedFilter);

							if( IsntPGPError(error) )
							{
								*filter = NegatedFilter;
							}
						}

						break;
					}

					case IS_NOT_SIGNED_BY:
						bNegate = TRUE;
					case IS_SIGNED_BY:
					{
						PGPKeySetRef	ringSet		= kPGPInvalidRef;
						PGPKeySetRef	filteredSet	= kPGPInvalidRef;


						error = PGPOpenDefaultKeyRings(	g_Context, 
														0, 
														&ringSet);

						if(ringSet && IsntPGPError(error))
						{
							PGPFilterRef userIdFilter = kPGPInvalidRef;

							error = PGPNewUserIDStringFilter(
												g_Context, 
												SpecifierBuffer, 
												kPGPMatchSubString,
												&userIdFilter );

							if( IsntPGPError(error) )
							{
								error = PGPFilterKeySet(ringSet, 
														userIdFilter, 
														&filteredSet);

								PGPFreeFilter(userIdFilter);
							}

							if( IsntPGPError(error) )
							{
								error = KeyIdFilterFromKeySet(	filteredSet,
																filter);
								PGPFreeKeySet(filteredSet);
							}

							if (!PGPRefIsValid (*filter))
							{
								error = kPGPError_Win32_NoSigningKey;
							}

							PGPFreeKeySet(ringSet);
						}

						if( bNegate && IsntPGPError(error) )
						{
							PGPFilterRef NegatedFilter = kPGPInvalidRef;

							error = PGPNegateFilter(*filter, 
													&NegatedFilter);

							if( IsntPGPError(error) )
							{
								*filter = NegatedFilter;
							}
						}

						break;
					}
				}

				free(SpecifierBuffer);
			}
			else
			{
				error = kPGPError_OutOfMemory;
			}

			break;
		}

		case IDS_ATTRIBUTE_2: // key id
		{
			SpecifierLength = Edit_GetTextLength(hwndEditSpecifier) + 1;
			SpecifierBuffer = (char*) malloc(SpecifierLength);

			if( SpecifierBuffer )
			{
				Edit_GetText(	hwndEditSpecifier, 
								SpecifierBuffer, 
								SpecifierLength);

				//MessageBox(NULL, SpecifierBuffer, "key id", MB_OK);

				switch( VerbSelection )
				{
					case IS_NOT:
						bNegate = TRUE;
					case IS:
					{
						PGPKeyID keyId;

						error = PGPGetKeyIDFromString(	SpecifierBuffer, 
														&keyId );
						if(IsntPGPError(error))
						{
							error = PGPNewKeyIDFilter(	g_Context, 
													&keyId,
													filter );
					
							if( bNegate && IsntPGPError(error) )
							{
								PGPFilterRef NegatedFilter = kPGPInvalidRef;

								error = PGPNegateFilter(*filter, 
														&NegatedFilter);

								if( IsntPGPError(error) )
								{
									*filter = NegatedFilter;
								}
							}
						}

						break;
					}
				}

				free(SpecifierBuffer);
			}
			else
			{
				error = kPGPError_OutOfMemory;
			}

			break;
		}

		case IDS_ATTRIBUTE_3: // key type
		{
			PGPByte encryptAlgorithm = 0;

			if( SpecifierSelection == DH_KEY_TYPE )
			{
				encryptAlgorithm = kPGPPublicKeyAlgorithm_ElGamal;
			}
			else if( SpecifierSelection == RSA_KEY_TYPE )
			{
				encryptAlgorithm = kPGPPublicKeyAlgorithm_RSA;
			}
			
			error = PGPNewKeyEncryptAlgorithmFilter(g_Context, 
													encryptAlgorithm, 
			 										filter);
			break;
		}

		case IDS_ATTRIBUTE_4: // creation date
		{
			struct tm time;

			memset(&time, 0x00, sizeof(time));

			time.tm_mday	= Day;    /* day of the month - [1,31] */
			time.tm_mon		= Month;  /* months since January - [0,11] */
			time.tm_year	= Year;   /* years since 1900 */

			switch( VerbSelection )
			{

				// In order to simulate an "is date" without the
				// user having to enter the exact hour, min, sec
				// the key was created, we fake it by doing an
				// intersection between the entire day.
				case IS:
				{
					PGPFilterRef filterAM;
					PGPFilterRef filterPM;

					/* midnight AM */
					time.tm_sec		= 0; 
					time.tm_min		= 0;     
					time.tm_hour	= 0;

					error = PGPNewKeyCreationTimeFilter( 
								g_Context, 
								PGPGetPGPTimeFromStdTime(mktime(&time)), 
								kPGPMatchGreaterOrEqual, 
								&filterAM );

					if(IsPGPError(error))
					{
						break;
					}

					/* just before midnight PM */
					time.tm_sec		= 59; 
					time.tm_min		= 59;     
					time.tm_hour	= 23;

					error = PGPNewKeyCreationTimeFilter( 
								g_Context, 
								PGPGetPGPTimeFromStdTime(mktime(&time)), 
								kPGPMatchLessOrEqual, 
								&filterPM );

					if(IsntPGPError(error))
					{
						error = PGPIntersectFilters(filterAM,
													filterPM,
													filter);
					}
					
					break;
				}

				case IS_ON_OR_BEFORE:
				{
					
					/* just before midnight PM*/
					time.tm_sec		= 59; 
					time.tm_min		= 59;     
					time.tm_hour	= 23;

					error = PGPNewKeyCreationTimeFilter( 
								g_Context, 
								PGPGetPGPTimeFromStdTime(mktime(&time)),  
								kPGPMatchLessOrEqual, 
								filter );
					break;
				}

				case IS_ON_OR_AFTER:
				{
					/* midnight AM*/
					time.tm_sec		= 0; 
					time.tm_min		= 0;     
					time.tm_hour	= 0;

					error = PGPNewKeyCreationTimeFilter( 
								g_Context, 
								PGPGetPGPTimeFromStdTime(mktime(&time)), 
								kPGPMatchGreaterOrEqual, 
								filter );

					break;
				}
			}
			break;
		}

		case IDS_ATTRIBUTE_5: // expiration date
		{
			struct tm time;
			
			memset(&time, 0x00, sizeof(time));

			time.tm_mday	= Day;    /* day of the month - [1,31] */
			time.tm_mon		= Month;  /* months since January - [0,11] */
			time.tm_year	= Year;   /* years since 1900 */

			switch( VerbSelection )
			{

				// In order to simulate an "is date" without the
				// user having to enter the exact hour, min, sec
				// the key was created, we fake it by doing an
				// intersection between the entire day.
				case IS:
				{
					PGPFilterRef filterAM;
					PGPFilterRef filterPM;

					/* midnight AM */
					time.tm_sec		= 0; 
					time.tm_min		= 0;     
					time.tm_hour	= 0;

					error = PGPNewKeyExpirationTimeFilter( 
								g_Context, 
								PGPGetPGPTimeFromStdTime(mktime(&time)), 
								kPGPMatchGreaterOrEqual, 
								&filterAM );

					if(IsPGPError(error))
					{
						break;
					}

					/* just before midnight PM */
					time.tm_sec		= 59; 
					time.tm_min		= 59;     
					time.tm_hour	= 23;

					error = PGPNewKeyExpirationTimeFilter( 
								g_Context, 
								PGPGetPGPTimeFromStdTime(mktime(&time)), 
								kPGPMatchLessOrEqual, 
								&filterPM );

					if(IsntPGPError(error))
					{
						error = PGPIntersectFilters(filterAM,
													filterPM,
													filter);

					}
					
					break;
				}

				case IS_ON_OR_BEFORE:
				{
					
					/* just before midnight PM*/
					time.tm_sec		= 59; 
					time.tm_min		= 59;     
					time.tm_hour	= 23;

					error = PGPNewKeyExpirationTimeFilter( 
								g_Context, 
								PGPGetPGPTimeFromStdTime(mktime(&time)),  
								kPGPMatchLessOrEqual, 
								filter );
					break;
				}

				case IS_ON_OR_AFTER:
				{
					/* midnight AM*/
					time.tm_sec		= 0; 
					time.tm_min		= 0;     
					time.tm_hour	= 0;

					error = PGPNewKeyExpirationTimeFilter( 
								g_Context, 
								PGPGetPGPTimeFromStdTime(mktime(&time)), 
								kPGPMatchGreaterOrEqual, 
								filter );

					break;
				}
			}
			break;
		}

		case IDS_ATTRIBUTE_6: // key
		{
			if( SpecifierSelection == REVOKED )
			{
				error = PGPNewKeyRevokedFilter(	
							g_Context, 
							(PGPBoolean)!VerbSelection, 
							filter );
			}
			else if( SpecifierSelection == DISABLED )
			{
				error = PGPNewKeyDisabledFilter(	
							g_Context, 
							(PGPBoolean)!VerbSelection, 
							filter );
			}
			break;
		}

		case IDS_ATTRIBUTE_7: // key size
		{
			SpecifierLength = ComboBox_GetTextLength(hwndComboSpecifier) + 1;
			SpecifierBuffer = (char*) malloc(SpecifierLength);

			if( SpecifierBuffer )
			{
				ComboBox_GetText(	hwndComboSpecifier, 
									SpecifierBuffer, 
									SpecifierLength);

				//MessageBox(NULL, SpecifierBuffer, "Key Size", MB_OK);

				switch( VerbSelection )
				{
					case IS:
					{
						error = PGPNewKeyEncryptKeySizeFilter( 
											g_Context, 
											atol(SpecifierBuffer), 
			 								kPGPMatchEqual, 
											filter );
						break;
					}

					case IS_AT_LEAST:
					{
						error = PGPNewKeyEncryptKeySizeFilter( 
											g_Context, 
											atol(SpecifierBuffer), 
			 								kPGPMatchGreaterOrEqual, 
											filter );
						break;
					}

					case IS_AT_MOST:
					{
						error = PGPNewKeyEncryptKeySizeFilter( 
											g_Context, 
											atol(SpecifierBuffer), 
			 								kPGPMatchLessOrEqual, 
											filter );
						break;
					}
				}

				free(SpecifierBuffer);
			}
			else
			{
				error = kPGPError_OutOfMemory;
			}
			break;
		}

		default:
		{
			MessageBox(NULL, "DefaultFilter: Unknown Attribute", 0, MB_OK);
		}
	}

	return error;
}

PGPError KeyIdFilterFromKeySet(	PGPKeySetRef keySet,
								PGPFilterRef* filter)
{
	PGPError		error	= kPGPError_NoErr;
	PGPKeyListRef	list	= kPGPInvalidRef;
	PGPKeyIterRef	iter	= kPGPInvalidRef;
	PGPKeyRef		key		= kPGPInvalidRef;
	PGPFilterRef	aFilter = kPGPInvalidRef;
	PGPFilterRef	idFilter= kPGPInvalidRef;
	PGPKeyID		keyId;

	error = PGPOrderKeySet(	keySet,
							kPGPAnyOrdering,
							&list);

	if( IsntPGPError(error) )
	{
		error = PGPNewKeyIter(	list,
								&iter);

		if(IsntPGPError(error) )
		{
			PGPError iterError = kPGPError_NoErr;

			do
			{
				iterError = PGPKeyIterNext(iter, &key);

				if( IsntPGPError(iterError) )
				{
					error = PGPGetKeyIDFromKey(	key,
												&keyId);

					if( IsntPGPError(error) )
					{
						error = PGPNewSigKeyIDFilter(	g_Context, 
														&keyId, 
														&aFilter );

						if(idFilter == kPGPInvalidRef)
						{
							idFilter = aFilter;
						}
						else
						{
							PGPFilterRef combinedFilter = NULL;

							error = PGPUnionFilters(idFilter,
													aFilter, 
													&combinedFilter);

							idFilter = combinedFilter;
						}
					}
				}

			}while(IsntPGPError(iterError) && IsntPGPError(error));

			PGPFreeKeyIter(iter);
		}

		PGPFreeKeyList(list);
	}


	if( IsntPGPError(error) )
	{
		*filter = idFilter;
	}

	return error;
}
