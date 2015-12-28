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


#ifndef __COLLADA_TO_SC2_IMPORTER_H__
#define __COLLADA_TO_SC2_IMPORTER_H__

#include "Collada/ColladaToSc2Importer/ImportLibrary.h"
#include "Collada/ColladaErrorCodes.h"

namespace DAVA
{

class Entity;
class ColladaSceneNode;
class ImportLibrary;

class ColladaToSc2Importer
{
public:
    eColladaErrorCodes SaveSC2(ColladaScene* colladaScene, const FilePath& scenePath);

private:
    void ImportAnimation(ColladaSceneNode * colladaNode, Entity * nodeEntity);
    void LoadMaterialParents(ColladaScene * colladaScene);
    void LoadAnimations(ColladaScene * colladaScene);
    eColladaErrorCodes VerifyDavaMesh(RenderObject* mesh, const FastName name);
    eColladaErrorCodes ImportMeshes(const Vector<ColladaMeshInstance*>& meshInstances, Entity* node);
    eColladaErrorCodes BuildSceneAsCollada(Entity* root, ColladaSceneNode* colladaNode);
    Mesh * GetMeshFromCollada(ColladaMeshInstance * mesh, const bool isShadow);


    ImportLibrary library;

    void ReportError(const String& errMessage);
    Set<String> errorLogs;
};

};

#endif 