#pragma once
#include <string>
#include <cassert>
#include <utility>
#include <map>
#include <memory>
#include <unordered_map>
#include "DrawInterface.h"

#define PROP_TYPE(propName) static constexpr char propName[] = #propName;
#define PROPERTY_DECLARE(PROPERTYNAME) PROPERTYNAME(#PROPERTYNAME)

struct BaseProperty;
struct PropertyPrototype;
using PropertyMap = std::unordered_map<std::string, BaseProperty*>;
using PropertyPrototypeMap = std::unordered_map<std::string, PropertyPrototype>;

namespace PropertyTypes{
   PROP_TYPE(String)
   PROP_TYPE(Int)
   PROP_TYPE(Float)
   PROP_TYPE(Bool)
   PROP_TYPE(Vec2)
   PROP_TYPE(Rect)
}

namespace PropertyMeta{
   static constexpr char INDENT[] = "   ";
   static constexpr char SEP = '|';
}

struct PropertyPrototype{
   std::string instanceName;
   std::string typeName;
   std::string data;
   std::map<std::string, PropertyPrototype> subproperties;
};

///Something can have properties (and therefore subproperties
struct PropertyContainer{
   ///make sure ALL register property functions are called
   virtual void registerProperties() = 0;
   void registerProperty(BaseProperty& property);
   PropertyMap& getProperties(){return _properties;}
protected:
   void _initProperties(){
   };
   PropertyMap _properties;
};

struct BaseProperty : PropertyContainer {
   BaseProperty(const std::string instanceName, const std::string& typeName)
   : _instanceName(instanceName)
   , _typeName(typeName){}
   void _load(const PropertyPrototype& data);
   virtual void load(const PropertyPrototype& data) = 0;
   virtual std::string toString() = 0;
   std::string instanceName() const {return _instanceName;}
   std::string typeName() const {return _typeName;}
   virtual void registerProperties() = 0;
private:
   const std::string _instanceName;
   const std::string _typeName;
};

template <typename T>
struct Property : public BaseProperty {
   Property(const std::string instanceName, const std::string& typeName, T defaultvalue)
   : BaseProperty(instanceName, typeName)
   , value(std::move(defaultvalue)){}
   void registerProperties() override {}
   Property& operator=(const T& newValue){
      value = newValue;
   }
   virtual T fromString(const std::string& str) = 0;
   void load(const PropertyPrototype& data) override {value = fromString(data.data);}
   T get(){return value;}
   void set(const T& newValue){
      value = newValue;
   }
   T value;
};

struct StringProperty : public Property<std::string>{
   StringProperty(const std::string& instanceName, const std::string defaultvalue = "")
   : Property(instanceName, PropertyTypes::String, defaultvalue)
   {}
   std::string toString() override {return value;}
   std::string fromString(const std::string& data) { return data;}
};

struct BoolProperty : public Property<bool>{
   BoolProperty(const std::string& instanceName, bool defaultvalue = false)
   : Property(instanceName, PropertyTypes::Bool, defaultvalue)
   {}
   std::string toString() override {return std::to_string(value);}
   bool fromString(const std::string& str) { return std::stoi(str);}
};

struct IntProperty : public Property<int>{
   IntProperty(const std::string& instanceName, int defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Int, defaultvalue)
   {}
   std::string toString() override {return std::to_string(value);}
   int fromString(const std::string& str) { return std::stoi(str);}
};

struct FloatProperty : public Property<double>{
   FloatProperty(const std::string& instanceName, int defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Int, defaultvalue)
   {}
   std::string toString() override {return std::to_string(value);}
   double fromString(const std::string& str) { return std::stod(str);}
};

template <typename T>
struct Vec2Property : public Property<GFCSDraw::Vec2<T>>{
   Vec2Property(const std::string& instanceName, int defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Vec2, defaultvalue)
   {}
   std::string toString() override {return value.toString();}
   GFCSDraw::Vec2<T> fromString(const std::string& str) {return GFCSDraw::Vec2<T>::fromString(str);}
};

template <typename T>
struct RectProperty : public Property<GFCSDraw::Rect<T>>{
   RectProperty(const std::string& instanceName, GFCSDraw::Rect<T> defaultvalue=GFCSDraw::Rect<T>())
   : Property(instanceName, PropertyTypes::Rect, defaultvalue)
   {}
   std::string toString() override {return value.toString();}
   GFCSDraw::Rect<T> fromString(const std::string& str) {return GFCSDraw::Rect<T>::fromString(str);}
};