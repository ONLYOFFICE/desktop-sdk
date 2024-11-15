#include "keyboardlayout.h"
//#include <stdio.h>
#include <list>
#include <sstream>
#include <cctype>
#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <xkbcommon/xkbcommon-x11.h>
#include "unicode/locid.h"

#define LANGIDFROMLCID(lcid) ((uint16_t)(lcid))


static std::string trim(const std::string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (first != std::string::npos) {
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, last - first + 1);
    }
    return "";
}

static uint16_t layoutNameToLangId(const std::string &layoutName)
{
    if (!layoutName.empty()) {
        std::string languageDisplayName, language, script, region;
        size_t bkt1 = layoutName.find_first_of('(');
        languageDisplayName = (bkt1 != std::string::npos) ? trim(layoutName.substr(0, bkt1)) : layoutName;
        std::list<std::string> parts;
        size_t bkt2 = layoutName.find_last_of(')');
        if (bkt2 != std::string::npos && bkt1 < bkt2) {
            std::stringstream strs(layoutName.substr(bkt1 + 1, bkt2 - bkt1 - 1));
            std::string part;
            while (std::getline(strs, part, ','))
                parts.push_back(trim(part));
        }

        if (parts.size() > 0) {
            std::string part1(parts.front());
            if (part1.length() == 2 && std::isupper(part1[0]) && std::isupper(part1[1])) {
                region = std::move(part1);
                parts.pop_front();
            }
        }

        int32_t count;
        icu::UnicodeString uname;
        const icu::Locale *availableLocales = icu::Locale::getAvailableLocales(count);
        for (int32_t i = 0; i < count; ++i) {
            std::string name;
            if (language.empty()) {
                availableLocales[i].getDisplayLanguage(uname);
                uname.toUTF8String(name);
                if (name.find(languageDisplayName) != std::string::npos)
                    language = availableLocales[i].getLanguage();
                name.clear();
            }

            if (script.empty()) {
                availableLocales[i].getDisplayScript(uname);
                uname.toUTF8String(name);
                if (!name.empty()) {
                    for (auto it = parts.cbegin(); it != parts.cend(); it++) {
                        if (name.find(*it) != std::string::npos) {
                            script = availableLocales[i].getScript();
                            parts.erase(it);
                            break;
                        }
                    }
                }
                name.clear();
            }

            if (region.empty()) {
                availableLocales[i].getDisplayCountry(uname);
                uname.toUTF8String(name);
                if (!name.empty()) {
                    for (auto it = parts.cbegin(); it != parts.cend(); it++) {
                        if (name.find(*it) != std::string::npos) {
                            region = availableLocales[i].getCountry();
                            parts.erase(it);
                            break;
                        }
                    }
                }
            }

            if (!language.empty() && !region.empty() && !script.empty())
               break;
        }

        if (!language.empty()) {
            if (!script.empty())
                language.append("_" + script);
            else
            if (region.empty()) {
                // try prevent neutral language
                UErrorCode status = U_ZERO_ERROR;
                char fullName[ULOC_FULLNAME_CAPACITY];
                uloc_addLikelySubtags(language.c_str(), fullName, ULOC_FULLNAME_CAPACITY, &status);
                if (U_SUCCESS(status)) {
                    icu::Locale loc(fullName);
                    region = loc.getCountry();
                }
            }

            if (!region.empty())
                language.append("_" + region);

            // fprintf(stderr, "Canonical name: %s\n", language.c_str());
            icu::Locale loc(language.c_str());
            return LANGIDFROMLCID(loc.getLCID());
        }
    }
    return 0;
}

class KeyboardLayoutPrivate
{
public:
    KeyboardLayoutPrivate();
    ~KeyboardLayoutPrivate();

    xcb_connection_t *connection;
    xkb_context *context;
    int32_t device_id;

private:
    Display *display;
};

KeyboardLayoutPrivate::KeyboardLayoutPrivate() :
    connection(nullptr),
    context(nullptr),
    device_id(-1),
    display(nullptr)
{
    if ((display = XOpenDisplay(NULL)) != nullptr) {
        if ((context = xkb_context_new(XKB_CONTEXT_NO_FLAGS)) != nullptr) {
            connection = XGetXCBConnection(display);
            if ((device_id = xkb_x11_get_core_keyboard_device_id(connection)) == -1) {
                // fprintf(stderr, "Cannot get device id\n");
                xkb_context_unref(context); context = nullptr;
                XCloseDisplay(display); display = nullptr;
            }
        } else {
            // fprintf(stderr, "Cannot create context xkb\n");
            XCloseDisplay(display); display = nullptr;
        }
    } else {
        // fprintf(stderr, "Cannot open display\n");
    }
}

KeyboardLayoutPrivate::~KeyboardLayoutPrivate()
{
    if (context)
        xkb_context_unref(context);
    if (display)
        XCloseDisplay(display);
}


KeyboardLayout::KeyboardLayout() :
    pimpl(new KeyboardLayoutPrivate)
{

}

KeyboardLayout::~KeyboardLayout()
{
    delete pimpl, pimpl = nullptr;
}

bool KeyboardLayout::IsKeyboardSupport() const
{
    return pimpl->device_id != -1;
}

uint16_t KeyboardLayout::GetKeyboardLayout() const
{
    uint16_t layout = 0;
    if (pimpl->device_id != -1) {
        if (xkb_keymap *keymap = xkb_x11_keymap_new_from_device(pimpl->context, pimpl->connection, pimpl->device_id, XKB_KEYMAP_COMPILE_NO_FLAGS)) {
            if (xkb_state *state = xkb_x11_state_new_from_device(keymap, pimpl->connection, pimpl->device_id)) {
                xkb_layout_index_t num_layouts = xkb_keymap_num_layouts(keymap);

                for (xkb_layout_index_t i = 0; i < num_layouts; ++i) {
                    if (xkb_state_layout_index_is_active(state, i, XKB_STATE_LAYOUT_EFFECTIVE) == 1) {
                        if (const char *layout_name = xkb_keymap_layout_get_name(keymap, i)) {
                            // fprintf(stderr, "\nCurrent keyboard layout: %s\n", layout_name);
                            layout = layoutNameToLangId(layout_name);
                        }
                        break;
                    }
                }
                xkb_state_unref(state);
            } else {
                // fprintf(stderr, "Cannot create keyboard state\n");
            }
            xkb_keymap_unref(keymap);
        } else {
            // fprintf(stderr, "Cannot get keymap\n");
        }
    }
    return layout;
}
