#include "TypeTree.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
using namespace Tree;

#define TYPENAME(CLASSNAME) static constexpr char TYPE_NAME[] = #CLASSNAME;

struct BaseCanvas {
   TYPENAME(BaseCanvas)
   void _on_added_to_tree();
};

struct Canvas : public BaseCanvas {
   Canvas(const std::string& blah): someData(blah){}
   Canvas(int a){}
   static constexpr char TYPE_NAME[] = "Canvas";
   std::string someData;
};


struct Component {
   Component(): componentData("Default args"){}
   Component(const std::string& blah): componentData(blah){}
   TYPENAME(Component);
   void _on_added_to_tree();
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
   void _on_added_to_tree();
};

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



   //abstract tree
   {
      static constexpr char CANVAS_NAME[] = "canvas";
      static constexpr char COMPONENT_NAME[] = "component";
      {

         auto wrapped = TypeWrapper<Canvas>("some text");

         auto root = make_node<Canvas>(CANVAS_NAME, "asdf");
         if (auto _canvas = root->is<Canvas>()){
            cout << root->instanceInfo.instanceName << ":" << _canvas.value()->someData << endl;
         }
         if (auto _baseCanvas = root->as<BaseCanvas>()){
            cout << root->instanceInfo.instanceName << " inherits from " << _baseCanvas.value()->TYPE_NAME << endl;
         } else {
            cout << root->instanceInfo.instanceName << " is not " << BaseCanvas::TYPE_NAME << endl;
         }
         root->addChild(make_node<Component>("component_instance"));
         if (auto found = root->getChild("component")){
            if (found){
//               auto _component = (*found.value()).as<Component>();
//               if (_component){
//                  std::cout << _component.value()->componentData << endl;
//               }
            }
         }
//         auto child = make_node<Buttz>("buttz", "Buttz", 1,2,"yo");
//         root.addChild(child);
      }
   }
//   TypeNode root2(make_unique<Canvas>());
//   root.addChild(root2)





   return 0;
}