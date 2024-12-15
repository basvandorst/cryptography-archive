# Automatic PGP encryption/decryption (mainly for emacs).
# Copyright (C) 1993 Ian Jackson.

# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# It is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with GNU Emacs; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

default:	ringsearch

install:	ringsearch editme.temp invoke.temp
	for f in pgpauto decrypt pixie ; do \
	  echo $$f ; \
	  cat invoke.temp COPYRIGHT editme.temp $$f.pl > script.temp ; \
	  perl install.pl script.temp $$f exec ; \
	  cat EDITME $$f.man | perl >manpage.temp ; \
	  perl install.pl manpage.temp $$f man ; \
	done
	perl install.pl ringsearch ringsearch exec
	perl changelisp.pl auto-pgp.el >lisp.temp
	perl install.pl lisp.temp lisp lisp
	perl install.pl auto-pgp.info info info
	rm -f script.temp lisp.temp manpage.temp

clean:
	rm -f *.temp *.newt *~ ringsearch

editme.temp:	EDITME
	perl -ne 'print if /=/' EDITME >editme.newt
	mv editme.newt editme.temp

invoke.temp:	EDITME
	head -1 EDITME >invoke.newt
	mv invoke.newt invoke.temp
