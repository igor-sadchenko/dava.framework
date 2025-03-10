#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Qt/WindowBackendQt.h"
#include "Input/InputSystem.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Window.h"
#include "Engine/EngineContext.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/Qt/WindowBackendQt.h"

#include "Input/InputSystem.h"
#include "Render/RHI/rhi_Public.h"

#include "Logger/Logger.h"
#include "Input/InputSystem.h"
#include "UI/UIEvent.h"
#include "Debug/DVAssert.h"

#include <QApplication>
#include <QObject>

namespace DAVA
{
// DavaQtApplyModifier is a friend for KeyboardDevice, so it should be in DAVA namespace only.
class DavaQtApplyModifier
{
public:
    void operator()(DAVA::KeyboardDevice& keyboard, const Qt::KeyboardModifiers& currentModifiers, Qt::KeyboardModifier qtModifier, DAVA::Key davaModifier)
    {
        if (true == (currentModifiers.testFlag(qtModifier)))
            keyboard.OnKeyPressed(davaModifier);
        else
            keyboard.OnKeyUnpressed(davaModifier);
    }
};

namespace Private
{
class TriggerProcessEvent : public QEvent
{
public:
    TriggerProcessEvent()
        : QEvent(eventType)
    {
    }

    static const Type eventType = static_cast<Type>(User + 1);
};

class WindowBackend::QtEventListener : public QObject
{
public:
    using TCallback = Function<void()>;
    QtEventListener(const TCallback& triggered_,
                    const TCallback& destroyed_,
                    QObject* parent)
        : QObject(parent)
        , triggered(triggered_)
        , destroyed(destroyed_)
    {
    }

    ~QtEventListener()
    {
        destroyed();
    }

protected:
    bool event(QEvent* e) override
    {
        if (e->type() == TriggerProcessEvent::eventType)
        {
            triggered();
            return true;
        }

        return false;
    }

private:
    TCallback triggered;
    TCallback destroyed;
};

WindowBackend::WindowBackend(EngineBackend* engineBackend, Window* window)
    : engineBackend(engineBackend)
    , window(window)
    , mainDispatcher(engineBackend->GetDispatcher())
    , uiDispatcher(MakeFunction(this, &WindowBackend::UIEventHandler), MakeFunction(this, &WindowBackend::TriggerPlatformEvents))
{
    QtEventListener::TCallback triggered = [this]()
    {
        // Prevent processing UI dispatcher events when modal dialog is open as Dispatcher::ProcessEvents is not reentrant now
        if (!EngineBackend::showingModalMessageBox)
        {
            uiDispatcher.ProcessEvents();
        }
    };

    QtEventListener::TCallback destroyed = [this]()
    {
        qtEventListener = nullptr;
    };

    qtEventListener = new QtEventListener(triggered, destroyed, PlatformApi::Qt::GetApplication());
}

WindowBackend::~WindowBackend()
{
    delete renderWidget;
}

void WindowBackend::Resize(float32 width, float32 height)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateResizeEvent(width, height));
}

void WindowBackend::Close(bool /*appIsTerminating*/)
{
    closeRequestByApp = true;
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateCloseEvent());
}

void WindowBackend::SetTitle(const String& title)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateSetTitleEvent(title));
}

void WindowBackend::SetMinimumSize(Size2f size)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateMinimumSizeEvent(size.dx, size.dy));
}

void WindowBackend::SetFullscreen(eFullscreen newMode)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateSetFullscreenEvent(newMode));
}

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateFunctorEvent(task));
}

void WindowBackend::RunAndWaitOnUIThread(const Function<void()>& task)
{
    uiDispatcher.SendEvent(UIDispatcherEvent::CreateFunctorEvent(task));
}

bool WindowBackend::IsWindowReadyForRender() const
{
    return renderWidget != nullptr && renderWidget->IsInitialized();
}

void WindowBackend::TriggerPlatformEvents()
{
    QApplication* app = PlatformApi::Qt::GetApplication();
    DVASSERT(app);
    if (app != nullptr)
    {
        app->postEvent(qtEventListener, new TriggerProcessEvent());
    }
}

void WindowBackend::SetSurfaceScaleAsync(const float32 scale)
{
    // Not supported natively on OpenGL
}

void WindowBackend::UIEventHandler(const UIDispatcherEvent& e)
{
    switch (e.type)
    {
    case UIDispatcherEvent::RESIZE_WINDOW:
        DoResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case UIDispatcherEvent::CLOSE_WINDOW:
        DoCloseWindow();
        break;
    case UIDispatcherEvent::SET_TITLE:
        DoSetTitle(e.setTitleEvent.title);
        delete[] e.setTitleEvent.title;
        break;
    case UIDispatcherEvent::SET_MINIMUM_SIZE:
        DoSetMinimumSize(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    // not implemented
    // case UIDispatcherEvent::CHANGE_MOUSE_MODE:
    default:
        break;
    }
}

void WindowBackend::OnCreated()
{
    uiDispatcher.LinkToCurrentThread();

    dpi = static_cast<float32>(renderWidget->logicalDpiX());
    float32 scale = static_cast<float32>(renderWidget->devicePixelRatio());
    float32 w = static_cast<float32>(renderWidget->width());
    float32 h = static_cast<float32>(renderWidget->height());
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window, w, h, w * scale, h * scale, dpi, eFullscreen::Off));

    OnVisibilityChanged(true);
    OnApplicationFocusChanged(true);
}

bool WindowBackend::OnUserCloseRequest()
{
    if (!closeRequestByApp)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateUserCloseRequestEvent(window));
    }
    return closeRequestByApp;
}

void WindowBackend::OnDestroyed()
{
    mainDispatcher->SendEvent(MainDispatcherEvent::CreateWindowDestroyedEvent(window));
}

void WindowBackend::OnFrame()
{
    // HACK Qt send key event to widget with focus not globaly
    // if user hold ALT(CTRL, SHIFT) and then clicked DavaWidget(focused)
    // we miss key down event, so we have to check for SHIFT, ALT, CTRL
    // read about same problem http://stackoverflow.com/questions/23193038/how-to-detect-global-key-sequence-press-in-qt
    Qt::KeyboardModifiers modifiers = qApp->queryKeyboardModifiers();
    KeyboardDevice& keyboard = engineBackend->GetContext()->inputSystem->GetKeyboard();
    DavaQtApplyModifier mod;
    mod(keyboard, modifiers, Qt::AltModifier, Key::LALT);
    mod(keyboard, modifiers, Qt::ShiftModifier, Key::LSHIFT);
    mod(keyboard, modifiers, Qt::ControlModifier, Key::LCTRL);

    engineBackend->OnFrame();
}

void WindowBackend::OnResized(uint32 width, uint32 height, bool isFullScreen)
{
    if (renderWidget && renderWidget->IsInitialized())
    {
        float32 scale = static_cast<float32>(renderWidget->devicePixelRatio());
        float32 w = static_cast<float32>(width);
        float32 h = static_cast<float32>(height);
        eFullscreen fullscreen = isFullScreen ? eFullscreen::On : eFullscreen::Off;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, w, h, w * scale, h * scale, 1.0f, dpi, fullscreen));
    }
}

void WindowBackend::OnDpiChanged(float32 dpi_)
{
    dpi = dpi_;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowDpiChangedEvent(window, dpi));
}

void WindowBackend::OnVisibilityChanged(bool isVisible)
{
    if (renderWidget && renderWidget->IsInitialized())
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, isVisible));
    }
}

void WindowBackend::OnMousePressed(QMouseEvent* qtEvent)
{
    const MainDispatcherEvent::eType type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
    eMouseButtons button = GetMouseButton(qtEvent->button());
    float32 x = static_cast<float32>(qtEvent->x());
    float32 y = static_cast<float32>(qtEvent->y());
    eModifierKeys modifierKeys = GetModifierKeys();
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window, type, button, x, y, 1, modifierKeys, false));
}

void WindowBackend::OnMouseReleased(QMouseEvent* qtEvent)
{
    const MainDispatcherEvent::eType type = MainDispatcherEvent::MOUSE_BUTTON_UP;
    eMouseButtons button = GetMouseButton(qtEvent->button());
    float32 x = static_cast<float32>(qtEvent->x());
    float32 y = static_cast<float32>(qtEvent->y());
    eModifierKeys modifierKeys = GetModifierKeys();
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window, type, button, x, y, 1, modifierKeys, false));
}

void WindowBackend::OnMouseMove(QMouseEvent* qtEvent)
{
    float32 x = static_cast<float32>(qtEvent->x());
    float32 y = static_cast<float32>(qtEvent->y());
    eModifierKeys modifierKeys = GetModifierKeys();
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, x, y, modifierKeys, false));
}

void WindowBackend::OnDragMoved(QDragMoveEvent* qtEvent)
{
    float32 x = static_cast<float32>(qtEvent->pos().x());
    float32 y = static_cast<float32>(qtEvent->pos().y());
    eModifierKeys modifierKeys = GetModifierKeys();
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, x, y, modifierKeys, false));
}

void WindowBackend::OnMouseDBClick(QMouseEvent* qtEvent)
{
    OnMousePressed(qtEvent);
    OnMouseReleased(qtEvent);
}

void WindowBackend::OnWheel(QWheelEvent* qtEvent)
{
    if (qtEvent->phase() != Qt::ScrollUpdate)
    {
        return;
    }

    float32 x = static_cast<float32>(qtEvent->x());
    float32 y = static_cast<float32>(qtEvent->y());
    float32 deltaX = 0.f;
    float32 deltaY = 0.f;

    QPoint pixelDelta = qtEvent->pixelDelta();
    if (!pixelDelta.isNull())
    {
        deltaX = static_cast<float32>(pixelDelta.x());
        deltaY = static_cast<float32>(pixelDelta.y());
    }
    else
    {
        //most mouse types work in steps of 15 degrees, in which case the delta value is a multiple of 120
        QPointF delta = QPointF(qtEvent->angleDelta()) / 120.0f;
        deltaX = delta.x();
        deltaY = delta.y();
    }
    eModifierKeys modifierKeys = GetModifierKeys();
#ifdef Q_OS_MAC
    if (qtEvent->source() == Qt::MouseEventSynthesizedBySystem)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSwipeGestureEvent(window, deltaX, deltaY, modifierKeys));
        return;
    }
#endif
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseWheelEvent(window, x, y, deltaX, deltaY, modifierKeys, false));
}

void WindowBackend::OnNativeGesture(QNativeGestureEvent* qtEvent)
{
    eModifierKeys modifierKeys = GetModifierKeys();
    //local coordinates don't work on OS X - https://bugreports.qt.io/browse/QTBUG-59595
    QPoint localPos = renderWidget->mapFromGlobal(qtEvent->globalPos());

    float32 x = static_cast<float32>(localPos.x());
    float32 y = static_cast<float32>(localPos.y());
    switch (qtEvent->gestureType())
    {
    case Qt::RotateNativeGesture:
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowRotationGestureEvent(window, qtEvent->value(), modifierKeys));
        break;
    case Qt::ZoomNativeGesture:
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMagnificationGestureEvent(window, x, y, qtEvent->value(), modifierKeys));
        break;
    default:
        break;
    }
}

void WindowBackend::OnKeyPressed(QKeyEvent* qtEvent)
{
    uint32 key = qtEvent->nativeVirtualKey();
#if defined(Q_OS_WIN)
    // How to distinguish left and right shift, control and alt: http://stackoverflow.com/a/15977613
    uint32 lparam = qtEvent->nativeModifiers();
    uint32 scanCode = qtEvent->nativeScanCode();
    bool isExtended = (HIWORD(lparam) & KF_EXTENDED) == KF_EXTENDED;
    if (isExtended || (key == VK_SHIFT && ::MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT))
    {
        key |= 0x100;
    }
#elif defined(Q_OS_OSX)
    if (key == 0)
    {
        key = ConvertQtKeyToSystemScanCode(qtEvent->key());
    }
#else
#error "Unsupported platform"
#endif

    bool isRepeated = qtEvent->isAutoRepeat();
    eModifierKeys modifierKeys = GetModifierKeys();
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_DOWN, key, modifierKeys, isRepeated));

    QString text = qtEvent->text();
    if (!text.isEmpty())
    {
        MainDispatcherEvent e = MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_CHAR, 0, modifierKeys, isRepeated);
        for (int i = 0, n = text.size(); i < n; ++i)
        {
            QCharRef charRef = text[i];
            e.keyEvent.key = charRef.unicode();
            mainDispatcher->PostEvent(e);
        }
    }
}

void WindowBackend::OnKeyReleased(QKeyEvent* qtEvent)
{
    //we don't support autorepeat key_up
    if (qtEvent->isAutoRepeat())
    {
        return;
    }
    uint32 key = qtEvent->nativeVirtualKey();
#if defined(Q_OS_WIN)
    // How to distinguish left and right shift, control and alt: http://stackoverflow.com/a/15977613
    uint32 lparam = qtEvent->nativeModifiers();
    uint32 scanCode = qtEvent->nativeScanCode();
    bool isExtended = (HIWORD(lparam) & KF_EXTENDED) == KF_EXTENDED;
    if (isExtended || (key == VK_SHIFT && ::MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT))
    {
        key |= 0x100;
    }
#elif defined(Q_OS_OSX)
    if (key == 0)
    {
        key = ConvertQtKeyToSystemScanCode(qtEvent->key());
    }
#else
#error "Unsupported platform"
#endif

    eModifierKeys modifierKeys = GetModifierKeys();
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_UP, key, modifierKeys, false));
}

void WindowBackend::DoResizeWindow(float32 width, float32 height)
{
    DVASSERT(renderWidget);
    renderWidget->resize(width, height);
}

void WindowBackend::DoCloseWindow()
{
    renderWidget->close();
}

void WindowBackend::DoSetTitle(const char8* title)
{
    renderWidget->setWindowTitle(title);
}

void WindowBackend::DoSetMinimumSize(float32 width, float32 height)
{
    renderWidget->setMinimumSize(static_cast<int>(width), static_cast<int>(height));
}

void WindowBackend::AcquireContext()
{
    renderWidget->AcquireContext();
}

void WindowBackend::ReleaseContext()
{
    renderWidget->ReleaseContext();
}

void WindowBackend::OnApplicationFocusChanged(bool isInFocus)
{
    if (renderWidget && renderWidget->IsInitialized())
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, isInFocus));
    }
}

void WindowBackend::Update()
{
    if (renderWidget != nullptr)
    {
        renderWidget->Update();
    }
}

DAVA::RenderWidget* WindowBackend::GetRenderWidget()
{
    if (renderWidget == nullptr)
    {
        renderWidget = new RenderWidget(this, 180.0f, 180.0f);
    }
    return renderWidget;
}

void WindowBackend::InitCustomRenderParams(rhi::InitParam& params)
{
    renderWidget->InitCustomRenderParams(params);
}

void WindowBackend::SetCursorCapture(eCursorCapture mode)
{
    // not implemented
}

void WindowBackend::SetCursorVisibility(bool visible)
{
    // not implemented
}

eModifierKeys WindowBackend::GetModifierKeys() const
{
    eModifierKeys result = eModifierKeys::NONE;
    Qt::KeyboardModifiers qmodifiers = QApplication::queryKeyboardModifiers();
    if (qmodifiers & Qt::ShiftModifier)
    {
        result |= eModifierKeys::SHIFT;
    }
    if (qmodifiers & Qt::AltModifier)
    {
        result |= eModifierKeys::ALT;
    }
#if defined(Q_OS_OSX)
    if (qmodifiers & Qt::ControlModifier)
    {
        result |= eModifierKeys::COMMAND;
    }
    if (qmodifiers & Qt::MetaModifier)
    {
        result |= eModifierKeys::CONTROL;
    }
#elif defined(Q_OS_WIN)
    if (qmodifiers & Qt::ControlModifier)
    {
        result |= eModifierKeys::CONTROL;
    }
#else
#error "Unsupported platform"
#endif
    return result;
}

eMouseButtons WindowBackend::GetMouseButton(Qt::MouseButton button)
{
    switch (button)
    {
    case Qt::LeftButton:
        return eMouseButtons::LEFT;
    case Qt::RightButton:
        return eMouseButtons::RIGHT;
    case Qt::MiddleButton:
        return eMouseButtons::MIDDLE;
    case Qt::XButton1:
        return eMouseButtons::EXTENDED1;
    case Qt::XButton2:
        return eMouseButtons::EXTENDED2;
    default:
        return eMouseButtons::NONE;
    }
}
    
#if defined(Q_OS_OSX)
class QtToSystemMacKeyTranslator
{
public:
    QtToSystemMacKeyTranslator()
    {
        keyTranslator[Qt::Key_Left] = 0x7B;
        keyTranslator[Qt::Key_Right] = 0x7C;
        keyTranslator[Qt::Key_Up] = 0x7E;
        keyTranslator[Qt::Key_Down] = 0x7D;
        keyTranslator[Qt::Key_Delete] = 0x75;
        keyTranslator[Qt::Key_Escape] = 0x35;
        keyTranslator[Qt::Key_Backspace] = 0x33;
        keyTranslator[Qt::Key_Enter] = 0x24;
        keyTranslator[Qt::Key_Tab] = 0x30;

        keyTranslator[Qt::Key_Meta] = 59;
        keyTranslator[Qt::Key_Alt] = 58;
        keyTranslator[Qt::Key_Shift] = 56;

        keyTranslator[Qt::Key_CapsLock] = 57;
        keyTranslator[Qt::Key_Control] = 55;
        keyTranslator[Qt::Key_Space] = 0x31;

        keyTranslator[Qt::Key_Equal] = 24;
        keyTranslator[Qt::Key_Minus] = 27;
        keyTranslator[Qt::Key_Period] = 47;
        keyTranslator[Qt::Key_Comma] = 43;
        keyTranslator[Qt::Key_Semicolon] = 41;
        keyTranslator[Qt::Key_Slash] = 44;
        keyTranslator[Qt::Key_BracketLeft] = 33;
        keyTranslator[Qt::Key_Backslash] = 42;
        keyTranslator[Qt::Key_BracketRight] = 30;
        keyTranslator[Qt::Key_Apostrophe] = 39;
        keyTranslator[Qt::Key_Insert] = 114;
        keyTranslator[Qt::Key_Home] = 115;
        keyTranslator[Qt::Key_PageUp] = 116;
        keyTranslator[Qt::Key_End] = 119;
        keyTranslator[Qt::Key_PageDown] = 121;
        keyTranslator[Qt::Key_multiply] = 67;

        keyTranslator[Qt::Key_A] = 0x00;
        keyTranslator[static_cast<Qt::Key>(1060)] = 0x00;
        keyTranslator[Qt::Key_B] = 0x0B;
        keyTranslator[Qt::Key_C] = 0x08;
        keyTranslator[Qt::Key_D] = 0x02;
        keyTranslator[Qt::Key_E] = 0x0E;
        keyTranslator[Qt::Key_F] = 0x03;
        keyTranslator[Qt::Key_G] = 0x05;
        keyTranslator[Qt::Key_H] = 0x04;
        keyTranslator[Qt::Key_I] = 0x22;
        keyTranslator[Qt::Key_J] = 0x26;
        keyTranslator[Qt::Key_K] = 0x28;
        keyTranslator[Qt::Key_L] = 0x25;
        keyTranslator[Qt::Key_N] = 0x2D;
        keyTranslator[Qt::Key_M] = 0x2E;
        keyTranslator[Qt::Key_O] = 0x1F;
        keyTranslator[Qt::Key_P] = 0x23;
        keyTranslator[Qt::Key_Q] = 0x0C;
        keyTranslator[Qt::Key_R] = 0x0F;
        keyTranslator[Qt::Key_S] = 0x01;
        keyTranslator[Qt::Key_T] = 0x11;
        keyTranslator[Qt::Key_U] = 0x20;
        keyTranslator[Qt::Key_V] = 0x09;
        keyTranslator[Qt::Key_W] = 0x0D;
        keyTranslator[Qt::Key_X] = 0x07;
        keyTranslator[Qt::Key_Y] = 0x10;
        keyTranslator[Qt::Key_Z] = 0x06;

        keyTranslator[Qt::Key_0] = 0x1D;
        keyTranslator[Qt::Key_1] = 0x12;
        keyTranslator[Qt::Key_2] = 0x13;
        keyTranslator[Qt::Key_3] = 0x14;
        keyTranslator[Qt::Key_4] = 0x15;
        keyTranslator[Qt::Key_5] = 0x17;
        keyTranslator[Qt::Key_6] = 0x16;
        keyTranslator[Qt::Key_7] = 0x1A;
        keyTranslator[Qt::Key_8] = 0x1C;
        keyTranslator[Qt::Key_9] = 0x19;

        keyTranslator[Qt::Key_F1] = 0x7A;
        keyTranslator[Qt::Key_F2] = 0x78;
        keyTranslator[Qt::Key_F3] = 0x63;
        keyTranslator[Qt::Key_F4] = 0x76;
        keyTranslator[Qt::Key_F5] = 0x60;
        keyTranslator[Qt::Key_F6] = 0x61;
        keyTranslator[Qt::Key_F7] = 0x62;
        keyTranslator[Qt::Key_F8] = 0x64;
        keyTranslator[Qt::Key_F9] = 0x65;
        keyTranslator[Qt::Key_F10] = 0x6D;
        keyTranslator[Qt::Key_F11] = 0x67;
        keyTranslator[Qt::Key_F12] = 0x6F;

        keyTranslator[Qt::Key_NumLock] = 71;
        keyTranslator[Qt::Key_F14] = 107;
        keyTranslator[Qt::Key_F13] = 105;
    }

    UnorderedMap<Qt::Key, uint32> keyTranslator;
};

uint32 WindowBackend::ConvertQtKeyToSystemScanCode(int key)
{
    static QtToSystemMacKeyTranslator tr;
    auto iter = tr.keyTranslator.find(static_cast<Qt::Key>(key));
    if (iter != tr.keyTranslator.end())
        return iter->second;

    DAVA::Logger::Warning("[WindowBackend::ConvertQtKeyToSystemScanCode] Unresolved Qt::Key: %d", key);
    return 0;
}

#endif

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
