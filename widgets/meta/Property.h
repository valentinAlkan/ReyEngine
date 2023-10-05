#pragma once
#include <string>
#include <cassert>
#include <utility>
#include <map>
#include <memory>
#include <unordered_map>
#include "DrawInterface.h"
#define PROP_TYPE(propName) static constexpr char propName[] = #propName;

namespace PropertyTypes{
   PROP_TYPE(String);
   PROP_TYPE(Int);
   PROP_TYPE(Float);
   PROP_TYPE(Bool);
   PROP_TYPE(Vec2);
   PROP_TYPE(Rect);
}

namespace PropertyMeta{
   static constexpr char INDENT[] = "   ";
   static constexpr char SEP = '|';
}

struct PropertyPrototype{
   std::string instanceName;
   std::string typeName;
   std::string data;
   std::vector<PropertyPrototype> subproperties;
};

struct BaseProperty{
   BaseProperty(const std::string instanceName, const std::string& typeName)
   : _instanceName(instanceName)
   , _typeName(typeName){}
   virtual void load(PropertyPrototype data) = 0;
   virtual std::string dump() = 0;
   std::string instanceName(){return _instanceName;}
   std::string typeName(){return _typeName;}
   std::vector<BaseProperty> subProperties;
private:
   const std::string _instanceName;
   const std::string _typeName;
};

template <typename T>
struct Property : public BaseProperty {
   Property(const std::string instanceName, const std::string& typeName, T defaultvalue)
   : BaseProperty(instanceName, typeName)
   , value(std::move(defaultvalue)){}
   T value;
};

struct StringProperty : public Property<std::string>{
   StringProperty(const std::string& instanceName, const std::string defaultvalue = "")
   : Property(instanceName, PropertyTypes::String, defaultvalue)
   {}
   std::string dump() override {return value;}
   void load(PropertyPrototype data) override { value = data.data;}
};

struct BoolProperty : public Property<bool>{
   BoolProperty(const std::string& instanceName, bool defaultvalue = false)
   : Property(instanceName, PropertyTypes::Bool, defaultvalue)
   {}
   std::string dump() override {return std::to_string(value);}
   void load(PropertyPrototype data) override { value = std::stoi(data.data);}
};

struct IntProperty : public Property<int>{
   IntProperty(const std::string& instanceName, int defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Int, defaultvalue)
   {}
   std::string dump() override {return std::to_string(value);}
   void load(PropertyPrototype data) override { value = std::stoi(data.data);}
};

struct FloatProperty : public Property<double>{
   FloatProperty(const std::string& instanceName, int defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Int, defaultvalue)
   {}
   std::string dump() override {return std::to_string(value);}
   void load(PropertyPrototype data) override { value = std::stod(data.data);}
};

template <typename T>
struct Vec2Property : public Property<GFCSDraw::Vec2<T>>{
   Vec2Property(const std::string& instanceName, int defaultvalue = 0)
   : Property(instanceName, PropertyTypes::Vec2, defaultvalue)
   {}
   std::string dump() override {return value.toString();}
   void load(PropertyPrototype data) override {value.fromString(data.data);}
};

template <typename T>
struct RectProperty : public Property<GFCSDraw::Rect<T>>{
   RectProperty(const std::string& instanceName, GFCSDraw::Rect<T> defaultvalue=GFCSDraw::Rect<T>())
   : Property(instanceName, PropertyTypes::Rect, defaultvalue)
   {}
   std::string dump() override {return value.toString();}
   void load(PropertyPrototype data) override {value.fromString(data.data);}
};

using PropertyMap = std::unordered_map<std::string, BaseProperty*>;
using PropertyPrototypeMap = std::unordered_map<std::string, PropertyPrototype>;