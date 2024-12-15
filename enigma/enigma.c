/*  3 rotor German Enigma simulation  */
/*  Written by Fauzan Mirza */

#include <stdio.h>

/* Rotor wirings */
char rotor[5][26]={
	/* Input "ABCDEFGHIJKLMNOPQRSTUVWXYZ" */
	/* 1: */ "EKMFLGDQVZNTOWYHXUSPAIBRCJ",
	/* 2: */ "AJDKSIRUXBLHWTMCQGZNPYFVOE",
	/* 3: */ "BDFHJLCPRTXVZNYEIWGAKMUSQO",
	/* 4: */ "ESOVPZJAYQUIRHXLNFTGKDCMWB",
	/* 5: */ "VZBRGITYUPSDNHLXAWMJQOFECK" };
char ref[26]="YRUHQSLDPXNGOKMIEBFZCWVJAT";

char notch[5]="QEVJZ";
int flag=0;

/* Encryption parameters follow */

char order[3]={ 3, 1, 2 };
char rings[3]={ 'W','X','T' };
char pos[3]={ 'A','W','E' };
char plug[]="AMTE";

void main()
{
	int i, j;
	int n=0;
	int ch;

	while((ch=getchar())!=EOF)
	{
		ch=toupper(ch);
		if (!isalpha(ch))
			continue;
		
		/* Step up first rotor */
		pos[0]++;
		if (pos[0]>'Z')
			pos[0] -= 26;

		/* Check if second rotor reached notch last time */
		if (flag)
		{
			/* Step up both second and third rotors */
			pos[1]++;
			if (pos[1]>'Z')
				pos[1] -= 26;
			pos[2]++;
			if (pos[2]>'Z')
				pos[2] -= 26;
			flag=0;
		}

		/*  Step up second rotor if first rotor reached notch */
		if (pos[0]==notch[order[0]-1])
		{
			pos[1]++;
			if (pos[1]>'Z')
				pos[1] -= 26;
			/* Set flag if second rotor reached notch */
			if (pos[1]==notch[order[1]-1])
				flag=1;
		}

		/*  Swap pairs of letters on the plugboard */
		for (i=0; plug[i]; i+=2)
		{
			if (ch==plug[i])
				ch=plug[i+1];
			else if (ch==plug[i+1])
				ch=plug[i];
		}

		/*  Rotors (forward) */
		for (i=0; i<3; i++)
		{
			ch += pos[i]-'A';
			if (ch>'Z')
				ch -= 26;

			ch -= rings[i]-'A';
			if (ch<'A')
				ch += 26;

			ch=rotor[order[i]-1][ch-'A'];

			ch += rings[i]-'A';
			if (ch>'Z')
				ch -= 26;

			ch -= pos[i]-'A';
			if (ch<'A')
				ch += 26;
		}

		/*  Reflecting rotor */
		ch=ref[ch-'A'];

		/*  Rotors (reverse) */
		for (i=3; i; i--)
		{
			ch += pos[i-1]-'A';
			if (ch>'Z')
				ch -= 26;

			ch -= rings[i-1]-'A';
			if (ch<'A')
				ch += 26;

			for (j=0; j<26; j++)
				if (rotor[order[i-1]-1][j]==ch)
					break;
			ch=j+'A';

			ch += rings[i-1]-'A';
			if (ch>'Z')
				ch -= 26;

			ch -= pos[i-1]-'A';
			if (ch<'A')
				ch += 26;
		}
		
		/*  Plugboard */
		for (i=0; plug[i]; i+=2)
		{
			if (ch==plug[i])
				ch=plug[i+1];
			else if (ch==plug[i+1])
				ch=plug[i];
		}

		n++;
		putchar(ch);
		if (n%5==0)
			if (n%55==0)
				putchar('\n');
			else
				putchar(' ');
	}
}

