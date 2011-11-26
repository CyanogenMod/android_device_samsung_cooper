ifeq ($(TARGET_BOOTLOADER_BOARD_NAME),cooper)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := setup_fs.c

LOCAL_MODULE := setup_fs

include $(BUILD_EXECUTABLE)
endif
