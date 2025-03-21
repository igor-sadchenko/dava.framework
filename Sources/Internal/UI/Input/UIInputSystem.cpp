#include "UIInputSystem.h"

#include "UI/UIAnalitycs.h"
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIEvent.h"
#include "UI/UIScreen.h"

#include "UI/Focus/UIFocusSystem.h"
#include "UI/Input/UIModalInputComponent.h"
#include "UI/Input/UIActionBindingComponent.h"
#include "UI/Input/UIActionComponent.h"

#include "Input/InputSystem.h"

namespace DAVA
{
const FastName UIInputSystem::ACTION_FOCUS_LEFT("FocusLeft");
const FastName UIInputSystem::ACTION_FOCUS_RIGHT("FocusRight");
const FastName UIInputSystem::ACTION_FOCUS_UP("FocusUp");
const FastName UIInputSystem::ACTION_FOCUS_DOWN("FocusDown");

const FastName UIInputSystem::ACTION_FOCUS_NEXT("FocusNext");
const FastName UIInputSystem::ACTION_FOCUS_PREV("FocusPrev");

const FastName UIInputSystem::ACTION_PERFORM("PerformAction");
const FastName UIInputSystem::ACTION_ESCAPE("Escape");

UIInputSystem::UIInputSystem()
{
    focusSystem = new UIFocusSystem();

    BindGlobalAction(ACTION_FOCUS_LEFT, MakeFunction(this, &UIInputSystem::MoveFocusLeft));
    BindGlobalAction(ACTION_FOCUS_RIGHT, MakeFunction(this, &UIInputSystem::MoveFocusRight));
    BindGlobalAction(ACTION_FOCUS_UP, MakeFunction(this, &UIInputSystem::MoveFocusUp));
    BindGlobalAction(ACTION_FOCUS_DOWN, MakeFunction(this, &UIInputSystem::MoveFocusDown));

    BindGlobalAction(ACTION_FOCUS_NEXT, MakeFunction(this, &UIInputSystem::MoveFocusForward));
    BindGlobalAction(ACTION_FOCUS_PREV, MakeFunction(this, &UIInputSystem::MoveFocusBackward));

    BindGlobalAction(ACTION_PERFORM, MakeFunction(this, &UIInputSystem::PerformActionOnFocusedControl));
}

UIInputSystem::~UIInputSystem()
{
    SafeDelete(focusSystem);

    currentScreen = nullptr; // we are not owner
    popupContainer = nullptr; // we are not owner
}

void UIInputSystem::SetCurrentScreen(UIScreen* screen)
{
    currentScreen = screen;
    UpdateModalControl();
}

void UIInputSystem::SetPopupContainer(UIControl* container)
{
    popupContainer = container;
}

void UIInputSystem::OnControlVisible(UIControl* control)
{
    if (control->GetComponent<UIModalInputComponent>() != nullptr)
    {
        UpdateModalControl();
    }
    focusSystem->OnControlVisible(control);
}

void UIInputSystem::OnControlInvisible(UIControl* control)
{
    if (control->GetHover())
    {
        SetHoveredControl(nullptr);
    }

    if (control->GetInputEnabled())
    {
        CancelInputs(control, false);
    }

    if (control->GetComponent<UIModalInputComponent>() != nullptr)
    {
        UpdateModalControl();
    }

    focusSystem->OnControlInvisible(control);
}

void UIInputSystem::HandleEvent(UIEvent* event)
{
    bool processed = false;

    if (currentScreen)
    {
        UIEvent::Phase phase = event->phase;

        if (phase == UIEvent::Phase::KEY_DOWN || phase == UIEvent::Phase::KEY_UP || phase == UIEvent::Phase::KEY_DOWN_REPEAT || phase == UIEvent::Phase::CHAR || phase == UIEvent::Phase::CHAR_REPEAT)
        {
            processed = HandleKeyEvent(event);
        }
        else if (phase == UIEvent::Phase::BEGAN || phase == UIEvent::Phase::DRAG || phase == UIEvent::Phase::ENDED || phase == UIEvent::Phase::CANCELLED)
        {
            processed = HandleTouchEvent(event);
        }
        else
        {
            processed = HandleOtherEvent(event); // joypad, geasture, mouse wheel
        }
    }

    if (!processed)
    {
        notProcessedEventSignal.Emit(event);
    }

    auto startRemoveIt = std::remove_if(begin(touchEvents), end(touchEvents), [this](UIEvent& ev) {
        bool shouldRemove = (ev.phase == UIEvent::Phase::ENDED || ev.phase == UIEvent::Phase::CANCELLED);
        if (shouldRemove)
        {
            CancelInput(&ev);
        }
        return shouldRemove;
    });
    touchEvents.erase(startRemoveIt, end(touchEvents));
}

void UIInputSystem::CancelInput(UIEvent* touch)
{
    if (touch->touchLocker)
    {
        touch->touchLocker->SystemInputCancelled(touch);
    }
    if (touch->touchLocker != currentScreen)
    {
        currentScreen->SystemInputCancelled(touch);
    }
}

void UIInputSystem::CancelAllInputs()
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        CancelInput(&(*it));
    }
    touchEvents.clear();
}

void UIInputSystem::CancelInputs(UIControl* control, bool hierarchical)
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        if (!hierarchical)
        {
            if (it->touchLocker == control)
            {
                CancelInput(&(*it));
                break;
            }
            continue;
        }
        UIControl* parentLockerControl = it->touchLocker;
        while (parentLockerControl)
        {
            if (control == parentLockerControl)
            {
                CancelInput(&(*it));
                break;
            }
            parentLockerControl = parentLockerControl->GetParent();
        }
    }
}

void UIInputSystem::SwitchInputToControl(uint32 eventID, UIControl* targetControl)
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        if ((*it).touchId == eventID)
        {
            CancelInput(&(*it));

            if (targetControl->IsPointInside((*it).point))
            {
                (*it).controlState = UIEvent::CONTROL_STATE_INSIDE;
                targetControl->touchesInside++;
            }
            else
            {
                (*it).controlState = UIEvent::CONTROL_STATE_OUTSIDE;
            }
            (*it).touchLocker = targetControl;
            targetControl->currentInputID = eventID;
            if (targetControl->GetExclusiveInput())
            {
                SetExclusiveInputLocker(targetControl, eventID);
            }
            else
            {
                SetExclusiveInputLocker(NULL, -1);
            }

            targetControl->totalTouches++;
        }
    }
}

const Vector<UIEvent>& UIInputSystem::GetAllInputs() const
{
    return touchEvents;
}

void UIInputSystem::SetExclusiveInputLocker(UIControl* locker, uint32 lockEventId)
{
    SafeRelease(exclusiveInputLocker);
    if (locker != NULL)
    {
        for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
        {
            if (it->touchId != lockEventId && it->touchLocker != locker)
            { //cancel all inputs excepts current input and inputs what allready handles by this locker.
                CancelInput(&(*it));
            }
        }
    }

    exclusiveInputLocker = SafeRetain(locker);
}

UIControl* UIInputSystem::GetExclusiveInputLocker() const
{
    return exclusiveInputLocker;
}

void UIInputSystem::SetHoveredControl(UIControl* newHovered)
{
    if (hovered != newHovered)
    {
        if (hovered)
        {
            hovered->SystemDidRemoveHovered();
            hovered->Release();
        }
        hovered = SafeRetain(newHovered);
        if (hovered)
        {
            hovered->SystemDidSetHovered();
        }
    }
}

UIControl* UIInputSystem::GetHoveredControl() const
{
    return hovered;
}

UIFocusSystem* UIInputSystem::GetFocusSystem() const
{
    return focusSystem;
}

void UIInputSystem::BindGlobalShortcut(const KeyboardShortcut& shortcut, const FastName& actionName)
{
    globalInputMap.BindAction(shortcut, actionName);
}

void UIInputSystem::BindGlobalAction(const FastName& actionName, const UIActionMap::Action& action)
{
    globalActions.Put(actionName, action);
}

void UIInputSystem::MoveFocusLeft()
{
    focusSystem->MoveFocus(UINavigationComponent::Direction::LEFT);
}

void UIInputSystem::MoveFocusRight()
{
    focusSystem->MoveFocus(UINavigationComponent::Direction::RIGHT);
}

void UIInputSystem::MoveFocusUp()
{
    focusSystem->MoveFocus(UINavigationComponent::Direction::UP);
}

void UIInputSystem::MoveFocusDown()
{
    focusSystem->MoveFocus(UINavigationComponent::Direction::DOWN);
}

void UIInputSystem::MoveFocusForward()
{
    focusSystem->MoveFocus(UITabOrderComponent::Direction::FORWARD, true);
}

void UIInputSystem::MoveFocusBackward()
{
    focusSystem->MoveFocus(UITabOrderComponent::Direction::BACKWARD, true);
}

void UIInputSystem::PerformActionOnControl(UIControl* control)
{
    if (control != nullptr)
    {
        UIActionComponent* actionComponent = control->GetComponent<UIActionComponent>();
        if (actionComponent != nullptr && actionComponent->GetAction().IsValid())
        {
            UIControl* c = control;
            bool processed = false;
            while (!processed && c != nullptr)
            {
                UIActionBindingComponent* actionBindingComponent = c->GetComponent<UIActionBindingComponent>();
                if (actionBindingComponent)
                {
                    processed = actionBindingComponent->GetActionMap().Perform(actionComponent->GetAction());
                }

                c = (c == modalControl.Get()) ? nullptr : c->GetParent();
            }
        }
    }
}

void UIInputSystem::PerformActionOnFocusedControl()
{
    PerformActionOnControl(focusSystem->GetFocusedControl());
}

bool UIInputSystem::HandleTouchEvent(UIEvent* event)
{
    DVASSERT(event->phase == UIEvent::Phase::BEGAN || event->phase == UIEvent::Phase::DRAG || event->phase == UIEvent::Phase::ENDED || event->phase == UIEvent::Phase::CANCELLED);

    UIEvent* eventToHandle = nullptr;
    auto it = std::find_if(begin(touchEvents), end(touchEvents), [event](const UIEvent& ev) {
        return ev.touchId == event->touchId;
    });
    if (it == end(touchEvents))
    {
        touchEvents.push_back(*event);
        eventToHandle = &touchEvents.back();
    }
    else
    {
        it->timestamp = event->timestamp;
        it->physPoint = event->physPoint;
        it->point = event->point;
        it->tapCount = event->tapCount;
        it->phase = event->phase;
        it->inputHandledType = event->inputHandledType;

        eventToHandle = &(*it);
    }

    UIEvent::Phase phase = eventToHandle->phase;
    if (phase == UIEvent::Phase::BEGAN)
    {
        focusedControlWhenTouchBegan = focusSystem->GetFocusedControl();
        positionOfTouchWhenTouchBegan = eventToHandle->point;
    }

    bool processed = false;
    if (modalControl.Valid())
    {
        RefPtr<UIControl> control = modalControl;
        processed = control->SystemInput(eventToHandle);
    }
    else
    {
        processed = popupContainer->SystemInput(eventToHandle);
        if (!processed)
        {
            processed = currentScreen->SystemInput(eventToHandle);
        }
    }

    if (phase == UIEvent::Phase::ENDED)
    {
        UIControl* focusedControl = focusSystem->GetFocusedControl();
        if (focusedControl != nullptr)
        {
            static const float32 draggingThresholdSq = 20.0f * 20.0f;
            bool focusWasntChanged = focusedControl == focusedControlWhenTouchBegan;

            bool touchWasntDragged = (positionOfTouchWhenTouchBegan - eventToHandle->point).SquareLength() < draggingThresholdSq;
            bool touchOutsideControl = !focusedControl->IsPointInside(eventToHandle->point);
            if (focusWasntChanged && touchWasntDragged && touchOutsideControl)
            {
                focusedControl->OnTouchOutsideFocus();
            }
        }
        focusedControlWhenTouchBegan = nullptr;
    }

    return processed;
}

bool UIInputSystem::HandleKeyEvent(UIEvent* event)
{
    UIEvent::Phase phase = event->phase;
    DVASSERT(phase == UIEvent::Phase::KEY_DOWN || phase == UIEvent::Phase::KEY_UP || phase == UIEvent::Phase::KEY_DOWN_REPEAT || phase == UIEvent::Phase::CHAR || phase == UIEvent::Phase::CHAR_REPEAT);

    KeyboardShortcut shortcut(event->key, event->modifiers);

    UIControl* focusedControl = focusSystem->GetFocusedControl();
    UIControl* rootControl = modalControl.Valid() ? modalControl.Get() : currentScreen;

    bool processed = false;
    if (focusedControl != nullptr || rootControl != nullptr)
    {
        UIControl* c = focusedControl != nullptr ? focusedControl : rootControl;
        while (c != nullptr)
        {
            RefPtr<UIControl> current;
            current = c;
            if (current->SystemProcessInput(event))
            {
                Analytics::EmitKeyEvent(current.Get(), event);
                processed = true;
                break;
            }

            if (phase == UIEvent::Phase::KEY_DOWN)
            {
                UIActionBindingComponent* actionBindingComponent = current->GetComponent<UIActionBindingComponent>();
                if (actionBindingComponent)
                {
                    FastName action = actionBindingComponent->GetInputMap().FindAction(shortcut);
                    if (action.IsValid() && actionBindingComponent->GetActionMap().Perform(action))
                    {
                        break;
                    }
                }
            }

            if (current.Get() == rootControl)
            {
                break;
            }

            c = current->GetParent();
        }
    }

    if (phase == UIEvent::Phase::KEY_DOWN || phase == UIEvent::Phase::KEY_DOWN_REPEAT)
    {
        FastName action = globalInputMap.FindAction(shortcut);
        if (action.IsValid())
        {
            globalActions.Perform(action);
        }
    }

    return processed;
}

bool UIInputSystem::HandleOtherEvent(UIEvent* event)
{
    if (modalControl.Valid())
    {
        RefPtr<UIControl> control = modalControl;
        return control->SystemInput(event);
    }

    if (popupContainer->SystemInput(event))
    {
        return true;
    }

    return currentScreen->SystemInput(event);
}

void UIInputSystem::UpdateModalControl()
{
    if (currentScreen != nullptr)
    {
        UIControl* root = FindNearestToUserModalControl();
        focusSystem->SetRoot(root == nullptr ? currentScreen : root);
        modalControl = root;

        if (root != nullptr)
        {
            CancelInputForAllOutsideChildren(root);
        }
    }
    else
    {
        focusSystem->SetRoot(nullptr);
        modalControl = nullptr;
    }
}

void UIInputSystem::CancelInputForAllOutsideChildren(UIControl* root)
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        UIControl* control = it->touchLocker;
        if (control != nullptr)
        {
            bool isInHierarchy = false;
            while (control != nullptr)
            {
                if (control == modalControl)
                {
                    isInHierarchy = true;
                    break;
                }
                control = control->GetParent();
            }

            if (!isInHierarchy)
            {
                CancelInput(&(*it));
            }
        }
    }
}

UIControl* UIInputSystem::FindNearestToUserModalControl() const
{
    UIControl* control = FindNearestToUserModalControlImpl(popupContainer);
    if (control != nullptr)
    {
        return control;
    }
    return FindNearestToUserModalControlImpl(currentScreen);
}

UIControl* UIInputSystem::FindNearestToUserModalControlImpl(UIControl* current) const
{
    const List<UIControl*>& children = current->GetChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it)
    {
        UIControl* result = FindNearestToUserModalControlImpl(*it);
        if (result != nullptr)
        {
            return result;
        }
    }

    UIModalInputComponent* modalInputComponent = current->GetComponent<UIModalInputComponent>();
    if (modalInputComponent != nullptr && modalInputComponent->IsEnabled() && current->IsVisible())
    {
        return current;
    }
    return nullptr;
}
}
