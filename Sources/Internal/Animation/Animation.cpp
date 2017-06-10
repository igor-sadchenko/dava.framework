#include "Animation/Animation.h"
#include "Animation/AnimationManager.h"
#include "Logger/Logger.h"

namespace DAVA
{
Animation::Animation(AnimatedObject* _owner, float32 _animationTimeLength, Interpolation::Func _interpolationFunc, int32 _defaultState)
{
    tagId = 0;
    owner = _owner;
    timeLength = _animationTimeLength;
    interpolationFunc = _interpolationFunc;
    state = _defaultState;
    next = 0;
    repeatCount = 0;
    timeMultiplier = 1.f;
    AnimationManager::Instance()->AddAnimation(this);
}

Animation::Animation(AnimatedObject* _owner, float32 _animationTimeLength, Interpolation::FuncType _interpolationFuncType, int32 _defaultState)
    : Animation(_owner, _animationTimeLength, Interpolation::GetFunction(_interpolationFuncType), _defaultState)
{
}

Animation::~Animation()
{
    AnimationManager::Instance()->RemoveAnimation(this);
}

void Animation::Reset()
{
    time = 0.0f;
    normalizedTime = 0.0f;
    next = 0;
}

void Animation::Start(int32 _groupId)
{
    //#ifdef ANIMATIONS_DEBUG
    //	if(AnimationManager::Instance()->IsAnimationLoggerEnabled())
    //	{
    //		Logger::FrameworkDebug("ANIMATION LOGGER: Animation::Start 0x%x    for owner 0x%x", (int)this, (int)owner);
    //	}
    //#endif
    //Logger::FrameworkDebug("Animation started: %d", _groupId);
    Reset();
    groupId = _groupId;

    Animation* prevAnimation = AnimationManager::Instance()->FindLastAnimation(owner, groupId);

    if (!prevAnimation || (prevAnimation == this))
    {
        //Logger::FrameworkDebug("real anim start");
        state |= STATE_IN_PROGRESS;
        OnStart();
    }
    else
    {
        //Logger::FrameworkDebug("add to queue");
        //#ifdef ANIMATIONS_DEBUG
        //		if(AnimationManager::Instance()->IsAnimationLoggerEnabled())
        //		{
        //			Logger::FrameworkDebug("ANIMATION LOGGER: Animation::Set animation 0x%x as next for 0x%x   for owner 0x%x", (int)this, (int)prevAnimation, (int)owner);
        //		}
        //#endif
        prevAnimation->next = this;
    }
}

void Animation::Stop()
{
    //Logger::FrameworkDebug("Animation stopped: %d", groupId);
    state &= ~STATE_IN_PROGRESS;
    state |= STATE_DELETE_ME;
    OnStop();
}

bool Animation::IsPlaying()
{
    return (state & STATE_IN_PROGRESS);
}

void Animation::Update(float32 timeElapsed)
{
    if (state & STATE_IN_PROGRESS)
    {
        if (state & STATE_PAUSED)
            return;

        if (state & STATE_REVERSE)
        {
            time += timeElapsed * timeMultiplier;

            float halfTimeLength = 0.5f * timeLength;
            if (time <= halfTimeLength)
            { // normal interpolation
                normalizedTime = interpolationFunc(time / halfTimeLength);
            }
            else
            { // reverse interpolation
                normalizedTime = interpolationFunc(2.0f - (time / halfTimeLength)); /*1.0f - ((time - halfTimeLength) / halfTimeLength)*/
            }

            if (time >= timeLength)
            {
                if (repeatCount == 0)
                {
                    time = timeLength;
                    normalizedTime = 0.0f;
                    state |= STATE_FINISHED;
                }
                else
                {
                    time -= timeLength;
                    // Do not decrement repeat counter for loop
                    if (repeatCount != INFINITE_LOOP)
                    {
                        repeatCount--;
                    }
                }
            }
        }
        else //
        {
            time += timeElapsed * timeMultiplier;
            normalizedTime = interpolationFunc(time / timeLength);
            if (time >= timeLength)
            {
                if (repeatCount == 0)
                {
                    time = timeLength;
                    normalizedTime = 1.0f;
                    state |= STATE_FINISHED;
                }
                else
                {
                    time -= timeLength;
                    // Do not decrement repeat counter for loop
                    if (repeatCount != INFINITE_LOOP)
                    {
                        repeatCount--;
                    }
                }
            }
        }
    }
}

void Animation::OnStart()
{
    //#ifdef ANIMATIONS_DEBUG
    //	if(AnimationManager::Instance()->IsAnimationLoggerEnabled())
    //	{
    //		Logger::FrameworkDebug("ANIMATION LOGGER: Animation::OnStart 0x%x    for owner 0x%x", (int)this, (int)owner);
    //	}
    //#endif
    PerformEvent(EVENT_ANIMATION_START);
};

void Animation::OnStop()
{
    //#ifdef ANIMATIONS_DEBUG
    //	if(AnimationManager::Instance()->IsAnimationLoggerEnabled())
    //	{
    //		Logger::FrameworkDebug("ANIMATION LOGGER: Animation::OnStop 0x%x    for owner 0x%x", (int)this, (int)owner);
    //	}
    //#endif
    PerformEvent(EVENT_ANIMATION_END);
    //#ifdef ANIMATIONS_DEBUG
    //	if(AnimationManager::Instance()->IsAnimationLoggerEnabled())
    //	{
    //		Logger::FrameworkDebug("ANIMATION LOGGER: Animation::OnStop DONE 0x%x    for owner 0x%x", (int)this, (int)owner);
    //	}
    //#endif
};

void Animation::OnCancel()
{
    PerformEvent(EVENT_ANIMATION_CANCELLED);
}

void Animation::Pause(bool _isPaused)
{
    if (_isPaused)
    {
        state |= STATE_PAUSED;
    }
    else
    {
        state &= ~STATE_PAUSED;
    }
}

bool Animation::IsPaused()
{
    return (0 != (state & STATE_PAUSED));
}

void Animation::SetRepeatCount(int32 _repeatCount)
{
    if (INFINITE_LOOP == _repeatCount)
    {
        repeatCount = _repeatCount;
    }
    else
    {
        repeatCount = _repeatCount - 1;
    }
}
}