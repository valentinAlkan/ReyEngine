#pragma once
#include <string>
#include "Localization.h"

namespace ReyEngine {
    // A std::string that additionally remembers which language its text is in,
    // for translation/localization purposes. In every other respect it behaves
    // like a std::string
    struct TrString : std::string {
        using std::string::string;
        using std::string::operator=;
        TrString() = default;
        TrString(const std::string& key, const std::string& text)
        : std::string(text)
        , _language(Localization::getDefaultLanguage())
        , _key(key)
        {}
        TrString(const std::string& key,std::string&& text)
        : std::string(std::move(text))
        , _key(key)
        {}
        TrString(const std::string& key,const std::string& text, Localization::Language* language)
        : std::string(text)
        , _language(language)
        , _key(key)
        {}
        TrString(const std::string& key,std::string&& text, Localization::Language* language)
        : std::string(std::move(text))
        , _language(language)
        , _key(key)
        {}

        [[nodiscard]] Localization::Language* getLanguage() const {return _language;}
        void setLanguage(Localization::Language* language) {_language = language;}
        [[nodiscard]] const std::string& str() const {return *this;}
        [[nodiscard]] std::string& str() {return *this;}

        std::string translate(Localization::Language* language) const {return "lorem ipsum";}

        Localization::Language* _language;
    private:
        std::string _key;
    };
}
