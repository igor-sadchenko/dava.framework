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

#ifndef __UIMOVIE_TEST_H__
#define __UIMOVIE_TEST_H__

#include "DAVAEngine.h"

#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

class UIMovieTest : public BaseScreen
{
protected:
    virtual ~UIMovieTest() = default;

public:
    UIMovieTest();

    void LoadResources() override;
    void UnloadResources() override;

    void DidAppear() override;
    void Update(float32 timeElapsed) override;

private:
    void UpdatePlayerStateText();
    UIButton* CreateUIButton(Font* font, const Rect& rect, const String& text,
                             void (UIMovieTest::*onClick)(BaseObject*, void*, void*));

    void ButtonPressed(BaseObject *obj, void *data, void *callerData);
    void ScaleButtonPressed(BaseObject *obj, void *data, void *callerData);

private:
    UIMovieView* movieView = nullptr;

    // Control buttons.
    UIButton* playButton = nullptr;
    UIButton* stopButton = nullptr;
    UIButton* pauseButton = nullptr;
    UIButton* resumeButton = nullptr;
    UIButton* hideButton = nullptr;
    UIButton* showButton = nullptr;

    UIButton* buttonScale0 = nullptr;
    UIButton* buttonScale1 = nullptr;
    UIButton* buttonScale2 = nullptr;
    UIButton* buttonScale3 = nullptr;

    UIStaticText* playerStateText = nullptr;
};

#endif  // __UIMOVIE_TEST_H__
