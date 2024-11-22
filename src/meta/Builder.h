#pragma once
#include <type_traits>
#include <tuple>
#include <memory>
#include "Logger.h"

namespace ReyEngine {
   namespace Internal{
      namespace AllocationTools {
         static constexpr bool verbose = false;
         static size_t currentMemoryBytes = 0;
         inline void* malloc(size_t nBytes){
            //placement new allocation - use a memory pool to minimize cache misses - one day - but not now.
            auto ptr = ::operator new(nBytes);
            if (verbose) Logger::debug() << "Allocating " << nBytes << " bytes @ " << ptr << std::endl;
            currentMemoryBytes += nBytes;
            if (verbose)Logger::debug() << "Total memory usage is now " << currentMemoryBytes << std::endl;
            return ptr;
         }
         inline void free(void* ptr, size_t nBytes){
            if (verbose)Logger::debug() << "Freeing " << nBytes << "bytes @ " << ptr << std::endl;
            ::operator delete(ptr);
            currentMemoryBytes -= nBytes;
            if (verbose)Logger::debug() << "Total memory usage is now " << currentMemoryBytes << std::endl;
         }
      }
   }

   // Check if build exists with correct signature and return type
   template<typename T, typename... Args>
   struct has_build {
   private:
      template<typename C>
      static auto test(int) -> decltype(C::build(std::declval<Args>()...),std::true_type{});
      template<typename C>
      static std::false_type test(...);
      template<typename C>
      static auto check_return_type(int) -> std::bool_constant<std::is_same_v<decltype(C::build(std::declval<Args>()...)),std::shared_ptr<C>>>;
      template<typename C>
      static std::false_type check_return_type(...);

   public:
      // Both conditions must be true: build exists and returns correct type
      static constexpr bool value =decltype(test<T>(0))::value && decltype(check_return_type<T>(0))::value;
   };

   // Helper to unpack tuple into arguments
   template<typename T, typename Tuple>
   struct has_build_from_tuple;

   template<typename T, typename... Args>
   struct has_build_from_tuple<T, std::tuple<Args...>> {
      static constexpr bool value = has_build<T, Args...>::value;
   };

   // Check all required build signatures
   template<typename T, typename Tuple>
   struct has_required_builds;

   template<typename T, typename... Tuples>
   struct has_required_builds<T, std::tuple<Tuples...>> {
   static constexpr bool value = (has_build_from_tuple<T, Tuples>::value && ...);
   };
}

/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_DECLARE_COMPONENT_FRIEND friend class ReyEngine::Internal::Component;
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(TYPENAME) \
static constexpr char TYPE_NAME[] = #TYPENAME;               \
std::string _get_static_constexpr_typename() override {return TYPE_NAME;}
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_CTOR_INIT_LIST(NAME, PARENT_CLASSNAME) \
PARENT_CLASSNAME(NAME, TYPE_NAME)                  \
, NamedInstance(NAME, TYPE_NAME)                     \
, Internal::Component(NAME, TYPE_NAME)                \
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_SERIALIZER(CLASSNAME, PARENT_CLASSNAME)
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME) \
   CLASSNAME(const std::string& name, const std::string& typeName): PARENT_CLASSNAME(name, typeName), NamedInstance(name, typeName), Component(name, typeName)
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_DEFAULT_BUILD(T) \
   static std::shared_ptr<T> build(const std::string& instanceName){ return T::_reyengine_make_shared(instanceName); }
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_PRIVATE_MAKE_SHARED(T) \
   template<typename... Args> \
   static std::shared_ptr<T> _reyengine_make_shared(Args&&... args) noexcept {   \
        auto mem = ReyEngine::Internal::AllocationTools::malloc(sizeof(T)); \
        auto obj = new (mem) T(std::forward<Args>(args)...);  \
        std::shared_ptr<T> ptr(obj, [](T* ptr) {   \
            ptr->~T();           \
            ReyEngine::Internal::AllocationTools::free(ptr, sizeof(T));  \
         });                      \
      return ptr;}

/////////////////////////////////////////////////////////////////////////////////////////
#define BUILD_ARGS_TO_TUPLE(...) std::tuple<__VA_ARGS__>
#define REYENGINE_ENSURE_IS_STATICALLY_BUILDABLE(T, ...) \
   static void _ensure_is_statically_buildable()          {                                        \
   static_assert( ReyEngine::has_required_builds<T, BUILD_ARGS_TO_TUPLE(__VA_ARGS__)>::value, \
  "Type must publicly implement all static build functions specified to the build system. \n" \
  "If you are using the REYENGINE_OBJECT_BUILD_ONLY macro, then you can place REYENGINE_DEFAULT_BUILD macro somewhere in the public section of your class definition. \n"               \
  "You may also implement this functionality yourself if you desire more control over the build process. If you are seeing this error despite \n" \
  "having declared build functions that accept all the necessary properties, ensure the functions are publicly accessible.");}   \
///////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_DEFAULT_CTOR(CLASSNAME) \
CLASSNAME(const std::string& name): CLASSNAME(name, _get_static_constexpr_typename()){}
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME) \
protected:                                                \
   void _register_parent_properties() override{           \
      PARENT_CLASSNAME::_register_parent_properties();    \
      PARENT_CLASSNAME::registerProperties();             \
   }

//to instantiate a virtual object with no deserializer
#define REYENGINE_VIRTUAL_OBJECT(CLASSNAME, PARENT_CLASSNAME)  \
public:                                                        \
   REYENGINE_DECLARE_COMPONENT_FRIEND                             \
   REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(CLASSNAME)  \
   protected:                                                     \
   REYENGINE_DEFAULT_CTOR(CLASSNAME)                       \
   REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME)  \
   REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME)

//to disallow building except via a factory function
#define REYENGINE_OBJECT_CUSTOM_BUILD(CLASSNAME, PARENT_CLASSNAME, ...)  \
private:                                                               \
   REYENGINE_ENSURE_IS_STATICALLY_BUILDABLE(CLASSNAME, __VA_ARGS__)    \
   REYENGINE_PRIVATE_MAKE_SHARED(CLASSNAME)                                       \
public:                                                           \
   REYENGINE_DECLARE_COMPONENT_FRIEND                             \
   REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(CLASSNAME)         \
protected:                                                     \
   REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME)                \

#define REYENGINE_OBJECT_BUILD_ONLY(CLASSNAME, PARENT_CLASSNAME) \
   REYENGINE_OBJECT_CUSTOM_BUILD(CLASSNAME, PARENT_CLASSNAME, std::tuple<const std::string&>) \
   REYENGINE_DEFAULT_CTOR(CLASSNAME)                       \
   REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME)
