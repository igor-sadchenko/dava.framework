#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#if !defined(__DAVAENGINE_COREV2__)

#include "UI/UITextField.h"
#include "Engine/Android/JNIBridge.h"

namespace DAVA
{
class JniTextField
{
public:
    JniTextField(uint32_t id);

    void Create(Rect rect);
    void Destroy();
    void UpdateRect(const Rect& rect);
    void SetText(const char* text);
    void SetTextColor(float r, float g, float b, float a);
    void SetFontSize(float size);
    void SetIsPassword(bool isPassword);
    void SetTextAlign(int32_t align);
    void SetTextUseRtlAlign(bool useRtlAlign);
    void SetInputEnabled(bool value);
    void SetAutoCapitalizationType(int32_t value);
    void SetAutoCorrectionType(int32_t value);
    void SetSpellCheckingType(int32_t value);
    void SetKeyboardAppearanceType(int32_t value);
    void SetKeyboardType(int32_t value);
    void SetReturnKeyType(int32_t value);
    void SetEnableReturnKeyAutomatically(bool value);
    void SetVisible(bool isVisible);
    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;
    void OpenKeyboard();
    void CloseKeyboard();
    uint32 GetCursorPos();
    void SetCursorPos(uint32 pos);
    void SetMaxLength(int32_t value);
    void SetMultiline(bool value);

private:
    uint32_t id;
    JNI::JavaClass jniTextField;
    Function<void(jint, jfloat, jfloat, jfloat, jfloat)> create;
    Function<void(jint)> destroy;
    Function<void(jint, jfloat, jfloat, jfloat, jfloat)> updateRect;
    Function<void(jint, jstring)> setText;
    Function<void(jint, jfloat, jfloat, jfloat, jfloat)> setTextColor;
    Function<void(jint, jfloat)> setFontSize;
    Function<void(jint, jboolean)> setIsPassword;
    Function<void(jint, jint)> setTextAlign;
    Function<void(jint, jboolean)> setTextUseRtlAlign;
    Function<void(jint, jboolean)> setInputEnabled;
    Function<void(jint, jint)> setAutoCapitalizationType;
    Function<void(jint, jint)> setAutoCorrectionType;
    Function<void(jint, jint)> setSpellCheckingType;
    Function<void(jint, jint)> setKeyboardAppearanceType;
    Function<void(jint, jint)> setKeyboardType;
    Function<void(jint, jint)> setReturnKeyType;
    Function<void(jint, jboolean)> setEnableReturnKeyAutomatically;
    Function<void(jint, jboolean)> setVisible;
    Function<void(jint, jboolean)> setRenderToTexture;
    Function<jboolean(jint)> isRenderToTexture;
    Function<void(jint)> openKeyboard;
    Function<void(jint)> closeKeyboard;
    Function<jint(jint)> getCursorPos;
    Function<void(jint, jint)> setCursorPos;
    Function<void(jint, jint)> setMaxLength;
    Function<void(jint, jboolean)> setMultiline;
};

class TextFieldPlatformImpl
{
public:
    TextFieldPlatformImpl(UITextField* textField);
    virtual ~TextFieldPlatformImpl();

    void Initialize()
    {
    }
    void OwnerIsDying()
    {
    }
    void SetDelegate(UITextFieldDelegate*)
    {
    }

    void OpenKeyboard();
    void CloseKeyboard();
    void GetText(WideString& string) const;
    void SetText(const WideString& string);
    void UpdateRect(const Rect& rect);

    void SetTextColor(const DAVA::Color& color);
    void SetFontSize(float size);

    void SetTextAlign(DAVA::int32 align);
    DAVA::int32 GetTextAlign();

    void SetTextUseRtlAlign(bool useRtlAlign);
    bool GetTextUseRtlAlign() const;

    void SetVisible(bool isVisible);

    void SetIsPassword(bool isPassword);

    void SetInputEnabled(bool value);

    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;

    // Keyboard traits.
    void SetAutoCapitalizationType(DAVA::int32 value);
    void SetAutoCorrectionType(DAVA::int32 value);
    void SetSpellCheckingType(DAVA::int32 value);
    void SetKeyboardAppearanceType(DAVA::int32 value);
    void SetKeyboardType(DAVA::int32 value);
    void SetReturnKeyType(DAVA::int32 value);
    void SetEnableReturnKeyAutomatically(bool value);
    uint32 GetCursorPos();
    void SetCursorPos(uint32 pos);
    void SetMaxLength(DAVA::int32 value);
    void SetMultiline(bool value);

    bool TextFieldKeyPressed(int32 replacementLocation, int32 replacementLength, WideString& text);
    void TextFieldOnTextChanged(const WideString& newText, const WideString& oldText);
    void TextFieldShouldReturn();
    void TextFieldKeyboardShown(const Rect& rect);
    void TextFieldKeyboardHidden();
    void TextFieldFocusChanged(bool hasFocus);
    static bool TextFieldKeyPressed(uint32_t id, int32 replacementLocation, int32 replacementLength, WideString& text);
    static void TextFieldOnTextChanged(uint32_t id, const WideString& newText, const WideString& oldText);
    static void TextFieldShouldReturn(uint32_t id);
    static void TextFieldKeyboardShown(uint32_t id, const Rect& rect);
    static void TextFieldKeyboardHidden(uint32_t id);
    static void TextFieldFocusChanged(uint32_t id, bool hasFocus);
    static void TextFieldUpdateTexture(uint32_t id, int32* pixels, int width, int height);

    void SystemDraw(const UIGeometricData& geometricData);

private:
    static TextFieldPlatformImpl* GetUITextFieldAndroid(uint32_t id);

protected:
    // Truncate the text to maxLength characters.
    WideString TruncateText(const WideString& text, int32 maxLength);

private:
    bool programmaticTextChange = false;
    std::shared_ptr<JniTextField> jniTextField;
    UITextField* textField = nullptr;
    static uint32_t sId;
    static UnorderedMap<uint32_t, TextFieldPlatformImpl*> controls;
    uint32_t id;
    Rect rect;
    WideString text;
    int32_t align;
    bool useRtlAlign;
};
};

#endif // !__DAVAENGINE_COREV2__
#endif //__DAVAENGINE_ANDROID__
