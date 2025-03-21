#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/EventDispatcher.h"
#include "Base/FastNameMap.h"
#include "Sound/SoundEvent.h"
#include "FMODUtils.h"

namespace FMOD
{
class Event;
};

namespace DAVA
{
class FMODSoundSystem;

class FMODSoundEvent final : public SoundEvent
{
public:
    static FMOD_RESULT F_CALLBACK FMODEventCallback(FMOD_EVENT* event, FMOD_EVENT_CALLBACKTYPE type, void* param1, void* param2, void* userdata);

    virtual ~FMODSoundEvent();

    virtual bool IsActive() const;
    virtual bool Trigger();
    virtual void Stop(bool force = false);
    virtual void SetPaused(bool paused);

    virtual void SetVolume(float32 volume);

    virtual void SetSpeed(float32 speed);

    virtual void SetPosition(const Vector3& position);
    virtual void SetDirection(const Vector3& direction);
    virtual void UpdateInstancesPosition();
    virtual void SetVelocity(const Vector3& velocity);

    virtual void SetParameterValue(const FastName& paramName, float32 value);
    virtual float32 GetParameterValue(const FastName& paramName);
    virtual bool IsParameterExists(const FastName& paramName);

    virtual void GetEventParametersInfo(Vector<SoundEventParameterInfo>& paramsInfo) const;

    virtual String GetEventName() const;
    virtual float32 GetMaxDistance() const;

private:
    FMODSoundEvent(const FastName& eventName, FMODSoundSystem* rootSoundSystem);
    void ApplyParamsToEvent(FMOD::Event* event);
    void InitParamsMap();

    void PerformCallback(FMOD::Event* event);

    bool is3D;
    FastName eventName;
    Vector3 position;
    Vector3 direction;

    FastNameMap<float32> paramsValues;
    Vector<FMOD::Event*> fmodEventInstances;

    FMODSoundSystem* soundSystem = nullptr;

    friend class FMODSoundSystem;
};
};
