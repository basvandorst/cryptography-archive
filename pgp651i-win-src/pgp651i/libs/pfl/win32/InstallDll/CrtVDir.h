BOOL CreateVirtualRoot(
		LPSTR szName,
		LPSTR szPhysicalPath,
		DWORD dwPermissions,
		DWORD dwSite,
		CHAR szStatus[STATUS_STRING_SIZE],
		BOOL bNewDefaultFile,
		char* NewDefaultFile,
		BOOL bSetCustomErr
		);

BOOL DeleteVirtualRoot(
    LPSTR szName,
    DWORD dwSite,
    CHAR szStatus[STATUS_STRING_SIZE]
    );

BOOL DeleteApplication(
    LPSTR szMetaPath,
    BOOL fRecoverable,
    BOOL fRecursive,
    CHAR szStatus[STATUS_STRING_SIZE]
    );

// Helper functions

LPWSTR MakeUnicode(
    LPSTR szString,
	BOOL bRegularString
    );

BOOL WrAddKey(
    LPSTR szMetaPath
    );

BOOL WrSetData(
    LPSTR szMetaPath,
    DWORD dwIdentifier,
    DWORD dwAttributes,
    DWORD dwUserType,
    DWORD dwDataType,
    DWORD dwDataSize,
    LPVOID pData
    );