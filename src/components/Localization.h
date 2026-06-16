#pragma once
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <FileSystem.h>

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

        static Language* add(Language&& language) {return instance()._add(std::move(language));}
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
        static void setDefaultLanguage(Language* language) {instance()._setDefaultLanguage(language);}
    private:
        Languages() {
            _setDefaultLanguage(_add(Language("English"))); //todo: lookup from config
        }
        //non static for ctor
        Language* _add(Language&& language) {
            auto [it, inserted] = _languages.emplace(std::move(language));
            return const_cast<Language*>(&*it);
        }
        void _setDefaultLanguage(Language* language) {
            if (language) {
                _defaultLanguage = language;
                return;
            }
            throw std::runtime_error("Failed to set default language!");
        }
        std::set<Language> _languages;
        Language* _defaultLanguage = nullptr;
    };
    inline Language* getDefaultLanguage(){return Languages::getDefault();}

    struct TranslationMap {
        static TranslationMap& instance(){
            static TranslationMap _instance;
            return _instance;
        }
        TranslationMap(const TranslationMap&) = delete;
        TranslationMap& operator=(const TranslationMap&) = delete;
        TranslationMap(TranslationMap&&) = delete;
        TranslationMap& operator=(TranslationMap&&) = delete;
    private:
        TranslationMap() = default;
    };
}