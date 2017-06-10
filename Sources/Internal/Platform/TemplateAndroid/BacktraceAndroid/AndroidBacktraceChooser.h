#ifndef __DAVAENGINE_ANDROID_BACKTRACE_CHOOSER_H__
#define __DAVAENGINE_ANDROID_BACKTRACE_CHOOSER_H__

#if !defined(__DAVAENGINE_COREV2__)

#include "BacktraceCorkscrewImpl.h"

namespace DAVA
{
class AndroidBacktraceChooser
{
public:
    static BacktraceInterface* ChooseBacktraceAndroid();
    static void ReleaseBacktraceInterface();

private:
    static BacktraceInterface* backtraceProvider;
};
}

#endif // !__DAVAENGINE_COREV2__
#endif //__DAVAENGINE_ANDROID_BACKTRACE_CHOOSER_H__
