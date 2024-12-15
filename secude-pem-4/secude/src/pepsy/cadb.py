CADB      

DEFINITIONS ::=

%{      /* surrounding global definitions */
#include "cadb.h"

	static OctetString * tmp_ostr, * save_serialnumber;
	static int i,j;
%}

PREFIXES build parse print

BEGIN


IMPORTS
	OURINTEGER
                FROM SEC;


IssuedCertificate [[P IssuedCertificate *]] 
	::=
	%E{
	tmp_ostr = NULLOCTETSTRING;

	if(parm->serial && parm->serial->noctets > 0 && (parm->serial->octets[0] & MSBITINBYTE)){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr = (OctetString * )malloc(sizeof(OctetString));
		tmp_ostr->noctets = parm->serial->noctets + 1;
		tmp_ostr->octets = (char * ) calloc( 1 , tmp_ostr->noctets );
		tmp_ostr->octets[0] = 0x00;
		for ( i = 0, j = 1; i < parm->serial->noctets; i++, j++)
			tmp_ostr->octets[j] = parm->serial->octets[i];
		save_serialnumber = parm->serial;
		parm->serial = tmp_ostr;
	}
	%}
        SEQUENCE
        {
        serial
        	SEC.OURINTEGER [[p serial]],

        issuedate
                UTCTime [[s date_of_issue]]
        }
	%E{
	if(tmp_ostr) {
		aux_free_OctetString(&tmp_ostr);
		parm->serial = save_serialnumber;
	}
	%}
	%D{
	if((*parm)->serial->noctets > 1 && !((*parm)->serial->octets[0] | 0x00)){    

		/* The bits of the most significant byte are all 0, so delete them */

		for ( i = 0; i < (*parm)->serial->noctets - 1; i++)
			(*parm)->serial->octets[i] = (*parm)->serial->octets[i + 1];
		(*parm)->serial->noctets -= 1;
		(*parm)->serial->octets = (char *)realloc((*parm)->serial->octets, (*parm)->serial->noctets);
	}
	%}


IssuedCertificateSet [[P SET_OF_IssuedCertificate *]] 
	::=
        SET OF [[T SET_OF_IssuedCertificate * $ *]] <<next>>
            IssuedCertificate [[p element]]


END
