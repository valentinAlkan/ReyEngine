#pragma once
#include <string>
#include <cassert>
#include <utility>

#define PROP_TYPE(propName) static constexpr char propName[] = #propName;

namespace PropertyTypes{
   PROP_TYPE(String);
   PROP_TYPE(Int);
   PROP_TYPE(Float);
   PROP_TYPE(Bool);
}


struct BaseProperty{
   BaseProperty(const std::string& typeName){}
   virtual void load(std::string) = 0;
   virtual std::string dump() = 0;
   std::string typeName(){return _typeName;}
private:
   const std::string _typeName;
};

template <typename T>
struct Property : public BaseProperty {
   Property(const std::string& typeName, T value): BaseProperty(typeName), _value(std::move(value)){}
   T _value;
   friend class Scene;
};

struct StringProperty : public Property<std::string>{
   StringProperty(const std::string& str): Property(PropertyTypes::String, str) {}
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






//Creates properties when loading
class PropertyFactory {
public:
//   static PropertyFactory & getInstance(){static PropertyFactory instance; return instance;}

private:
   PropertyFactory () {}
public:
//   PropertyFactory (PropertyFactory  const&) = delete;
//   void operator=(PropertyFactory const&) = delete;
};













