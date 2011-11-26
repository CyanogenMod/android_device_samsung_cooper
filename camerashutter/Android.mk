ifeq ($(TARGET_BOOTLOADER_BOARD_NAME),cooper)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := toggleshutter.c
LOCAL_MODULE := toggleshutter
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcutils
include $(BUILD_EXECUTABLE)

endif
