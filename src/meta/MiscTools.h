#pragma once
#include <utility>

namespace ReyEngine::Tools {
   /// Run an arbitrary function with arbitrary args when the scope is escaped.
   template<typename F, typename... Args>
   struct AnonymousDtor {
      AnonymousDtor(F func, Args&&... args)
      : dtorFunc(func), params(std::forward<Args>(args)...) {}
      ~AnonymousDtor() {
         std::apply(dtorFunc, params);
      }
   private:
      F dtorFunc;
      std::tuple<Args...> params;
   };
}