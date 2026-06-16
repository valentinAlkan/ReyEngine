#pragma once
#include <optional>
#include <set>
#include <stdexcept>
#include <string>

namespace ReyEngine::Localization{
    struct Language {
        Language(const std::string& name)
        : _name(name)
        {}
        Language(Language&& other) noexcept {
            std::swap(_name, other._name);
        }
        bool operator<(const Language& other) const {return _name < other._name;}
        std::string _name;
    };

    struct Languages {
        static Languages& instance(){
            static Languages _instance;
            return _instance;
        }
        Languages(const Languages&) = delete;
        Languages& operator=(const Languages&) = delete;
        Languages(Languages&&) = delete;
        Languages& operator=(Languages&&) = delete;

        static Language* add(Language&& language) {
            auto& thiz = instance();
            auto [it, inserted] = thiz._languages.emplace(std::move(language));
            return const_cast<Language*>(&*it);
        }
        static void remove(Language* language) {
            if (!language) return;
            auto& thiz = instance();
            if (thiz._defaultLanguage == language) {
                throw std::runtime_error("Cannot delete default language");
            }
            auto it = thiz._languages.find(*language);
            if (it != thiz._languages.end()) {
                thiz._languages.erase(it);
            }
        }
        static std::optional<Language*> get(const std::string& name) {
            auto& thiz = instance();
            auto it = thiz._languages.find(Language(name));
            if (it != thiz._languages.end()) {
                return const_cast<Language*>(&*it);
            }
            return std::nullopt;
        }
        static Language* getDefault(){return instance()._defaultLanguage;}
        static void setDefaultLanguage(Language* language) {
            if (language) {
                instance()._defaultLanguage = language;
                return;
            }
            throw std::runtime_error("Failed to set default language!");
        }
    private:
        Languages() {
            setDefaultLanguage(add(Language("English"))); //todo: lookup from config
        };
        std::set<Language> _languages;
        Language* _defaultLanguage = nullptr;
    };
    inline Language* getDefaultLanguage(){return Languages::getDefault();}
}