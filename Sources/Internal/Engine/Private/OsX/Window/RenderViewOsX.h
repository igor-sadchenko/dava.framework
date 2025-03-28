#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSOpenGLView.h>

#include "Engine/Private/EnginePrivateFwd.h"

@class NSTrackingArea;

// Subclass of NSOpenGLView
// Responsibilities:
//  - OpenGL-related tasks
//  - mouse and event forwarding to WindowNativeBridge
@interface RenderView : NSOpenGLView
{
    DAVA::Private::WindowNativeBridge* bridge;
    NSTrackingArea* trackingArea;
}

@property DAVA::float32 backbufferScale;

- (id)initWithBridge:(DAVA::Private::WindowNativeBridge*)nativeBridge;
@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
