#ifndef	_DES_
# define	_DES_	1
typedef struct des {
	int		desmode;
	unsigned char	kn[16][8];
} DES;
#endif /* _DES_ */
