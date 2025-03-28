#include "UI/Styles/UIStyleSheetPropertyDataBase.h"
#include "UI/UIControl.h"
#include "UI/UIStaticText.h"
#include "UI/UITextField.h"
#include "UI/UIControlBackground.h"
#include "UI/Layouts/UILinearLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "UI/Layouts/UIIgnoreLayoutComponent.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "UI/Sound/UISoundComponent.h"

namespace DAVA
{
UIStyleSheetPropertyDataBase::UIStyleSheetPropertyDataBase()
    : controlGroup("", -1, ReflectedTypeDB::Get<UIControl>())
    , bgGroup("bg", UIComponent::BACKGROUND_COMPONENT, ReflectedTypeDB::Get<UIControlBackground>())
    , staticTextGroup("text", -1, ReflectedTypeDB::Get<UIStaticText>())
    , textFieldGroup("textField", -1, ReflectedTypeDB::Get<UITextField>())
    , linearLayoutGroup("linearLayout", UIComponent::LINEAR_LAYOUT_COMPONENT, ReflectedTypeDB::Get<UILinearLayoutComponent>())
    , flowLayoutGroup("flowLayout", UIComponent::FLOW_LAYOUT_COMPONENT, ReflectedTypeDB::Get<UIFlowLayoutComponent>())
    , flowLayoutHintGroup("flowLayoutHint", UIComponent::FLOW_LAYOUT_HINT_COMPONENT, ReflectedTypeDB::Get<UIFlowLayoutHintComponent>())
    , ignoreLayoutGroup("ignoreLayout", UIComponent::IGNORE_LAYOUT_COMPONENT, ReflectedTypeDB::Get<UIIgnoreLayoutComponent>())
    , sizePolicyGroup("sizePolicy", UIComponent::SIZE_POLICY_COMPONENT, ReflectedTypeDB::Get<UISizePolicyComponent>())
    , anchorGroup("anchor", UIComponent::ANCHOR_COMPONENT, ReflectedTypeDB::Get<UIAnchorComponent>())
    , soundGroup("sound", UIComponent::SOUND_COMPONENT, ReflectedTypeDB::Get<UISoundComponent>())
    , properties({ { UIStyleSheetPropertyDescriptor(&controlGroup, "angle", 0.0f),
                     UIStyleSheetPropertyDescriptor(&controlGroup, "scale", Vector2(1.0f, 1.0f)),
                     UIStyleSheetPropertyDescriptor(&controlGroup, "pivot", Vector2(0.0f, 0.0f)),
                     UIStyleSheetPropertyDescriptor(&controlGroup, "visible", true),
                     UIStyleSheetPropertyDescriptor(&controlGroup, "noInput", false),
                     UIStyleSheetPropertyDescriptor(&controlGroup, "exclusiveInput", false),

                     UIStyleSheetPropertyDescriptor(&bgGroup, "drawType", UIControlBackground::DRAW_ALIGNED),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "sprite", FilePath()),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "frame", 0),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "mask", FilePath()),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "detail", FilePath()),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "gradient", FilePath()),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "contour", FilePath()),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "gradientMode", eGradientBlendMode::GRADIENT_MULTIPLY),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "spriteModification", 0),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "color", Color::White),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "colorInherit", UIControlBackground::COLOR_IGNORE_PARENT),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "align", ALIGN_HCENTER | ALIGN_VCENTER),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "leftRightStretchCap", 0.0f),
                     UIStyleSheetPropertyDescriptor(&bgGroup, "topBottomStretchCap", 0.0f),

                     UIStyleSheetPropertyDescriptor(&staticTextGroup, "font", String("")),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, "textColor", Color::White),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, "textcolorInheritType", UIControlBackground::COLOR_MULTIPLY_ON_PARENT),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, "shadowoffset", Vector2(0.0f, 0.0f)),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, "shadowcolor", Color::Black),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, "textalign", ALIGN_HCENTER | ALIGN_VCENTER),

                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, "enabled", true),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, "orientation", UILinearLayoutComponent::LEFT_TO_RIGHT),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, "padding", 0.0f),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, "dynamicPadding", false),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, "spacing", 0.0f),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, "dynamicSpacing", false),

                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, "enabled", true),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, "orientation", UIFlowLayoutComponent::ORIENTATION_LEFT_TO_RIGHT),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, "hPadding", 0.0f),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, "hDynamicPadding", false),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, "hDynamicInLinePadding", false),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, "hSpacing", 0.0f),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, "hDynamicSpacing", false),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, "vPadding", 0.0f),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, "vDynamicPadding", false),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, "vSpacing", 0.0f),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, "vDynamicSpacing", false),

                     UIStyleSheetPropertyDescriptor(&flowLayoutHintGroup, "newLineBeforeThis", false),
                     UIStyleSheetPropertyDescriptor(&flowLayoutHintGroup, "newLineAfterThis", false),

                     UIStyleSheetPropertyDescriptor(&ignoreLayoutGroup, "enabled", true),

                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, "horizontalPolicy", UISizePolicyComponent::IGNORE_SIZE),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, "horizontalValue", 100.0f),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, "horizontalMin", 0.0f),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, "horizontalMax", 99999.0f),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, "verticalPolicy", UISizePolicyComponent::IGNORE_SIZE),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, "verticalValue", 100.0f),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, "verticalMin", 0.0f),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, "verticalMax", 99999.0f),

                     UIStyleSheetPropertyDescriptor(&anchorGroup, "enabled", true),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "leftAnchorEnabled", false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "leftAnchor", 0.0f),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "rightAnchorEnabled", false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "rightAnchor", 0.0f),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "bottomAnchorEnabled", false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "bottomAnchor", 0.0f),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "topAnchorEnabled", false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "topAnchor", 0.0f),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "hCenterAnchorEnabled", false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "hCenterAnchor", 0.0f),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "vCenterAnchorEnabled", false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, "vCenterAnchor", 0.0f),

                     UIStyleSheetPropertyDescriptor(&soundGroup, "touchDown", FastName()),
                     UIStyleSheetPropertyDescriptor(&soundGroup, "touchUpInside", FastName()),
                     UIStyleSheetPropertyDescriptor(&soundGroup, "touchUpOutside", FastName()),
                     UIStyleSheetPropertyDescriptor(&soundGroup, "valueChanged", FastName()) } })
{
    UnorderedMap<FastName, FastName> legacyNames;
    legacyNames[FastName("bg-drawType")] = FastName("drawType");
    legacyNames[FastName("bg-sprite")] = FastName("sprite");
    legacyNames[FastName("bg-frame")] = FastName("frame");
    legacyNames[FastName("bg-color")] = FastName("color");
    legacyNames[FastName("bg-colorInherit")] = FastName("colorInherit");
    legacyNames[FastName("bg-align")] = FastName("align");
    legacyNames[FastName("bg-leftRightStretchCap")] = FastName("leftRightStretchCap");
    legacyNames[FastName("bg-topBottomStretchCap")] = FastName("topBottomStretchCap");

    legacyNames[FastName("text-font")] = FastName("font");
    legacyNames[FastName("text-textColor")] = FastName("textColor");
    legacyNames[FastName("text-textcolorInheritType")] = FastName("textcolorInheritType");
    legacyNames[FastName("text-shadowoffset")] = FastName("shadowoffset");
    legacyNames[FastName("text-shadowcolor")] = FastName("shadowcolor");
    legacyNames[FastName("text-textalign")] = FastName("textalign");

    legacyNames[FastName("anchor-leftAnchorEnabled")] = FastName("leftAnchorEnabled");
    legacyNames[FastName("anchor-leftAnchor")] = FastName("leftAnchor");
    legacyNames[FastName("anchor-rightAnchorEnabled")] = FastName("rightAnchorEnabled");
    legacyNames[FastName("anchor-rightAnchor")] = FastName("rightAnchor");
    legacyNames[FastName("anchor-bottomAnchorEnabled")] = FastName("bottomAnchorEnabled");
    legacyNames[FastName("anchor-bottomAnchor")] = FastName("bottomAnchor");
    legacyNames[FastName("anchor-topAnchorEnabled")] = FastName("topAnchorEnabled");
    legacyNames[FastName("anchor-topAnchor")] = FastName("topAnchor");
    legacyNames[FastName("anchor-hCenterAnchorEnabled")] = FastName("hCenterAnchorEnabled");
    legacyNames[FastName("anchor-hCenterAnchor")] = FastName("hCenterAnchor");
    legacyNames[FastName("anchor-vCenterAnchorEnabled")] = FastName("vCenterAnchorEnabled");
    legacyNames[FastName("anchor-vCenterAnchor")] = FastName("vCenterAnchor");

    for (int32 propertyIndex = 0; propertyIndex < STYLE_SHEET_PROPERTY_COUNT; propertyIndex++)
    {
        UIStyleSheetPropertyDescriptor& descr = properties[propertyIndex];
        FastName fullName = FastName(descr.GetFullName());
        propertyNameToIndexMap[fullName] = propertyIndex;

        auto legacyNameIt = legacyNames.find(fullName);
        if (legacyNameIt != legacyNames.end())
        {
            propertyNameToIndexMap[legacyNameIt->second] = propertyIndex;
        }
    }

    visiblePropertyIndex = GetStyleSheetPropertyIndex(FastName("visible"));
}

UIStyleSheetPropertyDataBase::~UIStyleSheetPropertyDataBase()
{
}

uint32 UIStyleSheetPropertyDataBase::GetStyleSheetPropertyIndex(const FastName& name) const
{
    const auto& iter = propertyNameToIndexMap.find(name);

    DVASSERT(iter != propertyNameToIndexMap.end());

    return iter->second;
}

bool UIStyleSheetPropertyDataBase::IsValidStyleSheetProperty(const FastName& name) const
{
    return propertyNameToIndexMap.find(name) != propertyNameToIndexMap.end();
}

const UIStyleSheetPropertyDescriptor& UIStyleSheetPropertyDataBase::GetStyleSheetPropertyByIndex(uint32 index) const
{
    return properties[index];
}

int32 UIStyleSheetPropertyDataBase::FindStyleSheetProperty(int32 componentType, const FastName& name) const
{
    for (size_t index = 0; index < properties.size(); index++)
    {
        const UIStyleSheetPropertyDescriptor& descr = properties[index];
        if (descr.group->componentType == componentType && descr.name == name)
        {
            return static_cast<int32>(index);
        }
    }
    return -1;
}
}
