#pragma once
#include <string>
#include <cassert>
#include <utility>
#include <map>
#include <memory>
#include <unordered_map>
#include <stdexcept>
#include <vector>
#include "StringTools.h"
#include "SharedFromThis.h"
#include <iostream>

#define PROP_TYPE(propName) static constexpr char propName[] = #propName;
#define PROPERTY_DECLARE(PROPERTYNAME, ...) PROPERTYNAME(#PROPERTYNAME, ##__VA_ARGS__)

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
   PROP_TYPE(Vec3)
   PROP_TYPE(Vec4)
   PROP_TYPE(Rect)
   PROP_TYPE(Size)
   PROP_TYPE(Circle)
   PROP_TYPE(Timer)
   PROP_TYPE(Enum)
   PROP_TYPE(Color)
   PROP_TYPE(Theme)
   PROP_TYPE(List)
   PROP_TYPE(Font)
   PROP_TYPE(Cursor)
   PROP_TYPE(LayoutMargin)
   PROP_TYPE(TileMapGridType)
   PROP_TYPE(TileMapCellData)
   PROP_TYPE(TileMapLayer)
   PROP_TYPE(FilePath)
   PROP_TYPE(BaseWidget)
}
namespace ReyEngine{

   template<typename T>
   class Vec2;
   template<typename T>
   class Vec3;
   template<typename T>
   class Vec4;
   template<typename T>
   struct Rect;
   class ColorRGBA;
   struct Circle;
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

///Something that can have properties (and therefore subproperties)
struct PropertyContainer : public inheritable_enable_shared_from_this<PropertyContainer> {
   ///make sure ALL register property functions are called
   virtual void registerProperties() = 0;
   void registerProperty(BaseProperty& property);
   void moveProperty(std::shared_ptr<BaseProperty>);
   void updateProperty(BaseProperty&);
   PropertyMap& getProperties(){return _properties;}
   template <typename T>
   T& getProperty(const std::string& instanceName){
      auto found = _properties.find(instanceName);
      if (found == _properties.end()){
         //check owned properties
         auto foundOwned = _ownedProperties.find(instanceName);
         if (foundOwned == _ownedProperties.end()) {
            throw std::runtime_error("Property " + instanceName + " not found");
         }
//         return (T&)(*foundOwned->second);
            return static_cast<T&>(*foundOwned->second);
      }
//      return (T&)*found->second;
      return static_cast<T&>(*found->second);
   }
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
//   ~BaseProperty(){std::cout << "property dtor " << _instanceName << std::endl;}
   void _load(const PropertyPrototype& data);
   std::string instanceName() const {return _instanceName;}
   std::string typeName() const {return _typeName;}
   void registerProperties() override = 0;
   virtual void load(const PropertyPrototype& data) = 0;
   virtual std::string toString() const = 0;
private:
   const std::string _instanceName;
   const std::string _typeName;
};

/////////////////////////////////////////////////////////////////////////////////////////
// A simple Dynamic Property that won't be serialized and can be overridden.
struct DynamicProperty : public BaseProperty {
    DynamicProperty(const std::string& instanceName): BaseProperty(instanceName, "DynamicProperty"){}
    void registerProperties() override{};
    void load(const PropertyPrototype& data) override {}
    std::string toString() const {return "";}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct Property : public BaseProperty {
   Property(const std::string instanceName, const std::string& typeName): BaseProperty(instanceName, typeName){}
   Property(const std::string instanceName, const std::string& typeName, T&& defaultvalue)
   : BaseProperty(instanceName, typeName)
   , value(std::move(defaultvalue)){}

   void registerProperties() override {}
   Property& operator=(const T& newValue){
      value = newValue;
      return *this;
   }
   bool operator!(){return !bool(value);}
   Property& operator=(const Property& other){
      value = other.value;
      return *this;
   }
   virtual T fromString(const std::string& str) = 0;
   void load(const PropertyPrototype& data) override {value = std::move(fromString(data.data));}
   inline operator const T&() const {return value;}
   inline operator T&(){return value;}
   void set(const T& newValue){
      value = newValue;
   }
   T value;
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct LambdaProperty : public Property<T>{
   using Property<T>::operator=;
   using fxSerializer = std::function<std::string(const T&)>;
   LambdaProperty(
      const std::string& instanceName,
      const std::string& propertyType,
      T&& defaultValue,
      fxSerializer serializer,
      std::function<T(const std::string&)> deserializer
      )
   : Property<T>(instanceName, propertyType, std::move(defaultValue))
   , _fxSerializer(serializer)
   , _fxDeserializer(deserializer)
   {}
   std::string toString() const override {return _fxSerializer(Property<T>::value);}
   T fromString(const std::string& data) override {return _fxDeserializer(data);}
private:
   fxSerializer _fxSerializer;
   std::function<T(const std::string&)> _fxDeserializer;
};

/////////////////////////////////////////////////////////////////////////////////////////
struct StringProperty : public Property<std::string>{
   using Property<std::string>::operator=;
   StringProperty(const std::string& instanceName, std::string&& defaultvalue = "")
   : Property(instanceName, PropertyTypes::String, std::move(defaultvalue))
   {}
   std::string toString() const override {return value;}
   std::string fromString(const std::string& data) override { return data;}
};

/////////////////////////////////////////////////////////////////////////////////////////
struct BoolProperty : public Property<bool>{
   using Property<bool>::operator=;
   BoolProperty(const std::string& instanceName, bool&& defaultvalue = false)
   : Property(instanceName, PropertyTypes::Bool, std::move(defaultvalue))
   {}
   std::string toString() const override {return std::to_string(value);}
   bool fromString(const std::string& str) override { return std::stoi(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
struct IntProperty : public Property<int>{
   using Property<int>::operator=;
   IntProperty(const std::string& instanceName, int&& defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Int, std::move(defaultvalue))
   {}
   std::string toString() const override {return std::to_string(value);}
   int fromString(const std::string& str) override { return std::stoi(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
struct FloatProperty : public Property<float>{
   using Property<float>::operator=;
   FloatProperty(const std::string& instanceName, float&& defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Int, std::move(defaultvalue))
   {}
   std::string toString() const override {return std::to_string(value);}
   float fromString(const std::string& str) override { return (float)std::stod(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct Vec2Property : public Property<ReyEngine::Vec2<T>>{
   using Property<ReyEngine::Vec2<T>>::operator=;
   Vec2Property(const std::string& instanceName, ReyEngine::Vec2<T> defaultvalue = 0)
   : Property<ReyEngine::Vec2<T>>(instanceName, PropertyTypes::Vec2, defaultvalue)
   {}
   std::string toString() const override {return Property<ReyEngine::Vec2<T>>::value.toString();}
   ReyEngine::Vec2<T> fromString(const std::string& str) override {return ReyEngine::Vec2<T>::fromString(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct Vec3Property : public Property<ReyEngine::Vec3<T>>{
    using Property<ReyEngine::Vec3<T>>::operator=;
    Vec3Property(const std::string& instanceName, ReyEngine::Vec3<T> defaultvalue = 0)
            : Property<ReyEngine::Vec3<T>>(instanceName, PropertyTypes::Vec3, defaultvalue)
    {}
    std::string toString() const override {return Property<ReyEngine::Vec3<T>>::value.toString();}
    ReyEngine::Vec3<T> fromString(const std::string& str) override {return ReyEngine::Vec3<T>::fromString(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct Vec4Property : public Property<ReyEngine::Vec4<T>>{
    using Property<ReyEngine::Vec4<T>>::operator=;
    Vec4Property(const std::string& instanceName, ReyEngine::Vec4<T> defaultvalue = ReyEngine::Vec4<T>())
    : Property<ReyEngine::Vec4<T>>(instanceName, PropertyTypes::Vec4, defaultvalue)
    {}
    Vec4Property(const std::string& instanceName, const std::string& typeName, ReyEngine::Vec4<T>&& defaultvalue = ReyEngine::Vec4<T>())
    : Property<ReyEngine::Vec4<T>>(instanceName, typeName, std::move(defaultvalue))
    {}
    std::string toString() const override {return Property<ReyEngine::Vec4<T>>::value.toString();}
    ReyEngine::Vec4<T> fromString(const std::string& str) override {return ReyEngine::Vec4<T>::fromString(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct RectProperty : public Property<ReyEngine::Rect<T>>{
   using Property<ReyEngine::Rect<T>>::operator=;
   RectProperty(const std::string& instanceName, ReyEngine::Rect<T>&& defaultvalue=ReyEngine::Rect<T>())
   : Property<ReyEngine::Rect<T>>(instanceName, PropertyTypes::Rect, std::move(defaultvalue))
   {}
   std::string toString() const override {return Property<ReyEngine::Rect<T>>::value.toString();}
   ReyEngine::Rect<T> fromString(const std::string& str) override {return ReyEngine::Rect<T>::fromString(str);}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T, auto C>
using EnumPair = std::array<std::pair<T, std::string_view>, C>;
#define ENUM_PAIR_DECLARE(ENUM_NAME, MEMBER_NAME) std::pair<ENUM_NAME, std::string_view>(ENUM_NAME::MEMBER_NAME, #MEMBER_NAME)
template <typename T, auto C>
struct EnumProperty : public Property<T>{
   using Property<T>::operator=;
   EnumProperty(const std::string& instanceName, T&& defaultvalue)
   : Property<T>(instanceName, PropertyTypes::Enum, std::move(defaultvalue))
   {}
   std::string toString() const override {
      for(long unsigned int i=0;i<getDict().size();i++){
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
      for(long unsigned int i=0;i<getDict().size();i++){
         auto name = getDict()[i].second;
         if (name.empty()) throw std::runtime_error("Empty EnumProperty value for enum " + this->instanceName());
         if (name == str){
            return getDict()[i].first;
         }
      }
      throw std::runtime_error("Invalid EnumProperty value " + str);
   }
   virtual const EnumPair<T, C>& getDict() const = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct ListProperty : public Property<std::vector<T>>{
   using Property<std::vector<T>>::operator=;
   ListProperty(const std::string& instanceName) :
   Property<std::vector<T>>(instanceName, PropertyTypes::List, {}){
   }
   std::string toString() const override {
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
   void set(int index, T newValue){
      if(Property<std::vector<T>>::value.size()<=index){
         throw std::runtime_error("List " + BaseProperty::instanceName() + " index " + std::to_string(index) + " out of range!");
      }
      Property<std::vector<T>>::value.at(index) = newValue;
   }
   T& get(int index) {return Property<std::vector<T>>::value.at(index);}
   size_t size() const {return Property<std::vector<T>>::value.size();}
   ListProperty<T>& operator=(const std::vector<T>& other){Property<std::vector<T>>::value = other; return *this;}
   void append(const T& t){Property<std::vector<T>>::value.push_back(t);}
   T& append(){Property<std::vector<T>>::value.emplace_back(); return Property<std::vector<T>>::value.back();} //create a new element at the end of the list
   void clear(){Property<std::vector<T>>::value.clear();}
   void erase(int index){auto it=Property<std::vector<T>>::value.begin() + index; Property<std::vector<T>>::value.erase(it);}
   void pop_back(){auto it=Property<std::vector<T>>::value.end() - 1; Property<std::vector<T>>::value.erase(it);}
   /**/
   virtual T stringToElement(const std::string&) const = 0;
   virtual std::string elementToString(const T&) const = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
struct BoolListProperty : public ListProperty<bool>{
   using ListProperty<bool>::operator=;
   BoolListProperty(const std::string& instanceName): ListProperty<bool>(instanceName){}
   bool stringToElement(const std::string& element) const override {return element == "true";}
   std::string elementToString(const bool& t)       const override {return t ? "true" : "false";}
};

/////////////////////////////////////////////////////////////////////////////////////////
struct FloatListProperty : public ListProperty<float>{
   using ListProperty<float>::operator=;
   FloatListProperty(const std::string& instanceName): ListProperty<float>(instanceName){}
   float stringToElement(const std::string& element) const override {return (float)stod(element);}
   std::string elementToString(const float& t)       const override {return std::to_string(t);}
   float sum() const {float total=0;for (const auto& v : value){total += v;}return total;}
};

/////////////////////////////////////////////////////////////////////////////////////////
struct IntListProperty : public ListProperty<int>{
   using ListProperty<int>::operator=;
   IntListProperty(const std::string& instanceName): ListProperty<int>(instanceName){}
   int stringToElement(const std::string& element) const override {return (int)stoi(element);}
   std::string elementToString(const int& t)       const override {return std::to_string(t);}
   int sum() const {int total=0;for (const auto& v : value){total += v;}return total;}
};

/////////////////////////////////////////////////////////////////////////////////////////
struct StringListProperty : public ListProperty<std::string>{
   using ListProperty<std::string>::operator=;
   StringListProperty(const std::string& instanceName): ListProperty<std::string>(instanceName){}
   std::string stringToElement(const std::string& element) const override {return element;}
   std::string elementToString(const std::string& t)       const override {return t;}
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//template <typename T, typename R>
//struct MapProperty : public Property<std::map<T, R>>{
//   using Property<std::map<T, R>>::operator=;
//   MapProperty(const std::string& instanceName) :
//      Property<std::map<T, R>>(instanceName, PropertyTypes::List, {}){
//   }
//   std::string toString() const override {
//      auto vec = Property<std::map<T, R>>::value;
//      std::vector<std::string> stringVec;
//      for (const auto& t : vec){
//         stringVec.push_back(elementToString(t));
//      }
//      return string_tools::listJoin(stringVec);
//   }
//   std::map<T, R> fromString(const std::string& str) override {
//      auto strList = string_tools::fromList(str);
//      for (const auto& s : strList){
//         Property<std::map<T, R>>::value.push_back(stringToElement(s));
//      }
//      return Property<std::map<T, R>>::value;
//   }
//   void set(int index, T newValue){
//      if(Property<std::map<T, R>>::value.size()<=index){
//         throw std::runtime_error("List " + BaseProperty::instanceName() + " index " + std::to_string(index) + " out of range!");
//      }
//      Property<std::map<T, R>>::value.at(index) = newValue;
//   }
//   T& get(int index) {return Property<std::map<T, R>>::value.at(index);}
//   size_t size() const {return Property<std::map<T, R>>::value.size();}
//   ListProperty<T>& operator=(const std::map<T, R>& other){Property<std::map<T, R>>::value = other; return *this;}
//   void append(const T& t){Property<std::map<T, R>>::value.push_back(t);}
//   T& append(){Property<std::map<T, R>>::value.emplace_back(); return Property<std::map<T, R>>::value.back();} //create a new element at the end of the list
//   void clear(){Property<std::map<T, R>>::value.clear();}
//   void erase(int index){auto it=Property<std::map<T, R>>::value.begin() + index; Property<std::map<T, R>>::value.erase(it);}
//   /**/
//   virtual T stringToElement(const std::string&) const = 0;
//   virtual std::string elementToString(const T&) const = 0;
//};