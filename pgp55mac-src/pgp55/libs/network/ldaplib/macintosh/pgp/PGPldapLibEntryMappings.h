/*
** Copyright (C) 1997 Pretty Good Privacy, Inc.
** All rights reserved.
**
** Note: This file contains ALL of the LDAP entrypoints, not just the ones
** implemented in the PGP library. This is done so that any items added to
** the export list will generate link errors until a wrapper entrypoint is
** added to PGPldapLibEntry.c
*/

#pragma once

#define cldap_close						internal_cldap_close
#define cldap_open						internal_cldap_open
#define cldap_search_s					internal_cldap_search_s
#define cldap_setretryinfo				internal_cldap_setretryinfo
#define ldap_8859_to_t61				internal_ldap_8859_to_t61
#define ldap_abandon					internal_ldap_abandon
#define ldap_add						internal_ldap_add
#define ldap_add_result_entry			internal_ldap_add_result_entry
#define ldap_add_s						internal_ldap_add_s
#define ldap_bind						internal_ldap_bind
#define ldap_bind_s						internal_ldap_bind_s
#define ldap_build_filter				internal_ldap_build_filter
#define ldap_compare					internal_ldap_compare
#define ldap_compare_s					internal_ldap_compare_s
#define ldap_count_entries				internal_ldap_count_entries
#define ldap_count_values				internal_ldap_count_values
#define ldap_count_values_len			internal_ldap_count_values_len
#define ldap_delete						internal_ldap_delete
#define ldap_delete_result_entry		internal_ldap_delete_result_entry
#define ldap_delete_s					internal_ldap_delete_s
#define ldap_destroy_cache				internal_ldap_destroy_cache
#define ldap_disable_cache				internal_ldap_disable_cache
#define ldap_dn2ufn						internal_ldap_dn2ufn
#define ldap_enable_cache				internal_ldap_enable_cache
#define ldap_enable_translation			internal_ldap_enable_translation
#define ldap_err2string					internal_ldap_err2string
#define ldap_explode_dn					internal_ldap_explode_dn
#define ldap_explode_dns				internal_ldap_explode_dns
#define ldap_first_attribute			internal_ldap_first_attribute
#define ldap_first_entry				internal_ldap_first_entry
#define ldap_flush_cache				internal_ldap_flush_cache
#define ldap_free_friendlymap			internal_ldap_free_friendlymap
#define ldap_free_urldesc				internal_ldap_free_urldesc
#define ldap_friendly_name				internal_ldap_friendly_name
#define ldap_getfilter_free				internal_ldap_getfilter_free
#define ldap_getfirstfilter				internal_ldap_getfirstfilter
#define ldap_getnextfilter				internal_ldap_getnextfilter
#define ldap_get_dn						internal_ldap_get_dn
#define ldap_get_values					internal_ldap_get_values
#define ldap_get_values_len				internal_ldap_get_values_len
#define ldap_init						internal_ldap_init
#define ldap_init_getfilter				internal_ldap_init_getfilter
#define ldap_init_getfilter_buf			internal_ldap_init_getfilter_buf
#define ldap_is_dns_dn					internal_ldap_is_dns_dn
#define ldap_is_ldap_url				internal_ldap_is_ldap_url
#define ldap_kerberos_bind1				internal_ldap_kerberos_bind1
#define ldap_kerberos_bind1_s			internal_ldap_kerberos_bind1_s
#define ldap_kerberos_bind2				internal_ldap_kerberos_bind2
#define ldap_kerberos_bind2_s			internal_ldap_kerberos_bind2_s
#define ldap_kerberos_bind_s			internal_ldap_kerberos_bind_s
#define ldap_memfree					internal_ldap_memfree
#define ldap_modify						internal_ldap_modify
#define ldap_modify_s					internal_ldap_modify_s
#define ldap_modrdn						internal_ldap_modrdn
#define ldap_modrdn2					internal_ldap_modrdn2
#define ldap_modrdn2_s					internal_ldap_modrdn2_s
#define ldap_modrdn_s					internal_ldap_modrdn_s
#define ldap_mods_free					internal_ldap_mods_free
#define ldap_msgdelete					internal_ldap_msgdelete
#define ldap_msgfree					internal_ldap_msgfree
#define ldap_next_attribute				internal_ldap_next_attribute
#define ldap_next_entry					internal_ldap_next_entry
#define ldap_open						internal_ldap_open
#define ldap_perror						internal_ldap_perror
#define ldap_result						internal_ldap_result
#define ldap_result2error				internal_ldap_result2error
#define ldap_search						internal_ldap_search
#define ldap_search_s					internal_ldap_search_s
#define ldap_search_st					internal_ldap_search_st
#define ldap_setfilteraffixes			internal_ldap_setfilteraffixes
#define ldap_set_cache_options			internal_ldap_set_cache_options
#define ldap_set_rebind_proc			internal_ldap_set_rebind_proc
#define ldap_set_string_translators		internal_ldap_set_string_translators
#define ldap_simple_bind				internal_ldap_simple_bind
#define ldap_simple_bind_s				internal_ldap_simple_bind_s
#define ldap_sort_entries				internal_ldap_sort_entries
#define ldap_sort_strcasecmp			internal_ldap_sort_strcasecmp
#define ldap_sort_values				internal_ldap_sort_values
#define ldap_t61_to_8859				internal_ldap_t61_to_8859
#define ldap_translate_from_t61			internal_ldap_translate_from_t61
#define ldap_translate_to_t61			internal_ldap_translate_to_t61
#define ldap_ufn_search_c				internal_ldap_ufn_search_c
#define ldap_ufn_search_ct				internal_ldap_ufn_search_ct
#define ldap_ufn_search_s				internal_ldap_ufn_search_s
#define ldap_ufn_setfilter				internal_ldap_ufn_setfilter
#define ldap_ufn_setprefix				internal_ldap_ufn_setprefix
#define ldap_ufn_timeout				internal_ldap_ufn_timeout
#define ldap_unbind						internal_ldap_unbind
#define ldap_unbind_s					internal_ldap_unbind_s
#define ldap_uncache_entry				internal_ldap_uncache_entry
#define ldap_uncache_request			internal_ldap_uncache_request
#define ldap_url_parse					internal_ldap_url_parse
#define ldap_url_search					internal_ldap_url_search
#define ldap_url_search_s				internal_ldap_url_search_s
#define ldap_url_search_st				internal_ldap_url_search_st
#define ldap_value_free					internal_ldap_value_free
#define ldap_value_free_len				internal_ldap_value_free_len
