#include "Window2.h"
#include "TypeTree.h"
#include "ReyObject.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyEngine::Internal::Tree;

/////////////////////////////////////////////////////////////////////////////////////////
//void Canvas::_on_descendant_added_to_tree(TypeNode *n) {
//   if (auto drawable = n->as<Drawable2D>()) {
//      std::cout << "Descendant " << n->name << " is drawable" << std::endl;
//      cacheDrawables();
//   }
//   if (auto handles = n->as<InputHandler>()) {
//      std::cout << "Descendant " << n->name << " is input handlerable" << std::endl;
//      handles.value()->__process_unhandled_input();
//      cacheDrawables();
//   }
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void Canvas::cacheDrawables() {
//   //walk the tree and cache all our drawables in the correct rendering order
//   drawOrder.clear();
//   std::string indent = "";
//   cout << _node->name << endl;
//   std::function<std::vector<TypeNode*>(TypeNode*)> getDrawables = [&](TypeNode* node){
//      indent += "   ";
//      for (const auto& child : node->getChildren()){
//         cout << indent << child->name << endl;
//         if (auto drawable = child->as<Drawable2D>()){
//            drawOrder.emplace_back(Matrix(), drawable.value());
//         }
//         getDrawables(child);
//      }
//      indent.resize(indent.length() - 3);
//      return node->getChildren();
//   };
//
//   auto parent = _node->getParent();
//   auto root = _node->getRoot();
//
//   getDrawables(_node);
//   std::reverse(drawOrder.begin(), drawOrder.end());
//   for (const auto& [Matrix, drawable] : drawOrder){
//      cout << drawable->getNode()->getScenePath() << ",";
//   }
//   cout << endl;
//}



int main(){
   //create window
   //createWindowPrototype(const std::string &title, int width, int height, const std::vector<ReyEngine::WindowFlags> &flags, int targetFPS) {
   {
      auto& window = Application2::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
//
//      //abstract tree
//      {
//         static constexpr char CANVAS_NAME[] = "canvas";
//         static constexpr char SPRITE_NAME[] = "SPRITE";
//         {
//            auto root = make_node<Canvas>(CANVAS_NAME, "asdf");
//            auto sprite = root->addChild(make_node<Sprite>(SPRITE_NAME, "texture_path"));
//            auto sprite2 = sprite->addChild(make_node<Sprite>(string(SPRITE_NAME) + "_2", "texture_path"));
//            auto sprite3 = sprite->addChild(make_node<Sprite>(string(SPRITE_NAME) + "_3", "texture_path"));
//            auto sprite33 = sprite3->addChild(make_node<Sprite>(string(SPRITE_NAME) + "_33", "texture_path"));
//            auto sprite4 = sprite->addChild(make_node<Sprite>(string(SPRITE_NAME) + "_4", "texture_path"));
//         }
//      }
//
//
      window.exec();
   }
   return 0;

}