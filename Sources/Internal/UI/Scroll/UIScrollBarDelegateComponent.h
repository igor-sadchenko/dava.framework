#pragma once

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIScrollBarDelegateComponent : public UIBaseComponent<UIComponent::SCROLL_BAR_DELEGATE_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UIScrollBarDelegateComponent, UIBaseComponent<UIComponent::SCROLL_BAR_DELEGATE_COMPONENT>);

public:
    UIScrollBarDelegateComponent();
    UIScrollBarDelegateComponent(const UIScrollBarDelegateComponent& src);

protected:
    virtual ~UIScrollBarDelegateComponent();

    UIScrollBarDelegateComponent& operator=(const UIScrollBarDelegateComponent&) = delete;

public:
    UIScrollBarDelegateComponent* Clone() const override;

    const String& GetPathToDelegate() const;
    void SetPathToDelegate(const String& path);

    bool IsPathToDelegateDirty() const;
    void ResetPathToDelegateDirty();

private:
    String pathToDelegate;
    bool pathToDelegateDirty = true;
};
}
