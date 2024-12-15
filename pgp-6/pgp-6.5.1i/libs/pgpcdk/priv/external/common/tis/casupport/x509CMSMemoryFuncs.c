#include "x509CMS.h"

#include "pgpMemoryMgr.h"

PKIMemoryMgr X509CMSMemoryMgr = {
    NULL,
    x509CMSAllocProc,
    x509CMSReallocProc,
    x509CMSDeallocProc
};

void *
x509CMSAllocProc (
        PKIMemoryMgr    *mem,
        size_t          requestSize
)
{
    return (PGPNewData ((PGPMemoryMgrRef) mem->customValue, requestSize, 0)); 
}

int
x509CMSDeallocProc (
        PKIMemoryMgr    *mem,
        void            *allocation
)
{
	(void) mem;
	
    PGPFreeData (allocation);
    return 0;
}

int
x509CMSReallocProc (
	PKIMemoryMgr	*mem,
	void		**allocation,
	size_t		requestSize)
{
    PGPReallocData ((PGPMemoryMgrRef) mem->customValue, allocation, requestSize, 0);
    return 0;
}
