LOCAL_PATH := $(call my-dir)/..
BASE_PATH := $(call my-dir)/..

#########################################################################
# for libfreetype2-static.a
include $(CLEAR_VARS)

FREETYPE_SRC_PATH := freetype2-android/

LOCAL_MODULE := freetype2-static

LOCAL_CFLAGS := -DANDROID_NDK \
		-DFT2_BUILD_LIBRARY=1

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include_all \
		$(LOCAL_PATH)/freetype2-android/include \
		$(LOCAL_PATH)/freetype2-android/src

LOCAL_SRC_FILES := \
	$(FREETYPE_SRC_PATH)src/autofit/autofit.c \
	$(FREETYPE_SRC_PATH)src/base/basepic.c \
	$(FREETYPE_SRC_PATH)src/base/ftapi.c \
	$(FREETYPE_SRC_PATH)src/base/ftbase.c \
	$(FREETYPE_SRC_PATH)src/base/ftbbox.c \
	$(FREETYPE_SRC_PATH)src/base/ftbitmap.c \
	$(FREETYPE_SRC_PATH)src/base/ftdbgmem.c \
	$(FREETYPE_SRC_PATH)src/base/ftdebug.c \
	$(FREETYPE_SRC_PATH)src/base/ftglyph.c \
	$(FREETYPE_SRC_PATH)src/base/ftinit.c \
	$(FREETYPE_SRC_PATH)src/base/ftpic.c \
	$(FREETYPE_SRC_PATH)src/base/ftstroke.c \
	$(FREETYPE_SRC_PATH)src/base/ftsynth.c \
	$(FREETYPE_SRC_PATH)src/base/ftsystem.c \
	$(FREETYPE_SRC_PATH)src/cff/cff.c \
	$(FREETYPE_SRC_PATH)src/pshinter/pshinter.c \
	$(FREETYPE_SRC_PATH)src/psnames/psnames.c \
	$(FREETYPE_SRC_PATH)src/raster/raster.c \
	$(FREETYPE_SRC_PATH)src/sfnt/sfnt.c \
	$(FREETYPE_SRC_PATH)src/smooth/smooth.c \
	$(FREETYPE_SRC_PATH)src/truetype/truetype.c

# LOCAL_LDLIBS := -ldl -llog

include $(BUILD_STATIC_LIBRARY)

#########################################################################
# for libonig.a
include $(CLEAR_VARS)

LOCAL_MODULE:= libonig

LOCAL_C_INCLUDES := $(LOCAL_PATH)/onig

LOCAL_CFLAGS += -DONIG_EXTERN=extern \
				-DHAVE_CONFIG_H \
				-DNOT_RUBY

LOCAL_SRC_FILES := \
onig/enc/ascii.c \
onig/enc/big5.c \
onig/enc/cp1251.c \
onig/enc/euc_jp.c \
onig/enc/euc_kr.c \
onig/enc/euc_tw.c \
onig/enc/gb18030.c \
onig/enc/iso8859_1.c \
onig/enc/iso8859_10.c \
onig/enc/iso8859_11.c \
onig/enc/iso8859_13.c \
onig/enc/iso8859_14.c \
onig/enc/iso8859_15.c \
onig/enc/iso8859_16.c \
onig/enc/iso8859_2.c \
onig/enc/iso8859_3.c \
onig/enc/iso8859_4.c \
onig/enc/iso8859_5.c \
onig/enc/iso8859_6.c \
onig/enc/iso8859_7.c \
onig/enc/iso8859_8.c \
onig/enc/iso8859_9.c \
onig/enc/koi8.c \
onig/enc/koi8_r.c \
onig/enc/sjis.c \
onig/enc/unicode.c \
onig/enc/utf16_be.c \
onig/enc/utf16_le.c \
onig/enc/utf32_be.c \
onig/enc/utf32_le.c \
onig/enc/utf8.c \
onig/regcomp.c \
onig/regenc.c \
onig/regerror.c \
onig/regexec.c \
onig/regext.c \
onig/reggnu.c \
onig/regparse.c \
onig/regposerr.c \
onig/regposix.c \
onig/regsyntax.c \
onig/regtrav.c \
onig/regversion.c \
onig/st.c

include $(BUILD_STATIC_LIBRARY)

#########################################################################
# for libpng.a
include $(CLEAR_VARS)
LOCAL_MODULE:= libpng
LOCAL_C_INCLUDES := $(LOCAL_PATH)/lpng
LOCAL_CFLAGS :=
LOCAL_SRC_FILES := \
lpng/png.c \
lpng/pngerror.c \
lpng/pngget.c \
lpng/pngmem.c \
lpng/pngpread.c \
lpng/pngread.c \
lpng/pngrio.c \
lpng/pngrtran.c \
lpng/pngrutil.c \
lpng/pngset.c \
lpng/pngtrans.c \
lpng/pngwio.c \
lpng/pngwrite.c \
lpng/pngwtran.c \
lpng/pngwutil.c
include $(BUILD_STATIC_LIBRARY)

