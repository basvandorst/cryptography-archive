
/*
 *      loadkey --- read STDIN using 1st letter of each word as key
 */
loadkey()
{
	register int i;
	char    buf[50];

	/* collect key characters */
	for(i=1; (i<MAXKEY) && (getwrd(buf)!=EOF) ; i++)
		key[i] = buf[0];
	for(   ;i<MAXKEY;i++)
		key[i] = '_';   /* unspecified key positions */
}
