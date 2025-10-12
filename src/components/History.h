#pragma once
#include <string>

namespace ReyEngine {
   
   template<typename T>
   struct History {
      void add(const T &dir) {
         if (_ptr != _history.size()) {
            //overwrite at ptr+1;
            auto iter = _history.begin() + (long long) _ptr + 1;
            //erase everything after that point
            _history.erase(iter, _history.end());
         }
         _history.push_back(dir);
         _ptr = _history.size() - 1;
      }

      std::optional<T> back() {
         if (!_history.empty() && _ptr <= _history.size()) {
            if (_ptr >= 1) {
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

      void clear() {
         _history.clear();
      }

      History& operator=(const std::vector<T>& vec){
         _history = vec;
         _ptr = 0;
      }
   protected:
      std::vector<T> _history = {};
      size_t _ptr = 0;
   };
}