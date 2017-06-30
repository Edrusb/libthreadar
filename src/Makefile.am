if BSD_SED
  SED_REGEX = -E
else
  SED_REGEX = -r
endif

localedir = @localedir@
pkgconfigdir = @pkgconfigdir@

AM_LDFLAGS = @LTLIBINTL@
LDADD =
lib_LTLIBRARIES = libthreadar.la

LIBTHREADAR_MAJOR=`sed -$(SED_REGEX) -n -e 's/^.*LIBTHREADAR_MAJOR\s*=\s+([^\s;]+).*/\1/p' libthreadar.cpp`
LIBTHREADAR_MEDIUM=`sed -$(SED_REGEX) -n -e 's/^.*LIBTHREADAR_MEDIUM\s*=\s+([^\s;]+).*/\1/p' libthreadar.cpp`
LIBTHREADAR_MEDIUM_000=`echo $(LIBDTHREADAR_MEDIUM) | sed $(SED_REGEX) -e 's% %%g' -e 's%([^0-9]|^)([0-9]{1})([^0-9]|$$)%0\2%' -e 's%([^0-9]|^)([0-9]{2})([^0-9]|$$)%0\2%'`
LIBTHREADAR_MINOR=`sed -$(SED_REGEX) -n -e 's/^.*LIBTHREADAR_MINOR\s*=\s+([^\s;]+).*/\1/p' libthreadar.cpp`
LIBTHREADAR_LIBTOOL_CURRENT=$(LIBTHREADAR_MAJOR)$(LIBTHREADAR_MEDIUM_000)
LIBTHREADAR_LIBTOOL_REVISION=$(LIBTHREADAR_MINOR)
LIBTHREADAR_LIBTOOL_AGE=$(LIBTHREADAR_MEDIUM)
LIBTHREADAR_VERSION_IN=$(LIBTHREADAR_LIBTOOL_CURRENT):$(LIBTHREADAR_LIBTOOL_REVISION):$(LIBTHREADAR_LIBTOOL_AGE)
LIBTHREADAR_VERSION_OUT=$(LIBTHREADAR_MAJOR).$(LIBTHREADAR_MEDIUM).$(LIBTHREADAR_MINOR)

dist_noinst_DATA = exceptions.hpp libthreadar.hpp mutex.hpp semaphore.hpp tampon.hpp thread.hpp barrier.hpp fast_tampon.hpp freezer.hpp

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

