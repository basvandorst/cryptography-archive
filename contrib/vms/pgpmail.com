$! From: cbl@uihepa.hep.uiuc.edu (Chris Luchini)
$! Subject: VMS mail script (?)
$! Date: Wed, 28 Jul 1993 23:14:54 GMT
$!
$! I have a .com file that will do pgp under VMS mail.
$! I can't claim authorship, someone sent it to me long
$! ago, but I had to modify it to make it work. If you are the
$! author, please contact me.
$!
$! In order to use this .com file, you need to extract it into
$! a file, for instance pgpmail.com
$!
$! Define the logical
$! $ define mail$edit sys$login:pgpmail.com
$!
$! and away you go. You will need to modify the line
$!
$! $	 edit/tpu 'infile' 'outfile' /section=DISK$CDF:[CBL]PEVE.TPU$SECTION
$!
$! to use your section file, or your editor of choice.
$!
$!
$!
$!Begin pgpmail.com
$!-----------------------------------------------------------------
$	verify = 'f$verify(0)'
$	set noon
$	on control_y then continue !ARGH
$if	p1 .eqs. ""
$then
$	outfile = ""
$	infile = P2
$else
$	outfile = "/out="+P2
$	if P1 .eqs. P2 then outfile = "/READ"
$	infile = P1
$endif
$ !
$if	P1 .nes. ""
$then
$	def/user sys$output nla0:
$	def/user sys$error nla0:
$	search 'P1' "-----BEGIN PGP"/out=sys$scratch:check_if_pgp_file.tmp
$	tmpfile = f$search("sys$scratch:check_if_pgp_file.tmp")
$	open/read/err=L10 tmpfile sys$scratch:check_if_pgp_file.tmp
$	read/end=L5 tmpfile inline
$	define/user sys$input sys$command
$	pgp 'infile' -o sys$scratch:pgp_output.tmp
$	infile = "sys$scratch:pgp_output.tmp"
$L5:
$	close tmpfile
$L10:
$	delete/erase sys$scratch:check_if_pgp_file.tmp;*
$endif
$	define/user sys$input sys$command
$     edit/tpu 'infile' 'outfile' /section=DISK$CDF:[CBL]PEVE.TPU$SECTION
$	xx = f$search("sys$scratch:pgp_output.tmp")
$	if xx .eqs. "" then goto L100
$	delete/erase sys$scratch:pgp_output.tmp.*
$L100:
$	if P2 .eqs. P1 then goto L300
$L110:
$	read/err=L150/end=L150 sys$command enctype-
	  /prompt="Encrypt (E), Sign (S) Both (B) or None (N) (default=N)? "
$	if enctype .eqs. "" then goto L150
$	enctype = f$edit(enctype,"UPCASE")
$	if enctype .eqs. "N" then goto L150
$if	enctype .eqs. "B"
$then	switch = "-seat"
$	goto L120
$endif
$if	enctype .eqs. "S"
$then	switch = "-sat +clearsig=on"
$	goto L120
$endif
$if	enctype .eqs. "E"
$then	switch = "-eat"
$	goto L120
$endif
$	write sys$output "{"+enctype+"} is not valid - try again"
$	goto L110
$L120:
$	if enctype .nes. "S" then -
		read	sys$command username /prompt="Username ? "
$	define/user sys$input sys$command
$	pgp 'switch' 'p2' 'username' -o sys$scratch:pgp_mail_tmp
$	delete/erase 'p2';*
$	copy sys$scratch:pgp_mail_tmp.asc 'p2';1
$	delete/erase sys$scratch:pgp_mail_tmp.asc;*
$L150:
$L300:
$	exit 1+0*f$verify(verify)
