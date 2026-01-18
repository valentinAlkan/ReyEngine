#pragma once
#include <functional>
#include <vector>

namespace ReyEngine {
   class Window;
   class DeferredCallList {
      std::vector<std::function<void()>> A;
      std::vector<std::function<void()>> B;
      std::vector<std::function<void()>>* _calls;
   protected:
      DeferredCallList(): _calls(&A){}
   public:
      template <typename F, typename... Args>
      void add(F&& func, Args&&... args) {
         _calls->push_back(
               [f = std::forward<F>(func),
                     ...captured_args = std::forward<Args>(args)]() mutable {
                  f(captured_args...);
               }
         );
      }

      void executeAll() {
         auto currentCallList = _calls;
         if (!currentCallList->empty()) {
            _calls = _calls == &A ? &B : &A; //swap call lists so we can't add to it while we're iterating
            for (auto& call: *currentCallList) call();
            executeAll(); //keep doing this until all calls are resolved
         }
      }

      void executeAllAndClear() {
         executeAll();
         A.clear();
         B.clear();
      }
      friend class Window;
   };
}