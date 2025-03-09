#pragma once
#include "Widget.h"

namespace ReyEngine {
   class Canvas: public Widget {
   public:
      REYENGINE_OBJECT(Canvas)
      Canvas(){}
      virtual ~Canvas() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
      void _on_descendant_added_to_tree(TypeNode* child) override;

      ///walk the tree and pin any drawables to us
      void cacheTree(size_t drawOrderSize, size_t inputOrderSize);
      void renderProcess();
      void render2D() const override;
      void render2DBegin() override;
      void render2DEnd() override;
      void tryRender(TypeNode* node);
      Handled tryHandle(InputEvent& event, TypeNode* node);
      Widget* tryHover(InputEventMouseMotion& event, TypeNode* node) const;
      CanvasSpace<Pos<float>> getMousePos();
      void updateGlobalTransforms();
   protected:
      Widget* hovered = nullptr;
      //naive implementation for now. I assume there's a smarter way to do this
      // than backtracking all the way up a drawable's heirarchy and pushing the transformation matrices
      template <typename T>
      struct OrderableData {
         OrderableData(T* data, TypeNode* node, size_t index, size_t parentIndex)
         : data(data)
         , node(node)
         {}
         T* data;
         TypeNode* node;
         size_t index;
         size_t parentIndex;
      };
      RenderTarget& getRenderTarget(){return renderTarget;}
//      std::vector<OrderableData<Drawable2D>> drawOrder;
//      std::vector<OrderableData<Widget>> inputOrder;
      Handled __process_unhandled_input(const InputEvent& event) override;
      void __process_hover(const InputEventMouseMotion& event);
   private:
      RenderTarget renderTarget;
      friend class Widget;
      friend class Window;
   };
}