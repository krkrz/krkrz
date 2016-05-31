## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign dist-zip

SUBDIRS = lib include doc examples vq

m4datadir = $(datadir)/aclocal
m4data_DATA = vorbis.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = vorbis.pc vorbisenc.pc vorbisfile.pc

# we include the whole debian/ dir in EXTRA_DIST because there's a problem
# with autotools and HFS+ MacOSX file systems that caused debian/Makefile.am
# to pick up on the lowercase changelog file and add ChangeLog to DIST_COMMON
# because of it, breaking make dist.  This works just as well.
EXTRA_DIST = \
	todo.txt autogen.sh \
	libvorbis.spec libvorbis.spec.in \
	vorbis.m4 \
	vorbis.pc.in vorbisenc.pc.in vorbisfile.pc.in \
	vorbis-uninstalled.pc.in \
	vorbisenc-uninstalled.pc.in \
	vorbisfile-uninstalled.pc.in \
	debian macos macosx win32


DISTCHECK_CONFIGURE_FLAGS = --enable-docs

dist-hook:
	rm -rf `find $(distdir)/debian -name .svn`
	rm -rf `find $(distdir)/debian -name "Makefile*"`
	rm -rf `find $(distdir)/macos -name .svn`
	rm -rf `find $(distdir)/macosx -name .svn`
	rm -rf `find $(distdir)/win32 -name .svn`

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
