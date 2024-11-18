#ifndef KEYBOARDLAYOUT_H
#define KEYBOARDLAYOUT_H

#include <cstdint>


class KeyboardLayoutPrivate;
class KeyboardLayout
{
public:
    KeyboardLayout();
    ~KeyboardLayout();

    bool IsKeyboardSupport() const;
    uint16_t GetKeyboardLayout() const;

private:
    KeyboardLayoutPrivate *pimpl;
};

#endif // KEYBOARDLAYOUT_H
