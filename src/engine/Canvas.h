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


      Widget* tryHandle(InputEvent& event, TypeNode* node, const Transform2D& inputTransform);
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
      Widget* __process_unhandled_input(const InputEvent& event) override;
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
         const Transform2D& getGlobalTransform() const {return globalTransform;}
      private:
         std::stack<Transform2D*> globalTransformStack;
         Transform2D globalTransform;
      } transformStack;


      //Scoping helpers
      struct TreeProcess{
         TreeProcess(Canvas* thisCanvas, Widget* processedWidget)
         : thisCanvas(thisCanvas)
         , processedWidget(processedWidget)
         , subCanvas(dynamic_cast<Canvas*>(processedWidget))
         {
            thisCanvas->transformStack.pushTransform(&processedWidget->getTransform());
         }
         ~TreeProcess(){
            thisCanvas->transformStack.popTransform();
         }

         Canvas* thisCanvas;
         Widget* processedWidget;
         Canvas* subCanvas; //only valid if the processed widget is a canvas
      };


      ////////// RENDERING
      //return value = not meaningful
      struct RenderProcess : public TreeProcess{
         template <typename ...Args>
         RenderProcess(Canvas* thisCanvas, Widget* processedWidget, Args&&... args)
         : TreeProcess(thisCanvas, processedWidget)
         {
         }

         Widget* subcanvasProcess(){
            thisCanvas->getRenderTarget()->endRenderMode();
            subCanvas->renderProcess();
            thisCanvas->getRenderTarget()->beginRenderMode();
            return nullptr;
         }

         void subcanvasCleanup() const {
            auto _renderTarget = subCanvas->getRenderTarget();
            DrawTextureRec(_renderTarget->getRenderTexture(), {0, 0, (float) _renderTarget->getSize().x, -(float) subCanvas->getSize().y}, {0, 0}, WHITE);
         }

         Widget* process(){
            processedWidget->render2DBegin();
            processedWidget->render2D();
            processedWidget->render2DEnd();
            return nullptr; //arbitrary return value
         };
      };

      ////////// INPUT
      // return value = who handled
      struct InputProcess : public TreeProcess {
         InputProcess(Canvas* thisCanvas, Widget* processedWidget, const InputEvent& event, const Transform2D& inputTransform)
         : TreeProcess(thisCanvas, processedWidget)
         , event(event)
         , inputTransform(inputTransform)
         {
            if (auto mouseData = event.isMouse()) {
//               printMatrix(inputTransform.matrix);
               mouseTransformer = std::make_unique<MouseEvent::ScopeTransformer>(*mouseData.value(), inputTransform, processedWidget->getSize());
            }
         }

         Widget* subcanvasProcess(){
            return subCanvas->__process_unhandled_input(event);
         }

         void subcanvasCleanup() const {}

         Widget* process(){
            return processedWidget->_unhandled_input(event);
         };
         //transforms mouse coordinates
         std::unique_ptr<MouseEvent::ScopeTransformer> mouseTransformer;
         const Transform2D& inputTransform;
         const InputEvent& event;
      };

      ////////// HOVERING
      // return value = who hovered
      struct HoverProcess : public TreeProcess {
         HoverProcess(Canvas* thisCanvas, Widget* processedWidget)
         : TreeProcess(thisCanvas, processedWidget)
         {}

         Widget* subcanvasProcess(){
            return nullptr;
         }

         void subcanvasCleanup() const {

         }

      };


      template <typename ProcessType, typename... Args>
      Widget* processChildren(TypeNode *thisNode, Args&&... args) {
         //dispatch to children
         for (auto& child: thisNode->getChildren()) {
            if (auto childWidget = child->as<Widget>()){
               auto& widget = childWidget.value();
               if (widget->_modal) {
                //save off global transformation matrix so we can redraw this widget
                  // later in its proper position
                  // Note: This encodes the drawables local transform, which needs to be subtracted off later.
                  modalXform = widget->getGlobalTransform();
               } else {
                  auto handled = processTree<ProcessType>(child, false, std::forward<Args>(args)...);
                  if (handled) return handled;
               }
               // modal widgets do not propogate to children except explicitly
               continue;
            }
            auto handled = processTree<ProcessType>(child, false, std::forward<Args>(args)...);
            if (handled) return handled;
         }
         return nullptr;
      };


      template <typename ProcessType, typename... Args>
      Widget* processTree(TypeNode *thisNode, bool isModal, Args&&... args ){
         auto isWidget = thisNode->as<Widget>();
         //processes actual widget itself
         if (!isWidget) {
            //non-widget node with children - widgets should never reach this
            return processChildren<ProcessType>(thisNode, std::forward<Args>(args)...);
         }
         Widget* handled = nullptr;
         auto& widget = isWidget.value();
         //ignore invisible
         if (!widget->_visible) return nullptr;

         //template magic!
         // Create the appropriate arguments tuple based on ProcessType
         auto createProcessTransformer = [this, &widget, &args...](auto paramters) {
            if constexpr (std::is_same_v<ProcessType, InputProcess>) {
               const auto& event = std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...));
               const auto& inputTransform = std::get<1>(std::forward_as_tuple(std::forward<Args>(args)...));
               return ProcessType(this, widget, event, inputTransform);
            } else {
               // For other types like RenderProcess, don't pass the extra args
               return ProcessType(this, widget);
            }
         };

         // Call the lambda with a dummy parameter to trigger template deduction
         auto processTransformer = createProcessTransformer( isModal ? modalXform : widget->getLocalTransform());


//         ProcessType processTransformer(this, widget, event, widget->getLocalTransform()); //pushes local transform
         if (processTransformer.subCanvas && processTransformer.subCanvas != this){
               // for Subcanvases, revert global transform and reapply after processing
               rlPopMatrix();
               handled = processTransformer.subcanvasProcess();
               rlPushMatrix();
               rlMultMatrixf(MatrixToFloat(transformStack.getGlobalTransform().matrix));
               processTransformer.subcanvasCleanup();
               //does not dispatch to children as this is handled explicitly by canvas
         } else {
            //normal process (self-first) with child dispatch
            if (!widget->_modal || isModal) {
               handled = processTransformer.process();
               if (handled) {
                  return handled;
               }
            }
            handled = processChildren<ProcessType>(thisNode, std::forward<Args>(args)...);
         }
         // go no further as a widget
         // pop local transform (processTransformer falls out of scope)
         return handled;
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