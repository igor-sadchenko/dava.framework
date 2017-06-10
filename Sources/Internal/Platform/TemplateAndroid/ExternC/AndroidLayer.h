#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "davaFrameworkLog", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "davaFrameworkLog", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "davaFrameworkLog", __VA_ARGS__)

#endif // __DAVAENGINE_ANDROID__
