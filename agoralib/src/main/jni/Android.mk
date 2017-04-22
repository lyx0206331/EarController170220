LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libapm-iwhere
LOCAL_SRC_FILES := NativeCallObject.cpp


include $(BUILD_SHARED_LIBRARY)

