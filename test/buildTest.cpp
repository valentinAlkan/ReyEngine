#include "BaseWidget.h"

using namespace std;
using namespace ReyEngine;

// A typical reyengine object that takes only an instance name parameter.
class DefaultBuildableObject : public BaseWidget{
private:
   REYENGINE_OBJECT_BUILD_ONLY(DefaultBuildableObject, BaseWidget){}
public:
   REYENGINE_DEFAULT_BUILD(DefaultBuildableObject)
   void render() const override{};
};

class CustomBuildableObject : public BaseWidget{
private:
   // specify required build signatures
REYENGINE_OBJECT_CUSTOM_BUILD(CustomBuildableObject, BaseWidget, std::tuple<const std::string&, const CustomBuildableObject&>, std::tuple<const std::string&, int>)
   //must implement all necessary constructors yourself
   CustomBuildableObject(const std::string& name, int& i): REYENGINE_CTOR_INIT_LIST(name, BaseWidget), localInt(i){}
   CustomBuildableObject(const std::string& name, const CustomBuildableObject& rhs) : REYENGINE_CTOR_INIT_LIST(name, BaseWidget), localInt(rhs.localInt) {}
public:
   //must implement all required build functions yourself
   static std::shared_ptr<CustomBuildableObject> build(const std::string& name, const CustomBuildableObject& object) { return CustomBuildableObject::_reyengine_make_shared(name, object);}
   static std::shared_ptr<CustomBuildableObject> build(const std::string& name, int i) { return CustomBuildableObject::_reyengine_make_shared(name, i);}
   void render() const override{};
private:
   int& localInt;
};

// No signatures are required
class NoSigs : public BaseWidget{
private:
REYENGINE_OBJECT_CUSTOM_BUILD(NoSigs, BaseWidget)
public:
   NoSigs(const std::string& name, int i): REYENGINE_CTOR_INIT_LIST(name, BaseWidget), localInt(i){}
   NoSigs(const std::string& name, const NoSigs& rhs) : REYENGINE_CTOR_INIT_LIST(name, BaseWidget), localInt(rhs.localInt) {}
   void render() const override{};
private:
   int& localInt;
};



int main(){
   //normal way to build ReyEngine objects
   auto defaultBuildable = DefaultBuildableObject::build("DefaultBuildable");

   // build reyengine objects with arbitrary parameters
   auto customBuildable = CustomBuildableObject::build("CustomBuilt", 1);

   // build functions are forwarded to underlying ctors - invalid params will result in compilation errors
   // doesn't build - extra parameter ->  auto defaultBuildable = DefaultBuildableObject::build("DefaultBuildable", 1);
   // doesn't build, missing parameter -> auto customBuildable = CustomBuildableObject::build("CustomBuilt");

   // As long as normal C++ rules are followed, objects can be built however you like.
   // This object does not implement build functions and allows public access to constructors.
   // Most reyengine objects need to be shared pointers. However, if you wish to build like this, you can do so.
   // Make sure you know  what you are doing if you are going to build objects like this.
   NoSigs noSig("No sigs", 1);
   return 0;
}