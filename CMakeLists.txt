cmake_minimum_required(VERSION 3.10.2)

if( ANDROID )
	set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
	set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s -DNDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "-O2 -s -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -Og")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g3 -Og")
	set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -s -DNDEBUG")
	set(CMAKE_C_FLAGS_MINSIZEREL "-Os -s -DNDEBUG")
endif()


set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUNICODE -DTJS_TEXT_OUT_CRLF -DTJS_JP_LOCALIZED -DTVP_ENABLE_EXECUTE_AT_EXCEPTION -DTVP_LOG_TO_COMMANDLINE_CONSOLE" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE -DTJS_TEXT_OUT_CRLF -DTJS_JP_LOCALIZED -DTVP_ENABLE_EXECUTE_AT_EXCEPTION -DTVP_LOG_TO_COMMANDLINE_CONSOLE" )


# 共通の include ディレクトリ
set( KRKRZ_INC
environ/
tjs2
external/onig/src
external/lpng
external/freetype/include
external/libjpeg-turbo
external/libjpeg-turbo/android
external/libogg/include
external/opus/include
external/opusfile/include
external/glm/
base
extension
sound
movie
msg
utils
visual
visual/gl
visual/opengl )

# Android 用の include ディレクトリ
if( ANDROID )
	list(APPEND KRKRZ_INC 
	environ/android
	base/android
	sound/android
	msg/android
	utils/android
	visual/android
	)
elseif( WIN32 )
	list(APPEND KRKRZ_INC 
	environ/win32
	base/win32
	sound/win32
	msg/win32
	utils/win32
	visual/win32
	)
endif()

include_directories( ${KRKRZ_INC} )

set( KRKRZ_SRC 
tjs2/tjs.cpp
tjs2/tjs.tab.cpp
tjs2/tjsArray.cpp
tjs2/tjsBinarySerializer.cpp
tjs2/tjsByteCodeLoader.cpp
tjs2/tjsCompileControl.cpp
tjs2/tjsConfig.cpp
tjs2/tjsConstArrayData.cpp
tjs2/tjsDate.cpp
tjs2/tjsdate.tab.cpp
tjs2/tjsDateParser.cpp
tjs2/tjsDebug.cpp
tjs2/tjsDictionary.cpp
tjs2/tjsDisassemble.cpp
tjs2/tjsError.cpp
tjs2/tjsException.cpp
tjs2/tjsGlobalStringMap.cpp
tjs2/tjsInterCodeExec.cpp
tjs2/tjsInterCodeGen.cpp
tjs2/tjsInterface.cpp
tjs2/tjsLex.cpp
tjs2/tjsMath.cpp
tjs2/tjsMessage.cpp
tjs2/tjsMT19937ar-cok.cpp
tjs2/tjsNamespace.cpp
tjs2/tjsNative.cpp
tjs2/tjsObject.cpp
tjs2/tjsObjectExtendable.cpp
tjs2/tjsOctPack.cpp
tjs2/tjspp.tab.cpp
tjs2/tjsRandomGenerator.cpp
tjs2/tjsRegExp.cpp
tjs2/tjsScriptBlock.cpp
tjs2/tjsScriptCache.cpp
tjs2/tjsSnprintf.cpp
tjs2/tjsString.cpp
tjs2/tjsUtils.cpp
tjs2/tjsVariant.cpp
tjs2/tjsVariantString.cpp
base/BinaryStream.cpp
base/CharacterSet.cpp
base/EventIntf.cpp
base/PluginIntf.cpp
base/ScriptMgnIntf.cpp
base/StorageIntf.cpp
base/SysInitIntf.cpp
base/SystemIntf.cpp
base/TextStream.cpp
base/UtilStreams.cpp
base/XP3Archive.cpp
base/android/AssetMedia.cpp
base/android/ContentMedia.cpp
base/android/EventImpl.cpp
base/android/FuncStubs.cpp
base/android/SystemImpl.cpp
base/android/NativeEventQueue.cpp
base/android/PluginImpl.cpp
base/android/ScriptMgnImpl.cpp
base/android/StorageImpl.cpp
base/android/SysInitImpl.cpp
environ/TouchPoint.cpp
environ/android/Application.cpp
environ/android/DetectCPU.cpp
environ/android/TVPScreen.cpp
environ/android/SystemControl.cpp
environ/android/VirtualKey.cpp
environ/android/WindowForm.cpp
extension/Extension.cpp
msg/MsgIntf.cpp
msg/android/MsgImpl.cpp
msg/android/MsgLoad.cpp
sound/MathAlgorithms.cpp
sound/PhaseVocoderDSP.cpp
sound/PhaseVocoderFilter.cpp
sound/QueueSoundBufferImpl.cpp
sound/RealFFT.cpp
sound/SoundBufferBaseIntf.cpp
sound/SoundBufferBaseImpl.cpp
sound/SoundDecodeThread.cpp
sound/SoundEventThread.cpp
sound/SoundPlayer.cpp
sound/WaveFormatConverter.cpp
sound/WaveIntf.cpp
sound/WaveLoopManager.cpp
sound/WaveSegmentQueue.cpp
sound/OpusCodecDecoder.cpp
sound/android/OpenSLESAudioDevice.cpp
utils/ClipboardIntf.cpp
utils/cp932_uni.cpp
utils/DebugIntf.cpp
utils/md5.c
utils/MiscUtility.cpp
utils/Random.cpp
utils/ThreadIntf.cpp
utils/TickCount.cpp
utils/TimerThread.cpp
utils/TimerIntf.cpp
utils/TVPTimer.cpp
utils/uni_cp932.cpp
utils/VelocityTracker.cpp
utils/android/ClipboardImpl.cpp
visual/BitmapIntf.cpp
visual/BitmapLayerTreeOwner.cpp
visual/CharacterData.cpp
visual/ComplexRect.cpp
visual/DrawDevice.cpp
visual/FontSystem.cpp
visual/FreeType.cpp
visual/FreeTypeFontRasterizer.cpp
visual/GraphicsLoaderIntf.cpp
visual/GraphicsLoadThread.cpp
visual/ImageFunction.cpp
visual/LayerBitmapImpl.cpp
visual/LayerBitmapIntf.cpp
visual/LayerIntf.cpp
visual/LayerManager.cpp
visual/LayerTreeOwnerImpl.cpp
visual/LoadJPEG.cpp
visual/LoadPNG.cpp
visual/LoadTLG.cpp
visual/NullDrawDevice.cpp
visual/PrerenderedFont.cpp
visual/RectItf.cpp
visual/SaveTLG5.cpp
visual/SaveTLG6.cpp
visual/TransIntf.cpp
visual/tvpgl.c
visual/VideoOvlIntf.cpp
visual/WindowIntf.cpp
visual/android/BitmapBitsAlloc.cpp
visual/android/BitmapInfomation.cpp
visual/android/LayerImpl.cpp
visual/android/VideoOvlImpl.cpp
visual/android/WindowImpl.cpp
visual/gl/blend_function.cpp
visual/gl/ResampleImage.cpp
visual/gl/WeightFunctor.cpp
visual/opengl/CanvasIntf.cpp
visual/opengl/DrawCycleTimer.cpp
visual/opengl/GLFrameBufferObject.cpp
visual/opengl/GLShaderUtil.cpp
visual/opengl/Matrix32Intf.cpp
visual/opengl/Matrix44Intf.cpp
visual/opengl/OffscreenIntf.cpp
visual/opengl/OpenGLPlatformAndroid.cpp
visual/opengl/OpenGLScreen.cpp
visual/opengl/ShaderProgramIntf.cpp
visual/opengl/TextureIntf.cpp
visual/opengl/VertexBinderIntf.cpp
visual/opengl/VertexBufferIntf.cpp )

if( ANDROID_ABI STREQUAL x86 OR ANDROID_ABI STREQUAL x86_64 )
	list(APPEND KRKRZ_SRC 
	sound/MathAlgorithms_SSE.cpp
	sound/RealFFT_SSE.cpp
	sound/WaveFormatConverter_SSE.cpp
	sound/xmmlib.cpp )
endif()

if( ANDROID )
	add_library( krkrz SHARED ${KRKRZ_SRC} )
endif()

# external library
add_subdirectory( external/freetype )
add_subdirectory( external/libjpeg-turbo )
add_subdirectory( external/lpng )
add_subdirectory( external/onig )
add_subdirectory( external/libogg )
add_subdirectory( external/opus )
add_subdirectory( external/opusfile )


if( ANDROID )
	# cpufeature
	include_directories( ${ANDROID_NDK}/sources/android/cpufeatures )
	add_library( cpufeatures ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c )

	# native_app_glue
	# include_directories( ${ANDROID_NDK}/sources/android/native_app_glue )
	# add_library(native_app_glue ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

	# ndk_helper
	#include_directories( ${ANDROID_NDK}/sources/android/ndk_helper ${ANDROID_NDK}/sources/android/native_app_glue )
	#file( GLOB _NDK_HELPER_SRCS ${ANDROID_NDK}/sources/android/ndk_helper/*.cpp ${ANDROID_NDK}/sources/android/ndk_helper/gl3stub.c )
	#add_library( ndk_helper ${_NDK_HELPER_SRCS} )
	include_directories( ${ANDROID_NDK}/sources/android/ndk_helper )
	add_library( ndk_helper ${ANDROID_NDK}/sources/android/ndk_helper/gl3stub.c )

	find_library( log-lib log )
	find_library( android-lib android )
	find_library( EGL-lib EGL )
	find_library( GLESv1_CM-lib GLESv1_CM )
	find_library( GLESv2-lib GLESv2 )
	find_library( z-lib z )
	find_library( opensles OpenSLES )

	# opusfile より opus/oggを後にリンクする必要があることに注意
	target_link_libraries( krkrz

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       ${android-lib}
                       ${EGL-lib}
                       ${GLESv1_CM-lib}
                       ${GLESv2-lib}
                       ${z-lib}
                       ${opensles}
                       atomic
                       
                       # Links my library
                       freetype
                       jpeg-turbo
                       png
                       onig
                       opusfile
                       opus
                       ogg
                       
                       # 
                       cpufeatures
                       ndk_helper
                        )
endif()

