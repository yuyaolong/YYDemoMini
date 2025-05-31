#pragma once
#include "android/log.h"

#ifndef LOG_TAG
#define LOG_TAG "YYALDemo"
#endif

#define GR_DEBUG_LOG_ON 1

#ifdef GR_DEBUG_LOG_ON
#define ALOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
    #define ALOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
    #define ALOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
    #define ALOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#endif
#ifndef GR_DEBUG_LOG_ON
#define ALOGI(...)
#define ALOGD(...)
#define ALOGW(...)
#define ALOGV(...)
#endif

#define ALOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGM(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)