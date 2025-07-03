#######################################################################
## libthreadar - is a library providing several C++ classes to work with threads
## Copyright (C) 2014-2024 Denis Corbin
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

SUBDIRS = examples

dist_noinst_DATA = Doxyfile portable_cp
dist_pkgdata_DATA = README

if USE_DOXYGEN
DOXYGEN = @DOXYGEN_PROG@

all-local: Doxyfile.tmp

Doxyfile.tmp:
	sed -e "s%##VERSION##%@PACKAGE_VERSION@%g" Doxyfile > Doxyfile.tmp
	cd .. ; $(DOXYGEN) doc/Doxyfile.tmp
	if [ -d html/search ]; then chmod u+x html/search ; fi

clean-local:
	rm -rf html Doxyfile.tmp

install-data-hook:
	./portable_cp html $(DESTDIR)$(pkgdatadir)
	$(INSTALL_HTML_MAN)

uninstall-hook:
	rm -rf $(DESTDIR)$(pkgdatadir)/html
	$(UNINSTALL_HTML_MAN)
	rmdir $(DESTDIR)$(pkgdatadir) || true

else

all-local:

clean-local:


endif



