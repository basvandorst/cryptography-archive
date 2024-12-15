/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: StPGPRefs.h,v 1.13.6.3 1999/06/04 19:00:52 heller Exp $
____________________________________________________________________________*/

#ifndef Included_StPGPRefs_h	/* [ */
#define Included_StPGPRefs_h

template <class T>

class StPGPRef {
public:
								StPGPRef()
									{ Set((T) NULL); }
								StPGPRef(const T & inRef)
									{ Set(inRef); }
#if PGP_COMPILER_SUN
						~StPGPRef()
#else
	virtual						~StPGPRef()
#endif
									{ Free(); }
					
	void						Set(T inRef)
									{ mRef = inRef; }
	T							Get()
									{ return mRef; }
	void						Free()
									{ if (mRef != (T) NULL) Dispose(); mRef = (T) NULL; }
						
								operator T() { return mRef; }
	T*							operator &() { return &mRef; }
					
	StPGPRef<T>&				operator=(const T& rhs) { mRef = rhs; return *this; }
	bool						operator==(const T& rhs) { return mRef == rhs; }
	bool						operator!=(const T& rhs) { return mRef != rhs; }

protected:
	T							mRef;
	
	inline void					Dispose();
};


#ifdef Included_pgpKeys_h
	typedef StPGPRef<PGPKeySetRef>	StPGPKeySetRef;
	inline void StPGPKeySetRef::Dispose() { PGPFreeKeySet(mRef); }

	typedef StPGPRef<PGPKeyListRef>	StPGPKeyListRef;
	inline void StPGPKeyListRef::Dispose() { PGPFreeKeyList(mRef); }

	typedef StPGPRef<PGPKeyIterRef>	StPGPKeyIterRef;
	inline void StPGPKeyIterRef::Dispose() { PGPFreeKeyIter(mRef); }
#endif

#ifdef Included_pgpHashing_h
	typedef StPGPRef<PGPHashContextRef>	StPGPHashContextRef;
	inline void StPGPHashContextRef::Dispose() { PGPFreeHashContext(mRef); }
#endif

#ifdef Included_PGPtls_h
	typedef StPGPRef<PGPtlsSessionRef>	StPGPtlsSessionRef;
	inline void StPGPtlsSessionRef::Dispose() { PGPFreeTLSSession(mRef); }
#endif

#ifdef Included_pgpMemoryMgr_h
	inline void 	StPGPRef<PGPByte *>::Dispose() { PGPFreeData(mRef); }

	class StPGPDataRef	:	public StPGPRef<PGPByte *> {
	public:
						StPGPDataRef() { }
						StPGPDataRef(PGPByte * inRef)
							{ Set(inRef); }

						operator char*() { return reinterpret_cast<char *>(mRef); }
		StPGPDataRef&	operator=(PGPByte * rhs) { mRef = rhs; return *this; }
#if PGP_COMPILER_SUN
		bool operator==(const int rhs) { return static_cast<int>(static_cast<void *>(mRef)) == rhs; }
#elif PGP_COMPILER_HPUX
		/* HACK ALERT: I'm not sure if this cast is OK, but it seems
		   to make aCC happy.  -elkins */
		bool operator==(const int rhs) { return (int) mRef == rhs; }
#endif
						operator void*() { return mRef; }
	};

	typedef StPGPRef<PGPMemoryMgrRef>	StPGPMemoryMgrRef;
	inline void StPGPMemoryMgrRef::Dispose() { PGPFreeMemoryMgr(mRef); }
#endif

#ifdef Included_pgpSockets_h
	typedef StPGPRef<PGPSocketRef>	StPGPSocketRef;
	inline void StPGPSocketRef::Dispose() { PGPCloseSocket(mRef); }
#endif

#ifdef Included_pflPrefs_h
	typedef StPGPRef<PGPPrefRef>	StPGPPrefRef;
	inline void StPGPPrefRef::Dispose() { PGPClosePrefFile(mRef); }
#endif

#ifdef Included_pgpIO_h
	typedef StPGPRef<PGPIORef>	StPGPIORef;
	inline void StPGPIORef::Dispose() { PGPFreeIO(mRef); }
#endif


#endif
