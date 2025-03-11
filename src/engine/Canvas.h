#pragma once
#include "Widget.h"

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
      }
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
      Handled __process_unhandled_input(const InputEvent& event) override;
      void __process_hover(const InputEventMouseMotion& event);
   private:
      template <WidgetStatus::StatusType Status>
      void setStatus(Widget* newWidget){
         constexpr std::size_t statusIndex = WidgetStatus::tuple_type_index_v<Status, WidgetStatus::StatusTypes>;
         //gauranteed safe by static checks so we can live dangerously
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
            if (oldWidget && statusChange){
               oldWidget->_on_modality_lost();
            }
            if (newWidget && statusChange){
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