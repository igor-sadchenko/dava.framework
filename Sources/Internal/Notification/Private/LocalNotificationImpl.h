#pragma once

namespace DAVA
{

#include "Base/BaseTypes.h"

class LocalNotificationImpl
{
public:
    virtual ~LocalNotificationImpl(){};

    // TODO: Remove this method, after transition on Core V2.
    virtual void SetAction(const WideString& action) = 0;
    virtual void Hide() = 0;
    virtual void ShowText(const WideString& title, const WideString& text, bool useSound) = 0;
    virtual void ShowProgress(const WideString& title, const WideString& text, uint32 total, uint32 progress, bool useSound) = 0;
    virtual void PostDelayedNotification(const WideString& title, const WideString& text, int delaySeconds, bool useSound) = 0;
    virtual void RemoveAllDelayedNotifications() = 0;

    static LocalNotificationImpl* Create(const String& _id);
    static void RequestPermissions();

    const DAVA::String& GetId()
    {
        return notificationId;
    }

protected:
    DAVA::String notificationId;
};
} // namespace DAVA
