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

#include "Core/Core.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/RHI/rhi_Type.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "GameCore.h"
#include "Version.h"

void FrameworkDidLaunched()
{
	DAVA::KeyedArchive * appOptions = new DAVA::KeyedArchive();

    appOptions->SetString("title", DAVA::Format("DAVA Framework - ResourceEditor | %s.%s", DAVAENGINE_VERSION, APPLICATION_BUILD_VERSION));

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetInt32("width", 1024);
    appOptions->SetInt32("height", 768);

    appOptions->SetInt32("max_index_buffer_count", 8192);
    appOptions->SetInt32("max_vertex_buffer_count", 8192);
    appOptions->SetInt32("max_const_buffer_count", 32767);
    appOptions->SetInt32("max_texture_count", 2048);

    appOptions->SetInt32("shader_const_buffer_size", 256 * 1024 * 1024);

    GameCore* core = new GameCore();
    DAVA::Core::SetApplicationCore(core);
    DAVA::Core::Instance()->SetOptions(appOptions);
    DAVA::VirtualCoordinatesSystem::Instance()->EnableReloadResourceOnResize(false);

//    DAVA::FilePath::SetBundleName("~/Sources/dava.framework/Tools/ResourceEditor/");
    
	SafeRelease(appOptions);
}


void FrameworkWillTerminate()
{ }
