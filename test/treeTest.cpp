#include "treeTest.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
using namespace Tree;


void Renderer2D::_on_child_added_to_tree(TypeNode *child) {
   if (child->as<Drawable2D>()) {
      std::cout << "Child " << child->instanceInfo.instanceName << " is a renderable" << std::endl;
   }
}

int main(){
   //abstract tree
   {
      static constexpr char CANVAS_NAME[] = "canvas";
      static constexpr char COMPONENT_NAME[] = "component";
      {
         auto root = make_node<Canvas>(CANVAS_NAME, "asdf");
         if (auto renderable = root->as<Drawable2D>()){
            cout << root->instanceInfo.instanceName << ":" << renderable.value()->someData << endl;
         }
         if (auto engineobj = root->as<EngineObject>()){
            cout << root->instanceInfo.instanceName << " inherits from " << engineobj.value()->TYPE_NAME << endl;
         } else {
            cout << root->instanceInfo.instanceName << " is not " << EngineObject::TYPE_NAME << endl;
         }
         root->addChild<Sprite, Canvas>(make_node<Sprite>("sprite_instance"));
         if (auto found = root->getChild("sprite")){
            if (found){
               auto _component = (*found.value()).as<Canvas>();
               if (_component){
                  std::cout << _component.value()->someOtherData << endl;
               }
            }
         }
//         auto unlreated = make_node<CompletelyUnrelatedType>("unrelatedType");
//         root->addChild<CompletelyUnrelatedType, Canvas>(std::move(unlreated));
      }
   }

   return 0;
}