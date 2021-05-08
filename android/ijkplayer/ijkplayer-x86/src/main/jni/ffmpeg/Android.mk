LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
# LOCAL_MODULE := ijkffmpeg
# LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/libijkffmpeg.so
# include $(PREBUILT_SHARED_LIBRARY)

LOCAL_MODULE    := avcodec
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libavcodec.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := avformat
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libavformat.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := swscale
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libswscale.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_OUTPUT_PATH)/include
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := avutil
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libavutil.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := avfilter
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libavfilter.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := swresample
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libswresample.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)