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

#include "UILayoutSystem.h"

#include "UILinearLayoutComponent.h"
#include "UIFlowLayoutComponent.h"
#include "UIAnchorComponent.h"
#include "UISizePolicyComponent.h"

#include "SizeMeasuringAlgorithm.h"
#include "LinearLayoutAlgorithm.h"
#include "FlowLayoutAlgorithm.h"
#include "AnchorLayoutAlgorithm.h"

#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

#include "Concurrency/Thread.h"

namespace DAVA
{
UILayoutSystem::UILayoutSystem()
{
}

UILayoutSystem::~UILayoutSystem()
{
}

bool UILayoutSystem::IsRtl() const
{
    return isRtl;
}

void UILayoutSystem::SetRtl(bool rtl)
{
    isRtl = rtl;
}

bool UILayoutSystem::IsAutoupdatesEnabled() const
{
    return autoupdatesEnabled;
}

void UILayoutSystem::SetAutoupdatesEnabled(bool enabled)
{
    autoupdatesEnabled = enabled;
}

void UILayoutSystem::ApplyLayout(UIControl* control, bool considerDenendenceOnChildren)
{
    DVASSERT(Thread::IsMainThread() || autoupdatesEnabled == false);

    UIControl* container = control;
    if (considerDenendenceOnChildren)
    {
        container = FindNotDependentOnChildrenControl(container);
    }

    CollectControls(container, true);

    ProcessAxis(Vector2::AXIS_X);
    ProcessAxis(Vector2::AXIS_Y);

    ApplySizesAndPositions();

    layoutData.clear();
}

void UILayoutSystem::ApplyLayoutNonRecursive(UIControl* control)
{
    DVASSERT(Thread::IsMainThread() || autoupdatesEnabled == false);

    CollectControls(control, false);

    ProcessAxis(Vector2::AXIS_X);
    ProcessAxis(Vector2::AXIS_Y);

    ApplyPositions();

    layoutData.clear();
}

UIControl* UILayoutSystem::FindNotDependentOnChildrenControl(UIControl* control) const
{
    UIControl* result = control;
    while (result->GetParent() != nullptr)
    {
        UISizePolicyComponent* sizePolicy = result->GetParent()->GetComponent<UISizePolicyComponent>();
        if (sizePolicy != nullptr && (sizePolicy->IsDependsOnChildren(Vector2::AXIS_X) || sizePolicy->IsDependsOnChildren(Vector2::AXIS_Y)))
        {
            result = result->GetParent();
        }
        else
        {
            break;
        }
    }

    if (result->GetParent())
    {
        result = result->GetParent();
    }

    return result;
}

void UILayoutSystem::CollectControls(UIControl* control, bool recursive)
{
    layoutData.clear();
    layoutData.emplace_back(ControlLayoutData(control));
    CollectControlChildren(control, 0, recursive);
}

void UILayoutSystem::CollectControlChildren(UIControl* control, int32 parentIndex, bool recursive)
{
    int32 index = static_cast<int32>(layoutData.size());
    const List<UIControl*> &children = control->GetChildren();

    layoutData[parentIndex].SetFirstChildIndex(index);
    layoutData[parentIndex].SetLastChildIndex(index + static_cast<int32>(children.size() - 1));

    for (UIControl* child : children)
    {
        layoutData.emplace_back(ControlLayoutData(child));
    }

    if (recursive)
    {
        for (UIControl* child : children)
        {
            CollectControlChildren(child, index, recursive);
            index++;
        }
    }
}

void UILayoutSystem::ProcessAxis(Vector2::eAxis axis)
{
    DoMeasurePhase(axis);
    DoLayoutPhase(axis);
}

void UILayoutSystem::DoMeasurePhase(Vector2::eAxis axis)
{
    int32 lastIndex = static_cast<int32>(layoutData.size() - 1);
    for (int32 index = lastIndex; index >= 0; index--)
    {
        SizeMeasuringAlgorithm(layoutData).Apply(layoutData[index], axis);
    }
}

void UILayoutSystem::DoLayoutPhase(Vector2::eAxis axis)
{
    for (auto it = layoutData.begin(); it != layoutData.end(); ++it)
    {
        UIFlowLayoutComponent* flowLayoutComponent = it->GetControl()->GetComponent<UIFlowLayoutComponent>();
        if (flowLayoutComponent && flowLayoutComponent->IsEnabled())
        {
            FlowLayoutAlgorithm(layoutData, isRtl).Apply(*it, axis);
        }
        else
        {
            UILinearLayoutComponent* linearLayoutComponent = it->GetControl()->GetComponent<UILinearLayoutComponent>();
            if (linearLayoutComponent != nullptr && linearLayoutComponent->IsEnabled() && linearLayoutComponent->GetAxis() == axis)
            {
                LinearLayoutAlgorithm alg(layoutData, isRtl);

                alg.SetInverse(isRtl && linearLayoutComponent->IsUseRtl() && linearLayoutComponent->GetOrientation() == UILinearLayoutComponent::HORIZONTAL);
                alg.SetSkipInvisible(linearLayoutComponent->IsSkipInvisibleControls());

                alg.SetPadding(linearLayoutComponent->GetPadding());
                alg.SetSpacing(linearLayoutComponent->GetSpacing());

                alg.SetDynamicPadding(linearLayoutComponent->IsDynamicPadding());
                alg.SetDynamicSpacing(linearLayoutComponent->IsDynamicSpacing());

                alg.Apply(*it, axis);
            }
            else
            {
                AnchorLayoutAlgorithm(layoutData, isRtl).Apply(*it, axis, false);
            }
        }
    }
}

void UILayoutSystem::ApplySizesAndPositions()
{
    for (ControlLayoutData& data : layoutData)
    {
        data.ApplyLayoutToControl();
    }
}

void UILayoutSystem::ApplyPositions()
{
    for (ControlLayoutData& data : layoutData)
    {
        data.ApplyOnlyPositionLayoutToControl();
    }
}
}
