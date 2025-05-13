#ifndef KEYBOARDLAYOUT_H
#define KEYBOARDLAYOUT_H

#ifdef __linux__
# include <cstdint>
#endif
#include <vector>
#include <string>


class KeyboardLayoutPrivate;
class KeyboardLayout
{
public:
    KeyboardLayout();
    ~KeyboardLayout();

    void GetKeyboardLayouts(std::vector<std::pair<std::string, std::string>>&) const;
#ifdef __linux__
    bool IsKeyboardSupport() const;
    uint16_t GetKeyboardLayout() const;

private:
    KeyboardLayoutPrivate *pimpl;
#endif
};

#endif // KEYBOARDLAYOUT_H
