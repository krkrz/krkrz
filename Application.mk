APP_PROJECT_PATH := $(NDK_PROJECT_PATH)
APP_BUILD_SCRIPT := $(APP_PROJECT_PATH)/Android.mk

#APP_ABI :=all
APP_ABI := armeabi-v7a
APP_PLATFORM := android-20
APP_STL := c++_static
#APP_STL := stlport_static
#APP_STL := gnustl_static
APP_MODULES := libfreetype libjpeg-turbo libonig libpng krkrz

