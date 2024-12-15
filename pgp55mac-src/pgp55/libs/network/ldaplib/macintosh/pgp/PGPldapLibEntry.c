/*
** Copyright (C) 1997 Pretty Good Privacy, Inc.
** All rights reserved.
**
** This file contains all of the exported entrypoints for this library. These entrypoints
** are simple wrappers around the UMich code which perform parameter validation and any
** other required pre and post processing. The C preprocessor is used to redefine the
** names of UMich entrypoints so that no name conflicts occur.
*/

#include "pgpMem.h"

#include "proto-ldap.h"						// VERY IMPORTANT: Do not change the #include
#include "PGPldapLibEntryUnmappings.h"		// order of these three files. The two includes 
#include "proto-ldap.h"						// of proto-ldap.h define two different sets of calls

#include "tcp.h"

#if DEBUG
	#define AssertLDAPIsValid(ld)	pgpAssertAddrValid( ld, LDAP );
#else
	#define AssertLDAPIsValid(ld)
#endif

	int
ldap_abandon(
	LDAP 	*ld,
	int 	msgid)
{
	AssertLDAPIsValid( ld );
	
	return( internal_ldap_abandon( ld, msgid ) );
}

	int
ldap_add(
	LDAP 	*ld,
	char 	*dn,
	LDAPMod **attrs)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	pgpAssertAddrValid( attrs, LDAPMod * );

	return( internal_ldap_add( ld, dn, attrs ) );
}

	int
ldap_add_s(
	LDAP 	*ld,
	char 	*dn,
	LDAPMod **attrs)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	pgpAssertAddrValid( attrs, LDAPMod * );

	return( internal_ldap_add_s( ld, dn, attrs ) );
}

	void
ldap_add_result_entry(
	LDAPMessage **list,
	LDAPMessage *e)
{
	pgpAssertAddrValid( list, LDAPMessage * );
	pgpAssertAddrValid( e, LDAPMessage );
	
	internal_ldap_add_result_entry( list, e );
}

	int
ldap_bind(
	LDAP 	*ld,
	char 	*dn,
	char 	*passwd,
	int 	authmethod)
{
	AssertLDAPIsValid( ld );
	AssertAddrNullOrValid( dn, char, "ldap_bind" );
	AssertAddrNullOrValid( passwd, char, "ldap_bind" );
	
	return( internal_ldap_bind( ld, dn, passwd, authmethod ) );
}

	int
ldap_bind_s(
	LDAP 	*ld,
	char 	*dn,
	char 	*passwd,
	int 	authmethod)
{
	AssertLDAPIsValid( ld );
	AssertAddrNullOrValid( dn, char, "ldap_bind_s" );
	AssertAddrNullOrValid( passwd, char, "ldap_bind_s" );
	
	return( internal_ldap_bind_s( ld, dn, passwd, authmethod ) );
}

	void
ldap_build_filter(
	char 			*filtbuf,
	unsigned long 	buflen,
	char 			*pattern,
	char 			*prefix,
	char 			*suffix,
	char 			*attr,
	char 			*value,
	char 			**valwords)
{
	pgpAssertAddrValid( filtbuf, char );
	pgpAssertAddrValid( pattern, char );
	AssertAddrNullOrValid( prefix, char, "ldap_build_filter" );
	AssertAddrNullOrValid( suffix, char, "ldap_build_filter" );
	AssertAddrNullOrValid( attr, char, "ldap_build_filter" );
	AssertAddrNullOrValid( value, char, "ldap_build_filter" );
	AssertAddrNullOrValid( valwords, char *, "ldap_build_filter" );
	
	internal_ldap_build_filter( filtbuf, buflen, pattern, prefix, suffix,
				attr, value, valwords );
}

	int
ldap_compare(
	LDAP *ld,
	char *dn,
	char *attr,
	char *value)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	pgpAssertAddrValid( attr, char );
	pgpAssertAddrValid( value, char );
	
	return( internal_ldap_compare( ld, dn, attr, value ) );
}

	int
ldap_compare_s(
	LDAP *ld,
	char *dn,
	char *attr,
	char *value)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	pgpAssertAddrValid( attr, char );
	pgpAssertAddrValid( value, char );
	
	return( internal_ldap_compare_s( ld, dn, attr, value ) );
}

	int
ldap_count_entries(
	LDAP 		*ld,
	LDAPMessage *chain)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( chain, LDAPMessage );

	return( internal_ldap_count_entries( ld, chain ) );
}

	int
ldap_count_values(char **vals)
{
	AssertAddrNullOrValid( vals, char *, "ldap_count_values" );
	
	return( internal_ldap_count_values( vals ) );
}

	int
ldap_count_values_len(struct berval **vals)
{
	AssertAddrNullOrValid( vals, struct berval *, "ldap_count_values_len" );
	
	return( internal_ldap_count_values_len( vals ) );
}

	int
ldap_delete(
	LDAP *ld,
	char *dn)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	
	return( internal_ldap_delete( ld, dn ) );
}

	int
ldap_delete_s(
	LDAP *ld,
	char *dn)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	
	return( internal_ldap_delete_s( ld, dn ) );
}

	LDAPMessage *
ldap_delete_result_entry(
	LDAPMessage **list,
	LDAPMessage *e)
{
	pgpAssertAddrValid( list, LDAPMessage * );
	pgpAssertAddrValid( e, LDAPMessage );
	
	return( internal_ldap_delete_result_entry( list, e ) );
}

	char *
ldap_dn2ufn(char *dn)
{
	pgpAssertAddrValid( dn, char );

	return( internal_ldap_dn2ufn( dn ) );
}

	char *
ldap_err2string(int err)
{
	return( internal_ldap_err2string( err ) );
}

	char **
ldap_explode_dn(
	char 	*dn,
	int 	notypes)
{
	pgpAssertAddrValid( dn, char );

	return( internal_ldap_explode_dn( dn, notypes ) );
}

	char **
ldap_explode_dns(char *dn)
{
	pgpAssertAddrValid( dn, char );

	return( internal_ldap_explode_dns( dn ) );
}

	char *
ldap_first_attribute(
	LDAP 		*ld,
	LDAPMessage *entry,
	BerElement 	**ber)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( entry, LDAPMessage );
	pgpAssertAddrValid( ber, BerElement * );
	
	return( internal_ldap_first_attribute( ld, entry, ber ) );
}

	LDAPMessage *
ldap_first_entry(
	LDAP 		*ld,
	LDAPMessage *chain)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( chain, LDAPMessage );
	
	return( internal_ldap_first_entry( ld, chain ) );
}

	void
ldap_free_friendlymap(FriendlyMap **map)
{
	AssertAddrNullOrValid( map, FriendlyMap *, "ldap_free_friendlymap" );
	
	internal_ldap_free_friendlymap( map );
}

	void
ldap_free_urldesc(LDAPURLDesc *ludp)
{
	AssertAddrNullOrValid( ludp, LDAPURLDesc, "ldap_free_urldesc" );
	
	internal_ldap_free_urldesc( ludp );
}

	char *
ldap_friendly_name(
	char 		*filename,
	char 		*uname,
	FriendlyMap **map)
{
	pgpAssertAddrValid( filename, char );
	pgpAssertAddrValid( uname, char );
	pgpAssertAddrValid( map, FriendlyMap * );
	
	return( internal_ldap_friendly_name( filename, uname, map ) );
}

	void
ldap_getfilter_free(LDAPFiltDesc *lfdp)
{
	pgpAssertAddrValid( lfdp, LDAPFiltDesc );
	
	internal_ldap_getfilter_free( lfdp );
}
	
	LDAPFiltInfo *
ldap_getfirstfilter(
	LDAPFiltDesc 	*lfdp,
	char 			*tagpat,
	char 			*value)
{
	pgpAssertAddrValid( lfdp, LDAPFiltDesc );
	pgpAssertAddrValid( tagpat, char );
	pgpAssertAddrValid( value, char );
	
	return( internal_ldap_getfirstfilter( lfdp, tagpat, value ) );
}

	LDAPFiltInfo *
ldap_getnextfilter(LDAPFiltDesc *lfdp)
{
	pgpAssertAddrValid( lfdp, LDAPFiltDesc );

	return( internal_ldap_getnextfilter( lfdp ) );
}

	char *
ldap_get_dn(
	LDAP 		*ld,
	LDAPMessage *entry)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( entry, LDAPMessage );

	return( internal_ldap_get_dn( ld, entry ) );
}

	char **
ldap_get_values(
	LDAP 		*ld,
	LDAPMessage *entry,
	char 		*target)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( entry, LDAPMessage );
	pgpAssertAddrValid( target, char );
	
	return( internal_ldap_get_values( ld, entry, target ) );
}

	struct berval **
ldap_get_values_len(
	LDAP 		*ld,
	LDAPMessage *entry,
	char 		*target)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( entry, LDAPMessage );
	pgpAssertAddrValid( target, char );
	
	return( internal_ldap_get_values_len( ld, entry, target ) );
}

	LDAP *
ldap_init(
	char	*defhost,
	int 	defport)
{
	pgpAssertAddrValid( defhost, char );

	return( internal_ldap_init( defhost, defport ) );
}

	LDAPFiltDesc *
ldap_init_getfilter(char *fname)
{
	pgpAssertAddrValid( fname, char );

	return( internal_ldap_init_getfilter( fname ) );
}

	LDAPFiltDesc *
ldap_init_getfilter_buf(
	char *buf,
	long buflen)
{
	pgpAssertAddrValid( buf, char );

	return( internal_ldap_init_getfilter_buf( buf, buflen ) );
}

	int
ldap_is_dns_dn(char *dn)
{
	pgpAssertAddrValid( dn, char );

	return( internal_ldap_is_dns_dn( dn ) );
}

	int
ldap_is_ldap_url(char *url)
{
	pgpAssertAddrValid( url, char );

	return( internal_ldap_is_ldap_url( url ) );
}

	int
ldap_modify(
	LDAP 	*ld,
	char	*dn,
	LDAPMod **mods)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	pgpAssertAddrValid( mods, LDAPMod * );

	return( internal_ldap_modify( ld, dn, mods ) );
}

	int
ldap_modify_s(
	LDAP 	*ld,
	char 	*dn,
	LDAPMod **mods)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	pgpAssertAddrValid( mods, LDAPMod * );

	return( internal_ldap_modify_s( ld, dn, mods ) );
}

	int
ldap_modrdn(
	LDAP *ld,
	char *dn,
	char *newrdn)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	pgpAssertAddrValid( newrdn, char );

	return( internal_ldap_modrdn( ld, dn, newrdn ) );
}

	int
ldap_modrdn_s(
	LDAP *ld,
	char *dn,
	char *newrdn)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	pgpAssertAddrValid( newrdn, char );

	return( internal_ldap_modrdn_s( ld, dn, newrdn ) );
}

	int
ldap_modrdn2(
	LDAP 	*ld,
	char 	*dn,
	char 	*newrdn,
	int 	deleteoldrdn)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	pgpAssertAddrValid( newrdn, char );

	return( internal_ldap_modrdn2( ld, dn, newrdn, deleteoldrdn ) );
}

	int
ldap_modrdn2_s(
	LDAP 	*ld,
	char 	*dn,
	char 	*newrdn,
	int 	deleteoldrdn)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( dn, char );
	pgpAssertAddrValid( newrdn, char );

	return( internal_ldap_modrdn2_s( ld, dn, newrdn, deleteoldrdn ) );
}

	void
ldap_mods_free(
	LDAPMod **mods,
	int 	freemods)
{
	pgpAssertAddrValid( mods, LDAPMod * );

	internal_ldap_mods_free( mods, freemods );
}

	int
ldap_msgdelete(
	LDAP 	*ld,
	int 	msgid)
{
	AssertLDAPIsValid( ld );

	return( internal_ldap_msgdelete( ld, msgid ) );
}

	int
ldap_msgfree(LDAPMessage *lm)
{
	pgpAssertAddrValid( lm, LDAPMessage * );

	return( internal_ldap_msgfree( lm ) );
}

	char *
ldap_next_attribute(
	LDAP 		*ld,
	LDAPMessage *entry,
	BerElement 	*ber)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( entry, LDAPMessage * );
	pgpAssertAddrValid( ber, BerElement * );

	return( internal_ldap_next_attribute( ld, entry, ber ) );
}

	LDAPMessage *
ldap_next_entry(
	LDAP 		*ld,
	LDAPMessage *entry)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( entry, LDAPMessage * );

	return( internal_ldap_next_entry( ld, entry ) );
}

	LDAP *
ldap_open(
	char 	*host,
	int 	port)
{
	pgpAssertAddrValid( host, char );
	
	return( internal_ldap_open( host, port ) );
}

	int
ldap_result(
	LDAP			*ld,
	int 			msgid,
	int 			all,
	struct timeval 	*timeout,
	LDAPMessage 	**result)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( timeout, struct timeval );
	pgpAssertAddrValid( result, LDAPMessage * );

	return( internal_ldap_result( ld, msgid, all, timeout, result ) );
}

	int
ldap_result2error(
	LDAP 		*ld,
	LDAPMessage *r,
	int 		freeit)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( r, LDAPMessage * );

	return( internal_ldap_result2error( ld, r, freeit ) );
}

	int
ldap_search(
	LDAP 	*ld,
	char 	*base,
	int 	scope,
	char 	*filter,
	char 	**attrs,
	int 	attrsonly)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( base, char );
	pgpAssertAddrValid( filter, char );
	AssertAddrNullOrValid( attrs, char, "ldap_search" );
	
	return( internal_ldap_search( ld, base, scope, filter, attrs, attrsonly ) );
}

	int
ldap_search_s(
	LDAP 		*ld,
	char 		*base,
	int 		scope,
	char 		*filter,
	char 		**attrs,
	int 		attrsonly,
	LDAPMessage **res)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( base, char );
	pgpAssertAddrValid( filter, char );
	AssertAddrNullOrValid( attrs, char, "ldap_search" );
	pgpAssertAddrValid( res, LDAPMessage * );
	
	return( internal_ldap_search_s( ld, base, scope, filter, attrs, attrsonly, res ) );
}

	int
ldap_search_st(
	LDAP 			*ld,
	char 			*base,
	int 			scope,
	char 			*filter,
	char 			**attrs,
	int 			attrsonly,
	struct timeval 	*timeout,
	LDAPMessage 	**res)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( base, char );
	pgpAssertAddrValid( filter, char );
	pgpAssertAddrValid( attrs, char * );
	pgpAssertAddrValid( timeout, struct timeval * );
	pgpAssertAddrValid( res, LDAPMessage * );
	
	return( internal_ldap_search_st( ld, base, scope, filter, attrs, attrsonly,
				timeout, res ) );
}

	void
ldap_setfilteraffixes(
	LDAPFiltDesc 	*lfdp,
	char 			*prefix,
	char 			*suffix)
{
	pgpAssertAddrValid( lfdp, LDAPFiltDesc );
	AssertAddrNullOrValid( prefix, char, "ldap_setfilteraffixes" );
	AssertAddrNullOrValid( suffix, char, "ldap_setfilteraffixes" );

	internal_ldap_setfilteraffixes( lfdp, prefix, suffix );
}


#ifdef LDAP_REFERRALS	/* [ */
	void
ldap_set_rebind_proc(
	LDAP 	*ld,
	int 	(*rebindproc)( LDAP *ld, char **dnp, char **passwdp, int *authmethodp, int freeit))
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( rebindproc, VoidAlign );

	internal_ldap_set_rebind_proc( ld, rebindproc );
}
#endif	*/ ] */

	int
ldap_simple_bind(
	LDAP *ld,
	char *dn,
	char *passwd)
{
	AssertLDAPIsValid( ld );
	AssertAddrNullOrValid( dn, char, "ldap_simple_bind" );
	AssertAddrNullOrValid( passwd, char, "ldap_simple_bind" );
	
	return( internal_ldap_simple_bind( ld, dn, passwd ) );
}

	int
ldap_simple_bind_s(
	LDAP *ld,
	char *dn,
	char *passwd)
{
	AssertLDAPIsValid( ld );
	AssertAddrNullOrValid( dn, char, "ldap_simple_bind_s" );
	AssertAddrNullOrValid( passwd, char, "ldap_simple_bind_s" );
	
	return( internal_ldap_simple_bind_s( ld, dn, passwd ) );
}

	int
ldap_sort_entries(
	LDAP 		*ld,
	LDAPMessage **chain,
	char 		*attr,
	int 		(*cmp)())
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( chain, LDAPMessage * );
	AssertAddrNullOrValid( attr, char, "ldap_sort_entries" );
	pgpAssertAddrValid( cmp, VoidAlign );

	return( internal_ldap_sort_entries( ld, chain, attr, cmp ) );
}

	int
ldap_sort_strcasecmp(
	char **a,
	char **b)
{
	pgpAssertAddrValid( a, char * );
	pgpAssertAddrValid( b, char * );
	
	return( internal_ldap_sort_strcasecmp( a, b ) );
}

	int
ldap_sort_values(
	LDAP 	*ld,
	char 	**vals,
	int 	(*cmp)())
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( vals, char * );
	pgpAssertAddrValid( cmp, VoidAlign );

	return( internal_ldap_sort_values( ld, vals, cmp ) );
}

	int
ldap_ufn_search_c(
	LDAP 		*ld,
	char 		*ufn,
	char 		**attrs,
	int 		attrsonly,
	LDAPMessage **res,
	int 		(*cancelproc)( void *cl ),
	void 		*cancelparm)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( ufn, char );
	pgpAssertAddrValid( attrs, char * );
	pgpAssertAddrValid( res, LDAPMessage * );
	AssertAddrNullOrValid( cancelproc, VoidAlign, "ldap_ufn_search_c" );
	AssertAddrNullOrValid( cancelparm, VoidAlign, "ldap_ufn_search_c" );

	return( internal_ldap_ufn_search_c( ld, ufn, attrs, attrsonly,
				res, cancelproc, cancelparm ) );
}

	int
ldap_ufn_search_ct(
	LDAP 		*ld,
	char 		*ufn,
	char 		**attrs,
	int 		attrsonly,
	LDAPMessage **res,
	int 		(*cancelproc)( void *cl ),
	void 		*cancelparm,
	char 		*tag1,
	char 		*tag2,
	char 		*tag3)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( ufn, char );
	pgpAssertAddrValid( attrs, char * );
	pgpAssertAddrValid( res, LDAPMessage * );
	AssertAddrNullOrValid( cancelproc, VoidAlign, "ldap_ufn_search_ct" );
	AssertAddrNullOrValid( cancelparm, VoidAlign, "ldap_ufn_search_ct" );
	AssertAddrNullOrValid( tag1, char, "ldap_ufn_search_ct" );
	AssertAddrNullOrValid( tag2, char, "ldap_ufn_search_ct" );
	AssertAddrNullOrValid( tag3, char, "ldap_ufn_search_ct" );

	return( internal_ldap_ufn_search_ct( ld, ufn, attrs, attrsonly,
				res, cancelproc, cancelparm, tag1, tag2, tag3 ) );
}

	int
ldap_ufn_search_s(
	LDAP 		*ld,
	char 		*ufn,
	char 		**attrs,
	int 		attrsonly,
	LDAPMessage **res)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( ufn, char );
	pgpAssertAddrValid( attrs, char * );
	pgpAssertAddrValid( res, LDAPMessage * );

	return( internal_ldap_ufn_search_s( ld, ufn, attrs, attrsonly, res ) );
}

	LDAPFiltDesc *
ldap_ufn_setfilter(
	LDAP *ld,
	char *fname)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( fname, char );

	return( internal_ldap_ufn_setfilter( ld, fname ) );
}

	void
ldap_ufn_setprefix(
	LDAP *ld,
	char *prefix)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( prefix, char );
	
	internal_ldap_ufn_setprefix( ld, prefix );
}

	int
ldap_ufn_timeout(void *tvparam)
{
	pgpAssertAddrValid( tvparam, VoidAlign );
	
	return( internal_ldap_ufn_timeout( tvparam ) );
}

	int
ldap_unbind(LDAP *ld)
{
	AssertLDAPIsValid( ld );

	return( internal_ldap_unbind( ld ) );
}

	int
ldap_unbind_s(LDAP *ld)
{
	AssertLDAPIsValid( ld );

	return( internal_ldap_unbind_s( ld ) );
}

	int
ldap_url_parse(
	char 		*url,
	LDAPURLDesc **ludpp)
{
	pgpAssertAddrValid( url, char );
	pgpAssertAddrValid( ludpp, LDAPURLDesc * );
	
	return( internal_ldap_url_parse( url, ludpp ) );
}

	int
ldap_url_search(
	LDAP 	*ld,
	char 	*url,
	int 	attrsonly)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( url, char );
	
	return( internal_ldap_url_search( ld, url, attrsonly ) );
}

	int
ldap_url_search_s(
	LDAP 		*ld,
	char 		*url,
	int 		attrsonly,
	LDAPMessage **res)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( url, char );
	pgpAssertAddrValid( res, LDAPMessage * );
	
	return( internal_ldap_url_search_s( ld, url, attrsonly, res ) );
}

	int
ldap_url_search_st(
	LDAP 			*ld,
	char 			*url,
	int 			attrsonly,
	struct timeval 	*timeout,
	LDAPMessage 	**res)
{
	AssertLDAPIsValid( ld );
	pgpAssertAddrValid( url, char );
	pgpAssertAddrValid( timeout, struct timeval );
	pgpAssertAddrValid( res, LDAPMessage * );
	
	return( internal_ldap_url_search_st( ld, url, attrsonly, timeout, res ) );
}

	void
ldap_value_free(char **vals)
{
	pgpAssertAddrValid( vals, char * );
	
	internal_ldap_value_free( vals );
}

	void
ldap_value_free_len(struct berval **vals)
{
	pgpAssertAddrValid( vals, struct berval * );

	internal_ldap_value_free_len( vals );
}

	void
pgp_ldap_free(void *ptr)
{
	pgpFree( ptr );
}




