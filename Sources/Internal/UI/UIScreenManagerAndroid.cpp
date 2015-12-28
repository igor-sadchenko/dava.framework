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

#include "Base/Platform.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Base/BaseObject.h"
#include "UI/UIScreenManager.h"

namespace DAVA
{
UIScreenManager::UIScreenManager()
{
    glControllerId = -1;
    activeControllerId = -1;
    activeScreenId = -1;
}

UIScreenManager::~UIScreenManager()
{
    Vector<Screen> releaseBuf;
    for (Map<int, Screen>::const_iterator it = screens.begin(); it != screens.end(); it++)
    {
        if (it->second.type == Screen::TYPE_SCREEN)
        {
            ((UIScreen*)it->second.value)->UnloadGroup();
            //			it->second.type == Screen::TYPE_NULL;
            releaseBuf.push_back(it->second);
        }
    }
    for (Vector<Screen>::const_iterator it = releaseBuf.begin(); it != releaseBuf.end(); it++)
    {
        ((UIScreen*)it->value)->Release();
    }
}

void UIScreenManager::SetFirst(int screenId)
{
    DVASSERT(activeScreenId == -1 && "[UIScreenManager::SetFirst] called twice");

    Screen& screen = screens[screenId];
    if (screen.type == Screen::TYPE_SCREEN)
    {
        activeScreenId = screenId;
        UIControlSystem::Instance()->SetScreen((UIScreen*)screen.value);
    }
    else
    {
        Logger::Error("[UIScreenManager::SetFirst] wrong type of screen");
    }
    Logger::Debug("[UIScreenManager::SetFirst] done");
}

void UIScreenManager::SetScreen(int screenId, UIScreenTransition* transition)
{
    Logger::Debug("[ScreenManager::SetScreen] screenID = %d", screenId);

    Screen& screen = screens[screenId];
    if (screen.type == Screen::TYPE_SCREEN)
    {
        activeScreenId = screenId;
        UIControlSystem::Instance()->SetScreen((UIScreen*)screen.value, transition);
    }

    Logger::Debug("[ScreenManager::SetScreen] done");
}

void UIScreenManager::RegisterScreen(int screenId, UIScreen* screen)
{
    screens[screenId] = Screen(Screen::TYPE_SCREEN, SafeRetain(screen));
}

UIScreen* UIScreenManager::GetScreen(int screenId)
{
    Screen& screen = screens[screenId];
    if (screen.type == Screen::TYPE_SCREEN)
    {
        return (UIScreen*)screen.value;
    }
    return NULL;
}

UIScreen* UIScreenManager::GetScreen()
{
    return GetScreen(activeScreenId);
}
int32 UIScreenManager::GetScreenId()
{
    return activeScreenId;
}

void UIScreenManager::ScreenSizeChanged()
{
    GetScreen()->SystemScreenSizeDidChanged(VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect());
}

/*void ScreenManager::StopGLAnimation()
{
	Screen & glController = screens[glControllerId];
	EAGLViewController * controller = (EAGLViewController *)glController.value;
	EAGLView * view = (EAGLView *)controller.view;
	[view stopAnimation];
}

void ScreenManager::StartGLAnimation()
{
	Screen & glController = screens[glControllerId];
	EAGLViewController * controller = (EAGLViewController *)glController.value;
	EAGLView * view = (EAGLView *)controller.view;
	[view startAnimation];
}*/
};

#endif // __DAVAENGINE_ANDROID__
