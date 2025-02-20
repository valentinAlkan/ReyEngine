#include "Window2.h"
#include "TypeTree.h"
#include "NewEngineTypes.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyEngine::Internal::Tree;

void Renderer2D::_on_child_added_to_tree(TypeNode *child) {
   if (auto drawable = child->as<Drawable2D>()) {
      std::cout << "Child " << child->name << " is a renderable" << std::endl;
      drawOrder.push_back(drawable.value());
      for (const auto& drawable : drawOrder){
//         cout << drawable.getNode()
      }
   }

}



int main(){
   //abstract tree
   {
      static constexpr char CANVAS_NAME[] = "canvas";
      static constexpr char SPRITE_NAME[] = "SPRITE";
      {
         auto root = make_node<Canvas>(CANVAS_NAME, "asdf");
         if (auto renderable = root->as<Drawable2D>()){
            cout << root->name << ":" << renderable.value()->someData << endl;
         }
         if (auto engineobj = root->as<EngineObject>()){
            cout << root->name << " inherits from " << engineobj.value()->TYPE_NAME << endl;
         } else {
            cout << root->name << " is not " << EngineObject::TYPE_NAME << endl;
            assert(false);
         }
         root->addChild(make_node<Sprite>(SPRITE_NAME, "texture_path"));
         if (auto found = root->getChild(SPRITE_NAME)){
            if (found){
               if (auto sprite = (*found.value()).as<Sprite>()){
                  std::cout << "Found " << SPRITE_NAME << " : " << sprite.value()->texPath << endl;
               } else {
                  assert(false);
               }
            }
         }
//         auto unlreated = make_node<CompletelyUnrelatedType>("unrelatedType");
//         root->addChild<CompletelyUnrelatedType, Canvas>(std::move(unlreated));
      }
   }

   return 0;
}