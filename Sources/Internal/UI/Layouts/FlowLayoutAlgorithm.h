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

#ifndef __DAVAENGINE_FLOW_LAYOUT_ALGORITHM_H__
#define __DAVAENGINE_FLOW_LAYOUT_ALGORITHM_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

#include "ControlLayoutData.h"

namespace DAVA
{
class UIControl;
class UIFlowLayoutComponent;
class UISizePolicyComponent;

class FlowLayoutAlgorithm
{
public:
    FlowLayoutAlgorithm(Vector<ControlLayoutData>& layoutData_, bool isRtl_);
    ~FlowLayoutAlgorithm();

    void Apply(ControlLayoutData& data, Vector2::eAxis axis);

private:
    struct LineInfo;

    void ProcessXAxis(ControlLayoutData& data, const UIFlowLayoutComponent* component);
    void CollectLinesInformation(ControlLayoutData& data, Vector<LineInfo>& lines);
    void FixHorizontalPadding(ControlLayoutData& data, Vector<LineInfo>& lines);
    void LayoutLine(ControlLayoutData& data, int32 firstIndex, int32 lastIndex, int32 childrenCount, float32 childrenSize);
    void CalculateHorizontalDynamicPaddingAndSpaces(ControlLayoutData& data, int32 firstIndex, int32 lastIndex);

    void ProcessYAxis(ControlLayoutData& data);
    void CalculateVerticalDynamicPaddingAndSpaces(ControlLayoutData& data);
    void LayoutLineVertically(ControlLayoutData& data, int32 firstIndex, int32 lastIndex, float32 top, float32 bottom);

    void CorrectPaddingAndSpacing(float32& padding, float32& spacing, bool dynamicPadding, bool dynamicSpacing, float32 restSize, int32 childrenCount);

private:
    Vector<ControlLayoutData>& layoutData;
    const bool isRtl;

    bool inverse = false;
    bool skipInvisible = true;

    float32 horizontalPadding = 0.0f;
    float32 horizontalSpacing = 0.0f;
    bool dynamicHorizontalPadding = false;
    bool dynamicHorizontalInLinePadding = false;
    bool dynamicHorizontalSpacing = false;

    float32 verticalPadding = 0.0f;
    float32 verticalSpacing = 0.0f;
    bool dynamicVerticalPadding = false;
    bool dynamicVerticalSpacing = false;
};
}


#endif //__DAVAENGINE_FLOW_LAYOUT_ALGORITHM_H__
