#pragma once
#include <optional>
#include <utility>
#include <vector>

namespace ReyEngine {
   
   template<typename T>
   struct History {
      void add(const T &dir) {
         if (_history.empty()) {
            _ptr = 0;
         } else if (_history.size() >= _ptr + 1) {
            //overwrite at ptr+1;
            auto iter = _history.begin() + (long long) _ptr + 1;
            //erase everything after that point
            _history.erase(iter, _history.end());
         }
         _history.push_back(dir);
         _ptr = _history.size() - 1;
      }

      std::optional<T> back() {
         if (hasBack()){
            if (_ptr > 0) {
               _ptr--;
            }
            return _history.at(_ptr);
         }
         return {};
      }

      std::optional<T> fwd() {
         if (hasFwd()) {
            return _history.at(++_ptr);
         }
         return {};
      }

      bool hasBack() const {
         return !_history.empty() && _ptr < _history.size() && _ptr > 0;
      }

      bool hasFwd() const {
         return !_history.empty() && _ptr < _history.size() - 1; // never test size_t against -1
      }

      /// the entry it's at, or nullopt if there are none. The pointer is good until the history next changes.
      std::optional<T*> current() {
         if (_history.empty()) return {};
         return &_history.at(_ptr);
      }

      std::optional<const T*> current() const {
         if (_history.empty()) return {};
         return &_history.at(_ptr);
      }

      void clear() {
         _history.clear();
         _ptr = 0;
      }

      History& operator=(const std::vector<T>& vec){
         _history = vec;
         _ptr = 0;
         return *this;
      }
   protected:
      std::vector<T> _history = {};
      size_t _ptr = 0;
   };

   /// An entry for a History of things done rather than places been: how things stood before it and after it. To undo one,
   /// go by the current entry's undoAction and then back(); to redo, fwd() and go by its doAction. The first entry can't be
   /// gone back from, so start with one that does nothing.
   template <typename T>
   struct HistoryAction {
      HistoryAction() = default;
      HistoryAction(T before, T after) : _before(std::move(before)), _after(std::move(after)) {}
      const T& doAction() const {return _after;}
      const T& undoAction() const {return _before;}
   protected:
      T _before{};
      T _after{};
   };

}