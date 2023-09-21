#pragma once
#include <string>

//Use private inheritence to generate compiler errors!
template <typename T>
struct Property {
   Property(T value): _value(value){}
   Property(): _value(0){};
protected:
   void load(std::string); // do not define
   std::string dump(); //do not define

private:
   T _value;
   friend class Scene;
};

struct StringProperty : private Property<std::string>{
};