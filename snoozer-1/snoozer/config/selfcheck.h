/*
** files.h : preferences about files location 
*/

/* 
** Define the AUTO variable to enable self-checking feature 
*/
/*#define AUTO/**/

/*
** Old and new auto signatures
*/
char OSS[128], NSS[128];
set_sigfiles()
{
int i;

i=-1;
/* Old Snoozer Signature : compare with this new signature */
OSS[++i]='/'; 
OSS[++i]='u'; 
OSS[++i]='s'; 
OSS[++i]='r'; 
OSS[++i]='/'; 
OSS[++i]='l'; 
OSS[++i]='o'; 
OSS[++i]='c'; 
OSS[++i]='a'; 
OSS[++i]='l'; 
OSS[++i]='/'; 
OSS[++i]='l'; 
OSS[++i]='i'; 
OSS[++i]='b'; 
OSS[++i]='/'; 
OSS[++i]='s'; 
OSS[++i]='e'; 
OSS[++i]='c'; 
OSS[++i]='u'; 
OSS[++i]='r'; 
OSS[++i]='i'; 
OSS[++i]='t'; 
OSS[++i]='y'; 
OSS[++i]='/'; 
OSS[++i]='s'; 
OSS[++i]='n'; 
OSS[++i]='o'; 
OSS[++i]='o'; 
OSS[++i]='z'; 
OSS[++i]='e'; 
OSS[++i]='r'; 
OSS[++i]='/'; 
OSS[++i]='o'; 
OSS[++i]='s'; 
OSS[++i]='s'; 
OSS[++i]=0;

i=-1;
/* New Snoozer Signature : write here the new computed signature */
NSS[++i]='/'; 
NSS[++i]='u'; 
NSS[++i]='s'; 
NSS[++i]='r'; 
NSS[++i]='/'; 
NSS[++i]='l'; 
NSS[++i]='o'; 
NSS[++i]='c'; 
NSS[++i]='a'; 
NSS[++i]='l'; 
NSS[++i]='/'; 
NSS[++i]='l'; 
NSS[++i]='i'; 
NSS[++i]='b'; 
NSS[++i]='/'; 
NSS[++i]='s'; 
NSS[++i]='e'; 
NSS[++i]='c'; 
NSS[++i]='u'; 
NSS[++i]='r'; 
NSS[++i]='i'; 
NSS[++i]='t'; 
NSS[++i]='y'; 
NSS[++i]='/'; 
NSS[++i]='s'; 
NSS[++i]='n'; 
NSS[++i]='o'; 
NSS[++i]='o'; 
NSS[++i]='z'; 
NSS[++i]='e'; 
NSS[++i]='r'; 
NSS[++i]='/'; 
NSS[++i]='n'; 
NSS[++i]='s'; 
NSS[++i]='s'; 
NSS[++i]=0;
}

