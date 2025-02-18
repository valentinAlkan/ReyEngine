#include "TypeTree.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
using namespace Tree;

int main(){
   // Example usage:
   {
      WeakRef<int> weak;
      {
         auto ref = make_ref_counted<int>(42);
         weak = ref.getWeakRef();
         if (auto handle = weak.lock()) {
            // Use the handle
            *handle = 43;
            assert(handle);
         }

         assert(!weak.expired());
      }
      assert(weak.expired());
   }
   struct Canvas  {
      Canvas(const std::string& blah): someData(blah){}
      std::string someData;
   };

   struct Component  {
      Component(const std::string& blah): componentData(blah){}
      std::string componentData;
   };

   struct Buttz : public Component{
      Buttz(int a, int b, const std::string& someBullshit)
      : Component(someBullshit)
      , a(a)
      , b(b)
      {}
      int a;
      int b;
   };

   //abstract tree
   {
      static constexpr char CANVAS_NAME[] = "canvas";
      static constexpr char COMPONENT_NAME[] = "component";
      {
         auto root = make_node<Canvas>(CANVAS_NAME, "CanvasType", "hey fuck you guy");
         if (auto _canvas = root.as<Canvas>()){
            cout << root.instanceInfo.instanceName << ":" << _canvas.value()->someData << endl;
         }
         root.addChild(make_node<Component>(COMPONENT_NAME, "ComponentType", "fuck dude what the h"));
         if (auto found = root.getChild("component")){
            if (found){
               auto _component = (*found.value()).as<Component>();
               if (_component){
                  std::cout << _component.value()->componentData << endl;
               }
            }
         }
         auto child = make_node_refcounted<Buttz>("buttz", "Buttz", 1,2,"yo");
         root.addChild(child);
      }
   }
//   TypeNode root2(make_unique<Canvas>());
//   root.addChild(root2)





   return 0;
}