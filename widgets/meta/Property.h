#pragma once
#include <string>
#include <cassert>
#include <utility>
#include <map>
#include <memory>
#include <unordered_map>
#define PROP_TYPE(propName) static constexpr char propName[] = #propName;

namespace PropertyTypes{
   PROP_TYPE(String);
   PROP_TYPE(Int);
   PROP_TYPE(Float);
   PROP_TYPE(Bool);

}

namespace PropertyMeta{
   static constexpr char INDENT[] = "   ";
   static constexpr char SEP = '|';
}

struct PropertyPrototype{
   std::string instanceName;
   std::string typeName;
   std::string data;
};

struct BaseProperty{
   BaseProperty(const std::string instanceName, const std::string& typeName)
   : _instanceName(instanceName)
   , _typeName(typeName){}
   virtual void load(std::string) = 0;
   virtual std::string dump() = 0;
   std::string instanceName(){return _instanceName;}
   std::string typeName(){return _typeName;}
private:
   const std::string _instanceName;
   const std::string _typeName;
};

template <typename T>
struct Property : public BaseProperty {
   Property(const std::string instanceName, const std::string& typeName, T defaultvalue)
   : BaseProperty(instanceName, typeName)
   , _value(std::move(defaultvalue)){}
   T _value;
   friend class Scene;
};

struct StringProperty : public Property<std::string>{
   StringProperty(const std::string& instanceName, const std::string defaultvalue = "")
   : Property(instanceName, PropertyTypes::String, defaultvalue)
   {}
   std::string dump() override {return _value;}
   void load(std::string) override {assert(0);}
};

struct BoolProperty : public Property<bool>{
   std::string dump() override {return std::to_string(_value);}
   void load(std::string) override {assert(0);}
};

struct IntProperty : public Property<int>{
   std::string dump() override {return std::to_string(_value);}
   void load(std::string) override {assert(0);}
};

struct FloatProperty : public Property<double>{
   std::string dump() override {return std::to_string(_value);}
   void load(std::string) override {assert(0);}
};

using PropertyMap = std::unordered_map<std::string, BaseProperty*>;
using PropertyPrototypeMap = std::unordered_map<std::string, PropertyPrototype>;