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
         camera = {0};
         camera.zoom = 1.0f;
      }
      ~Canvas() override { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
      enum class IntrinsicRenderType {
         CanvasOverlay, // intrinsic children are drawn like any other - on top of the canvas, using the canvas' own transform (however are drawn before real children)
         CanvasUnderlay, // intrinsic children are drawn behind the canvas, using the canvas' own transform
         ViewportOverlay, // intrinsic children are drawn on top of the canvas, using the canvas' parent transform (pinned to viewport)
         ViewportUnderlay// intrinsic children are drawn behind the canvas, using the canvas' parent transform (pinned to viewport)
      };
      void _on_descendant_added_to_tree(TypeNode* child) override;
      void render2D() const override {};
      CanvasSpace<Pos<float>> getMousePos();
      inline const Transform2D getCameraTransform() const {return GetCameraMatrix2D(camera);}
      inline Transform2D getCameraTransform() {return GetCameraMatrix2D(camera);}
      void setCaptureOutsideInput(bool newValue){_rejectOutsideInput = newValue;}
   protected:
      virtual void renderProcess();
      [[nodiscard]] const RenderTarget& getRenderTarget() const {return _renderTarget;}
      Widget* __process_unhandled_input(const InputEvent& event) override;
      Widget* __process_hover(const InputEventMouseHover& event);
      void _on_rect_changed() override;
      virtual IntrinsicRenderType getIntrinsicRenderType(){return _intrinsicRenderType;}
      Camera2D camera;
      std::vector<std::unique_ptr<TypeNode>> _intrinsicChildren; //children that belong to the canvas but are treated differently for various reasons
      IntrinsicRenderType _intrinsicRenderType = IntrinsicRenderType::CanvasOverlay;
      bool _rejectOutsideInput = false; //rejects input that is outside the canvas
      bool _retained = false; //set to true if you want to retain the image between draw calls. Requires manually clearing the render target.
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
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
      RenderTarget _renderTarget;

      /////////////////////////////////////////////////////////////////////////////////////////
      struct TransformStack{
         void pushTransform(Transform2D* transform2D);
         void popTransform();
         const Transform2D& getGlobalTransform() const {return globalTransform;}
      private:
         std::stack<Transform2D*> globalTransformStack;
         Transform2D globalTransform;
      } transformStack;

      /////////////////////////////////////////////////////////////////////////////////////////
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

      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      ////////// RENDERING
      //return value = not meaningful
      struct RenderProcess : public TreeProcess{
         RenderProcess(Canvas* thisCanvas, Widget* processedWidget)
         : TreeProcess(thisCanvas, processedWidget)
         {
         }

         Widget* subcanvasProcess(){
            //render intrinsic viewport underlay
            if (subCanvas->getIntrinsicRenderType() == IntrinsicRenderType::ViewportUnderlay){
               for (auto& intrinsicChild : subCanvas->_intrinsicChildren){
                  subCanvas->processNode<RenderProcess>(intrinsicChild.get(), false);
               }
            }
            rlPopMatrix();
            thisCanvas->_renderTarget.endRenderMode();
            subCanvas->renderProcess();
            thisCanvas->_renderTarget.beginRenderMode();
            rlPushMatrix();
            rlMultMatrixf(MatrixToFloat(thisCanvas->transformStack.getGlobalTransform().matrix));
            drawRenderTargetRect(subCanvas->getRenderTarget(), subCanvas->getSizeRect(), subCanvas->getRect(), Colors::none);
            //render intrinsic viewport overlay
            if (subCanvas->getIntrinsicRenderType() == IntrinsicRenderType::ViewportOverlay){
               for (auto& intrinsicChild : subCanvas->_intrinsicChildren){
                  subCanvas->processNode<RenderProcess>(intrinsicChild.get(), false);
               }
            }
            return nullptr;
         }

         Widget* process(){
            processedWidget->render2DBegin();
            processedWidget->render2D();
            processedWidget->render2DEnd();
            return nullptr; //arbitrary return value
         };
      };

      /////////////////////////////////////////////////////////////////////////////////////////
      ////////// INPUT
      // return value = who handled
      struct InputProcess : public TreeProcess {
         InputProcess(Canvas* thisCanvas, Widget* processedWidget, const InputEvent& event, const Transform2D& inputTransform)
         : TreeProcess(thisCanvas, processedWidget)
         , event(event)
         , inputTransform(inputTransform)
         {
            if (auto mouseData = event.isMouse()) {
               mouseTransformer = std::make_unique<MouseEvent::ScopeTransformer>(*mouseData.value(), inputTransform, processedWidget->getSize());
            }
         }

         Widget* subcanvasProcess(){
            return subCanvas->__process_unhandled_input(event);
         }

         Widget* process(){
            return processedWidget->_unhandled_input(event);
         };
         //transforms mouse coordinates
         std::unique_ptr<MouseEvent::ScopeTransformer> mouseTransformer;
         Transform2D inputTransform;
         const InputEvent& event;
      };

      /////////////////////////////////////////////////////////////////////////////////////////
      ////////// HOVERING
      // return value = who hovered
      struct HoverProcess : public InputProcess {
         HoverProcess(Canvas* thisCanvas, Widget* processedWidget, const InputEvent& event, const Transform2D& inputTransform)
         : InputProcess(thisCanvas, processedWidget, event, inputTransform)
         {}

         Widget* subcanvasProcess(){
            return subCanvas->__process_hover(event.toEvent<InputEventMouseHover>());
         }

         Widget* process(){
            if (processedWidget->acceptsHover && event.isMouse().value()->isInside()){
               return processedWidget;
            }
            return nullptr;
         };
      };

      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      template <typename ProcessType, typename... Args>
      Widget* processChildren(TypeNode *thisNode, Args&&... args) {
         //dispatch to children
         for (auto& child: thisNode->getChildren()) {
            if (auto childWidget = child->as<Widget>()){
               auto& _childWidget = childWidget.value();
               if (_childWidget->_modal) {
                  //we will come back to this later
                  continue;
               } else {
                  auto handled = processNode<ProcessType>(child, false, std::forward<Args>(args)...);
                  if (handled) return handled;
               }
            }
            auto handled = processNode<ProcessType>(child, false, std::forward<Args>(args)...);
            if (handled) return handled;
         }
         return nullptr;
      };

      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      template <typename ProcessType, typename... Args>
      Widget* processNode(TypeNode *thisNode, bool isGlobal, Args&&... args )  {
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
         // Create the appropriate arguments based on ProcessType
         // Basically what we're doing is taking arbitrary args to the processNode function and
         //  seeing if we can match them to the ctor for the given ProcessType. Each ctor is conditionally compiled
         //  so that only one actaually exists at any given time. This keeps us from having to write
         //  two large and similar blocks of code that both iterate over the tree and 'doStuff' in slightly
         //  different ways.
         auto createProcessTransformer = [this, &widget, &args...](const Transform2D& inputTransform) {
            if constexpr (std::is_same_v<ProcessType, InputProcess> || std::is_same_v<ProcessType, HoverProcess>) {
               constexpr size_t argCount = sizeof...(args);
               const auto& event = std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...));
               return ProcessType(this, widget, event, inputTransform);
            } else {
               // For other types like RenderProcess, don't pass the extra args
               return ProcessType(this, widget);
            }
         };

         // Call the lambda to trigger template deduction
         auto processTransformer = createProcessTransformer(isGlobal ? widget->getGlobalTransform().get() : widget->getLocalTransform());

         if (processTransformer.subCanvas && processTransformer.subCanvas != this){
               handled = processTransformer.subcanvasProcess();
               //does not dispatch to children as this is handled explicitly by canvas
         } else {
            //render process (self-first) with child dispatch
            if constexpr (std::is_same_v<ProcessType, RenderProcess>) {
               if (!widget->_modal || isGlobal) {
                  handled = processTransformer.process();
                  if (handled) return handled;
               }
               return processChildren<ProcessType>(thisNode, std::forward<Args>(args)...);
            }

            if constexpr (std::is_same_v<ProcessType, InputProcess> || std::is_same_v<ProcessType, HoverProcess>) {
               //process children first for input
               handled = processChildren<ProcessType>(thisNode, std::forward<Args>(args)...);
               if (handled) return handled;
               if (!widget->_modal || isGlobal) {
                  handled = processTransformer.process();
                  if (handled) return handled;
               }
            }
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