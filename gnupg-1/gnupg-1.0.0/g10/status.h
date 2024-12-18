/* status.h
 *	Copyright (C) 1998 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifndef G10_STATUS_H
#define G10_STATUS_H


#define STATUS_ENTER	 1
#define STATUS_LEAVE	 2
#define STATUS_ABORT	 3

#define STATUS_GOODSIG	 4
#define STATUS_BADSIG	 5
#define STATUS_ERRSIG	 6


#define STATUS_BADARMOR  7

#define STATUS_RSA_OR_IDEA 8
#define STATUS_SIGEXPIRED  9
#define STATUS_KEYREVOKED  10

#define STATUS_TRUST_UNDEFINED 11
#define STATUS_TRUST_NEVER     12
#define STATUS_TRUST_MARGINAL  13
#define STATUS_TRUST_FULLY     14
#define STATUS_TRUST_ULTIMATE  15

#define STATUS_SHM_INFO        16
#define STATUS_SHM_GET	       17
#define STATUS_SHM_GET_BOOL    18
#define STATUS_SHM_GET_HIDDEN  19

#define STATUS_NEED_PASSPHRASE 20
#define STATUS_VALIDSIG        21
#define STATUS_SIG_ID	       22
#define STATUS_ENC_TO	       23
#define STATUS_NODATA	       24
#define STATUS_BAD_PASSPHRASE  25
#define STATUS_NO_PUBKEY       26
#define STATUS_NO_SECKEY       27
#define STATUS_NEED_PASSPHRASE_SYM 28
#define STATUS_DECRYPTION_FAILED 29
#define STATUS_DECRYPTION_OKAY	 30
#define STATUS_MISSING_PASSPHRASE 31
#define STATUS_GOOD_PASSPHRASE	32
#define STATUS_GOODMDC		33
#define STATUS_BADMDC		34
#define STATUS_ERRMDC		35
#define STATUS_IMPORTED 	36
#define STATUS_IMPORT_RES	37


/*-- status.c --*/
void set_status_fd ( int fd );
int  is_status_enabled ( void );
void write_status ( int no );
void write_status_text ( int no, const char *text );

#ifdef USE_SHM_COPROCESSING
  void init_shm_coprocessing ( ulong requested_shm_size, int lock_mem );
#endif /*USE_SHM_COPROCESSING*/

int cpr_enabled(void);
char *cpr_get( const char *keyword, const char *prompt );
char *cpr_get_utf8( const char *keyword, const char *prompt );
char *cpr_get_hidden( const char *keyword, const char *prompt );
void cpr_kill_prompt(void);
int  cpr_get_answer_is_yes( const char *keyword, const char *prompt );
int  cpr_get_answer_yes_no_quit( const char *keyword, const char *prompt );


#endif /*G10_STATUS_H*/
