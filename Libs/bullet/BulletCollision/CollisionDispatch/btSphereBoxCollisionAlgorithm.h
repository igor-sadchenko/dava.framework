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

/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BT_SPHERE_BOX_COLLISION_ALGORITHM_H
#define BT_SPHERE_BOX_COLLISION_ALGORITHM_H

#include "btActivatingCollisionAlgorithm.h"
#include "BulletCollision/BroadphaseCollision/btBroadphaseProxy.h"
#include "BulletCollision/CollisionDispatch/btCollisionCreateFunc.h"
class btPersistentManifold;
#include "btCollisionDispatcher.h"

#include "LinearMath/btVector3.h"

/// btSphereBoxCollisionAlgorithm  provides sphere-box collision detection.
/// Other features are frame-coherency (persistent data) and collision response.
class btSphereBoxCollisionAlgorithm : public btActivatingCollisionAlgorithm
{
    bool m_ownManifold;
    btPersistentManifold* m_manifoldPtr;
    bool m_isSwapped;

public:
    btSphereBoxCollisionAlgorithm(btPersistentManifold* mf, const btCollisionAlgorithmConstructionInfo& ci, btCollisionObject* col0, btCollisionObject* col1, bool isSwapped);

    virtual ~btSphereBoxCollisionAlgorithm();

    virtual void processCollision(btCollisionObject* body0, btCollisionObject* body1, const btDispatcherInfo& dispatchInfo, btManifoldResult* resultOut);

    virtual btScalar calculateTimeOfImpact(btCollisionObject* body0, btCollisionObject* body1, const btDispatcherInfo& dispatchInfo, btManifoldResult* resultOut);

    virtual void getAllContactManifolds(btManifoldArray& manifoldArray)
    {
        if (m_manifoldPtr && m_ownManifold)
        {
            manifoldArray.push_back(m_manifoldPtr);
        }
    }

    btScalar getSphereDistance(btCollisionObject* boxObj, btVector3& v3PointOnBox, btVector3& v3PointOnSphere, const btVector3& v3SphereCenter, btScalar fRadius);

    btScalar getSpherePenetration(btCollisionObject* boxObj, btVector3& v3PointOnBox, btVector3& v3PointOnSphere, const btVector3& v3SphereCenter, btScalar fRadius, const btVector3& aabbMin, const btVector3& aabbMax);

    struct CreateFunc : public btCollisionAlgorithmCreateFunc
    {
        virtual btCollisionAlgorithm* CreateCollisionAlgorithm(btCollisionAlgorithmConstructionInfo& ci, btCollisionObject* body0, btCollisionObject* body1)
        {
            void* mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(btSphereBoxCollisionAlgorithm));
            if (!m_swapped)
            {
                return new (mem) btSphereBoxCollisionAlgorithm(0, ci, body0, body1, false);
            }
            else
            {
                return new (mem) btSphereBoxCollisionAlgorithm(0, ci, body0, body1, true);
            }
        }
    };
};

#endif //BT_SPHERE_BOX_COLLISION_ALGORITHM_H
