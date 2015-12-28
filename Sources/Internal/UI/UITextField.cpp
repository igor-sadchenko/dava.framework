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


#include "UI/UITextField.h"
#include "Input/KeyboardDevice.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/FontManager.h"
#include "FileSystem/YamlNode.h"

// Use NO_REQUIRED_SIZE to notify textFieldImpl->SetText that we don't want
// to enable of any kind of static text fitting
static const DAVA::Vector2 NO_REQUIRED_SIZE = DAVA::Vector2(-1, -1);

#if defined(__DAVAENGINE_ANDROID__)
#   include "UITextFieldAndroid.h"
#   include "Utils/UTF8Utils.h"

extern void CreateTextField(DAVA::UITextField *);
extern void ReleaseTextField();
extern void OpenKeyboard();
extern void CloseKeyboard();

#elif defined(__DAVAENGINE_IPHONE__)
#   include "UI/UITextFieldiPhone.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#   include "UI/UITextFieldWinUAP.h"
#else
#include "UI/UIStaticText.h"
#include "Platform/SystemTimer.h"
namespace DAVA
{
// This implementation simulate iOS/Android native contols,
// so no hierarchy for internal UIStaticText, and call UpdateRect
// every frame, and render directly in SyctemDraw. This helps
// to find similar bugs in all implementations
class TextFieldPlatformImpl
{
public:
    friend class UITextField;
    TextFieldPlatformImpl(UITextField* control)
        : staticText_(new UIStaticText(Rect(Vector2(0, 0), control->GetSize())))
        , control_(control)
    {
        staticText_->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
    }
    ~TextFieldPlatformImpl()
    {
        SafeRelease(staticText_);
        control_ = nullptr;
    }
    void CopyDataFrom(TextFieldPlatformImpl* t)
    {
        staticText_->CopyDataFrom(t->staticText_);
        cursorTime = t->cursorTime;
        showCursor = t->showCursor;
    }
    void OpenKeyboard()
    {
    }
    void CloseKeyboard()
    {
    }
    void SetRenderToTexture(bool)
    {
    }
    void SetIsPassword(bool)
    {
        needRedraw = true;
    }
    void SetFontSize(float32)
    {
        // TODO: implement in staticText_->SetFontSize(float32);
    }
    void SetText(const WideString& text_, const Vector2& requestedTextRectSize = Vector2(0, 0))
    {
        WideString prevText = staticText_->GetText();
        staticText_->SetText(text_, requestedTextRectSize);
        if (requestedTextRectSize != NO_REQUIRED_SIZE && control_->GetDelegate() && prevText != text_)
        {
            control_->GetDelegate()->TextFieldOnTextChanged(control_, text_, prevText);
        }
        needRedraw = true;
    }
    void UpdateRect(const Rect&)
    {
        // see comment for TextFieldPlatformImpl class above

        if (control_ == UIControlSystem::Instance()->GetFocusedControl())
        {
            float32 timeElapsed = SystemTimer::Instance()->FrameDelta();
            cursorTime += timeElapsed;

            if (cursorTime >= 0.5f)
            {
                cursorTime = 0;
                showCursor = !showCursor;
                needRedraw = true;
            }
        }
        else if (showCursor)
        {
            cursorTime = 0;
            showCursor = false;
            needRedraw = true;
        }

        if (!needRedraw)
        {
            return;
        }

        const WideString& txt = control_->GetVisibleText();
        if (control_ == UIControlSystem::Instance()->GetFocusedControl())
        {
            WideString txtWithCursor = txt + (showCursor ? L"_" : L" ");
            SetText(txtWithCursor, NO_REQUIRED_SIZE);
        }
        else
        {
            SetText(txt, NO_REQUIRED_SIZE);
        }
        needRedraw = false;
    }
    void SetAutoCapitalizationType(int32)
    {
    }
    void SetAutoCorrectionType(int32)
    {
    }
    void SetSpellCheckingType(int32)
    {
    }
    void SetKeyboardAppearanceType(int32)
    {
    }
    void SetKeyboardType(int32)
    {
    }
    void SetReturnKeyType(int32)
    {
    }
    void SetEnableReturnKeyAutomatically(int32)
    {
    }
    bool IsRenderToTexture() const
    {
        return false;
    }
    uint32 GetCursorPos() const
    {
        return 0;
    }
    void SetCursorPos(int32)
    {
    }
    void SetMaxLength(int32)
    {
    }
    void GetText(WideString&)
    {
    }
    void SetInputEnabled(bool, bool hierarchic = true)
    {
    }
    void SetVisible(bool v)
    {
        staticText_->SetVisible(v);
    }
    void SetFont(Font* f)
    {
        staticText_->SetFont(f);
    }
    Font* GetFont() const
    {
        return staticText_->GetFont();
    }
    void SetTextColor(Color c)
    {
        staticText_->SetTextColor(c);
    }
    void SetShadowOffset(const Vector2& v)
    {
        staticText_->SetShadowOffset(v);
    }
    void SetShadowColor(Color c)
    {
        staticText_->SetShadowColor(c);
    }
    void SetTextAlign(int32 align)
    {
        staticText_->SetTextAlign(align);
    }
    TextBlock::eUseRtlAlign GetTextUseRtlAlign()
    {
        return staticText_->GetTextUseRtlAlign();
    }
    void SetTextUseRtlAlign(TextBlock::eUseRtlAlign align)
    {
        staticText_->SetTextUseRtlAlign(align);
    }
    void SetSize(const Vector2 vector2)
    {
        staticText_->SetSize(vector2);
    }
    void SetMultiline(bool is_multiline)
    {
        staticText_->SetMultiline(is_multiline);
    }
    Color GetTextColor()
    {
        return staticText_->GetTextColor();
    }
    Vector2 GetShadowOffset()
    {
        return staticText_->GetShadowOffset();
    }
    Color GetShadowColor()
    {
        return staticText_->GetShadowColor();
    }
    int32 GetTextAlign()
    {
        return staticText_->GetTextAlign();
    }
    void SetRect(const Rect& rect)
    {
        staticText_->SetSize(rect.GetSize());
    }
    void SystemDraw(const UIGeometricData& d)
    {
        // see comment for TextFieldPlatformImpl class above
        staticText_->SystemDraw(d);
    }

private:
    UIStaticText* staticText_ = nullptr;
    UITextField* control_ = nullptr;
    float32 cursorTime = 0.0f;
    bool needRedraw = true;
    bool showCursor = true;
};
} // end namespace DAVA
#endif

#if defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_WIN_UAP__)
#   define DAVA_TEXTFIELD_USE_NATIVE
#endif

namespace DAVA
{
UITextField::UITextField(const Rect& rect)
    : UIControl(rect)
{
    textFieldImpl = new TextFieldPlatformImpl(this);
    textFieldImpl->SetVisible(false);

    SetupDefaults();
}
    
void UITextField::SetupDefaults()
{
    SetInputEnabled(true, false);
    
    SetAutoCapitalizationType(AUTO_CAPITALIZATION_TYPE_SENTENCES);
    SetAutoCorrectionType(AUTO_CORRECTION_TYPE_DEFAULT);
    SetSpellCheckingType(SPELL_CHECKING_TYPE_DEFAULT);
    SetKeyboardAppearanceType(KEYBOARD_APPEARANCE_DEFAULT);
    SetKeyboardType(KEYBOARD_TYPE_DEFAULT);
    SetReturnKeyType(RETURN_KEY_DEFAULT);
    SetEnableReturnKeyAutomatically(false);
    SetTextUseRtlAlign(TextBlock::RTL_DONT_USE);
    
    SetMaxLength(-1);
    
    SetIsPassword(false);
    SetTextColor(GetTextColor());
    SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    
    SetFontSize(26); //12 is default size for IOS
    
    SetText(L"");
    SetRenderToTexture(true);
}

UITextField::~UITextField()
{
    SafeDelete(textFieldImpl);
    UIControl::RemoveAllControls();
}

void UITextField::OpenKeyboard()
{
    textFieldImpl->OpenKeyboard();
}

void UITextField::CloseKeyboard()
{
    textFieldImpl->CloseKeyboard();
}

void UITextField::Update(float32 timeElapsed)
{
    textFieldImpl->UpdateRect(GetGeometricData().GetUnrotatedRect());
}

void UITextField::WillAppear()
{
    if (delegate != nullptr && delegate->IsTextFieldShouldSetFocusedOnAppear(this)) 
    {
        UIControlSystem::Instance()->SetFocusedControl(this, false);
    }
}

void UITextField::DidAppear()
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldImpl->ShowField();
    textFieldImpl->SetVisible(IsOnScreen());
#endif
}

void UITextField::WillDisappear()
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldImpl->HideField();
#endif
}
    
void UITextField::OnFocused()
{
    SetRenderToTexture(false);
    textFieldImpl->OpenKeyboard();
}

void UITextField::SetFocused()
{
    UIControlSystem::Instance()->SetFocusedControl(this, true);
}

void UITextField::OnFocusLost(UIControl *newFocus)
{
    SetRenderToTexture(true);

    textFieldImpl->CloseKeyboard();

    if (delegate != nullptr)
    {
        delegate->TextFieldLostFocus(this);
    }
}

bool UITextField::IsLostFocusAllowed(UIControl *newFocus)
{
    if (delegate != nullptr)
    {
        return delegate->IsTextFieldCanLostFocus(this);
    }
    return true;
}

void UITextField::ReleaseFocus()
{
    if(this == UIControlSystem::Instance()->GetFocusedControl())
    {
        UIControlSystem::Instance()->SetFocusedControl(nullptr, true);
    }
}
    
void UITextField::SetFont(Font * font)
{
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    textFieldImpl->SetFont(font);
#endif  // !defined(DAVA_TEXTFIELD_USE_NATIVE)
}

void UITextField::SetTextColor(const Color& fontColor)
{
    textFieldImpl->SetTextColor(fontColor);
}

void UITextField::SetShadowOffset(const DAVA::Vector2 &offset)
{
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    textFieldImpl->SetShadowOffset(offset);
#endif
}

void UITextField::SetShadowColor(const Color& color)
{
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    textFieldImpl->SetShadowColor(color);
#endif
}

void UITextField::SetTextAlign(int32 align)
{
    textFieldImpl->SetTextAlign(align);
}

TextBlock::eUseRtlAlign UITextField::GetTextUseRtlAlign() const
{
#ifdef DAVA_TEXTFIELD_USE_NATIVE
    return textFieldImpl->GetTextUseRtlAlign() ? TextBlock::RTL_USE_BY_CONTENT : TextBlock::RTL_DONT_USE;
#else
    return textFieldImpl->GetTextUseRtlAlign();
#endif
}

void UITextField::SetTextUseRtlAlign(TextBlock::eUseRtlAlign useRtlAlign)
{
#ifdef DAVA_TEXTFIELD_USE_NATIVE
    textFieldImpl->SetTextUseRtlAlign(useRtlAlign == TextBlock::RTL_USE_BY_CONTENT);
#else
    textFieldImpl->SetTextUseRtlAlign(useRtlAlign);
#endif
}

void UITextField::SetTextUseRtlAlignFromInt(int32 value)
{
    SetTextUseRtlAlign(static_cast<TextBlock::eUseRtlAlign>(value));
}
    
int32 UITextField::GetTextUseRtlAlignAsInt() const
{
    return static_cast<TextBlock::eUseRtlAlign>(GetTextUseRtlAlign());
}

void UITextField::SetFontSize(float32 size)
{
    textFieldImpl->SetFontSize(size);
}

void UITextField::SetDelegate(UITextFieldDelegate * _delegate)
{
    delegate = _delegate;
#if defined(__DAVAENGINE_WIN_UAP__)
    textFieldImpl->SetDelegate(_delegate);
#endif
}

UITextFieldDelegate * UITextField::GetDelegate()
{
    return delegate;
}

void UITextField::SetSpriteAlign(int32 align)
{
    UIControl::SetSpriteAlign(align);
}

void UITextField::SetSize(const DAVA::Vector2 &newSize)
{
    UIControl::SetSize(newSize);
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    textFieldImpl->SetSize(newSize);
#endif
}
    
void UITextField::SetPosition(const DAVA::Vector2 &position)
{
    UIControl::SetPosition(position);
}

void UITextField::SetMultiline(bool value)
{
    if (value != isMultiline_)
    {
        isMultiline_ = value;
        textFieldImpl->SetMultiline(isMultiline_);
    }
}

bool UITextField::IsMultiline() const
{
    return isMultiline_;
}
    
void UITextField::SetText(const WideString& text_)
{
    textFieldImpl->SetText(text_);
    text = text_;
}

const WideString & UITextField::GetText()
{
    textFieldImpl->GetText(text);
    return text;
}
    
Font* UITextField::GetFont() const
{
#if defined(DAVA_TEXTFIELD_USE_NATIVE)
    return nullptr;
#else
    return textFieldImpl->GetFont();
#endif
}

Color UITextField::GetTextColor() const
{
#if defined(DAVA_TEXTFIELD_USE_NATIVE)
    return Color::White;
#else
    return textFieldImpl->GetTextColor();
#endif
}

Vector2 UITextField::GetShadowOffset() const
{
#if defined(DAVA_TEXTFIELD_USE_NATIVE)
    return Vector2(0, 0);
#else
    return textFieldImpl->GetShadowOffset();
#endif
}

Color UITextField::GetShadowColor() const
{
#if defined(DAVA_TEXTFIELD_USE_NATIVE)
    return Color::White;
#else
    return textFieldImpl->GetShadowColor();
#endif
}

int32 UITextField::GetTextAlign() const
{
    return textFieldImpl->GetTextAlign();
}

void UITextField::Input(UIEvent *currentInput)
{
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    if (nullptr == delegate)
    {
        return;
    }

    if (this != UIControlSystem::Instance()->GetFocusedControl())
        return;

    if (currentInput->phase == UIEvent::Phase::KEY_DOWN ||
        currentInput->phase == UIEvent::Phase::KEY_DOWN_REPEAT)
    {
        if (currentInput->tid == DVKEY_BACKSPACE)
        {
            WideString str;
            int32 length = static_cast<int32>(GetText().length() - 1);
            if (delegate->TextFieldKeyPressed(this, length, 1, str))
            {
                SetText(GetAppliedChanges(length, 1, str));
            }
        }
        else if (currentInput->tid == DVKEY_ENTER)
        {
            delegate->TextFieldShouldReturn(this);
        }
        else if (currentInput->tid == DVKEY_ESCAPE)
        {
            delegate->TextFieldShouldCancel(this);
        }
    }
    else if (currentInput->phase == UIEvent::Phase::CHAR ||
             currentInput->phase == UIEvent::Phase::CHAR_REPEAT)
    {
        if ('\r' == currentInput->keyChar)
        {
            if (IsMultiline())
            {
                currentInput->keyChar = '\n';
            }
            else
            {
                currentInput->keyChar = '\0';
            }
        }
        if (currentInput->keyChar != 0 && currentInput->keyChar != '\b' && currentInput->keyChar != 0x7f // 0x7f del key (on mac backspace)
            && currentInput->keyChar != 0xf728) // on mac fn+backspace
        {
            WideString str;
            str += currentInput->keyChar;
            int32 length = static_cast<int32>(GetText().length());
            if (delegate->TextFieldKeyPressed(this, length, 0, str))
            {
                SetText(GetAppliedChanges(length, 0, str));
            }
        }
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
#endif
}
    
WideString UITextField::GetAppliedChanges(int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{//TODO: fix this for copy/paste
    WideString txt = GetText();
    
    if(replacementLocation >= 0)
    {
        if (replacementLocation <= static_cast<int32>(txt.length()))
        {
            txt.replace(replacementLocation, replacementLength, replacementString);
        }
        else
        {
            Logger::Error("[UITextField::GetAppliedChanges] - index out of bounds.");
        }
    }
    
    return txt;
}

void UITextField::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
    UIControl::LoadFromYamlNode(node, loader);

    const YamlNode * textNode = node->Get("text");
    if (textNode)
    {
        SetText(textNode->AsWString());
    }

    const YamlNode * fontNode = node->Get("font");
    if (fontNode)
    {
        Font * font = loader->GetFontByName(fontNode->AsString());
        if (font)
        {
            SetFont(font);
            SetFontSize((float32)font->GetFontHeight());
        }
    }
    
    const YamlNode * passwordNode = node->Get("isPassword");
    if (passwordNode)
    {
        SetIsPassword(passwordNode->AsBool());
    }

    // Keyboard customization params.
    const YamlNode* autoCapitalizationTypeNode = node->Get("autoCapitalizationType");
    if (autoCapitalizationTypeNode)
    {
        SetAutoCapitalizationType((eAutoCapitalizationType)autoCapitalizationTypeNode->AsInt32());
    }

    const YamlNode* autoCorrectionTypeNode = node->Get("autoCorrectionType");
    if (autoCorrectionTypeNode)
    {
        SetAutoCorrectionType((eAutoCorrectionType)autoCorrectionTypeNode->AsInt32());
    }

    const YamlNode* spellCheckingTypeNode = node->Get("spellCheckingType");
    if (spellCheckingTypeNode)
    {
        SetSpellCheckingType((eSpellCheckingType)spellCheckingTypeNode->AsInt32());
    }

    const YamlNode* keyboardAppearanceTypeNode = node->Get("keyboardAppearanceType");
    if (keyboardAppearanceTypeNode)
    {
        SetKeyboardAppearanceType((eKeyboardAppearanceType)keyboardAppearanceTypeNode->AsInt32());
    }

    const YamlNode* keyboardTypeNode = node->Get("keyboardType");
    if (keyboardTypeNode)
    {
        SetKeyboardType((eKeyboardType)keyboardTypeNode->AsInt32());
    }

    const YamlNode* returnKeyTypeNode = node->Get("returnKeyType");
    if (returnKeyTypeNode)
    {
        SetReturnKeyType((eReturnKeyType)returnKeyTypeNode->AsInt32());
    }

    const YamlNode* enableReturnKeyAutomaticallyNode = node->Get("enableReturnKeyAutomatically");
    if (enableReturnKeyAutomaticallyNode)
    {
        SetEnableReturnKeyAutomatically(enableReturnKeyAutomaticallyNode->AsBool());
    }

#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    textFieldImpl->SetRect(Rect(0, 0, GetRect().dx, GetRect().dy));

    const YamlNode* shadowColorNode = node->Get("shadowcolor");
    const YamlNode* shadowOffsetNode = node->Get("shadowoffset");
    if (shadowColorNode)
    {
        SetShadowColor(shadowColorNode->AsColor());
    }
    if (shadowOffsetNode)
    {
        SetShadowOffset(shadowOffsetNode->AsVector2());
    }
#endif

    const YamlNode * textColorNode = node->Get("textcolor");
    const YamlNode * textAlignNode = node->Get("textalign");

    if(textColorNode)
    {
        SetTextColor(textColorNode->AsColor());
    }

    if(textAlignNode)
    {
        SetTextAlign(loader->GetAlignFromYamlNode(textAlignNode));
    }

    const YamlNode * textUseRtlAlign = node->Get("textUseRtlAlign");
    if(textUseRtlAlign)
    {
        SetTextUseRtlAlign(static_cast<TextBlock::eUseRtlAlign>(textUseRtlAlign->AsInt32()));
    }

    const YamlNode* maxLengthNode = node->Get("maxLength");
    if (maxLengthNode)
    {
        SetMaxLength(maxLengthNode->AsInt32());
    }
}

YamlNode * UITextField::SaveToYamlNode(UIYamlLoader * loader)
{
    ScopedPtr<UITextField> baseTextField(new UITextField());

    YamlNode *node = UIControl::SaveToYamlNode(loader);

    //Text
    if (baseTextField->GetText() != this->GetText())
    {
        node->Set("text", GetText());
    }

    //Font
    //Get font name and put it here
    node->Set("font", FontManager::Instance()->GetFontName(this->GetFont()));
    
    //TextColor
    const Color &textColor = GetTextColor();
    if (baseTextField->GetTextColor() != textColor)
    {
        node->Set("textcolor", VariantType(textColor));
    }

    // ShadowColor
    const Color &shadowColor = GetShadowColor();
    if (baseTextField->GetShadowColor() != GetShadowColor())
    {
        node->Set("shadowcolor", VariantType(shadowColor));
    }

    // ShadowOffset
    if (baseTextField->GetShadowOffset() != GetShadowOffset())
    {
        node->Set("shadowoffset", GetShadowOffset());
    }

    // Text align
    if (baseTextField->GetTextAlign() != GetTextAlign())
    {
        node->SetNodeToMap("textalign", loader->GetAlignNodeValue(GetTextAlign()));
    }
    
    // Text use rtl align
    if (baseTextField->GetTextUseRtlAlign() != GetTextUseRtlAlign())
    {
        node->Set("textUseRtlAlign", this->GetTextUseRtlAlign());
    }
    

    // Draw Type must be overwritten fot UITextField.
    UIControlBackground::eDrawType drawType = GetBackground()->GetDrawType();
    
    if (baseTextField->GetBackground()->GetDrawType() != drawType)
    {
        node->Set("drawType", loader->GetDrawTypeNodeValue(drawType));
    }

    // Is password
    if (baseTextField->IsPassword() != IsPassword())
    {
        node->Set("isPassword", IsPassword());
    }

    // Keyboard customization params.
    if (baseTextField->GetAutoCapitalizationType() != GetAutoCapitalizationType())
    {
        node->Set("autoCapitalizationType", GetAutoCapitalizationType());
    }
    if (baseTextField->GetAutoCorrectionType() != GetAutoCorrectionType())
    {
        node->Set("autoCorrectionType", GetAutoCorrectionType());
    }
    if (baseTextField->GetSpellCheckingType() != GetSpellCheckingType())
    {
        node->Set("spellCheckingType", GetSpellCheckingType());
    }
    if (baseTextField->GetKeyboardAppearanceType() != GetKeyboardAppearanceType())
    {
        node->Set("keyboardAppearanceType", GetKeyboardAppearanceType());
    }
    if (baseTextField->GetKeyboardType() != GetKeyboardType())
    {
        node->Set("keyboardType", GetKeyboardType());
    }
    if (baseTextField->GetReturnKeyType() != GetReturnKeyType())
    {
        node->Set("returnKeyType", GetReturnKeyType());
    }
    if (baseTextField->IsEnableReturnKeyAutomatically() != IsEnableReturnKeyAutomatically())
    {
        node->Set("enableReturnKeyAutomatically", IsEnableReturnKeyAutomatically());
    }

    // Max length.
    if (baseTextField->GetMaxLength() != GetMaxLength())
    {
        node->Set("maxLength", GetMaxLength());
    }

    return node;
}

UITextField* UITextField::Clone()
{
    UITextField *t = new UITextField();
    t->CopyDataFrom(this);
    return t;
}

void UITextField::CopyDataFrom(UIControl *srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    UITextField* t = static_cast<UITextField*>(srcControl);

    isPassword = t->isPassword;
    SetText(t->text);
    SetRect(t->GetRect());
    
    cursorBlinkingTime = t->cursorBlinkingTime;
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    textFieldImpl->CopyDataFrom(t->textFieldImpl);
#endif

    SetAutoCapitalizationType(t->GetAutoCapitalizationType());
    SetAutoCorrectionType(t->GetAutoCorrectionType());
    SetSpellCheckingType(t->GetSpellCheckingType());
    SetKeyboardAppearanceType(t->GetKeyboardAppearanceType());
    SetKeyboardType(t->GetKeyboardType());
    SetReturnKeyType(t->GetReturnKeyType());
    SetEnableReturnKeyAutomatically(t->IsEnableReturnKeyAutomatically());
    SetTextUseRtlAlign(t->GetTextUseRtlAlign());
    SetMaxLength(t->GetMaxLength());
}
    
void UITextField::SetIsPassword(bool isPassword_)
{
    isPassword = isPassword_;
    textFieldImpl->SetIsPassword(isPassword_);
}
    
bool UITextField::IsPassword() const
{
    return isPassword;
}
    
WideString UITextField::GetVisibleText() const
{
    if (!isPassword)
    {
        return text;
    }

    return WideString(text.length(), L'*');
}
    
int32 UITextField::GetAutoCapitalizationType() const
{
    return autoCapitalizationType;
}

void UITextField::SetAutoCapitalizationType(int32 value)
{
    autoCapitalizationType = static_cast<eAutoCapitalizationType>(value);
    textFieldImpl->SetAutoCapitalizationType(value);
}

int32 UITextField::GetAutoCorrectionType() const
{
    return autoCorrectionType;
}

void UITextField::SetAutoCorrectionType(int32 value)
{
    autoCorrectionType = static_cast<eAutoCorrectionType>(value);
    textFieldImpl->SetAutoCorrectionType(value);
}

int32 UITextField::GetSpellCheckingType() const
{
    return spellCheckingType;
}

void UITextField::SetSpellCheckingType(int32 value)
{
    spellCheckingType = static_cast<eSpellCheckingType>(value);
    textFieldImpl->SetSpellCheckingType(value);
}

int32 UITextField::GetKeyboardAppearanceType() const
{
    return keyboardAppearanceType;
}

void UITextField::SetKeyboardAppearanceType(int32 value)
{
    keyboardAppearanceType = static_cast<eKeyboardAppearanceType>(value);
    textFieldImpl->SetKeyboardAppearanceType(value);
}

int32 UITextField::GetKeyboardType() const
{
    return keyboardType;
}

void UITextField::SetKeyboardType(int32 value)
{
    keyboardType = static_cast<eKeyboardType>(value);
    textFieldImpl->SetKeyboardType(value);
}

int32 UITextField::GetReturnKeyType() const
{
    return returnKeyType;
}

void UITextField::SetReturnKeyType(int32 value)
{
    returnKeyType = static_cast<eReturnKeyType>(value);
    textFieldImpl->SetReturnKeyType(value);
}

bool UITextField::IsEnableReturnKeyAutomatically() const
{
    return enableReturnKeyAutomatically;
}

void UITextField::SetEnableReturnKeyAutomatically(bool value)
{
    enableReturnKeyAutomatically = value;
    textFieldImpl->SetEnableReturnKeyAutomatically(value);
}

void UITextField::SetInputEnabled(bool isEnabled, bool hierarchic)
{
    UIControl::SetInputEnabled(isEnabled, hierarchic);
    textFieldImpl->SetInputEnabled(isEnabled);
}

void UITextField::SetRenderToTexture(bool value)
{
    // Workaround! Users need scrolling of large texts in
    // multiline mode so we have to disable render into texture
    if (isMultiline_)
    {
        value = false;
    }

    textFieldImpl->SetRenderToTexture(value);
}

bool UITextField::IsRenderToTexture() const
{
    return textFieldImpl->IsRenderToTexture();
}

uint32 UITextField::GetCursorPos()
{
    return textFieldImpl->GetCursorPos();
}

void UITextField::SetCursorPos(uint32 pos)
{
    textFieldImpl->SetCursorPos(pos);
}

void UITextField::SetMaxLength(int32 newMaxLength)
{
    maxLength = Max(-1, newMaxLength); //-1 valid value
    textFieldImpl->SetMaxLength(maxLength);
}

int32 UITextField::GetMaxLength() const
{
    return maxLength;
}

void UITextField::WillBecomeVisible()
{
    UIControl::WillBecomeVisible();
    textFieldImpl->SetVisible(visible);
}

void UITextField::WillBecomeInvisible()
{
    UIControl::WillBecomeInvisible();
    textFieldImpl->SetVisible(false);
}

String UITextField::GetFontPresetName() const
{
    String name;
    Font* font = GetFont();
    if (font != nullptr)
    {
        name = FontManager::Instance()->GetFontName(font);
    }
    return name;
}

void UITextField::SetFontByPresetName(const String &presetName)
{
    Font *font = nullptr;
    if (!presetName.empty())
    {
        font = FontManager::Instance()->GetFont(presetName);
    }

    SetFont(font);
    if (font)
    {
        SetFontSize(static_cast<float32>(font->GetFontHeight()));
    }
}

void UITextField::SystemDraw(const UIGeometricData& geometricData)
{
    UIControl::SystemDraw(geometricData);

    UIGeometricData localData = GetLocalGeometricData();
    localData.AddGeometricData(geometricData);
    textFieldImpl->SystemDraw(localData);
}

}   // namespace DAVA
