#include $(call all-subdir-makefiles)


LOCAL_PATH := $(call my-dir)

#include $(CLEAR_VARS)
#LOCAL_MODULE := libfreetype
#LOCAL_SRC_FILES := $(LOCAL_PATH)/obj/local/$(TARGET_ARCH_ABI)/libfreetype.a
#include $(PREBUILT_STATIC_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := libjpeg-turbo
#LOCAL_SRC_FILES := $(LOCAL_PATH)/obj/local/$(TARGET_ARCH_ABI)/libjpeg-turbo.a
#include $(PREBUILT_STATIC_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := libonig
#LOCAL_SRC_FILES := $(LOCAL_PATH)/obj/local/$(TARGET_ARCH_ABI)/libonig.a
#include $(PREBUILT_STATIC_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := libpng
#LOCAL_SRC_FILES := $(LOCAL_PATH)/obj/local/$(TARGET_ARCH_ABI)/libpng.a
#include $(PREBUILT_STATIC_LIBRARY)

#########################################################################
# for libkrkrz.so
include $(CLEAR_VARS)

LOCAL_MODULE    := krkrz

LOCAL_CFLAGS += \
				-DUNICODE \
				-DTJS_TEXT_OUT_CRLF \
				-DTJS_JP_LOCALIZED \
				-DTVP_ENABLE_EXECUTE_AT_EXCEPTION
LOCAL_CPPFLAGS += -std=c++11
LOCAL_CPP_FEATURES += exceptions
LOCAL_C_INCLUDES += \
$(LOCAL_PATH)/environ/android \
$(LOCAL_PATH)/tjs2 \
$(LOCAL_PATH)/external/onig/src \
$(LOCAL_PATH)/external/lpng \
$(LOCAL_PATH)/external/freetype/include \
$(LOCAL_PATH)/external/libjpeg-turbo \
$(LOCAL_PATH)/base \
$(LOCAL_PATH)/base/android \
$(LOCAL_PATH)/sound \
$(LOCAL_PATH)/msg \
$(LOCAL_PATH)/msg/android \
$(LOCAL_PATH)/utils \
$(LOCAL_PATH)/utils/android \
$(LOCAL_PATH)/visual \
$(LOCAL_PATH)/visual/android \
$(NDK)/source/cpufeatures

LOCAL_SRC_FILES := \
tjs2/tjs.cpp \
tjs2/tjs.tab.cpp \
tjs2/tjsArray.cpp \
tjs2/tjsBinarySerializer.cpp \
tjs2/tjsByteCodeLoader.cpp \
tjs2/tjsCompileControl.cpp \
tjs2/tjsConfig.cpp \
tjs2/tjsConstArrayData.cpp \
tjs2/tjsDate.cpp \
tjs2/tjsdate.tab.cpp \
tjs2/tjsDateParser.cpp \
tjs2/tjsDebug.cpp \
tjs2/tjsDictionary.cpp \
tjs2/tjsDisassemble.cpp \
tjs2/tjsError.cpp \
tjs2/tjsException.cpp \
tjs2/tjsGlobalStringMap.cpp \
tjs2/tjsInterCodeExec.cpp \
tjs2/tjsInterCodeGen.cpp \
tjs2/tjsInterface.cpp \
tjs2/tjsLex.cpp \
tjs2/tjsMath.cpp \
tjs2/tjsMessage.cpp \
tjs2/tjsMT19937ar-cok.cpp \
tjs2/tjsNamespace.cpp \
tjs2/tjsNative.cpp \
tjs2/tjsObject.cpp \
tjs2/tjsObjectExtendable.cpp \
tjs2/tjsOctPack.cpp \
tjs2/tjspp.tab.cpp \
tjs2/tjsRandomGenerator.cpp \
tjs2/tjsRegExp.cpp \
tjs2/tjsScriptBlock.cpp \
tjs2/tjsScriptCache.cpp \
tjs2/tjsString.cpp \
tjs2/tjsUtils.cpp \
tjs2/tjsVariant.cpp \
tjs2/tjsVariantString.cpp \
base/BinaryStream.cpp \
base/CharacterSet.cpp \
base/EventIntf.cpp \
base/PluginIntf.cpp \
base/ScriptMgnIntf.cpp \
base/StorageIntf.cpp \
base/SysInitIntf.cpp \
base/SystemIntf.cpp \
base/TextStream.cpp \
base/UtilStreams.cpp \
base/XP3Archive.cpp \
base/android/SystemImpl.cpp \
base/android/NativeEventQueue.cpp \
msg/MsgIntf.cpp \
msg/android/MsgLoad.cpp \
msg/android/string_table_jp.cpp \
environ/android/Application.cpp \
environ/android/TVPScreen.cpp \
environ/android/SystemControl.cpp \
visual/BitmapIntf.cpp \
visual/BitmapLayerTreeOwner.cpp \
visual/CharacterData.cpp \
visual/ComplexRect.cpp \
visual/DrawDevice.cpp \
visual/FontSystem.cpp \
visual/FreeType.cpp \
visual/FreeTypeFontRasterizer.cpp \
visual/GraphicsLoaderIntf.cpp \
visual/GraphicsLoadThread.cpp \
visual/ImageFunction.cpp \
visual/LayerBitmapImpl.cpp \
visual/LayerBitmapIntf.cpp \
visual/LayerIntf.cpp \
visual/LayerManager.cpp \
visual/LayerTreeOwnerImpl.cpp \
visual/LoadPNG.cpp \
visual/LoadTLG.cpp \
visual/PrerenderedFont.cpp \
visual/RectItf.cpp \
visual/Resampler.cpp \
visual/SaveTLG5.cpp \
visual/SaveTLG6.cpp \
visual/tvpgl.c \
visual/VideoOvlIntf.cpp \
visual/WindowIntf.cpp \
visual/android/BitmapBitsAlloc.cpp \
visual/android/BitmapInfomation.cpp \
visual/android/LayerImpl.cpp \
visual/android/WindowImpl.cpp \
utils/cp932_uni.cpp \
utils/DebugIntf.cpp \
utils/md5.c \
utils/MiscUtility.cpp \
utils/Random.cpp \
utils/ThreadIntf.cpp \
utils/TickCount.cpp \
utils/TimerImpl.cpp \
utils/TimerIntf.cpp \
utils/uni_cp932.cpp \
utils/android/ThreadImpl.cpp
#graphic/LoadJPEG.cpp \
# environ/android/FontFace.cpp \

LOCAL_LDFLAGS += -Wl,--version-script=$(LOCAL_PATH)/libkrkrz.map
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -lz -latomic
LOCAL_WHOLE_STATIC_LIBRARIES += libonig libpng libfreetype libjpeg-turbo
LOCAL_STATIC_LIBRARIES := android_native_app_glue cpufeatures

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_EXECUTABLE)


$(call import-add-path,$(call my-dir))
$(call import-module,android/cpufeatures)
$(call import-module,android/native_app_glue)

