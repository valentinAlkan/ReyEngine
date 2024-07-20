// for testing various cpp stuff
#include <iostream>
#include <memory>
#include <cassert>
using namespace std;

#define DESERIALIZER  \
template <typename T> \
static typename std::enable_if<std::is_same<T, Args>::value, Args>::type

#define DESERIALIZE(CLASSNAME) CLASSNAME::Args::fromString<CLASSNAME::Args>

class BaseClass {
protected:
   struct Args {
      virtual std::string toString(){};
      DESERIALIZER fromString(const std::string& str){
         return Args();
      }
      string baseArg = "this is the baseArg";
   };
   BaseClass(Args& args) : baseArg(args.baseArg){}
   BaseClass(const std::string& data){}
   const string baseArg;
public:
   static Args newArgs(){
      return Args();
   }
};


class Derived : public BaseClass {
protected:
   struct Args : public BaseClass::Args{
      string derivedArg = "this is the derived arg";
      string toString(){return derivedArg;}
      DESERIALIZER fromString(const std::string& str){
         return Args();
      }
   };

//   static Derived fromString(const std::string& data){
//      return Derived(data);
//   }

   Derived(Args& derivedArgs)
   : BaseClass(derivedArgs)
   , derivedValue(derivedArgs.derivedArg)
   {

   }
   Derived(const std::string& data) : BaseClass(data){}
   const string derivedValue;
};

class SuperDerived : public Derived{

};


int main (int argc, char** argv){
//   Derived::Args dargs;
//   Derived d(dargs);
//   auto base = DESERIALIZE(BaseClass)("sjadfkjasdf");
//   auto derived = DESERIALIZE(BaseClass)("data");
//   auto superDervied = SuperDerived::Args::fromString<SuperDerived::Args>("junk");
//
//   cout << dargs.baseArg << endl;
//   cout << dargs.derivedArg << endl;
//   cout << dargs.toString() << endl;
//
//   cout << "-------------------" << endl;
//   cout << base.toString() << endl;
//   cout << derived.toString() << endl;
//   cout << superDervied.toString() << endl;
}