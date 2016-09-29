LOCAL_PATH := $(call my-dir)/..

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
$(LOCAL_PATH)/environ/ \
$(LOCAL_PATH)/environ/android \
$(LOCAL_PATH)/tjs2 \
$(LOCAL_PATH)/external/onig/src \
$(LOCAL_PATH)/external/lpng \
$(LOCAL_PATH)/external/freetype/include \
$(LOCAL_PATH)/external/libjpeg-turbo \
$(LOCAL_PATH)/external/libjpeg-turbo/android \
$(LOCAL_PATH)/base \
$(LOCAL_PATH)/base/android \
$(LOCAL_PATH)/extension \
$(LOCAL_PATH)/sound \
$(LOCAL_PATH)/sound/android \
$(LOCAL_PATH)/movie \
$(LOCAL_PATH)/msg \
$(LOCAL_PATH)/msg/android \
$(LOCAL_PATH)/utils \
$(LOCAL_PATH)/utils/android \
$(LOCAL_PATH)/visual \
$(LOCAL_PATH)/visual/android \
$(LOCAL_PATH)/visual/gl \
$(NDK)/source/cpufeatures

LOCAL_SRC_FILES := \
$(LOCAL_PATH)/tjs2/tjs.cpp \
$(LOCAL_PATH)/tjs2/tjs.tab.cpp \
$(LOCAL_PATH)/tjs2/tjsArray.cpp \
$(LOCAL_PATH)/tjs2/tjsBinarySerializer.cpp \
$(LOCAL_PATH)/tjs2/tjsByteCodeLoader.cpp \
$(LOCAL_PATH)/tjs2/tjsCompileControl.cpp \
$(LOCAL_PATH)/tjs2/tjsConfig.cpp \
$(LOCAL_PATH)/tjs2/tjsConstArrayData.cpp \
$(LOCAL_PATH)/tjs2/tjsDate.cpp \
$(LOCAL_PATH)/tjs2/tjsdate.tab.cpp \
$(LOCAL_PATH)/tjs2/tjsDateParser.cpp \
$(LOCAL_PATH)/tjs2/tjsDebug.cpp \
$(LOCAL_PATH)/tjs2/tjsDictionary.cpp \
$(LOCAL_PATH)/tjs2/tjsDisassemble.cpp \
$(LOCAL_PATH)/tjs2/tjsError.cpp \
$(LOCAL_PATH)/tjs2/tjsException.cpp \
$(LOCAL_PATH)/tjs2/tjsGlobalStringMap.cpp \
$(LOCAL_PATH)/tjs2/tjsInterCodeExec.cpp \
$(LOCAL_PATH)/tjs2/tjsInterCodeGen.cpp \
$(LOCAL_PATH)/tjs2/tjsInterface.cpp \
$(LOCAL_PATH)/tjs2/tjsLex.cpp \
$(LOCAL_PATH)/tjs2/tjsMath.cpp \
$(LOCAL_PATH)/tjs2/tjsMessage.cpp \
$(LOCAL_PATH)/tjs2/tjsMT19937ar-cok.cpp \
$(LOCAL_PATH)/tjs2/tjsNamespace.cpp \
$(LOCAL_PATH)/tjs2/tjsNative.cpp \
$(LOCAL_PATH)/tjs2/tjsObject.cpp \
$(LOCAL_PATH)/tjs2/tjsObjectExtendable.cpp \
$(LOCAL_PATH)/tjs2/tjsOctPack.cpp \
$(LOCAL_PATH)/tjs2/tjspp.tab.cpp \
$(LOCAL_PATH)/tjs2/tjsRandomGenerator.cpp \
$(LOCAL_PATH)/tjs2/tjsRegExp.cpp \
$(LOCAL_PATH)/tjs2/tjsScriptBlock.cpp \
$(LOCAL_PATH)/tjs2/tjsScriptCache.cpp \
$(LOCAL_PATH)/tjs2/tjsString.cpp \
$(LOCAL_PATH)/tjs2/tjsUtils.cpp \
$(LOCAL_PATH)/tjs2/tjsVariant.cpp \
$(LOCAL_PATH)/tjs2/tjsVariantString.cpp \
$(LOCAL_PATH)/base/BinaryStream.cpp \
$(LOCAL_PATH)/base/CharacterSet.cpp \
$(LOCAL_PATH)/base/EventIntf.cpp \
$(LOCAL_PATH)/base/PluginIntf.cpp \
$(LOCAL_PATH)/base/ScriptMgnIntf.cpp \
$(LOCAL_PATH)/base/StorageIntf.cpp \
$(LOCAL_PATH)/base/SysInitIntf.cpp \
$(LOCAL_PATH)/base/SystemIntf.cpp \
$(LOCAL_PATH)/base/TextStream.cpp \
$(LOCAL_PATH)/base/UtilStreams.cpp \
$(LOCAL_PATH)/base/XP3Archive.cpp \
$(LOCAL_PATH)/base/android/AssetMedia.cpp \
$(LOCAL_PATH)/base/android/EventImpl.cpp \
$(LOCAL_PATH)/base/android/FuncStubs.cpp \
$(LOCAL_PATH)/base/android/SystemImpl.cpp \
$(LOCAL_PATH)/base/android/NativeEventQueue.cpp \
$(LOCAL_PATH)/base/android/PluginImpl.cpp \
$(LOCAL_PATH)/base/android/ScriptMgnImpl.cpp \
$(LOCAL_PATH)/base/android/StorageImpl.cpp \
$(LOCAL_PATH)/base/android/SysInitImpl.cpp \
$(LOCAL_PATH)/environ/TouchPoint.cpp \
$(LOCAL_PATH)/environ/android/Application.cpp \
$(LOCAL_PATH)/environ/android/DetectCPU.cpp \
$(LOCAL_PATH)/environ/android/TVPScreen.cpp \
$(LOCAL_PATH)/environ/android/SystemControl.cpp \
$(LOCAL_PATH)/extension/Extension.cpp \
$(LOCAL_PATH)/msg/MsgIntf.cpp \
$(LOCAL_PATH)/msg/android/MsgImpl.cpp \
$(LOCAL_PATH)/msg/android/MsgLoad.cpp \
$(LOCAL_PATH)/msg/android/string_table_jp.cpp \
$(LOCAL_PATH)/sound/MathAlgorithms.cpp \
$(LOCAL_PATH)/sound/PhaseVocoderDSP.cpp \
$(LOCAL_PATH)/sound/PhaseVocoderFilter.cpp \
$(LOCAL_PATH)/sound/RealFFT.cpp \
$(LOCAL_PATH)/sound/SoundBufferBaseIntf.cpp \
$(LOCAL_PATH)/sound/SoundBufferBaseImpl.cpp \
$(LOCAL_PATH)/sound/WaveFormatConverter.cpp \
$(LOCAL_PATH)/sound/WaveIntf.cpp \
$(LOCAL_PATH)/sound/WaveLoopManager.cpp \
$(LOCAL_PATH)/sound/WaveSegmentQueue.cpp \
$(LOCAL_PATH)/sound/android/WaveImpl.cpp \
$(LOCAL_PATH)/utils/ClipboardIntf.cpp \
$(LOCAL_PATH)/utils/cp932_uni.cpp \
$(LOCAL_PATH)/utils/DebugIntf.cpp \
$(LOCAL_PATH)/utils/md5.c \
$(LOCAL_PATH)/utils/MiscUtility.cpp \
$(LOCAL_PATH)/utils/Random.cpp \
$(LOCAL_PATH)/utils/ThreadIntf.cpp \
$(LOCAL_PATH)/utils/TickCount.cpp \
$(LOCAL_PATH)/utils/TimerImpl.cpp \
$(LOCAL_PATH)/utils/TimerIntf.cpp \
$(LOCAL_PATH)/utils/uni_cp932.cpp \
$(LOCAL_PATH)/utils/VelocityTracker.cpp \
$(LOCAL_PATH)/utils/android/ClipboardImpl.cpp \
$(LOCAL_PATH)/utils/android/ThreadImpl.cpp \
$(LOCAL_PATH)/utils/android/TVPTimer.cpp \
$(LOCAL_PATH)/visual/BitmapIntf.cpp \
$(LOCAL_PATH)/visual/BitmapLayerTreeOwner.cpp \
$(LOCAL_PATH)/visual/CharacterData.cpp \
$(LOCAL_PATH)/visual/ComplexRect.cpp \
$(LOCAL_PATH)/visual/DrawDevice.cpp \
$(LOCAL_PATH)/visual/FontSystem.cpp \
$(LOCAL_PATH)/visual/FreeType.cpp \
$(LOCAL_PATH)/visual/FreeTypeFontRasterizer.cpp \
$(LOCAL_PATH)/visual/GraphicsLoaderIntf.cpp \
$(LOCAL_PATH)/visual/GraphicsLoadThread.cpp \
$(LOCAL_PATH)/visual/ImageFunction.cpp \
$(LOCAL_PATH)/visual/LayerBitmapImpl.cpp \
$(LOCAL_PATH)/visual/LayerBitmapIntf.cpp \
$(LOCAL_PATH)/visual/LayerIntf.cpp \
$(LOCAL_PATH)/visual/LayerManager.cpp \
$(LOCAL_PATH)/visual/LayerTreeOwnerImpl.cpp \
$(LOCAL_PATH)/visual/LoadJPEG.cpp \
$(LOCAL_PATH)/visual/LoadPNG.cpp \
$(LOCAL_PATH)/visual/LoadTLG.cpp \
$(LOCAL_PATH)/visual/PrerenderedFont.cpp \
$(LOCAL_PATH)/visual/RectItf.cpp \
$(LOCAL_PATH)/visual/SaveTLG5.cpp \
$(LOCAL_PATH)/visual/SaveTLG6.cpp \
$(LOCAL_PATH)/visual/TransIntf.cpp \
$(LOCAL_PATH)/visual/tvpgl.c \
$(LOCAL_PATH)/visual/VideoOvlIntf.cpp \
$(LOCAL_PATH)/visual/WindowIntf.cpp \
$(LOCAL_PATH)/visual/android/BitmapBitsAlloc.cpp \
$(LOCAL_PATH)/visual/android/BitmapInfomation.cpp \
$(LOCAL_PATH)/visual/android/LayerImpl.cpp \
$(LOCAL_PATH)/visual/android/VideoOvlImpl.cpp \
$(LOCAL_PATH)/visual/android/WindowForm.cpp \
$(LOCAL_PATH)/visual/android/WindowImpl.cpp \
$(LOCAL_PATH)/visual/gl/blend_function.cpp \
$(LOCAL_PATH)/visual/gl/ResampleImage.cpp \
$(LOCAL_PATH)/visual/gl/WeightFunctor.cpp

ifneq ($(filter $(TARGET_ARCH_ABI), x86_64 x86),)
LOCAL_SRC_FILES += \
$(LOCAL_PATH)/sound/MathAlgorithms_SSE.cpp \
$(LOCAL_PATH)/sound/RealFFT_SSE.cpp \
$(LOCAL_PATH)/sound/WaveFormatConverter_SSE.cpp \
$(LOCAL_PATH)/sound/xmmlib.cpp

endif

LOCAL_LDFLAGS += -Wl,--version-script=$(LOCAL_PATH)/libkrkrz.map
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -lz -latomic
LOCAL_WHOLE_STATIC_LIBRARIES += libonig libpng libfreetype libjpeg-turbo
LOCAL_STATIC_LIBRARIES := android_native_app_glue cpufeatures

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_EXECUTABLE)


$(call import-add-path,$(call my-dir))
$(call import-module,android/cpufeatures)
$(call import-module,android/native_app_glue)
