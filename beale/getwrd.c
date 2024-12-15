/*
 *      getwrd --- get next word from standard input
 */
getwrd(p)
char *p;
{
	register char *rp = p;
	char c;

	while( (c=getchar()) != EOF )
		if(isalpha(c))
			break;
	if( c==EOF)
		return(EOF);
	*rp++ = c;
	while( (c=getchar()) != EOF ){
		if(!isalpha(c))
			break;
		*rp++ = c;
		}
	*rp = '\0';
	return(!EOF);
}
