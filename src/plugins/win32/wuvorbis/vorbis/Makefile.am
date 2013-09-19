## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign dist-zip

SUBDIRS = lib include doc examples win32 debian vq

m4datadir = $(datadir)/aclocal
m4data_DATA = vorbis.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = vorbis.pc vorbisenc.pc vorbisfile.pc

EXTRA_DIST = todo.txt macos macosx autogen.sh \
	libvorbis.spec libvorbis.spec.in \
	vorbis.m4 vorbis.pc.in vorbisenc.pc.in vorbisfile.pc.in \
       vorbis-uninstalled.pc.in \
       vorbisenc-uninstalled.pc.in \
       vorbisfile-uninstalled.pc.in

DISTCHECK_CONFIGURE_FLAGS = --enable-docs

dist-hook:
	rm -rf `find $(distdir)/macos -name CVS`
	rm -rf `find $(distdir)/macos -name .svn`
	rm -rf `find $(distdir)/macosx -name CVS`
	rm -rf `find $(distdir)/macosx -name .svn`

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
