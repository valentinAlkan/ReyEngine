#include "TypeTree.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
using namespace Tree;

#define TYPENAME(CLASSNAME) \
static constexpr char TYPE_NAME[] = #CLASSNAME; \

#define REYENGINE_OBJECT(CLASSNAME, PARENT_NAME) \
TYPENAME(CLASSNAME) \
virtual std::string getTypeName(){return TYPE_NAME;}
//void __on_added_to_tree(){__on_added_to_tree();}

//satisfies typewrappable
struct EngineObject {
   TYPENAME(EngineObject)
   void _on_added_to_tree(){};
public:
   TypeNode* getNode(){return node;}
private:
   TypeNode* node;
};

struct Renderable2D : public EngineObject {
   REYENGINE_OBJECT(Renderable2D, EngineObject)
   Renderable2D(const std::string& blah): someData(blah){}
   Renderable2D(int a){}
   ~Renderable2D() { cout << "Peace!!" << endl; }
   void _on_added_to_tree(){
      cout << "Fuck I'm a canvas and i got added to a tree" << endl;
   };
   std::string someData;
};


struct Canvas : public EngineObject {
   REYENGINE_OBJECT(Canvas, EngineObject)
   Canvas(): componentData("Default args"){}
   ~Canvas() { cout << "So long suckers!" << endl; }
   Canvas(const std::string& blah): componentData(blah){}
   void _on_added_to_tree(){
      cout << "Fuck I'm a component and i got added to a tree" << endl;
   };
   std::string componentData;
};

struct Buttz : public Canvas{
   Buttz(int a, int b, const std::string& someBullshit)
   : Canvas(someBullshit)
   , a(a)
   , b(b)
   {}
   int a;
   int b;
//   void _on_added_to_tree(){};
};

int main(){
   // Example usage:
//   {
//      WeakRef<int> weak;
//      {
//         auto ref = make_ref_counted<int>(42);
//         weak = ref.getWeakRef();
//         if (auto handle = weak.lock()) {
//            // Use the handle
//            *handle = 43;
//            assert(handle);
//         }
//
//         assert(!weak.expired());
//      }
//      assert(weak.expired());
//   }



   //abstract tree
   {
      static constexpr char CANVAS_NAME[] = "canvas";
      static constexpr char COMPONENT_NAME[] = "component";
      {
         auto root = make_node<Canvas>(CANVAS_NAME, "asdf");
         if (auto renderable = root->as<Renderable2D>()){
            cout << root->instanceInfo.instanceName << ":" << renderable.value()->someData << endl;
         }
         if (auto engineobj = root->as<EngineObject>()){
            cout << root->instanceInfo.instanceName << " inherits from " << engineobj.value()->TYPE_NAME << endl;
         } else {
            cout << root->instanceInfo.instanceName << " is not " << EngineObject::TYPE_NAME << endl;
         }
         root->addChild<Canvas>(make_node<Canvas>("component_instance"));
         if (auto found = root->getChild("component")){
            if (found){
               auto _component = (*found.value()).as<Canvas>();
               if (_component){
                  std::cout << _component.value()->componentData << endl;
               }
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