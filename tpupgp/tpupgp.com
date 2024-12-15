From tbrowne@netcom.com Mon May 15 13:50:39 1995
Newsgroups: alt.security.pgp
Path: comlab.ox.ac.uk!bhamcs!bham!warwick!lyra.csx.cam.ac.uk!sunsite.doc.ic.ac.uk!qmw!demon!doc.news.pipex.net!pipex!howland.reston.ans.net!news.sprintlink.net!noc.netcom.net!ix.netcom.com!netcom.com!tbrowne
From: tbrowne@netcom.com (T. Erik Browne)
Subject: Using pgp with VMS TPU
Message-ID: <tbrowneD8BnyD.4HF@netcom.com>
Organization: NETCOM On-line Communication Services (408 261-4700 guest)
Date: Tue, 9 May 1995 17:44:37 GMT
Lines: 195
Sender: tbrowne@netcom18.netcom.com

Here it is folks, a way to sign and encrypt documents, as well as extract
keys, from within TPU/EVE.  This should be very helpful when writing email,
since you can now write and encrypt on the fly.  How to use it is simple:

First, save the signed message below into some file, like tpupgp.com (don't
forget to strip the signature off (BTW, I just put my key on the keyservers,
so you can check me out)).

Second, define the logical TPU$COMMAND to be the TPUPGP command file.  A
good place to do this is in your login.com.  For instance,

$ define tpu$command disk_name:[directory.subdir]tpupgp.com

Third, get into the editor and write a file.  Place the cursor at the
beginning of the section you want signed/encrypted, or where you want to
insert a public key block.

Fourth, give the command TPU PGP.  The interface is simple and fairly
self-explanatory (the only exception is that I couldn't figure out how to
stop the cursor from blinking while you are typing in your passphrase, so
while your passphrase never shows up on the screen, the cursor sits in the
document and blinks).  TPUPGP will encrypt/sign everything from the cursor
to the end of the document, except when you are only signing, in which case
it will stop at an Internet-standard signature ("--" at the beginning of a
line).

That's all there is to it.  If you see any bugs (I don't think there are any,
but you can't be too sure) please email me.  Use tbrowne@thuban.ac.hmc.edu for
correspondence.


-----BEGIN PGP SIGNED MESSAGE-----

! TPUPGP 1.0
! written by T. Erik Browne
! May 12, 1994
!
! Feel free to change anything you want, just give me credit.
!

procedure pgp
  pgp_func := read_line ("Encrypt/sign or Key extraction [EK]? ", 1);
  edit (pgp_func,upper);
  if pgp_func = "E"
  then
    pgp_encrypt;
  else
    if pgp_func = "K"
    then
      pgp_keyutil;
    else
      message ("Not a choice.  TPUPGP aborting.");
    endif;
  endif;
endprocedure

procedure pgp_encrypt
  message ("Encrypt");
  pgp_command := "pgp ";
  pgp_func2 := read_line ("Encrypt, Sign, Both [ESB]? ", 1);
  edit (pgp_func2,upper);
  if pgp_func2 = "E"
  then
    pgp_command := pgp_command + "-eat +clearsig +verbose=0 ";
    message ("Enter one or more userid's to encrypt with.");
    pgp_userid := read_line ("> ");
    if pgp_userid = ""
    then
      message ("No userid specified.  TPUPGP aborting.");
      return;
    endif;
    pgp_comsuffix := " "+pgp_userid;
!
! The command is "pgp -eat +clearsig +verbose=0 "
! The comsuffix is " userid [userid...]"
!
  else
    if pgp_func2 = "S"
    then
      pgp_command := pgp_command + "-sat +clearsig +verbose=0 ";
      pgp_comsuffix := "";
!
! The command is "pgp -sat +clearsig +verbose=0 "
! The comsuffix is ""
!
    else
      if pgp_func2 = "B"
      then
        pgp_command := pgp_command + "-seat +clearsig +verbose=0 ";
        message ("Enter one or more userid's to encrypt with.");
        pgp_userid := read_line ("> ");
        if pgp_userid = ""
        then
          message ("No userid specified.  TPUPGP aborting.");
          return;
        endif;
        pgp_comsuffix := " "+pgp_userid;
!
! The command is "pgp -seat +clearsig +verbose=0 "
! The comsuffix is " userid [userid...]"
!
      else
        message ("Not a choice.  TPUPGP aborting.");
        return
      endif;
    endif;
  endif;
  pgp_start := mark(none);
  if pgp_func2 = "S"
  then
    pgp_rangetmp := search_quietly (line_begin+"--",forward);
    if pgp_rangetmp <> 0
    then
      position (pgp_rangetmp);
      move_horizontal (-1);
      pgp_end := mark (none);
    else
      pgp_end := end_of (current_buffer);
    endif;
  else
    pgp_end := end_of (current_buffer);
  endif;
  pgp_text := create_range (pgp_start,pgp_end);
  position (pgp_text);
  pgp_buffer := create_buffer ("pgp_buffer");
  pgp_proc := create_process (pgp_buffer);
  pgp_pid := get_info(pgp_proc,"pid");
  pgp_filename := "SYS$SCRATCH:pgptemp_"+str(pgp_pid);
  pgp_file := write_file (pgp_text,pgp_filename+".txt");
  if (pgp_func2="S") or (pgp_func2="B")
  then
    pgp_pass := "";
    message ("Please type passphrase for signing. [Ignore cursor position]");
    loop
      pgp_keyword := read_char;
      exitif key_name (pgp_keyword) = ret_key;
      pgp_pass := pgp_pass + pgp_keyword;
    endloop;
    send ("pgppass == """+pgp_pass+"""",pgp_proc);
    delete (pgp_pass);
  endif;
  send (pgp_command+pgp_file+pgp_comsuffix,pgp_proc);
  if get_info (pgp_proc,"type") <> process
  then
    pgp_proc := create_process (pgp_buffer, "delete/erase "+pgp_file);
    delete (pgp_proc);
    delete (pgp_buffer);
    return;
  endif;
  send ("pgppass == ""foobar""",pgp_proc);
  erase (pgp_text);
  pgp_file2 := read_file (pgp_filename+".asc");
  send ("delete/erase "+pgp_file+","+pgp_file2,pgp_proc);
  delete (pgp_proc);
  delete (pgp_buffer);
endprocedure

procedure pgp_keyutil
  message ("Key Extract");
  pgp_command := "pgp -kxat +verbose=0 ";
  pgp_userid := read_line ("userid of extracted key> ");
  if pgp_userid = ""
  then
    message ("No userid specified.  TPUPGP aborting.");
    return;
  endif;
  pgp_userid := pgp_userid+" ";
  message ("Enter keyring to extract from [defaults to pubring]");
  pgp_ring := read_line ("keyring> ");
  pgp_buffer := create_buffer ("pgp_buffer");
  pgp_proc := create_process (pgp_buffer);
  pgp_pid := get_info(pgp_proc,"pid");
  pgp_filename := "SYS$SCRATCH:pgptemp_"+str(pgp_pid);
  send (pgp_command+pgp_userid+pgp_filename+".asc ",pgp_proc);
  if get_info (pgp_proc,"type") <> process
  then
    delete (pgp_proc);
    delete (pgp_buffer);
  endif;
  pgp_file := read_file (pgp_filename+".asc");
  send ("delete/erase "+pgp_file,pgp_proc);
  delete (pgp_proc);
  delete (pgp_buffer);
endprocedure

-----BEGIN PGP SIGNATURE-----
Version: 2.3a

iQCVAgUBLdKiv7OZoPsUMKl1AQF03wP/TMqsqUCvEd3PnV9i6zpsaYEnvmx4VfwV
uNZnINoikOlOBnfsTZKqMhiR4CPkPonI6+NTNpYAfG2xjAV37QiTdubJbIZPJNQA
LizsvFn0zr0ghqJ4tP+mpecPxAQZ9VP3okqLmH0/qPNwmlFMu+5o1XUsLZJrU4AO
ErW773aoJkc=
=dvXb
-----END PGP SIGNATURE-----


