#pragma once
#include <utility>

namespace ReyEngine::Tools {
//   /// Run an arbitrary function with arbitrary args when the scope is escaped.
//   template<typename F, typename... Args>
//   struct AnonymousDtor {
//      AnonymousDtor(F func, Args&... args)
//      : dtorFunc(func), params(std::forward<Args>(args)...) {}
//      ~AnonymousDtor() {
//         std::apply(dtorFunc, params);
//      }
//   private:
//      F dtorFunc;
//      std::tuple<Args...> params;
//   };
//
//   //convenience lazy unique_ptr dtor
//   template<typename F, typename... Args>
//   auto make_dtor_ptr(F func, Args&&... args) {
//      return std::make_unique<AnonymousDtor<F, std::decay_t<Args>...>>(func,std::forward<Args>(args)...);
//   }
   template <typename F, typename... Args>
   class scope_exit {
   private:
      F func;
      std::tuple<Args...> args;
      bool active;

   public:
      explicit scope_exit(F f, Args... arguments)
      : func(std::move(f))
      , args(std::move(arguments)...)
      , active(true) {}

      ~scope_exit() {
         if (active) {
            // Call the function with stored arguments
            std::apply(func, args);
         }
      }

      void release() { active = false; }

      // Prevent copying
      scope_exit(const scope_exit&) = delete;
      scope_exit& operator=(const scope_exit&) = delete;

      // Allow moving
      scope_exit(scope_exit&& other)
      : func(std::move(other.func))
      , args(std::move(other.args))
      , active(other.active) {
         other.active = false;
      }
      scope_exit& operator=(scope_exit&&) = delete;
   };

   // Helper function for cleaner syntax with argument deduction
   template <typename F, typename... Args>
   auto make_scope_exit(F&& f, Args&&... args) {
      return scope_exit<std::decay_t<F>, std::decay_t<Args>...>(
            std::forward<F>(f),
            std::forward<Args>(args)...
      );
   }

}