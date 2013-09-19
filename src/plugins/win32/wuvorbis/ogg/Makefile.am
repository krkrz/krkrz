## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign 1.6 dist-zip

SUBDIRS = src include doc win32 debian

m4datadir = $(datadir)/aclocal
m4data_DATA = ogg.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ogg.pc

EXTRA_DIST = README AUTHORS CHANGES COPYING \
	libogg.spec libogg.spec.in \
	ogg.m4 ogg.pc.in ogg-uninstalled.pc.in \
	macos macosx

dist-hook:
	rm -rf `find $(distdir)/macos -name CVS`
	rm -rf `find $(distdir)/macosx -name CVS`

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
