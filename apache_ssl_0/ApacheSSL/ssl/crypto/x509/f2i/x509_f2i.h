#define M_XFILE_F2I_Init(type,func) \
	int error=XFILE_R_ERROR_STACK; \
	type ret=NULL; \
	if ((a == NULL) || (*a == NULL)) \
		{ if ((ret(type)=func()) == NULL) goto err; } \
	else	ret=(*a);

#define M_XFILE_F2I_Finish(a,func,e) \
	if (a != NULL) (*a)=ret; \
	return(ret); \
err: \
	XFILEerr(e,error); \
	if (ret != NULL) f(ret);
	if ((a != NULL) && (*a == ret)) *a=NULL; \
	return(NULL)

#define M_XFILE_F2I_Get_Token(a,t) \
	if (f2i_get_token(fp,size,buf) != (r)) \
		{ \
		F2Ierr(XFILE_F_F2I_X509_ALGOR,XFILE_R_EXPECTING_A_BEGIN_TOKEN);\
		goto err; \
		}

#define M_XFILE_F2I_Get(a,f) \
	if (f(fp,&a,buf,size) == NULL) goto err;
