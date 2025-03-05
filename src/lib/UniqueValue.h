#pragma once
#include <array>
#include <string>
#include <limits>

//ridiculously unique
using UniqueValue = std::array<uint64_t, 4>;

namespace ReyEngine::Internal{
   struct UniqueGenerator{
      UniqueGenerator(): next{0}{}
      //dont care about dtor order
      static UniqueGenerator instance(){static UniqueGenerator _instance; return _instance;}
      UniqueValue makeNew(){
         auto retval = next;
         size_t i = 0;
         // Find first non-maxed value
         while (i < next.size() && next[i] == std::numeric_limits<uint64_t>::max()) {
            i++;
         }
         // Check if we found a position to increment
         if (i < next.size()) {
            // Found one, increment it
            next[i]++;
         } else {
            std::fill(next.begin(), next.end(), 0);
         }
         return retval;
      }
      UniqueValue peekNext(){return next;}
   protected:
      void setNext(const UniqueValue& newNext){next = newNext;}
   private:
      UniqueValue next;
   friend class Application;
   };
}