#pragma once
#include "Property.h"
#include "DrawInterface.h"

/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(TYPENAME) \
static constexpr char TYPE_NAME[] = #TYPENAME;               \
std::string _get_static_constexpr_typename() override {return TYPE_NAME;}
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_SERIALIZER(CLASSNAME, PARENT_CLASSNAME) \
   public:                                           \
   static std::shared_ptr<BaseWidget> deserialize(const std::string& instanceName, PropertyPrototypeMap& properties) { \
   auto retval = std::make_shared<CLASSNAME>(instanceName); \
   retval->BaseWidget::_deserialize(properties);        \
   return retval;}                                       \
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME) \
   CLASSNAME(const std::string& name, const std::string& typeName): PARENT_CLASSNAME(name, typeName)
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

//to instantiate a deserializable object
#define REYENGINE_OBJECT(CLASSNAME, PARENT_CLASSNAME)  \
public:                                                   \
   REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(CLASSNAME)  \
   REYENGINE_SERIALIZER(CLASSNAME, PARENT_CLASSNAME)       \
   REYENGINE_DEFAULT_CTOR(CLASSNAME)                       \
   REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME)  \
   REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME)

//to instantiate a virtual object with no deserializer
#define REYENGINE_VIRTUAL_OBJECT(CLASSNAME, PARENT_CLASSNAME)  \
public:                                                   \
   REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(CLASSNAME)  \
   REYENGINE_DEFAULT_CTOR(CLASSNAME)                       \
   REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME)  \
   REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME)



// A thing which does stuff.
class Window;
class Component
: public PropertyContainer
      {
public:
   Component(const std::string& name);
   inline uint64_t getRid() const {return _resourceId;}
   inline std::string getName() const {return _name;}

   inline bool operator==(const std::shared_ptr<Component>& other) const {if (other){return other->getRid()==_resourceId;}return false;}
   inline bool operator==(const Component& other) const{return other._resourceId == _resourceId;}
   void registerProperties() override {}
protected:
   std::string _name;
   BoolProperty _isProcessed;
   IntProperty _resourceId;
};
