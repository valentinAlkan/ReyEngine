#pragma once
#include "Property.h"
#include "DrawInterface.h"
#include "TypeManager.h"
#include "TypeContainer.h"
#include "Event.h"

namespace ReyEngine {
#include <type_traits>
#include <tuple>
#include <memory>

   // Check if build exists with correct signature and return type
   template<typename T, typename... Args>
   struct has_build {
   private:
      template<typename C>
      static auto test(int) -> decltype(
      C::build(std::declval<Args>()...),
            std::true_type{}
      );

      template<typename C>
      static std::false_type test(...);

      template<typename C>
      static auto check_return_type(int) -> std::bool_constant<
            std::is_same_v<
                  decltype(C::build(std::declval<Args>()...)),
                  std::shared_ptr<C>
            >
      >;
      template<typename C>
      static std::false_type check_return_type(...);

   public:
      // Both conditions must be true: build exists and returns correct type
      static constexpr bool value =
            decltype(test<T>(0))::value &&
            decltype(check_return_type<T>(0))::value;
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
   void _ensure_is_statically_buildable()          {                                        \
   static_assert( has_required_builds<T, BUILD_ARGS_TO_TUPLE(__VA_ARGS__)>::value, \
  "Type must publicly implement all static build functions specified to the build system. \n" \
  "If you are using the REYENGINE_OBJECT_BUILD_ONLY macro, then you can place REYENGINE_DEFAULT_BUILD macro somewhere in the public section of your class definition. \n"               \
  "You may also implement this functionality yourself if you desire more control over the build process. ");}   \
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
#define REYENGINE_OBJECT_BUILD_ONLY(CLASSNAME, PARENT_CLASSNAME, ...)  \
private:                                                               \
   REYENGINE_ENSURE_IS_STATICALLY_BUILDABLE(CLASSNAME, std::tuple<const std::string&>)    \
   REYENGINE_PRIVATE_MAKE_SHARED(CLASSNAME)                                       \
public:                                                           \
   REYENGINE_DECLARE_COMPONENT_FRIEND                             \
   REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(CLASSNAME)         \
protected:                                                     \
   REYENGINE_DEFAULT_CTOR(CLASSNAME)                       \
   REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME)  \
   REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME)

namespace ReyEngine{
   class Application;
      namespace Internal{
      namespace AllocationTools {
         static size_t currentMemoryBytes = 0;
         inline void* malloc(size_t nBytes){
            //placement new allocation - use a memory pool to minimize cache misses - one day - but not now.
            auto ptr = ::operator new(nBytes);
            Logger::debug() << "Allocating " << nBytes << " bytes @ " << ptr << std::endl;
            currentMemoryBytes += nBytes;
            Logger::debug() << "Total memory usage is now " << currentMemoryBytes << std::endl;
            return ptr;
         }
         inline void free(void* ptr, size_t nBytes){
            Logger::debug() << "Freeing " << nBytes << "bytes @ " << ptr << std::endl;
            ::operator delete(ptr);
            currentMemoryBytes -= nBytes;
            Logger::debug() << "Total memory usage is now " << currentMemoryBytes << std::endl;
         }
      }
      // A thing which does stuff.
      class Component
      : public inheritable_enable_shared_from_this<Component>
      , public PropertyContainer
      , public TypeContainer<Component>
      {
      public:
         using RID = uint64_t;
         REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(TypeContainer)
         Component(const std::string& name, const std::string& typeName);
         inline RID getRid() const {return _resourceId;}

         inline bool operator==(const std::shared_ptr<Component>& other) const {return other && other->getRid() == (unsigned long) _resourceId;}
         inline bool operator==(const Component& other) const{return other._resourceId == _resourceId;}
         void registerProperties() override {}
         uint64_t getFrameCounter() const;
         std::shared_ptr<Component> toComponent();
         static void registerType(const std::string& typeName, const std::string& parentType, bool isVirtual, Deserializer fx){TypeManager::registerType(typeName, parentType, isVirtual, fx);}
         std::string serialize();
         void __init(){
            _init();
            _has_inited = true;
         }
      protected:
         virtual void _init(){}
         void _deserialize(PropertyPrototypeMap&);
         virtual void _on_deserialize(PropertyPrototypeMap&){} //used to do any deserializations specific to this type
         const std::string _typeName; //can't just use static constexpr TYPE_NAME since we need to know what the type is if using type-erasure
         std::string _name;
         bool _has_inited = false; //set true THE FIRST TIME a widget enters the tree. Can do constructors of children and other stuff requiring shared_from_this();

         virtual void _on_application_ready(){};
         virtual void _register_parent_properties(){};
         void __on_child_added_immediate(ChildPtr&) {
            Logger::debug() << "component init" << std::endl;
            _init();
            _has_inited = true;
         };

         BoolProperty _isProcessed;
         IntProperty _resourceId;

         friend class TypeManager;
         friend class ReyEngine::Application;
      };
   }
}