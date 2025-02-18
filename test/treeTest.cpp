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

   //abstract tree
   {
      static constexpr char CANVAS_NAME[] = "canvas";
      static constexpr char COMPONENT_NAME[] = "component";
      auto canvas = make_ref_counted<TypeWrapper<Canvas>>(Canvas("blah blah blah"));
      auto component = make_ref_counted<TypeWrapper<Component>>(Component("fuck dude what the h"));
      {
         auto root = make_node<Canvas>(CANVAS_NAME, "CanvasType", "hey fuck you guy");
         if (auto _canvas = root.as<Canvas>()){
            cout << root.instanceInfo.instanceName << ":" << _canvas.value()->someData << endl;
         }
         auto nakednode = new TypeNode(std::move(component), COMPONENT_NAME, "Component");
         auto node = RefCounted<TypeNode>(nakednode);
         root.addChild(node);
         if (auto found = root.getChild("component")){
            if (found){
               auto _component = (*found.value()).as<Component>();
               if (_component){
                  std::cout << _component.value()->componentData << endl;
               }
            }
         }
      }
   }
//   TypeNode root2(make_unique<Canvas>());
//   root.addChild(root2)





   return 0;
}