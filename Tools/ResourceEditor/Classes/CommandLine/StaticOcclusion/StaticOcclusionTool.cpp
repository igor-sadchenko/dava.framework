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

#include "CommandLine/StaticOcclusion/StaticOcclusionTool.h"
#include "CommandLine/SceneUtils/SceneUtils.h"
#include "Scene/SceneEditor2.h"
#include "CommandLine/OptionName.h"

using namespace DAVA;

StaticOcclusionTool::StaticOcclusionTool()
    : CommandLineTool("-staticocclusion")
{
    options.AddOption(OptionName::Build, VariantType(false), "Enables build of static occlusion");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Full pathname to scene file *.sc2");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
}

void StaticOcclusionTool::ConvertOptionsToParamsInternal()
{
    if (options.GetOption(OptionName::Build).AsBool())
    {
        commandAction = ACTION_BUILD;
    }

    scenePathname = options.GetOption(OptionName::ProcessFile).AsString();
    qualityConfigPath = options.GetOption(OptionName::QualityConfig).AsString();
}

bool StaticOcclusionTool::InitializeInternal()
{
    if (commandAction == ACTION_NONE)
    {
        AddError("Wrong action was selected");
        return false;
    }

    if (scenePathname.IsEmpty())
    {
        AddError("Filename was not set");
        return false;
    }

    return true;
}

void StaticOcclusionTool::ProcessInternal()
{
    const rhi::HTexture nullTexture;
    const rhi::Viewport nullViewport(0, 0, 1, 1);

    if (commandAction == ACTION_BUILD)
    {
        ScopedPtr<SceneEditor2> scene(new SceneEditor2());
        if (scene->Load(scenePathname))
        {
            scene->Update(0.1f); // we need to call update to initialize (at least) QuadTree.
            scene->staticOcclusionBuildSystem->Build();
            RenderObjectsFlusher::Flush();

            while (scene->staticOcclusionBuildSystem->IsInBuild())
            {
                Renderer::BeginFrame();
                RenderHelper::CreateClearPass(nullTexture, 0, DAVA::Color::Clear, nullViewport);
                scene->Update(0.1f);
                Renderer::EndFrame();
            }

            scene->Save();
        }
        RenderObjectsFlusher::Flush();
    }
}

DAVA::FilePath StaticOcclusionTool::GetQualityConfigPath() const
{
    if (qualityConfigPath.IsEmpty())
    {
        return CreateQualityConfigPath(scenePathname);
    }

    return qualityConfigPath;
}

