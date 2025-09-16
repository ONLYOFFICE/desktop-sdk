
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#include "mac_keyboardlayout.h"
#include <unicode/locid.h>
#include <string>

#define LANGIDFROMLCID(lcid) ((uint16_t)(lcid))

static std::string NSStringToStdString(NSString* nsString)
{
    if (!nsString) return {};
    const char* utf8Str = [nsString UTF8String];
    if (!utf8Str) return {};
    return std::string(utf8Str);
}

static uint16_t localeNameToLangId(const std::string &localeName)
{
    if (!localeName.empty()) {
        icu::Locale loc(localeName.c_str());
        return LANGIDFROMLCID(loc.getLCID());
    }
    return 0;
}

uint16_t GetKeyboardLayout(void)
{
    TISInputSourceRef src = TISCopyCurrentKeyboardInputSource();
    if (!src) return 0;
    CFArrayRef languages = (CFArrayRef)TISGetInputSourceProperty(src, kTISPropertyInputSourceLanguages);
    CFStringRef lang = nullptr;
    if (languages && CFArrayGetCount(languages) > 0) {
        lang = (CFStringRef)CFArrayGetValueAtIndex(languages, 0);
    }
    if (!lang) {
       CFRelease(src);
       return 0;
    }
    NSLocale *locale = [[NSLocale alloc] initWithLocaleIdentifier:(__bridge NSString*)lang];
    CFRelease(src);

    std::string language = NSStringToStdString([locale languageCode]);
    std::string region = NSStringToStdString([locale countryCode]);
    std::string script = NSStringToStdString([locale scriptCode]);
    
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
        return localeNameToLangId(language);
    }
    return 0;
}
