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

#include "UI/UIControlSystem.h"
#include "UI/UIScreen.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "FileSystem/Logger.h"
#include "Render/OcclusionQuery.h"
#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"
#include "Debug/Replay.h"
#include "Debug/Stats.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "UI/Layouts/UILayoutSystem.h"
#include "Render/Renderer.h"
#include "Render/RenderHelper.h"
#include "UI/UIScreenshoter.h"
#include "Debug/Profiler.h"

namespace DAVA
{
const FastName FRAME_QUERY_UI_DRAW("OcclusionStatsUIDraw");

UIControlSystem::~UIControlSystem()
{
    SafeRelease(currentScreen);
    SafeRelease(popupContainer);
    SafeDelete(styleSheetSystem);
    SafeDelete(layoutSystem);
    SafeDelete(screenshoter);
}

UIControlSystem::UIControlSystem()
    : layoutSystem(nullptr)
    , clearColor(Color::Clear)
{
    screenLockCount = 0;
    frameSkip = 0;
    transitionType = 0;

    nextScreenTransition = 0;
    currentScreen = 0;
    nextScreen = 0;
    prevScreen = NULL;
    removeCurrentScreen = false;
    hovered = NULL;
    focusedControl = NULL;

    popupContainer = new UIControl(Rect(0, 0, 1, 1));
    popupContainer->SetName("UIControlSystem_popupContainer");
    popupContainer->SetInputEnabled(false);

    exclusiveInputLocker = NULL;

    lockInputCounter = 0;

    baseGeometricData.position = Vector2(0, 0);
    baseGeometricData.size = Vector2(0, 0);
    baseGeometricData.pivotPoint = Vector2(0, 0);
    baseGeometricData.scale = Vector2(1.0f, 1.0f);
    baseGeometricData.angle = 0;

    layoutSystem = new UILayoutSystem();
    styleSheetSystem = new UIStyleSheetSystem();
    screenshoter = new UIScreenshoter();
}

void UIControlSystem::SetScreen(UIScreen* _nextScreen, UIScreenTransition* _transition)
{
    if (_nextScreen == currentScreen)
    {
        if (nextScreen != 0)
        {
            SafeRelease(nextScreenTransition);
            SafeRelease(nextScreen);
        }
        return;
    }

    if (nextScreen)
    {
        Logger::Warning("2 screen switches during one frame.");
    }

    // 2 switches on one frame can cause memory leak
    SafeRelease(nextScreenTransition);
    SafeRelease(nextScreen);

    nextScreenTransition = SafeRetain(_transition);

    if (_nextScreen == 0)
    {
        removeCurrentScreen = true;
    }

    nextScreen = SafeRetain(_nextScreen);
}

void UIControlSystem::ReplaceScreen(UIScreen* newMainControl)
{
    prevScreen = currentScreen;
    currentScreen = newMainControl;
    NotifyListenersDidSwitch(currentScreen);
}

UIScreen* UIControlSystem::GetScreen()
{
    return currentScreen;
}

void UIControlSystem::AddPopup(UIPopup* newPopup)
{
    Set<UIPopup*>::const_iterator it = popupsToRemove.find(newPopup);
    if (popupsToRemove.end() != it)
    {
        popupsToRemove.erase(it);
        return;
    }

    newPopup->LoadGroup();
    popupContainer->AddControl(newPopup);
}

void UIControlSystem::RemovePopup(UIPopup* popup)
{
    if (popupsToRemove.count(popup))
    {
        Logger::Warning("[UIControlSystem::RemovePopup] attempt to double remove popup during one frame.");
        return;
    }

    const List<UIControl*>& popups = popupContainer->GetChildren();
    if (popups.end() == std::find(popups.begin(), popups.end(), DynamicTypeCheck<UIPopup*>(popup)))
    {
        Logger::Error("[UIControlSystem::RemovePopup] attempt to remove uknown popup.");
        DVASSERT(false);
        return;
    }

    popupsToRemove.insert(popup);
}

void UIControlSystem::RemoveAllPopups()
{
    popupsToRemove.clear();
    const List<UIControl*>& totalChilds = popupContainer->GetChildren();
    for (List<UIControl*>::const_iterator it = totalChilds.begin(); it != totalChilds.end(); it++)
    {
        popupsToRemove.insert(DynamicTypeCheck<UIPopup*>(*it));
    }
}

UIControl* UIControlSystem::GetPopupContainer()
{
    return popupContainer;
}

void UIControlSystem::Reset()
{
    SetScreen(0);
}

void UIControlSystem::ProcessScreenLogic()
{
    /*
	 if next screen or we need to removecurrent screen
	 */
    if (screenLockCount == 0 && (nextScreen || removeCurrentScreen))
    {
        UIScreen* nextScreenProcessed = 0;
        UIScreenTransition* transitionProcessed = 0;

        nextScreenProcessed = nextScreen;
        transitionProcessed = nextScreenTransition;
        nextScreen = 0; // functions called by this method can request another screen switch (for example, LoadResources)
        nextScreenTransition = 0;

        LockInput();

        CancelAllInputs();

        NotifyListenersWillSwitch(nextScreenProcessed);

        // If we have transition set
        if (transitionProcessed)
        {
            LockSwitch();

            // check if we have not loading transition
            if (!transitionProcessed->IsLoadingTransition())
            {
                // start transition and set currentScreen
                transitionProcessed->StartTransition(currentScreen, nextScreenProcessed);
                currentScreen = transitionProcessed;
            }
            else
            {
                // if we got loading transition
                UILoadingTransition* loadingTransition = dynamic_cast<UILoadingTransition*>(transitionProcessed);
                DVASSERT(loadingTransition);

                // Firstly start transition
                loadingTransition->StartTransition(currentScreen, nextScreenProcessed);

                // Manage transfer to loading transition through InTransition of LoadingTransition
                if (loadingTransition->GetInTransition())
                {
                    loadingTransition->GetInTransition()->StartTransition(currentScreen, loadingTransition);
                    currentScreen = SafeRetain(loadingTransition->GetInTransition());
                }
                else
                {
                    if (currentScreen)
                    {
                        if (currentScreen->IsOnScreen())
                            currentScreen->SystemWillBecomeInvisible();
                        currentScreen->SystemWillDisappear();
                        if ((nextScreenProcessed == 0) || (currentScreen->GetGroupId() != nextScreenProcessed->GetGroupId()))
                        {
                            currentScreen->UnloadGroup();
                        }
                        currentScreen->SystemDidDisappear();
                    }
                    // if we have next screen we load new resources, if it equal to zero we just remove screen
                    loadingTransition->LoadGroup();
                    loadingTransition->SystemWillAppear();
                    currentScreen = loadingTransition;
                    loadingTransition->SystemDidAppear();
                    if (loadingTransition->IsOnScreen())
                        loadingTransition->SystemWillBecomeVisible();
                }
            }
        }
        else // if there is no transition do change immediatelly
        {
            // if we have current screen we call events, unload resources for it group
            if (currentScreen)
            {
                if (currentScreen->IsOnScreen())
                    currentScreen->SystemWillBecomeInvisible();
                currentScreen->SystemWillDisappear();
                if ((nextScreenProcessed == 0) || (currentScreen->GetGroupId() != nextScreenProcessed->GetGroupId()))
                {
                    currentScreen->UnloadGroup();
                }
                currentScreen->SystemDidDisappear();
            }
            // if we have next screen we load new resources, if it equal to zero we just remove screen
            if (nextScreenProcessed)
            {
                nextScreenProcessed->LoadGroup();
                nextScreenProcessed->SystemWillAppear();
            }
            currentScreen = nextScreenProcessed;
            NotifyListenersDidSwitch(currentScreen);
            if (nextScreenProcessed)
            {
                nextScreenProcessed->SystemDidAppear();
                if (nextScreenProcessed->IsOnScreen())
                    nextScreenProcessed->SystemWillBecomeVisible();
            }

            UnlockInput();
        }
        frameSkip = FRAME_SKIP;
        removeCurrentScreen = false;
    }

    /*
	 if we have popups to remove, we removes them here
	 */
    for (Set<UIPopup*>::iterator it = popupsToRemove.begin(); it != popupsToRemove.end(); it++)
    {
        UIPopup* p = *it;
        if (p)
        {
            p->Retain();
            popupContainer->RemoveControl(p);
            p->UnloadGroup();
            p->Release();
        }
    }
    popupsToRemove.clear();
}

void UIControlSystem::Update()
{
    TIME_PROFILE("UIControlSystem::Update");

    updateCounter = 0;
    ProcessScreenLogic();

    float32 timeElapsed = SystemTimer::FrameDelta();

    if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_UI_CONTROL_SYSTEM))
    {
        if (currentScreen)
        {
            currentScreen->SystemUpdate(timeElapsed);
        }

        popupContainer->SystemUpdate(timeElapsed);
    }

    SafeRelease(prevScreen);
    //Logger::Info("UIControlSystem::updates: %d", updateCounter);
}

void UIControlSystem::Draw()
{
    TIME_PROFILE("UIControlSystem::Draw");

    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "UIControlSystem::Draw")

    FrameOcclusionQueryManager::Instance()->BeginQuery(FRAME_QUERY_UI_DRAW);

    drawCounter = 0;

    if (useClearPass)
    {
        rhi::Viewport viewport;
        viewport.x = viewport.y = 0U;
        viewport.width = (uint32)Renderer::GetFramebufferWidth();
        viewport.height = (uint32)Renderer::GetFramebufferHeight();
        RenderHelper::CreateClearPass(rhi::HTexture(), PRIORITY_CLEAR, clearColor, viewport);
    }

    if (currentScreen)
    {
        currentScreen->SystemDraw(baseGeometricData);
    }

    popupContainer->SystemDraw(baseGeometricData);

    if (frameSkip > 0)
    {
        frameSkip--;
    }
    //Logger::Info("UIControlSystem::draws: %d", drawCounter);

    FrameOcclusionQueryManager::Instance()->EndQuery(FRAME_QUERY_UI_DRAW);

    GetScreenshoter()->OnFrame();

    TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "UIControlSystem::Draw")
}

void UIControlSystem::SwitchInputToControl(int32 eventID, UIControl* targetControl)
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        if ((*it).tid == eventID)
        {
            CancelInput(&(*it));

            if (targetControl->IsPointInside((*it).point))
            {
                (*it).controlState = UIEvent::CONTROL_STATE_INSIDE;
                targetControl->touchesInside++;
            }
            else
            {
                (*it).controlState = UIEvent::CONTROL_STATE_OUTSIDE;
            }
            (*it).touchLocker = targetControl;
            targetControl->currentInputID = eventID;
            if (targetControl->GetExclusiveInput())
            {
                SetExclusiveInputLocker(targetControl, eventID);
            }
            else
            {
                SetExclusiveInputLocker(NULL, -1);
            }

            targetControl->totalTouches++;
        }
    }
}

void UIControlSystem::OnInput(UIEvent* newEvent)
{
    inputCounter = 0;

    newEvent->point = VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(newEvent->physPoint);

    if (Replay::IsPlayback())
    {
        return;
    }

    if (lockInputCounter > 0)
    {
        return;
    }

    if (frameSkip <= 0)
    {
        if (Replay::IsRecord())
        {
            Replay::Instance()->RecordEvent(newEvent);
        }

        UIEvent* eventToHandle = nullptr;

        if (newEvent->phase == UIEvent::Phase::BEGAN || newEvent->phase == UIEvent::Phase::DRAG || newEvent->phase == UIEvent::Phase::ENDED || newEvent->phase == UIEvent::Phase::CANCELLED)
        {
            auto it = std::find_if(begin(touchEvents), end(touchEvents), [newEvent](const UIEvent& ev) {
                return ev.tid == newEvent->tid;
            });
            if (it == end(touchEvents))
            {
                touchEvents.push_back(*newEvent);
                eventToHandle = &touchEvents.back();
            }
            else
            {
                it->timestamp = newEvent->timestamp;
                it->physPoint = newEvent->physPoint;
                it->point = newEvent->point;
                it->tapCount = newEvent->tapCount;
                it->phase = newEvent->phase;
                it->inputHandledType = newEvent->inputHandledType;

                eventToHandle = &(*it);
            }
        }
        else
        {
            eventToHandle = newEvent;
        }

        if (currentScreen)
        {
            if (!popupContainer->SystemInput(eventToHandle))
            {
                currentScreen->SystemInput(eventToHandle);
            }
        }

        auto startRemoveIt = std::remove_if(begin(touchEvents), end(touchEvents), [this](UIEvent& ev) {
            bool shouldRemove = (ev.phase == UIEvent::Phase::ENDED || ev.phase == UIEvent::Phase::CANCELLED);
            if (shouldRemove)
            {
                CancelInput(&ev);
            }
            return shouldRemove;
        });
        touchEvents.erase(startRemoveIt, end(touchEvents));
    } // end if frameSkip <= 0
}

void UIControlSystem::CancelInput(UIEvent* touch)
{
    if (touch->touchLocker)
    {
        touch->touchLocker->SystemInputCancelled(touch);
    }
    if (touch->touchLocker != currentScreen)
    {
        currentScreen->SystemInputCancelled(touch);
    }
}
void UIControlSystem::CancelAllInputs()
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        CancelInput(&(*it));
    }
    touchEvents.clear();
}

void UIControlSystem::CancelInputs(UIControl* control, bool hierarchical)
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        if (!hierarchical)
        {
            if (it->touchLocker == control)
            {
                CancelInput(&(*it));
                break;
            }
            continue;
        }
        UIControl* parentLockerControl = it->touchLocker;
        while (parentLockerControl)
        {
            if (control == parentLockerControl)
            {
                CancelInput(&(*it));
                break;
            }
            parentLockerControl = parentLockerControl->GetParent();
        }
    }
}

int32 UIControlSystem::LockInput()
{
    lockInputCounter++;
    if (lockInputCounter > 0)
    {
        CancelAllInputs();
    }
    return lockInputCounter;
}

int32 UIControlSystem::UnlockInput()
{
    DVASSERT(lockInputCounter != 0);

    lockInputCounter--;
    if (lockInputCounter == 0)
    {
        // VB: Done that because hottych asked to do that.
        CancelAllInputs();
    }
    return lockInputCounter;
}

int32 UIControlSystem::GetLockInputCounter() const
{
    return lockInputCounter;
}

const Vector<UIEvent>& UIControlSystem::GetAllInputs()
{
    return touchEvents;
}

void UIControlSystem::SetExclusiveInputLocker(UIControl* locker, int32 lockEventId)
{
    SafeRelease(exclusiveInputLocker);
    if (locker != NULL)
    {
        for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
        {
            if (it->tid != lockEventId && it->touchLocker != locker)
            { //cancel all inputs excepts current input and inputs what allready handles by this locker.
                CancelInput(&(*it));
            }
        }
    }

    exclusiveInputLocker = SafeRetain(locker);
}

UIControl* UIControlSystem::GetExclusiveInputLocker()
{
    return exclusiveInputLocker;
}

void UIControlSystem::ScreenSizeChanged()
{
    popupContainer->SystemScreenSizeDidChanged(VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect());
}

void UIControlSystem::SetHoveredControl(UIControl* newHovered)
{
    if (hovered != newHovered)
    {
        if (hovered)
        {
            hovered->SystemDidRemoveHovered();
            hovered->Release();
        }
        hovered = SafeRetain(newHovered);
        if (hovered)
        {
            hovered->SystemDidSetHovered();
        }
    }
}

UIControl* UIControlSystem::GetHoveredControl(UIControl* newHovered)
{
    return hovered;
}

void UIControlSystem::SetFocusedControl(UIControl* newFocused, bool forceSet)
{
    if (focusedControl)
    {
        if (forceSet || focusedControl->IsLostFocusAllowed(newFocused))
        {
            focusedControl->SystemOnFocusLost(newFocused);
            SafeRelease(focusedControl);
            focusedControl = SafeRetain(newFocused);
            if (focusedControl)
            {
                focusedControl->SystemOnFocused();
            }
        }
    }
    else
    {
        focusedControl = SafeRetain(newFocused);
        if (focusedControl)
        {
            focusedControl->SystemOnFocused();
        }
    }
}

UIControl* UIControlSystem::GetFocusedControl()
{
    return focusedControl;
}

const UIGeometricData& UIControlSystem::GetBaseGeometricData() const
{
    return baseGeometricData;
}

void UIControlSystem::ReplayEvents()
{
    while (Replay::Instance()->IsEvent())
    {
        int32 activeCount = Replay::Instance()->PlayEventsCount();
        while (activeCount--)
        {
            UIEvent ev = Replay::Instance()->PlayEvent();
            OnInput(&ev);
        }
    }
}

int32 UIControlSystem::LockSwitch()
{
    screenLockCount++;
    return screenLockCount;
}

int32 UIControlSystem::UnlockSwitch()
{
    screenLockCount--;
    DVASSERT(screenLockCount >= 0);
    return screenLockCount;
}

void UIControlSystem::AddScreenSwitchListener(ScreenSwitchListener* listener)
{
    screenSwitchListeners.push_back(listener);
}

void UIControlSystem::RemoveScreenSwitchListener(ScreenSwitchListener* listener)
{
    Vector<ScreenSwitchListener*>::iterator it = std::find(screenSwitchListeners.begin(), screenSwitchListeners.end(), listener);
    if (it != screenSwitchListeners.end())
        screenSwitchListeners.erase(it);
}

void UIControlSystem::NotifyListenersWillSwitch(UIScreen* screen)
{
    Vector<ScreenSwitchListener*> screenSwitchListenersCopy = screenSwitchListeners;
    uint32 listenersCount = (uint32)screenSwitchListenersCopy.size();
    for (uint32 i = 0; i < listenersCount; ++i)
        screenSwitchListenersCopy[i]->OnScreenWillSwitch(screen);
}

void UIControlSystem::NotifyListenersDidSwitch(UIScreen* screen)
{
    Vector<ScreenSwitchListener*> screenSwitchListenersCopy = screenSwitchListeners;
    uint32 listenersCount = (uint32)screenSwitchListenersCopy.size();
    for (uint32 i = 0; i < listenersCount; ++i)
        screenSwitchListenersCopy[i]->OnScreenDidSwitch(screen);
}

bool UIControlSystem::IsRtl() const
{
    return layoutSystem->IsRtl();
}

void UIControlSystem::SetRtl(bool rtl)
{
    layoutSystem->SetRtl(rtl);
}

UILayoutSystem* UIControlSystem::GetLayoutSystem() const
{
    return layoutSystem;
}

UIStyleSheetSystem* UIControlSystem::GetStyleSheetSystem() const
{
    return styleSheetSystem;
}

UIScreenshoter* UIControlSystem::GetScreenshoter()
{
    return screenshoter;
}

void UIControlSystem::SetClearColor(const DAVA::Color& _clearColor)
{
    clearColor = _clearColor;
}

void UIControlSystem::SetUseClearPass(bool use)
{
    useClearPass = use;
}
};
