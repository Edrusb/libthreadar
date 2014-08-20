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




