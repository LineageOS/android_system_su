LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := su
LOCAL_SRC_FILES := utils.c su.c db.c activity.c

LOCAL_C_INCLUDES += external/sqlite/dist

LOCAL_STATIC_LIBRARIES := \
    liblog \
    libsqlite \
    libc \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := eng
LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)
