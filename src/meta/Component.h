#pragma once
#include "Property.h"
#include "DrawInterface.h"
#include "TypeManager.h"
#include "TypeContainer.h"
#include "Event.h"

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
   CLASSNAME(const std::string& name, const std::string& typeName): PARENT_CLASSNAME(name, typeName), NamedInstance(name, typeName)
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_DEFAULT_BUILD \
   template <typename T>    \
   static std::shared_ptr<T> build(const std::string& name) noexcept {  \
      auto me = std::shared_ptr<T>(new T(name)); \
      return me; }
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
public:                                                   \
   REYENGINE_DECLARE_COMPONENT_FRIEND                             \
   REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(CLASSNAME)  \
   protected:                                                     \
   REYENGINE_DEFAULT_CTOR(CLASSNAME)                       \
   REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME)  \
   REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME)

//to disallow building except via a factory function
#define REYENGINE_OBJECT_BUILD_ONLY(CLASSNAME, PARENT_CLASSNAME)  \
public:                                                           \
   REYENGINE_DECLARE_COMPONENT_FRIEND                             \
   REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(CLASSNAME)  \
   protected:                                                     \
   REYENGINE_DEFAULT_CTOR(CLASSNAME)                       \
   REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME)  \
   REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME)

namespace ReyEngine{
   class Application;
      namespace Internal{

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

//         friend class TypeContainer<Component>;
//         friend class TypeContainer<BaseWidget>;
         friend class TypeManager;
         friend class ReyEngine::Application;
      };
   }
}