#include "keyboardlayout.h"
#include "unicode/locid.h"
#ifdef _WIN32
# include <Windows.h>
#else
//#include <stdio.h>
#include <list>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <xkbcommon/xkbcommon-x11.h>

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

static std::string layoutNameToLocaleName(const std::string &layoutName)
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
        icu::Locale loc("en");
        icu::UnicodeString uname;
        const icu::Locale *availableLocales = icu::Locale::getAvailableLocales(count);
        for (int32_t i = 0; i < count; ++i) {
            std::string name;
            if (language.empty()) {
                availableLocales[i].getDisplayLanguage(loc, uname);
                uname.toUTF8String(name);
                if (name.find(languageDisplayName) != std::string::npos)
                    language = availableLocales[i].getLanguage();
                name.clear();
            }

            if (script.empty()) {
                availableLocales[i].getDisplayScript(loc, uname);
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
                availableLocales[i].getDisplayCountry(loc, uname);
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
            return language;
        }
    }
    return "";
}

static uint16_t layoutNameToLangId(const std::string &layoutName)
{
    std::string localeName = layoutNameToLocaleName(layoutName);
    if (!localeName.empty()) {
        icu::Locale loc(localeName.c_str());
        return LANGIDFROMLCID(loc.getLCID());
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
#endif

static std::string getNativeLanguageName(const std::string &localeName)
{
    std::string displayName;
    icu::Locale loc(localeName.c_str());
    icu::UnicodeString uname;
    loc.getDisplayLanguage(loc, uname);
    if (!uname.isEmpty()) {
        icu::UnicodeString first = uname.tempSubString(0, 1);
        icu::UnicodeString rest = uname.tempSubString(1);
        first.toUpper();
        uname = first + rest;
        uname.toUTF8String(displayName);
    }
    return displayName;
}

KeyboardLayout::KeyboardLayout()
#ifdef __linux__
    : pimpl(new KeyboardLayoutPrivate)
#endif
{

}

KeyboardLayout::~KeyboardLayout()
{
#ifdef __linux__
    delete pimpl, pimpl = nullptr;
#endif
}

#ifdef __linux__
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
#endif

std::vector<std::pair<std::string, std::string>> KeyboardLayout::GetKeyboardLayoutList() const
{
    std::vector<std::pair<std::string, std::string>> layouts;
#ifdef _WIN32
    int nBuff = ::GetKeyboardLayoutList(0, NULL);
    if (nBuff > 0) {
        HKL *lpList = new HKL[nBuff];
        int count = ::GetKeyboardLayoutList(nBuff, lpList);
        for (int i = 0; i < count; ++i) {
            LANGID langID = LOWORD(lpList[i]);
            LCID lcid = MAKELCID(langID, SORT_DEFAULT);

            char localeNameA[LOCALE_NAME_MAX_LENGTH] = {0};
#ifdef WIN_XP_OR_VISTA
            if (GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, localeNameA, LOCALE_NAME_MAX_LENGTH) > 0) {
                std::string locName(localeNameA);
                if (GetLocaleInfoA(lcid, LOCALE_SISO3166CTRYNAME, localeNameA, LOCALE_NAME_MAX_LENGTH) > 0) {
                    locName.append("-");
                    locName.append(localeNameA);
                }
                std::string displayName = getNativeLanguageName(locName);
                layouts.emplace_back(locName, displayName);
            }
#else
            WCHAR localeName[LOCALE_NAME_MAX_LENGTH];
            if (LCIDToLocaleName(lcid, localeName, LOCALE_NAME_MAX_LENGTH, 0) > 0) {
                WideCharToMultiByte(CP_UTF8, 0, localeName, -1, localeNameA, LOCALE_NAME_MAX_LENGTH, NULL, NULL);
                std::string displayName = getNativeLanguageName(localeNameA);
                layouts.emplace_back(localeNameA, displayName);
            }
#endif
        }
        delete[] lpList;
    }
#else
    if (pimpl->device_id != -1) {
        if (xkb_keymap *keymap = xkb_x11_keymap_new_from_device(pimpl->context, pimpl->connection, pimpl->device_id, XKB_KEYMAP_COMPILE_NO_FLAGS)) {
            xkb_layout_index_t num_layouts = xkb_keymap_num_layouts(keymap);

            for (xkb_layout_index_t i = 0; i < num_layouts; ++i) {
                if (const char *layout_name = xkb_keymap_layout_get_name(keymap, i)) {
                    std::string localeName = layoutNameToLocaleName(layout_name);
                    if (!localeName.empty()) {
                        std::replace(localeName.begin(), localeName.end(), '_', '-');
                        std::string displayName = getNativeLanguageName(localeName);
                        layouts.emplace_back(localeName, displayName);
                        // fprintf(stderr, "%s %s\n", localeName.c_str(), displayName.c_str());
                    }
                }
            }
            xkb_keymap_unref(keymap);
        } else {
            // fprintf(stderr, "Cannot get keymap\n");
        }
    }
#endif
    return layouts;
}
