#
# Arithmetic in Global Fields
# Copyright (C) 1990 Kevin R. Coombes and David R. Grant
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 1, or (at your option)
#  any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

# The @(#)Makefile 1.1, created 3/29/90, contains 
# dependency information for the header files which are needed with
# the Arithmetic package.

Mstruct.h : Qstruct.h
	touch Mstruct.h

fraction.h : integer.h Qstruct.h
	touch fraction.h

rational.h : polynomial.h Qstruct.h
	touch rational.h

Qstruct.h : normal.h
	touch Qstruct.h

integer.h : utility.h Imacro.h Ztools.h
	touch integer.h

polynomial.h : utility.h Pmacro.h Ztools.h
	touch polynomial.h

utility.h : normal.h
	touch utility.h

normal.h Pmacro.h Imacro.h Ztools.h :
	@ echo "we got some serious trouble"
