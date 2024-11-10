#pragma once
#include "Property.h"
#include "DrawInterface.h"
#include "TypeManager.h"
#include "TypeContainer.h"
#include "Event.h"

namespace ReyEngine {
   template<typename T>
   class has_build {
   private:
      template<typename C>
      static auto test(int)-> decltype(C::build(std::declval<std::string>()), std::true_type{});
      static std::false_type test(...);
      template<typename C>
      static auto check_return_type(int)-> std::is_same<decltype(C::build(std::declval<std::string>())),std::shared_ptr<T>>;
      static std::false_type check_return_type(...);
   public:
      static constexpr bool value = decltype(test<T>(0))::value && decltype(check_return_type<T>(0))::value;
   };
}

/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_DECLARE_COMPONENT_FRIEND friend class ReyEngine::Internal::Component;
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(TYPENAME) \
static constexpr char TYPE_NAME[] = #TYPENAME;               \
std::string _get_static_constexpr_typename() override {return TYPE_NAME;}
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_SERIALIZER(CLASSNAME, PARENT_CLASSNAME)
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME) \
   CLASSNAME(const std::string& name, const std::string& typeName): PARENT_CLASSNAME(name, typeName), NamedInstance(name, typeName), Component(name, typeName)
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_DEFAULT_BUILD(T) \
   template<typename... Args> \
   static std::shared_ptr<T> build(Args&&... args) noexcept {   \
        auto mem = ReyEngine::Internal::AllocationTools::malloc(sizeof(T)); \
        auto obj = new (mem) T(std::forward<Args>(args)...);  \
        std::shared_ptr<T> ptr(obj, [](T* ptr) {   \
            ptr->~T();           \
            ReyEngine::Internal::AllocationTools::free(ptr, sizeof(T));  \
         });                      \
      return ptr;}

/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_ENSURE_IS_STATICALLY_BUILDABLE(CLASSNAME) \
   void _ensure_is_statically_buildable(){                                                         \
   static_assert(ReyEngine::has_build<CLASSNAME>::value,    \
   "\nError: Type must implement a publicly accessible static build(const std::string& name) method that returns a shared_ptr of its type! \n" \
   "You can resolve this by placing REYENGINE_DEFAULT_BUILD macro somewhere in the public section of your class definition. \n"               \
   "You may also implement this functionality yourself if you desire more control over the build process. "); \
   }
/////////////////////////////////////////////////////////////////////////////////////////
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
#define REYENGINE_OBJECT_BUILD_ONLY(CLASSNAME, PARENT_CLASSNAME)  \
private:                                                          \
   REYENGINE_ENSURE_IS_STATICALLY_BUILDABLE(CLASSNAME)                       \
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