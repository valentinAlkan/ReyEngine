#pragma once
#include <functional>
#include <vector>
#include <memory>

namespace ReyEngine {
   namespace Internal::Tree{
      class Windowable;
   }
   class Window;
   class DeferredCallList {
      using Windowable = Internal::Tree::Windowable;
      struct DeferredCall{
         bool active = true; //set to false and it won't fire
         Windowable* caller; //whoever called it
         std::function<void()> func;
      };
      std::vector<std::unique_ptr<DeferredCall>> A;
      std::vector<std::unique_ptr<DeferredCall>> B;
      std::vector<std::unique_ptr<DeferredCall>>* _calls;
      std::mutex mtx;
   protected:
      DeferredCallList(): _calls(&A){}
   public:
      template <typename... Args>
      void add(Windowable* caller, std::function<void()>&& func, Args&&... args) {
         auto newCall = std::make_unique<DeferredCall>();
         newCall->caller = caller;
         newCall->func = [func = std::move(func), ...captured_args = std::forward<Args>(args)]() mutable {
            func(captured_args...);
         };
         std::unique_lock<std::mutex>sl(mtx);
         _calls->push_back(std::move(newCall));
      }

      void removeCalls(Windowable* callerToRemove) {
         std::unique_lock<std::mutex>sl(mtx);
         for (const auto& call : A) {
            if (call && call->caller == callerToRemove) {
               call->active = false;
            }
         }
         for (const auto& call : B) {
            if (call && call->caller == callerToRemove) {
               call->active = false;
            }
         }
      }

      void executeAll() {
         if (!_calls->empty()) {
            {
               std::unique_lock<std::mutex> sl(mtx); //swap should be locked but otherwise adding and removing is done to the inactive list
               _calls = _calls == &A ? &B : &A; //swap call lists so we can't add to it while we're iterating
            }
            for (auto& call: *_calls) {
               if (call->active) call->func();
            }
            _calls->clear(); //clear out any inactive calls (missing callers, etc)
            executeAll(); //keep doing this until all calls are resolved
         }
      }

      void executeAllAndClear() {
         executeAll();
      }
      friend class Window;
   };
}