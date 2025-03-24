#pragma once
#include "Widget.h"
#include <stack>
#include "rlgl.h"

namespace ReyEngine {
   namespace WidgetStatus{
      //give special status to widgets - define new status here and add to tuple
      struct Modal{};
      struct Focus{};
      struct Hover{};
      using StatusTypes = std::tuple<Modal, Focus, Hover>;
      // Type trait to check if a type is in the tuple
      template <typename T, typename Tuple> struct is_in_tuple;
      template <typename T, typename... Ts> struct is_in_tuple<T, std::tuple<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {};
      template <typename T, typename Tuple> inline static constexpr bool is_in_tuple_v = is_in_tuple<T, Tuple>::value;
      template <typename T, typename Tuple, std::size_t Index = 0>  struct tuple_type_index;

      // Base case: type found
      template <typename T, typename... Rest, std::size_t Index>
      struct tuple_type_index<T, std::tuple<T, Rest...>, Index> : std::integral_constant<std::size_t, Index> {};

      // Recursive case: keep searching
      template <typename T, typename First, typename... Rest, std::size_t Index>
      struct tuple_type_index<T, std::tuple<First, Rest...>, Index> : tuple_type_index<T, std::tuple<Rest...>, Index + 1> {};

      template <typename T, typename Tuple> inline static constexpr std::size_t tuple_type_index_v = tuple_type_index<T, Tuple>::value;

      template <typename T>
      concept StatusType = is_in_tuple_v<T, StatusTypes>;
   }
   class Canvas: public Widget {
   public:
      REYENGINE_OBJECT(Canvas)
      Canvas(){
         isGlobalTransformBoundary = true;
         _isCanvas = true;
      }
      ~Canvas() override { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
      void _on_descendant_added_to_tree(TypeNode* child) override;

      ///walk the tree and pin any drawables to us
      void cacheTree(size_t drawOrderSize, size_t inputOrderSize);
      void renderProcess();
      void render2D() const override;
      void render2DBegin() override;
      void render2DEnd() override;


      Handled tryHandle(InputEvent& event, TypeNode* node, const Transform2D& inputTransform);
      Widget* tryHover(InputEventMouseMotion& event, TypeNode* node, const Transform2D& inputTransform) const;
      CanvasSpace<Pos<float>> getMousePos();
      void updateGlobalTransforms();
   protected:
      //naive implementation for now. I assume there's a smarter way to do this
      // than backtracking all the way up a drawable's heirarchy and pushing the transformation matrices
//      template <typename T>
//      struct OrderableData {
//         OrderableData(T* data, TypeNode* node, size_t index, size_t parentIndex)
//         : data(data)
//         , node(node)
//         {}
//         T* data;
//         TypeNode* node;
//         size_t index;
//         size_t parentIndex;
//      };
      RenderTarget* getRenderTarget() override {return &renderTarget;}
      Handled __process_unhandled_input(const InputEvent& event) override;
      void __process_hover(const InputEventMouseMotion& event);
   private:
      void _on_rect_changed() override;
      template <WidgetStatus::StatusType Status>
      void setStatus(Widget* newWidget){
         constexpr std::size_t statusIndex = WidgetStatus::tuple_type_index_v<Status, WidgetStatus::StatusTypes>;
         //gauranteed safe against overflow by static index check so we can live dangerously by slicing arrays
         auto oldWidget = statusWidgetStorage[statusIndex];
         statusWidgetStorage[statusIndex] = newWidget;
         bool statusChange = newWidget != oldWidget;
         if constexpr (std::is_same_v<Status, WidgetStatus::Hover>){
            if (oldWidget && statusChange){
               oldWidget->_on_mouse_exit();
            }
            if (newWidget && statusChange){
               newWidget->_on_mouse_enter();
            }
         }
         if constexpr (std::is_same_v<Status, WidgetStatus::Focus>){
            if (oldWidget && statusChange){
               oldWidget->_on_focus_lost();
            }
            if (newWidget && statusChange){
               newWidget->_on_focus_gained();
            }
         }
         if constexpr (std::is_same_v<Status, WidgetStatus::Modal>){
            //only drawables can be modal so we can set some extra statuses to help us out
            if (oldWidget && statusChange){
               oldWidget->_modal = false;
               oldWidget->_on_modality_lost();
            }
            if (newWidget && statusChange){
               newWidget->_modal = true;
               newWidget->_on_modality_gained();
            }
         }
      }
      template <WidgetStatus::StatusType Status>
      Widget* getStatus(){
         constexpr std::size_t statusIndex = WidgetStatus::tuple_type_index_v<Status, WidgetStatus::StatusTypes>;
         //gauranteed safe so no safety checks required
         return statusWidgetStorage[statusIndex];
      }
      template <WidgetStatus::StatusType Status>
      [[nodiscard]] const Widget* getStatus() const {return const_cast<Canvas*>(this)->getStatus<Status>();}
      std::array<Widget*, std::tuple_size_v<WidgetStatus::StatusTypes>> statusWidgetStorage = {0};
      RenderTarget renderTarget;
      Transform2D modalXform;

      //////////////////////////
      struct TransformStack{
         void pushTransform(Transform2D* transform2D);
         void popTransform();
         const Transform2D& getGloablTransform() const {return globalTransform;}
      private:
         std::stack<Transform2D*> globalTransformStack;
         Transform2D globalTransform;
      } transformStack;

      //////////////////////////
      //Scoped Helper
      struct Transformer {
         inline Transformer(Canvas& canvas, Widget* widget)
         : canvas(canvas)
         {
            canvas.transformStack.pushTransform(&widget->getTransform());
         }
         inline ~Transformer(){
            canvas.transformStack.popTransform();
         }
      private:
         Canvas& canvas;
      };



      struct TreeProcess{
         TreeProcess(Canvas* thisCanvas, Canvas* subCanvas)
         : thisCanvas(thisCanvas)
         , subCanvas(subCanvas)
         {}
      protected:
         Canvas* thisCanvas;
         Canvas* subCanvas;
      };


      //return value = not meaningful
      struct RenderProcess : public TreeProcess{
         RenderProcess(Canvas* thisCanvas, Canvas* subCanvas) : TreeProcess(thisCanvas, subCanvas){
            thisCanvas->getRenderTarget()->endRenderMode();
            subCanvas->renderProcess();
            thisCanvas->getRenderTarget()->beginRenderMode();
         }

         ~RenderProcess(){
            auto _renderTarget = subCanvas->getRenderTarget();
            DrawTextureRec(_renderTarget->getRenderTexture(), {0, 0, (float) _renderTarget->getSize().x, -(float) subCanvas->getSize().y}, {0, 0}, WHITE);
         }
      };
      // return value = who handled
      struct InputProcess : public TreeProcess {
         InputProcess(Canvas* thisCanvas, Canvas* subCanvas) : TreeProcess(thisCanvas, subCanvas){}
         ~InputProcess(){}
      };
      // return value = who hovered
      struct HoverProcess : public TreeProcess {
         HoverProcess(Canvas* thisCanvas, Canvas* subCanvas) : TreeProcess(thisCanvas, subCanvas){}
         ~HoverProcess(){}
      };


      template <typename ProcessType>
      void processChildren(TypeNode *thisNode, bool drawModal){
         //dispatch to children
         for (auto& child: thisNode->getChildren()) {
            if (auto childDrawable = child->as<Widget>()){
               auto& drawable = childDrawable.value();
               if (drawable->_modal) {
                //save off global transformation matrix so we can redraw this widget
                  // later in its proper position
                  // Note: This encodes the drawables local transform, which needs to be subtracted off later.
                  modalXform = drawable->getGlobalTransform();
               } else {
                  processTree<RenderProcess>(child, false);
               }
            }
         }
      };


      template <typename ProcessType>
      Widget* processTree(TypeNode *thisNode, bool isModal){
         auto isWidget = thisNode->as<Widget>();

         //draws actual drawable itself
         if (isWidget){
            auto& widget = isWidget.value();
            //ignore invisible
            if (!isWidget.value()->_visible) return nullptr;
            Transformer transformer(*this, widget);
            if (widget->_isCanvas){
               auto canvas = thisNode->as<Canvas>().value();
               if (canvas != this) {
                  // for Subcanvases, revert global transform and reapply after processing
                  rlPopMatrix();
                  /*<------ process ctor*/ ProcessType process(this, canvas);
                  rlPushMatrix();
                  rlMultMatrixf(MatrixToFloat(transformStack.getGloablTransform().matrix));
                  //<------ process dtor
                  //does not dispatch draw to children as this is done by renderProcess
               }
            } else {
               //normal render with child dispatch
               if (!widget->_modal || isModal) {
                  widget->render2DBegin();
                  widget->render2D();
                  widget->render2DEnd();
               }
               processChildren<ProcessType>(thisNode, isModal);
            }
            // go no further
            return nullptr;
         }

         //non-widget node with children - widgets should never reach this
         processChildren<ProcessType>(thisNode, isModal);
         return nullptr;
      }










   public:
      void setHover(Widget* w){setStatus<WidgetStatus::Hover>(w);}
      void setFocus(Widget* w){setStatus<WidgetStatus::Focus>(w);}
      void setModal(Widget* w){setStatus<WidgetStatus::Modal>(w);}
      Widget* getHover(){return getStatus<WidgetStatus::Hover>();}
      Widget* getFocus(){return getStatus<WidgetStatus::Focus>();}
      Widget* getModal(){return getStatus<WidgetStatus::Modal>();}

      friend class Widget;
      friend class Window;
   };
}