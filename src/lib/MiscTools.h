#pragma once
#include <utility>

namespace ReyEngine::Tools {
   template <typename F, typename... Args>
   class ScopeExit {
   private:
      F func;
      std::tuple<Args...> args;
      bool active;

   public:
      explicit ScopeExit(F f, Args... arguments)
      : func(std::move(f))
      , args(std::move(arguments)...)
      , active(true) {}

      ~ScopeExit() {
         if (active) {
            // Call the function with stored arguments
            std::apply(func, args);
         }
      }

      void release() { active = false; }

      // Prevent copying
      ScopeExit(const ScopeExit&) = delete;
      ScopeExit& operator=(const ScopeExit&) = delete;

      // Allow moving
      ScopeExit(ScopeExit&& other)
      : func(std::move(other.func))
      , args(std::move(other.args))
      , active(other.active) {
         other.active = false;
      }
      ScopeExit& operator=(ScopeExit&&) = delete;
   };

   // Helper function for cleaner syntax with argument deduction
   template <typename F, typename... Args>
   auto make_scope_exit(F&& f, Args&&... args) {
      return ScopeExit < std::decay_t<F>, std::decay_t<Args>...>(
            std::forward<F>(f),
            std::forward<Args>(args)...
      );
   }

}