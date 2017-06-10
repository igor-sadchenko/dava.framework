#ifndef __DAVAENGINE_ANIMATION_H__
#define __DAVAENGINE_ANIMATION_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/EventDispatcher.h"
#include "Base/AllocatorFactory.h"
#include "Animation/Interpolation.h"
#include "Animation/AnimatedObject.h"

namespace DAVA
{
class AnimationManager;

/**
	\ingroup animationsystem
	\brief Animation is a base class that helps to handle animation process inside AnimationManager. 
	All animations inside the system derived from this class. You cannot use the Animation class directly to instantiate animations. 
	It instead defines the common interface and behavioral structure for all its subclasses.
 */
class Animation : public EventDispatcher
{
public:
    /*
 */
    static const int32 INFINITE_LOOP = -1;

    enum
    {
        EVENT_ANIMATION_START = 1,
        EVENT_ANIMATION_END,
        EVENT_ANIMATION_CANCELLED,
    };

    /*
		animationState is binary flags that define animation state & it params
	 */
    enum
    {
        STATE_STOPPED = 0,
        STATE_IN_PROGRESS = 1 << 0,
        STATE_DELETE_ME = 1 << 1, // flag is set if animation is marked for deletion
        STATE_REVERSE = 1 << 2,
        STATE_FINISHED = 1 << 3,
        STATE_PAUSED = 1 << 4
    };

protected:
    virtual ~Animation();

public:
    Animation(AnimatedObject* _owner, float32 _animationTimeLength, Interpolation::Func _interpolationFunc, int32 _defaultState = STATE_STOPPED);
    Animation(AnimatedObject* _owner, float32 _animationTimeLength, Interpolation::FuncType _interpolationFuncType, int32 _defaultState = STATE_STOPPED);

    virtual void Reset();
    virtual void Start(int32 _groupId);
    virtual void Stop();
    virtual void Pause(bool isPaused);

    bool IsPaused();
    bool IsPlaying();

    virtual void Update(float32 timeElapsed);

    // animation virtual events
    virtual void OnStart();
    virtual void OnStop();
    virtual void OnCancel();

    inline void EnableReverse();
    void SetRepeatCount(int32 k);

    inline void SetTagId(int32 tag);
    inline int32 GetTagId();
    inline void SetTimeMultiplier(float32 m);
    inline float32 GetTimeMultiplier();
    inline AnimatedObject* GetOwner();

protected:
    int32 state;
    float32 time; // [0, animationTimeLength]
    float32 timeLength; // length of animation in seconds
    float32 normalizedTime; // [0, 1];
    Interpolation::Func interpolationFunc;
    AnimatedObject* owner;

    Animation* next;
    int32 groupId; //	animation group id to group animations one after another
    int32 repeatCount;

    int32 tagId; // tag animations with numbers
    float32 timeMultiplier;

    friend class AnimationManager;
};

inline void Animation::EnableReverse()
{
    state |= STATE_REVERSE;
}

inline void Animation::SetTagId(int32 tag)
{
    tagId = tag;
}

inline int32 Animation::GetTagId()
{
    return tagId;
}

inline void Animation::SetTimeMultiplier(float32 m)
{
    timeMultiplier = m;
}

inline float32 Animation::GetTimeMultiplier()
{
    return timeMultiplier;
}

inline AnimatedObject* Animation::GetOwner()
{
    return owner;
}
};


#endif // __DAVAENGINE_ANIMATION_STATE_H__