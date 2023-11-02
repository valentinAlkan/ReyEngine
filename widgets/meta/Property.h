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
   PROP_TYPE(Color)
   PROP_TYPE(Theme)
   PROP_TYPE(List)
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

///Something that can have properties (and therefore subproperties
struct PropertyContainer{
   ///make sure ALL register property functions are called
   virtual void registerProperties() = 0;
   void registerProperty(BaseProperty& property);
   void moveProperty(std::shared_ptr<BaseProperty>);
   void updateProperty(BaseProperty&);
   PropertyMap& getProperties(){return _properties;}
protected:
   void _initProperties(){};
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
   void registerProperties() override = 0;
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
   [[nodiscard]] T& ref() {return value;}
   [[nodiscard]] const T& get() const {return value;}
   void set(const T& newValue){
      value = newValue;
   }
   T value;
};

/////////////////////////////////////////////////////////////////////////////////////////
struct StringProperty : public Property<std::string>{
   StringProperty(const std::string& instanceName, const std::string& defaultvalue = "")
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
struct FloatProperty : public Property<float>{
   FloatProperty(const std::string& instanceName, float defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Int, defaultvalue)
   {}
   std::string toString() override {return std::to_string(value);}
   float fromString(const std::string& str) override { return (float)std::stod(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct Vec2Property : public Property<GFCSDraw::Vec2<T>>{
   Vec2Property(const std::string& instanceName, GFCSDraw::Vec2<T> defaultvalue = 0)
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
struct ColorProperty : public Property<GFCSDraw::ColorRGBA>{
   ColorProperty(const std::string& instanceName,  GFCSDraw::ColorRGBA defaultvalue)
   : Property<GFCSDraw::ColorRGBA>(instanceName, PropertyTypes::Color, defaultvalue)
   {}
   std::string toString() override {return "{" + std::to_string(value.r) + ", " + std::to_string(value.g) + ", " + std::to_string(value.b) + ", "  + std::to_string(value.a) + "}";}
   GFCSDraw::ColorRGBA fromString(const std::string& str) override {
      auto split = string_tools::fromList(str);
      return {std::stoi(split[0]), std::stoi(split[1]), std::stoi(split[2]), std::stoi(split[3])};
   }
};

/////////////////////////////////////////////////////////////////////////////////////////
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
         auto _name = getDict()[i].second;
         if (_name.empty())throw std::runtime_error("Empty EnumProperty value for enum " + this->instanceName());
         if (_value == Property<T>::value){
            return std::string(_name);
         }
      }
      throw std::runtime_error("Invalid EnumProperty lookup");
   }
   T fromString(const std::string& str) override {
      for(int i=0;i<getDict().size();i++){
         auto name = getDict()[i].second;
         if (name.empty()) throw std::runtime_error("Empty EnumProperty value for enum " + this->instanceName());
         if (name == str){
            return getDict()[i].first;
         }
      }
      throw std::runtime_error("Invalid EnumProperty value " + str);
   }
   virtual const EnumPair<T, C>& getDict() = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct ListProperty : public Property<std::vector<T>>{
   ListProperty(const std::string& instanceName) :
   Property<std::vector<T>>(instanceName, PropertyTypes::List, {}){

   }
   std::string toString() override {
      auto vec = Property<std::vector<T>>::value;
      std::vector<std::string> stringVec;
      for (const auto& t : vec){
         stringVec.push_back(elementToString(t));
      }
      return string_tools::listJoin(stringVec);
   }
   std::vector<T> fromString(const std::string& str) override {
      auto strList = string_tools::fromList(str);
      for (const auto& s : strList){
         Property<std::vector<T>>::value.push_back(stringToElement(s));
      }
      return Property<std::vector<T>>::value;
   }
   virtual T stringToElement(const std::string& s) = 0;
   virtual std::string elementToString(const T& t){return std::to_string(t);}
};

struct FloatListProperty : public ListProperty<float>{
   FloatListProperty(const std::string& instanceName): ListProperty<float>(instanceName){}
   float stringToElement(const std::string& element){return (float)stod(element);}
   float sum() const {float total=0;for (const auto& v : value){total += v;}return total;}
};