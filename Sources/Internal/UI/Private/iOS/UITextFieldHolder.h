#ifndef __DAVAENGINE_UI_TEXT_FIELD_HOLDER_H__
#define __DAVAENGINE_UI_TEXT_FIELD_HOLDER_H__

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <UIKit/UIKit.h>
#import "UI/UITextField.h"

namespace DAVA
{
class Window;
}

@interface UITextFieldHolder : UIView<UITextFieldDelegate, UITextViewDelegate>
{
    NSString* cachedText;
    BOOL isKeyboardHidden;
#if defined(__DAVAENGINE_COREV2__)
    DAVA::Window* window;
#endif
@public
    // hold single line text field if user switch to multiline mode
    // otherwise nullptr
    UITextField* textField;
    // hold UITextField(singleline) or UITextView(multiline)
    UIView* textCtrl;
    DAVA::UITextField* cppTextField;
    BOOL textInputAllowed;
    BOOL useRtlAlign;

    CGRect lastKeyboardFrame;
}

- (void)setTextField:(DAVA::UITextField*)tf;
- (id)init;
#if defined(__DAVAENGINE_COREV2__)
- (void)attachWindow:(DAVA::Window*)w;
#endif
- (void)dealloc;
- (BOOL)textFieldShouldReturn:(UITextField*)textField;
- (BOOL)textField:(UITextField*)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString*)string;
- (BOOL)textView:(UITextView*)textView_ shouldChangeTextInRange:(NSRange)range replacementText:(NSString*)string;

- (void)dropCachedText;
- (void)setIsPassword:(bool)isPassword;
- (void)setTextInputAllowed:(bool)value;
- (void)setUseRtlAlign:(bool)value;

- (void)eventEditingChanged:(UIView*)sender;
- (void)textViewDidChange:(UITextView*)textView;

- (void)setupTraits;

- (UITextAutocapitalizationType)convertAutoCapitalizationType:(DAVA::UITextField::eAutoCapitalizationType)davaType;
- (UITextAutocorrectionType)convertAutoCorrectionType:(DAVA::UITextField::eAutoCorrectionType)davaType;
- (UITextSpellCheckingType)convertSpellCheckingType:(DAVA::UITextField::eSpellCheckingType)davaType;
- (BOOL)convertEnablesReturnKeyAutomatically:(bool)davaType;
- (UIKeyboardAppearance)convertKeyboardAppearanceType:(DAVA::UITextField::eKeyboardAppearanceType)davaType;
- (UIKeyboardType)convertKeyboardType:(DAVA::UITextField::eKeyboardType)davaType;
- (UIReturnKeyType)convertReturnKeyType:(DAVA::UITextField::eReturnKeyType)davaType;

@end


#endif //#if defined (__DAVAENGINE_IPHONE__)

#endif // __DAVAENGINE_UI_TEXT_FIELD_HOLDER_H__
