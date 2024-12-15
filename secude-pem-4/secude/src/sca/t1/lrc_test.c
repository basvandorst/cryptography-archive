char            rpstr[16] = {0x7B, 0x02};

main()
{

	int             i;
	int             bcc, bcclen;

	bcc = 0;
	bcclen = 2;
	for (i = 0; i < bcclen; i++)
		bcc = bcc ^ rpstr[i];


	printf("bcc= %d\n", bcc);
}
