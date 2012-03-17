LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := su
LOCAL_SRC_FILES := utils.c su.c db.c activity.cpp

LOCAL_C_INCLUDES += external/sqlite/dist

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libsqlite \
    libbinder \
    libutils \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := eng

include $(BUILD_EXECUTABLE)
