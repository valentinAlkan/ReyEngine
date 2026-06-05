#pragma once
#include <stack>
#include <ranges>
#include "Widget.h"
#include "CacheVectorMap.h"

namespace ReyEngine {
   namespace WidgetStatus{
      //give special status to widgets - define new status here and add to tuple
      struct Modal{static constexpr char NAME[] = "Modal";}; // Modal widgets get first dibs on input events, *and (by default) consume those events that are not handled explicitly* AND are drawn last by the canvas
      struct Focus{static constexpr char NAME[] = "Focus";}; // Focused widgets get first dibs on non-modal input events
      struct Hover{static constexpr char NAME[] = "Hover";};
      struct ToolTip{static constexpr char NAME[] = "ToolTip";}; //The currently displayed tool-tip
      using StatusTypes = std::tuple<Modal, Focus, Hover, ToolTip>;
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

   namespace Internal {
      struct BackgroundSpaceParam{};
   }

   class Canvas: public Widget {
      template <WidgetStatus::StatusType Status>
      void _setStatus(Widget* newWidget){
         constexpr std::size_t statusIndex = WidgetStatus::tuple_type_index_v<Status, WidgetStatus::StatusTypes>;
         //gauranteed safe against overflow by static index check, so we can live dangerously by slicing arrays
         auto oldWidget = statusWidgetStorage[statusIndex];
         statusWidgetStorage[statusIndex] = newWidget;
         bool statusChange = newWidget != oldWidget;
         if (!statusChange) return;
         if constexpr (std::is_same_v<Status, WidgetStatus::Hover>){
            if (oldWidget) oldWidget->_on_mouse_exit();
            if (newWidget) newWidget->_on_mouse_enter();
         }
         if constexpr (std::is_same_v<Status, WidgetStatus::Focus>){
            if (oldWidget) oldWidget->_on_focus_lost();
            if (newWidget) newWidget->_on_focus_gained();
         }
         if constexpr (std::is_same_v<Status, WidgetStatus::Modal>){
            //only drawables can be modal, so we can set some extra statuses to help us out
            if (oldWidget){
               oldWidget->_modal = false;
               oldWidget->_on_modality_lost();
            }
            if (newWidget){
               newWidget->_modal = true;
               newWidget->_on_modality_gained();
            }
         }
      }
   protected:
      //Coordinates that are in background space as opposed to foreground space. Filtered through camera transform.
      using BackgroundSpace = NamedType<Pos<float>, Internal::BackgroundSpaceParam>;
   public:
      enum class CanvasLayer {FOREGROUND, BACKGROUND};
      REYENGINE_OBJECT(Canvas)
      Canvas()
      : camera({{0}}){
         isGlobalTransformBoundary = true;
         _isCanvas = true;
         camera.zoom = 1.0f;
         camera.target = (Vector2)(size / 2);
         _inputFilter = InputFilter::PASS_PROCESS_PUBLISH; //canvas don't actually respect input filter, but they pass, process, and publish in that order
      }
      ~Canvas() override = default;
      void __init() override {
         Canvas::_init();
         _init();
      };
      void _init() override;
      CanvasSpace<Pos<float>> getMousePos();
      inline Transform2D getCameraTransform() const {return GetCameraMatrix2D(camera);}
      inline Transform2D getCameraTransform() {return GetCameraMatrix2D(camera);}
      Camera2D& getCamera(){return camera;}
      const Camera2D& getCamera() const {return camera;}
      void moveToForeground(Widget*);
      void moveToBackground(Widget*);
      BackgroundSpace toBackgroundPos(const Pos<float>& p) const;
      Pos<float> toForegroundPos(const BackgroundSpace& p) const;
      //converts a point in local canvas space to a local pos in the child, and applies camera transform if neccessary.
      //If it is not a child of this canvas, returns nullopt.
      std::optional<Pos<float>> toChildPos(const Widget* child, const Pos<float>& p) const;
      std::optional<Transform2D> getChildXform(const Widget* child) const;
      bool isInBackground(const Widget* w) const {return _background.contains(w->_node);}
      bool isInForeground(const Widget* w) const {return _foreground.contains(w->_node);}
   protected:
      const RenderTarget& readRenderTarget() const {return _renderTarget;}
      RenderContext createRenderContext(){return _renderTarget;}
      Handled _processInput(const InputEvent& e) override;
      void __on_descendant_added_to_tree(TypeNode* child) override;
      void __on_descendant_removed_from_tree(TypeNode* child) override;
      void render2D(RenderContext&) const override {}
      void renderProcess(RenderContext&);
      void __on_rect_changed(const Rect<R_FLOAT>& oldRect, const Rect<R_FLOAT>& newRect, bool allowsAnchor, bool byLayout = false) override;
      template <WidgetStatus::StatusType Status>
      void _removeStatus(Widget* widget){
         //removes all instances of the widget from the status stack.
         auto pruneStack = [&](auto& statusVector) {
            for (auto it = statusVector.begin(); it != statusVector.end(); /**/) {
               auto _widget = *it;
               if (_widget == widget) {
                  it = statusVector.erase(it);
               } else {
                  ++it;
               }
            }
         };

         if constexpr (std::is_same_v<Status, WidgetStatus::Focus>){
            pruneStack(_focusStack);
            if (!_focusStack.empty()){
               _setStatus<WidgetStatus::Focus>(_focusStack.back());
               _focusStack.pop_back();
            }
         }
         if constexpr (std::is_same_v<Status, WidgetStatus::Modal>){
            pruneStack(_modalStack);
            if (!_modalStack.empty()){
               _setStatus<WidgetStatus::Focus>(_modalStack.back());
               _modalStack.pop_back();
            }
         } else {
            constexpr std::size_t statusIndex = WidgetStatus::tuple_type_index_v<Status, WidgetStatus::StatusTypes>;
            if (statusWidgetStorage[statusIndex] == widget) {
               _setStatus<Status>(nullptr);
            }
         }
      }
      void _removeAllStatus(Widget* widget){
         std::apply([this, widget](auto... dummyStatuses) {(this->_removeStatus<decltype(dummyStatuses)>(widget), ...);}, WidgetStatus::StatusTypes{});
      }

      RenderTarget _renderTarget;
      Camera2D camera;
      // std::unique_ptr<InputContext> _inputContext;
      bool _retained = false; //set to true if you want to retain the image between draw calls. Requires manually clearing the render target.
      std::vector<Widget*> _modalStack;
      std::vector<Widget*> _focusStack;
      OrderedCache<TypeNode*> _foreground;
      OrderedCache<TypeNode*> _background;
      std::array<Widget*, std::tuple_size_v<WidgetStatus::StatusTypes>> statusWidgetStorage = {0};

      static void doRender(RenderContext&, Widget*, bool isModal=false);
      static void doRenderModal(RenderContext&, Widget*);
   private:
      void __on_child_added_to_tree(TypeNode* child) override;
      void __on_child_removed_from_tree(TypeNode* child) override;
      template <WidgetStatus::StatusType Status>
      void popStatus(Widget* popWidget) {
         static_assert(std::is_same_v<Status, WidgetStatus::Focus> || std::is_same_v<Status, WidgetStatus::Modal>);
         constexpr bool isFocus = std::is_same_v<Status, WidgetStatus::Focus>;
         auto& stack = isFocus ? _focusStack : _modalStack;
         if (auto current = getStatus<Status>() ; current == popWidget){
            if (!stack.empty()) {
               if (auto top = stack.back()) {
                  _setStatus<Status>(stack.back());
                  stack.pop_back();
               }
            } else {
               //stack is empty so the current widget lives in widget status
               _setStatus<Status>(nullptr);
            }
         }

         //ensure invisible widgets can't have status
         while (auto widget = getStatus<Status>()){
            if (!widget->getIsRendering()){
               _removeAllStatus(widget);
            } else {
               break;
            }
         }
      }
      template <WidgetStatus::StatusType Status>
      void pushStatus(Widget* newWidget) {
         static_assert(std::is_same_v<Status, WidgetStatus::Focus> || std::is_same_v<Status, WidgetStatus::Modal>);
         constexpr bool isFocus = std::is_same_v<Status, WidgetStatus::Focus>;
         auto& stack = isFocus ? _focusStack : _modalStack;
         if (getStatus<Status>() == newWidget) return; //don't double apply status
         if (!newWidget) {
            stack.push_back(getStatus<Status>());
         }
         _setStatus<Status>(newWidget);
      }
      template <WidgetStatus::StatusType Status>
      requires (!std::is_same_v<Status, WidgetStatus::Modal> && !std::is_same_v<Status, WidgetStatus::Focus>)
      void setStatus(Widget* newWidget) {
         _setStatus<Status>(newWidget);
      }
      template <WidgetStatus::StatusType Status>
      [[nodiscard]] Widget* getStatus(){
         constexpr std::size_t statusIndex = WidgetStatus::tuple_type_index_v<Status, WidgetStatus::StatusTypes>;
         //gauranteed safe so no safety checks required
         return statusWidgetStorage[statusIndex];
      }
      template <WidgetStatus::StatusType Status>
      [[nodiscard]] const Widget* getStatus() const {return const_cast<Canvas*>(this)->getStatus<Status>();}
   public:
      void setHover(Widget* w) { setStatus<WidgetStatus::Hover>(w);}
      void pushFocus(Widget* w){ pushStatus<WidgetStatus::Focus>(w);}
      void pushModal(Widget* w){ pushStatus<WidgetStatus::Modal>(w);}
      void popFocus(Widget* w) { popStatus<WidgetStatus::Focus>(w);}
      void revokeFocus(Widget* w) { _removeStatus<WidgetStatus::Focus>(w);}
      void popModal(Widget* w) { popStatus<WidgetStatus::Modal>(w);}
      void revokeModal(Widget* w) { _removeStatus<WidgetStatus::Modal>(w);}
      void setToolTip(Widget* w){ setStatus<WidgetStatus::ToolTip>(w);}
      Widget* getHover() {return   getStatus<WidgetStatus::Hover>();}
      Widget* getFocus() {return getStatus<WidgetStatus::Focus>();}
      Widget* getModal() {return   getStatus<WidgetStatus::Modal>();}
      Widget* getToolTip() {return getStatus<WidgetStatus::ToolTip>();}
      const Widget* getHover() const {return   getStatus<WidgetStatus::Hover>();}
      const Widget* getFocus() const {return   getStatus<WidgetStatus::Focus>();}
      const Widget* getModal() const {return   getStatus<WidgetStatus::Modal>();}
      const Widget* getToolTip() const {return getStatus<WidgetStatus::ToolTip>();}
      friend class Window;
   };
}