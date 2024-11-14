#pragma once
#include "Property.h"
#include "DrawInterface.h"
#include "TypeManager.h"
#include "TypeContainer.h"
#include "Event.h"
#include "Builder.h"

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

         friend class TypeManager;
         friend class ReyEngine::Application;
      };
   }
}