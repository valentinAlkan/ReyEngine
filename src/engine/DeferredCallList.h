#pragma once
#include <functional>
#include <vector>
#include <memory>
#include <mutex>

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
         //keep going until no call defers another
         while (true) {
            std::vector<std::unique_ptr<DeferredCall>>* toRun;
            {
               std::unique_lock<std::mutex> sl(mtx);
               if (_calls->empty()) return;
               toRun = _calls;
               _calls = _calls == &A ? &B : &A; //new calls go to the other list while we iterate this one
            }
            for (auto& call: *toRun) {
               if (call->active) call->func();
            }
            toRun->clear();
         }
      }

      void executeAllAndClear() {
         executeAll();
      }
      friend class Window;
   };
}