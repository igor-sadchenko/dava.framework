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


#ifndef __TEXT_ALIGH_TEST_H__
#define __TEXT_ALIGH_TEST_H__

#include "Infrastructure/BaseScreen.h"

class StaticTextTest : public BaseScreen
{
public:
    StaticTextTest();

    void LoadResources() override;
    void UnloadResources() override;
    
    void SetPreviewText(const DAVA::WideString& text);
    void SetPreviewAlign(DAVA::int32 align);
    void SetPreviewFitting(DAVA::int32 fitting);
    void SetPreviewRequiredTextSize(bool enable);
    void SetPreviewMultiline(DAVA::int32 multilineType);
    
private:
    DAVA::UIButton* CreateButton(const DAVA::WideString& caption, const DAVA::Rect& rect, DAVA::int32 tag, DAVA::Font* font, const DAVA::Message& msg);
    
    void OnAlignButtonClick(BaseObject* sender, void * data, void * callerData);
    void OnFittingButtonClick(BaseObject* sender, void * data, void * callerData);
    void OnRequireTextSizeButtonClick(BaseObject* sender, void * data, void * callerData);
    void OnMultilineButtonClick(BaseObject* sender, void * data, void * callerData);

    DAVA::UIStaticText* previewText = nullptr;
    DAVA::UITextField* inputText = nullptr;
    DAVA::UITextFieldDelegate* inputDelegate = nullptr;
    DAVA::UIButton* requireTextSizeButton = nullptr;
    DAVA::List<DAVA::UIButton*> alignButtons;
    DAVA::List<DAVA::UIButton*> fittingButtons;
    DAVA::List<DAVA::UIButton*> multilineButtons;
    
    bool needRequiredSize = false;
};

#endif //__MULTILINETEST_TEST_H__
