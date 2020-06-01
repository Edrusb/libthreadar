#######################################################################
## libthreadar - is a library providing several C++ classes to work with threads
## Copyright (C) 2014-2018 Denis Corbin
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

localedir = @localedir@
pkgconfigdir = @pkgconfigdir@

AM_LDFLAGS = @LTLIBINTL@
LDADD =
lib_LTLIBRARIES = libthreadar.la

LIBTHREADAR_MEDIUM_000=`echo $(LIBTHREADAR_MEDIUM) | sed $(SED_REGEX) -e 's% %%g' -e 's%([^0-9]|^)([0-9]{1})([^0-9]|$$)%0\2%' -e 's%([^0-9]|^)([0-9]{2})([^0-9]|$$)%0\2%'`
LIBTHREADAR_LIBTOOL_CURRENT=$(LIBTHREADAR_MAJOR)$(LIBTHREADAR_MEDIUM_000)
LIBTHREADAR_LIBTOOL_REVISION=$(LIBTHREADAR_MINOR)
LIBTHREADAR_LIBTOOL_AGE=$(LIBTHREADAR_MEDIUM)
LIBTHREADAR_VERSION_IN=$(LIBTHREADAR_LIBTOOL_CURRENT):$(LIBTHREADAR_LIBTOOL_REVISION):$(LIBTHREADAR_LIBTOOL_AGE)
LIBTHREADAR_VERSION_OUT=$(LIBTHREADAR_MAJOR).$(LIBTHREADAR_MEDIUM).$(LIBTHREADAR_MINOR)

dist_noinst_DATA = exceptions.hpp libthreadar.hpp mutex.hpp semaphore.hpp tampon.hpp thread.hpp barrier.hpp fast_tampon.hpp freezer.hpp condition.hpp ratelier.hpp

install-data-local:
	mkdir -p $(DESTDIR)$(pkgincludedir)
	cp ../config.h $(DESTDIR)$(pkgincludedir)/libthreadar_config.h
	for file in $(dist_noinst_DATA) ; do sed -e 's%#include \"config.h\"%MY_CONFIG_FILE_H%g' -e "s%#include \"%#include \"$(pkgincludedir)/%g" -e "s%MY_CONFIG_FILE_H%#include \"$(pkgincludedir)/libthreadar_config.h\"%g" "$$file" > $(DESTDIR)$(pkgincludedir)/"$$file"  ; done
	for file in $(DESTDIR)$(pkgincludedir)/* ; do sed -e 's%HAVE_%LIBTHREADAR_HAS_%g' -e 's%VERSION%LIBTHREADAR_VERSION%g' -e 's%_AVAILABLE%_AVAILABLE_FOR_LIBTHREDAR%g' -e 's%STDC_HEADERS%LIBTHREADAR_STDC_HEADERS%g' -e 's%ENABLE_NLS%LIBTHREADAR_ENABLE_NLS%g' -e 's%PACKAGE_%LIBTHREADAR_PACKAGE_%g' "$$file" > "$$file.tmp" && mv "$$file.tmp" "$$file" ; done
	egrep 'LIBTHREADAR' $(DESTDIR)$(pkgincludedir)/libthreadar_config.h | grep -v "#undef" >  $(DESTDIR)$(pkgincludedir)/config.h.tmp
	mv  $(DESTDIR)$(pkgincludedir)/config.h.tmp  $(DESTDIR)$(pkgincludedir)/libthreadar_config.h
	chmod 0644 $(DESTDIR)$(pkgincludedir)/*
	$(INSTALL) -d $(DESTDIR)$(pkgconfigdir)
	$(INSTALL) -m 0644 libthreadar.pc $(DESTDIR)$(pkgconfigdir)/libthreadar.pc

uninstall-local:
	rm -rf $(DESTDIR)$(pkgincludedir) $(DESTDIR)$(pkgconfigdir)/libthreadar.pc

all-local : libthreadar.pc

libthreadar.pc : libthreadar.pc.tmpl
	sed -e "s%#LIBTHREADAR_VERSION#%$(LIBTHREADAR_VERSION_OUT)%g" libthreadar.pc.tmpl > libthreadar.pc

clean-local:
	rm -rf libthreadar.pc

ALL_SOURCES = exceptions.cpp libthreadar.cpp mutex.cpp semaphore.cpp thread.cpp barrier.cpp freezer.cpp condition.cpp

libthreadar_la_LDFLAGS = -version-info $(LIBTHREADAR_VERSION_IN)
libthreadar_la_SOURCES = $(ALL_SOURCES)

