Russian translation and russian PGP tuning are
Copyright (C) 1992-1996 Andrey A. Chernov, Moscow, Russia
			Internet: ache@astral.msk.su

To setup PGP for russian language you must set
Language = ru
in your config.txt file and probably CharSet, see below.

Currently, two charsets handled by PGP, it is koi8-r and Alternate
Codes also known as IBM Code Page 866.
Internal PGP representation of russian text files is koi8-r.
Visit http://dt.demos.su/~ache/koi8.html to find more about koi8-r.
If you transfer text file out of your local system, don't
forget to add -t (text) option to 'pgp' call.
Without it, your text can't be read if local and remote system
charsets are different, f.e. for Unix<-->MSDOS transfer.

If your Unix have alternate codes charset, or you are in pure
MSDOS with alternate codes, don't forget to set
CharSet = alt_codes
in your config.txt file.
WARNING: You MUST set alt_codes in this case, otherwise PGP don't
work properly.

If you have some kind of Unix with koi8-r russian codes or
MSDOS with koi8-r (strange?), PGP is ready to work without
additional settings.
Anyway, you can set
CharSet = koi8
in your config.txt file in this case.

					Yours, Ache.
