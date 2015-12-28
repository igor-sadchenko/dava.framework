/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_ANDROID_SCREENMANAGER_C_H__
#define __DAVAENGINE_ANDROID_SCREENMANAGER_C_H__

#include "DAVAEngine.h"

namespace DAVA
{
class UIScreenManager : public Singleton<UIScreenManager>
{
public:
    UIScreenManager();
    virtual ~UIScreenManager();

    void RegisterScreen(int screenId, UIScreen* screen);

    void SetFirst(int screenId);
    void SetScreen(int screenId, UIScreenTransition* transition = 0);

    UIScreen* GetScreen(int screenId);
    UIScreen* GetScreen();
    int32 GetScreenId();

    void ScreenSizeChanged();

private:
    void ActivateGLController();

    struct Screen
    {
        enum eType
        {
            TYPE_NULL = 0,
            TYPE_SCREEN,
        };
        Screen(eType _type = TYPE_NULL, void* _value = 0)
        {
            type = _type;
            value = _value;
        }
        eType type;
        void* value;
    };

    Map<int, Screen> screens;
    int glControllerId;
    int activeControllerId;
    int activeScreenId;
};
};

#endif // __DAVAENGINE_ANDROID_SCREENMANAGER_C_H__