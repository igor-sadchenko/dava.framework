#ifndef __DAVAENGINE_LINEAR_LAYOUT_ALGORITHM_H__
#define __DAVAENGINE_LINEAR_LAYOUT_ALGORITHM_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

#include "ControlLayoutData.h"

namespace DAVA
{
class UIControl;
class UISizePolicyComponent;

class LinearLayoutAlgorithm
{
public:
    LinearLayoutAlgorithm(Vector<ControlLayoutData>& layoutData_, bool isRtl_);
    ~LinearLayoutAlgorithm();

    void SetInverse(bool inverse_);
    void SetSkipInvisible(bool skipInvisible_);
    void SetPadding(float32 padding_);
    void SetSpacing(float32 spacing_);
    void SetDynamicPadding(bool dynamicPadding_);
    void SetDynamicSpacing(bool dynamicSpacing_);

    void Apply(ControlLayoutData& data, Vector2::eAxis axis);
    void Apply(ControlLayoutData& data, Vector2::eAxis axis, int32 firstIndex, int32 lastIndex);

private:
    void InitializeParams(ControlLayoutData& data, Vector2::eAxis axis, int32 firstIndex, int32 lastIndex);
    void CalculateDependentOnParentSizes(ControlLayoutData& data, Vector2::eAxis axis, int32 firstIndex, int32 lastIndex);
    bool CalculateChildDependentOnParentSize(ControlLayoutData& data, Vector2::eAxis axis);
    void CalculateDynamicPaddingAndSpaces(ControlLayoutData& data, Vector2::eAxis axis);
    void PlaceChildren(ControlLayoutData& data, Vector2::eAxis axis, int32 firstIndex, int32 lastIndex);

private:
    Vector<ControlLayoutData>& layoutData;
    const bool isRtl;

    bool inverse = false;
    bool skipInvisible = true;

    float32 currentSize = 0.0f;
    float32 fixedSize = 0.0f;
    float32 totalPercent = 0.0f;

    float32 contentSize = 0.0f;
    float32 restSize = 0.0f;

    int32 childrenCount = 0;
    int32 spacesCount = 0;

    float32 initialPadding = 0.0f;
    float32 initialSpacing = 0.0f;

    float32 padding = 0.0f;
    float32 spacing = 0.0f;

    bool dynamicPadding = false;
    bool dynamicSpacing = false;
};
}


#endif //__DAVAENGINE_LINEAR_LAYOUT_ALGORITHM_H__
