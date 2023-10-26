#pragma once
#include <string>
#include <cassert>
#include <utility>
#include <map>
#include <memory>
#include <unordered_map>
#include "DrawInterface.h"

#define PROP_TYPE(propName) static constexpr char propName[] = #propName;
#define PROPERTY_DECLARE(PROPERTYNAME, ...) PROPERTYNAME(#PROPERTYNAME, __VA_ARGS__)

struct BaseProperty;
struct PropertyPrototype;
using PropertyMap = std::unordered_map<std::string, BaseProperty*>;
using OwnedPropertyMap = std::unordered_map<std::string, std::shared_ptr<BaseProperty>>;
using PropertyPrototypeMap = std::unordered_map<std::string, PropertyPrototype>;

namespace PropertyTypes{
   PROP_TYPE(String)
   PROP_TYPE(Int)
   PROP_TYPE(Float)
   PROP_TYPE(Bool)
   PROP_TYPE(Vec2)
   PROP_TYPE(Rect)
   PROP_TYPE(Timer)
   PROP_TYPE(Enum)
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
   void moveProperty(std::shared_ptr<BaseProperty>);
   PropertyMap& getProperties(){return _properties;}
protected:
   void _initProperties(){
   };
   PropertyMap _properties;
   OwnedPropertyMap _ownedProperties; //properties we own
};

/////////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct Property : public BaseProperty {
   Property(const std::string instanceName, const std::string& typeName, T defaultvalue)
   : BaseProperty(instanceName, typeName)
   , value(std::move(defaultvalue)){}
   void registerProperties() override {}
   Property& operator=(const T& newValue){
      value = newValue;
   }
   Property& operator=(const Property& other){
      value = other.value;
   }
   virtual T fromString(const std::string& str) = 0;
   void load(const PropertyPrototype& data) override {value = fromString(data.data);}
   T get(){return value;}
   void set(const T& newValue){
      value = newValue;
   }
   T value;
};

/////////////////////////////////////////////////////////////////////////////////////////
struct StringProperty : public Property<std::string>{
   StringProperty(const std::string& instanceName, const std::string defaultvalue = "")
   : Property(instanceName, PropertyTypes::String, defaultvalue)
   {}
   std::string toString() override {return value;}
   std::string fromString(const std::string& data) override { return data;}
};

/////////////////////////////////////////////////////////////////////////////////////////
struct BoolProperty : public Property<bool>{
   BoolProperty(const std::string& instanceName, bool defaultvalue = false)
   : Property(instanceName, PropertyTypes::Bool, defaultvalue)
   {}
   std::string toString() override {return std::to_string(value);}
   bool fromString(const std::string& str) override { return std::stoi(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
struct IntProperty : public Property<int>{
   IntProperty(const std::string& instanceName, int defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Int, defaultvalue)
   {}
   std::string toString() override {return std::to_string(value);}
   int fromString(const std::string& str) override { return std::stoi(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
struct FloatProperty : public Property<double>{
   FloatProperty(const std::string& instanceName, int defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Int, defaultvalue)
   {}
   std::string toString() override {return std::to_string(value);}
   double fromString(const std::string& str) override { return std::stod(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct Vec2Property : public Property<GFCSDraw::Vec2<T>>{
   Vec2Property(const std::string& instanceName, int defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Vec2, defaultvalue)
   {}
   std::string toString() override {return value.toString();}
   GFCSDraw::Vec2<T> fromString(const std::string& str) override {return GFCSDraw::Vec2<T>::fromString(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct RectProperty : public Property<GFCSDraw::Rect<T>>{
   RectProperty(const std::string& instanceName, GFCSDraw::Rect<T> defaultvalue=GFCSDraw::Rect<T>())
   : Property(instanceName, PropertyTypes::Rect, defaultvalue)
   {}
   std::string toString() override {return value.toString();}
   GFCSDraw::Rect<T> fromString(const std::string& str) override {return GFCSDraw::Rect<T>::fromString(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
//template <typename T>
//struct EnumProperty : public Property<T>{
//   EnumProperty(const std::string& instanceName, T defaultvalue)
//   : Property(instanceName, PropertyTypes::Enum, defaultvalue)
//   {}
//   std::string toString() override {
//      for(int i=0;i<size;i++){
//         auto _value = valueDict[i];
//         if (_value == Property<T>::value){
//            return std::string(nameDict[i]);
//         }
//      }
//      throw std::runtime_error("Invalid EnumProperty lookup");
//   }
//   T fromString(const std::string& str) override {
//      for(int i=0;i<size;i++){
//         auto name = nameDict[i];
//         if (name == str){
//            return valueDict[i];
//         }
//      }
//      throw std::runtime_error("Invalid EnumProperty value " + str);
//   }
//   virtual std::string_view& getNameDict(){nameDict;}
//   static constexpr std::string_view nameDict[] = {{}};
//   static constexpr T valueDict[] = {{}};
//   static constexpr size_t size = 0;
//};

template <typename T, auto C>
using EnumPair = std::array<std::pair<T, std::string_view>, C>;
#define ENUM_PAIR_DECLARE(ENUM_NAME, MEMBER_NAME) std::pair<ENUM_NAME, std::string_view>(ENUM_NAME::MEMBER_NAME, #MEMBER_NAME)
template <typename T, auto C>
struct EnumProperty : public Property<T>{
   EnumProperty(const std::string& instanceName, T defaultvalue)
         : Property(instanceName, PropertyTypes::Enum, defaultvalue)
   {}
   std::string toString() override {
      for(int i=0;i<getDict().size();i++){
         auto _value = getDict()[i].first;
         if (_value == Property<T>::value){
            return std::string(getDict()[i].second);
         }
      }
      throw std::runtime_error("Invalid EnumProperty lookup");
   }
   T fromString(const std::string& str) override {
      for(int i=0;i<getDict().size();i++){
         auto name = getDict()[i].second;
         if (name == str){
            return getDict()[i].first;
         }
      }
      throw std::runtime_error("Invalid EnumProperty value " + str);
   }
   virtual const EnumPair<T, C>& getDict() = 0;
};