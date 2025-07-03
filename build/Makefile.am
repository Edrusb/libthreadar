#######################################################################
## libthreadar - is a library providing several C++ classes to work with threads
## Copyright (C) 2014-2025 Denis Corbin
##
## This file is part of libthreadar
##
##  libthreadar is free software: you can redistribute it and/or modify
##  it under the terms of the GNU Lesser General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  libhtreadar is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU Lesser General Public License for more details.
##
##  You should have received a copy of the GNU Lesser General Public License
##  along with libthreadar.  If not, see <http:##www.gnu.org/licenses/>
##
######
##  to contact the author: dar.linux@free.fr
#######################################################################

SUBDIRS = src doc
dist_noinst_DATA = INSTALL README THANKS AUTHORS COPYING COPYING.LESSER

CPPCHECKDIR=./cppcheckbuilddir

cppcheck:
	@if cppcheck --help > /dev/null ; then \
	( mkdir -p $(CPPCHECKDIR) && cppcheck --force --file-filter="*.?pp" --cppcheck-build-dir=$(CPPCHECKDIR) `pwd`/src ) || exit 1 ; \
	else \
	echo "cppcheck not present, aborting" || exit 1 ; \
	fi

clean-local:
	rm -rf $(CPPCHECKDIR)
