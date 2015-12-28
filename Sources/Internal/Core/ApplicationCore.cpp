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


#include "Core/ApplicationCore.h"
#include "Animation/AnimationManager.h"
#include "UI/UIControlSystem.h"
#include "Render/OcclusionQuery.h"
#include "Sound/SoundSystem.h"
#include "Debug/Stats.h"
#include "Platform/SystemTimer.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Notification/LocalNotificationController.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Debug/Profiler.h"
#include "Concurrency/Thread.h"
#ifdef __DAVAENGINE_AUTOTESTING__
#include "Autotesting/AutotestingSystem.h"
#endif

namespace DAVA
{

ApplicationCore::ApplicationCore()
	: BaseObject()
#if defined(__DAVAENGINE_ANDROID__)
	, backgroundTicker(NULL)
	, backgroundTickerFinishing(false)
	, backgroundTickTimeMs(250)
#endif
{

}

ApplicationCore::~ApplicationCore()
{

}
	
void ApplicationCore::Update(float32 timeElapsed)
{
	TIME_PROFILE("ApplicationCore::Update");
#ifdef __DAVAENGINE_AUTOTESTING__
    AutotestingSystem::Instance()->Update(timeElapsed);
#endif
    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "SoundSystem::Update")
    SoundSystem::Instance()->Update(timeElapsed);
    TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "SoundSystem::Update")

    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "AnimationManager::Update")
    AnimationManager::Instance()->Update(timeElapsed);
    TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "AnimationManager::Update")

    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "UIControlSystem::Update")
    UIControlSystem::Instance()->Update();
    TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "UIControlSystem::Update")
}
    
void ApplicationCore::OnEnterFullscreen()
{ }

void ApplicationCore::OnExitFullscreen()
{
}

void ApplicationCore::Draw()
{
	TIME_PROFILE("ApplicationCore::Draw");

    FrameOcclusionQueryManager::Instance()->ResetFrameStats();
    UIControlSystem::Instance()->Draw();
#ifdef __DAVAENGINE_AUTOTESTING__
    AutotestingSystem::Instance()->Draw();
#endif
    FrameOcclusionQueryManager::Instance()->ProccesRenderedFrame();
}

void ApplicationCore::BeginFrame()
{
    Renderer::BeginFrame();
    RenderSystem2D::Instance()->BeginFrame();
}

void ApplicationCore::EndFrame()
{
    RenderSystem2D::Instance()->EndFrame();
    Renderer::EndFrame();
    //RenderManager::Instance()->ProcessStats();
}

void ApplicationCore::OnSuspend()
{
	SoundSystem::Instance()->Suspend();
	Core::Instance()->SetIsActive(false);

#if defined(__DAVAENGINE_ANDROID__)
	StartBackbroundTicker();
#endif
}

void ApplicationCore::OnResume()
{
#if defined(__DAVAENGINE_ANDROID__)
	StopBackgroundTicker();
#endif

	Core::Instance()->SetIsActive(true);
	SoundSystem::Instance()->Resume();
}

#if defined(__DAVAENGINE_ANDROID__)

void ApplicationCore::StartBackbroundTicker(uint32 tickPeriod)
{
	if (NULL == backgroundTicker)
	{
		Logger::Debug("[ApplicationCore: OnSuspend] Background tick Thread Create Start");
		backgroundTicker = Thread::Create(Message(this, &ApplicationCore::BackgroundTickerHandler));
		backgroundTickerFinishing = false;
		if (backgroundTicker)
		{
			backgroundTickTimeMs = tickPeriod;
			backgroundTicker->Start();
		}
		Logger::Debug("[ApplicationCore: OnSuspend] Background tick  Thread Create End");
	}
}

void ApplicationCore::StopBackgroundTicker()
{
	if (NULL != backgroundTicker)
	{
		Logger::Debug("[ApplicationCore: OnResume] Background tick Thread Finish start");
		backgroundTickerFinishing = true;
		backgroundTicker->Join();
		SafeRelease(backgroundTicker);
		Logger::Debug("[ApplicationCore: OnResume] Background tick Thread Finish end");
	}
}

void ApplicationCore::BackgroundTickerHandler(BaseObject * caller, void * callerData, void * userData)
{
	while(!backgroundTickerFinishing)
	{
		Thread::Sleep(backgroundTickTimeMs);
		OnBackgroundTick();
	}
}
#endif
    
void ApplicationCore::OnBackgroundTick()
{
	DownloadManager::Instance()->Update();
	LocalNotificationController::Instance()->Update();
}
    
bool ApplicationCore::OnQuit()
{
	return false;
}

void ApplicationCore::OnAppFinished()
{
    // Default implementation is empty.
}

void ApplicationCore::OnBackground()
{
    // Default implementation is empty.
}

void ApplicationCore::OnForeground()
{
	// Default implementation is empty.
}

void ApplicationCore::OnDeviceLocked()
{
    // Default implementation is empty.
}

void ApplicationCore::OnFocusLost()
{
    // Default implementation is empty.
}

void ApplicationCore::OnFocusReceived()
{
    // Default implementation is empty.
}
};
