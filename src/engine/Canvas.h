#pragma once
#include <stack>
#include <ranges>
#include "Widget.h"
#include "CacheVectorMap.h"

namespace ReyEngine {
   namespace WidgetStatus{
      //give special status to widgets - define new status here and add to tuple
      struct Modal{static constexpr char NAME[] = "Modal";}; // Focused widgets get first dibs on input events, *and (by default) consume those events that are not handled explicitly*
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
   protected:
      //Coordinates that are in background space as opposed to foreground space. Filtered through camera transform.
      using BackgroundSpace = NamedType<Pos<float>, Internal::BackgroundSpaceParam>;
   public:
      enum class CanvasLayer {FOREGROUND, BACKGROUND};
      REYENGINE_OBJECT(Canvas)
      Canvas()
      : camera({0}){
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
      void render2D() const override {}
      void renderProcess(RenderContext&);
      void __on_rect_changed(const Rect<R_FLOAT>& oldRect, const Rect<R_FLOAT>& newRect, bool allowsAnchor, bool byLayout = false) override;
      void _removeAllStatus(Widget*);

      RenderTarget _renderTarget;
      Camera2D camera;
      // std::unique_ptr<InputContext> _inputContext;
      bool _retained = false; //set to true if you want to retain the image between draw calls. Requires manually clearing the render target.

      std::map<size_t, std::vector<Widget*>> _processLayers; //different layers of widgets that can be processed differently
      OrderedCache<TypeNode*> _foreground;
      OrderedCache<TypeNode*> _background;
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      template <WidgetStatus::StatusType Status>
      void setStatus(Widget* newWidget){
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
            //only drawables can be modal, so we can set some extra statuses to help us outd
            if (oldWidget){
               oldWidget->_modal = false;
               oldWidget->_on_modality_lost();
            }
            if (newWidget){
               newWidget->_modal = true;
               newWidget->_on_modality_gained();
            }
         }
         //Note: focus should NOT propagate upwards - each canvas manages its own focus
         //Propagating would cause widgets to be focused on canvases they don't belong to
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

      static void doRender(RenderContext&, Widget*, bool isModal=false);
      static void doRenderModal(RenderContext&, Widget*);
   private:
      void __on_child_added_to_tree(TypeNode* child) override;
      void __on_child_removed_from_tree(TypeNode* child) override;
   public:
      void setHover(Widget* w){   setStatus<WidgetStatus::Hover>(w);}
      void setFocus(Widget* w){   setStatus<WidgetStatus::Focus>(w);}
      void setModal(Widget* w){   setStatus<WidgetStatus::Modal>(w);}
      void setToolTip(Widget* w){ setStatus<WidgetStatus::ToolTip>(w);}
      Widget* getHover(){return   getStatus<WidgetStatus::Hover>();}
      Widget* getFocus(){return   getStatus<WidgetStatus::Focus>();}
      Widget* getModal(){return   getStatus<WidgetStatus::Modal>();}
      Widget* getToolTip(){return getStatus<WidgetStatus::ToolTip>();}

      friend class Widget;
      friend class Window;
   };
}