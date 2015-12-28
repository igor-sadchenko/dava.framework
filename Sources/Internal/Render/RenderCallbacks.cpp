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


#include "RenderCallbacks.h"
#include "Concurrency/LockGuard.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace
{
Mutex callbackListMutex;
Vector<Function<void()>> resourceRestoreCallbacks;
Vector<Function<void()>> postRestoreCallbacks;

struct SyncCallback
{
    rhi::HSyncObject syncObject;
    Function<void(rhi::HSyncObject)> callback;
};
Vector<SyncCallback> syncCallbacks;

bool isInRestore = false;
}

namespace RenderCallbacks
{
void RegisterResourceRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> guard(callbackListMutex);
    resourceRestoreCallbacks.push_back(callback);
}
void UnRegisterResourceRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> guard(callbackListMutex);
    for (size_t i = 0, sz = resourceRestoreCallbacks.size(); i < sz; ++i)
    {
        if (resourceRestoreCallbacks[i].Target() == callback.Target())
        {
            RemoveExchangingWithLast(resourceRestoreCallbacks, i);
            return;
        }
    }
    DVASSERT_MSG(false, "trying to unregister callback that was not perviously registered");
}

void RegisterPostRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> guard(callbackListMutex);
    postRestoreCallbacks.push_back(callback);
}
void UnRegisterPostRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> guard(callbackListMutex);
    for (size_t i = 0, sz = postRestoreCallbacks.size(); i < sz; ++i)
    {
        if (postRestoreCallbacks[i].Target() == callback.Target())
        {
            RemoveExchangingWithLast(postRestoreCallbacks, i);
            return;
        }
    }
    DVASSERT_MSG(false, "trying to unregister callback that was not perviously registered");
}

void ProcessFrame()
{
    if (rhi::NeedRestoreResources())
    {
        isInRestore = true;
        LockGuard<Mutex> guard(callbackListMutex);
        for (auto& callback : resourceRestoreCallbacks)
        {
            callback();
        }
        Logger::Debug("Resources still need restore: ");
        rhi::NeedRestoreResources();
    }
    else
    {
        if (isInRestore)
        {
            isInRestore = false;
            LockGuard<Mutex> guard(callbackListMutex);
            for (auto& callback : postRestoreCallbacks)
            {
                callback();
            }
        }
    }

    for (size_t i = 0, sz = syncCallbacks.size(); i < sz;)
    {
        if (rhi::SyncObjectSignaled(syncCallbacks[i].syncObject))
        {
            syncCallbacks[i].callback(syncCallbacks[i].syncObject);
            RemoveExchangingWithLast(syncCallbacks, i);
            --sz;
        }
        else
        {
            ++i;
        }
    }
}

void RegisterSyncCallback(rhi::HSyncObject syncObject, Function<void(rhi::HSyncObject)> callback)
{
    syncCallbacks.push_back({ syncObject, callback });
}

void UnRegisterSyncCallback(Function<void(rhi::HSyncObject)> callback)
{
    for (size_t i = 0, sz = syncCallbacks.size(); i < sz;)
    {
        if (syncCallbacks[i].callback.Target() == callback.Target())
        {
            RemoveExchangingWithLast(syncCallbacks, i);
            --sz;
        }
        else
        {
            ++i;
        }
    }
}
}
}