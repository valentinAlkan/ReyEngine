#pragma once
#include "Drawable2D.h"
#include "InputHandler2.h"

namespace ReyEngine {
   class Canvas
         : public Internal::Drawable2D
         , public Internal::InputHandler
      {
      REYENGINE_OBJECT(Canvas)
      Canvas(const std::string& blah){}
      ~Canvas() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
      void _on_descendant_added_to_tree(TypeNode* child) override;
      ///walk the tree and pin any drawables to us
      void cacheDrawables();
      std::vector<std::pair<Matrix, Drawable2D*>> drawOrder;
      friend class Window2;
   };
}