#pragma once
#include "raylib.h"
#include "DrawInterface.h"
#include "Property.h"
#include "TypeManager.h"
#include "InputManager.h"
#include "EventManager.h"
#include "Theme.h"
#include "Property.h"
#include <iostream>
#include <stack>
#include <utility>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <optional>
#include "Node.h"

using Handled = bool;

//#define CTOR_RECT const ReyEngine::Rect<float>& r
/////////////////////////////////////////////////////////////////////////////////////////
//#define REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(TYPENAME) \
//static constexpr char TYPE_NAME[] = #TYPENAME;               \
//std::string _get_static_constexpr_typename() override {return TYPE_NAME;}
/////////////////////////////////////////////////////////////////////////////////////////
//#define REYENGINE_NODE_SERIALIZER(CLASSNAME, PARENT_CLASSNAME) \
//   public:                                           \
//   static std::shared_ptr<BaseWidget> deserialize(const std::string& instanceName, PropertyPrototypeMap& properties) { \
//   CTOR_RECT = {0,0,0,0}; \
//   auto retval = std::make_shared<CLASSNAME>(instanceName, r); \
//   retval->BaseWidget::_deserialize(properties);        \
//   return retval;}                                       \
///////////////////////////////////////////////////////////////////////////////////////////
//#define REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME) \
//   CLASSNAME(const std::string& name, const std::string& typeName, CTOR_RECT): PARENT_CLASSNAME(name, typeName, r)
///////////////////////////////////////////////////////////////////////////////////////////
//#define REYENGINE_DEFAULT_CTOR(CLASSNAME) \
//   CLASSNAME(const std::string& name, CTOR_RECT): CLASSNAME(name, _get_static_constexpr_typename(), r){}
///////////////////////////////////////////////////////////////////////////////////////////
//#define REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME) \
//protected:                                                \
//   void _register_parent_properties() override{           \
//      PARENT_CLASSNAME::_register_parent_properties();    \
//      PARENT_CLASSNAME::registerProperties();             \
//   }

#define REYENGINE_NODE(CLASSNAME, PARENT_CLASSNAME)  \
public:                                                   \
   REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(CLASSNAME)  \
   REYENGINE_NODE_SERIALIZER(CLASSNAME, PARENT_CLASSNAME)       \
   REYENGINE_DEFAULT_CTOR(CLASSNAME)                       \
   REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME)  \
   REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME)


class Scene;
class Draggable;
class  BaseWidget
: public Node
{
   using ChildIndex = unsigned long;
   using WidgetPtr = std::shared_ptr<BaseWidget>;
   using ChildMap = std::map<std::string, std::pair<ChildIndex, WidgetPtr>>;
   using ChildOrder = std::vector<std::shared_ptr<BaseWidget>>;
   using fVec = ReyEngine::Vec2<float>;
   using iVec = ReyEngine::Vec2<int>;
   using dVec = ReyEngine::Vec2<double>;
public:

   struct WidgetResizeEvent : public Event<WidgetResizeEvent> {
      EVENT_CTOR_SIMPLE(WidgetResizeEvent, Event<WidgetResizeEvent>){
         size = std::static_pointer_cast<BaseWidget>(publisher)->getSize();
      }
      ReyEngine::Size<float> size;
   };

   enum class Anchor{NONE, LEFT, RIGHT, TOP, BOTTOM, FILL, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT};
   /////////////////////////////////////////////////////////////////////////////////////////
   struct AnchorProperty : public EnumProperty<Anchor, 10>{
      AnchorProperty(const std::string& instanceName,  Anchor defaultvalue)
      : EnumProperty<Anchor, 10>(instanceName, defaultvalue)
      {}
      const EnumPair<Anchor, 10>& getDict() const  override {return dict;}
      static constexpr EnumPair<Anchor, 10> dict = {
            ENUM_PAIR_DECLARE(Anchor, NONE),
            ENUM_PAIR_DECLARE(Anchor, LEFT),
            ENUM_PAIR_DECLARE(Anchor, RIGHT),
            ENUM_PAIR_DECLARE(Anchor, TOP),
            ENUM_PAIR_DECLARE(Anchor, BOTTOM),
            ENUM_PAIR_DECLARE(Anchor, BOTTOM_LEFT),
            ENUM_PAIR_DECLARE(Anchor, BOTTOM_RIGHT),
            ENUM_PAIR_DECLARE(Anchor, TOP_LEFT),
            ENUM_PAIR_DECLARE(Anchor, TOP_RIGHT),
            ENUM_PAIR_DECLARE(Anchor, FILL)
      };
   };

   static constexpr char TYPE_NAME[] = "BaseWidget";
   BaseWidget(const std::string& name, std::string  typeName, ReyEngine::Rect<float> rect);
   ~BaseWidget() override;
   //rect stuff
   ReyEngine::Rect<int> getRect() const {return _rect.value;}
   ReyEngine::Rect<int> getGlobalRect() const {auto globalPos = getGlobalPos(); return {globalPos.x, globalPos.y, getSize().x, getSize().y};}
   ReyEngine::Pos<int> getGlobalPos() const;
   ReyEngine::Size<int> getChildBoundingBox() const; //get the smallest rectangle that contains all children, starting from 0,0. Does not include grandchildren.
   ReyEngine::Pos<int> getPos() const {return {getRect().x, getRect().y};}
   ReyEngine::Size<int> getSize() const {return getRect().size();}
   int getWidth() const {return _rect.value.width;}
   int getHeight() const {return _rect.value.height;}
   iVec getHeightRange() const {return {0, getRect().size().y};}
   iVec getWidthtRange() const {return {0, getRect().size().x};}
   ReyEngine::Size<int> getMinSize(){return minSize;}
   ReyEngine::Size<int> getMaxSize(){return maxSize;}
   ReyEngine::Size<int> getClampedSize();
   ReyEngine::Size<int> getClampedSize(ReyEngine::Size<int>);
   void setVisible(bool visible){_visible = visible;}
   bool getVisible() const {return _visible;}
   //sizing
   void setAnchoring(Anchor newAnchor){_anchor.set(newAnchor);setRect(getRect());}
   Anchor getAnchoring(){return _anchor.value;}
   void setMaxSize(const ReyEngine::Size<int>& size){maxSize = size;}
   void setMinSize(const ReyEngine::Size<int>& size){minSize = size;}
   void setRect(const ReyEngine::Rect<int>& r);
   void setPos(int x, int y);
   void setPos(const ReyEngine::Pos<int>& pos);
   void setSize(const ReyEngine::Size<int>& size);
   void scale(const ReyEngine::Vec2<float>& scale);
   void setWidth(int width);
   void setHeight(int height);

   ReyEngine::Pos<int> getLocalMousePos() const {return globalToLocal(InputManager::getMousePos());}
   ReyEngine::Pos<int> globalToLocal(const ReyEngine::Pos<int>& global) const;
   ReyEngine::Pos<int> localToGlobal(const ReyEngine::Pos<int>& local) const;
   void setGlobalPos(const iVec&);
   bool isInside(const iVec& point){return _rect.value.toSizeRect().isInside(point);}

   std::optional<std::shared_ptr<BaseWidget>> getWidgetAt(ReyEngine::Pos<int> pos);
   bool isHovered() const{return _hovered;}

   virtual void render() const = 0; //draw the widget
   static void registerType(const std::string& typeName, const std::string& parentType, bool isVirtual, Deserializer fx){TypeManager::registerType(typeName, parentType, isVirtual, fx);}

protected:

   // Drawing functions
   virtual void renderBegin(ReyEngine::Pos<double>& textureOffset){}
   void renderEditorFeatures();
   void renderChain(ReyEngine::Pos<double>& textureOffset);
   virtual void renderEnd(){}
   ReyEngine::Vec2<float> getRenderOffset() const {return _renderOffset;}
   void setRenderOffset(ReyEngine::Pos<double> offset){_renderOffset = offset;}
   void _drawText(const std::string& text, const ReyEngine::Pos<int>& pos, const ReyEngine::ReyEngineFont& font) const;
   void _drawTextCentered(const std::string& text, const ReyEngine::Pos<int>& pos, const ReyEngine::ReyEngineFont& font) const;
   void _drawRectangle(const ReyEngine::Rect<int>& rect, ReyEngine::ColorRGBA color) const;
   void _drawRectangleLines(const ReyEngine::Rect<int>& rect, float lineThick, ReyEngine::ColorRGBA color) const;
   void _drawRectangleRounded(const ReyEngine::Rect<int>& rect,  float roundness, int segments, ReyEngine::ColorRGBA color) const;
   void _drawRectangleRoundedLines(const ReyEngine::Rect<int>& rect, float roundness, int segments, float lineThick, ReyEngine::ColorRGBA color) const;
   void _drawRectangleGradientV(const ReyEngine::Rect<int>& rect, ReyEngine::ColorRGBA color1, ReyEngine::ColorRGBA color2) const;
   void registerProperties() override;
   RectProperty<int> _rect;

   //input

   //convenience
   void _publishSize(){WidgetResizeEvent event(toEventPublisher());publish<decltype(event)>(event);}

   bool isLayout = false;
   bool isInLayout = false;
   ReyEngine::Size<int> maxSize = {INT_MAX, INT_MAX};
   ReyEngine::Size<int> minSize = {0,0};

   //editor stuff
public:
   void setInEditor(bool state){_isEditorWidget = state;}
   bool isInEditor(){return _isEditorWidget;}
   void setEditorSelected(bool selected){_editor_selected = selected;}
   bool isEditorSelected(){return _editor_selected;}
protected:
   bool _isEditorWidget = false; //true if this is a widget THE USER HAS PLACED IN THE EDITOR WORKSPACE (not a widget that the editor uses for normal stuff)
   bool _editor_selected = false; // true when the object is *selected* in the editor
   static constexpr int GRAB_HANDLE_SIZE = 10;
   ReyEngine::Rect<int> _getGrabHandle(int index);// 0-3 clockwise starting in top left (TL,TR,BR,BL)
   int _editor_grab_handles_dragging = -1; //which grab handle is being drug around
private:
   AnchorProperty _anchor;
   uint64_t _rid; //unique identifier
   std::string _name;
   BoolProperty _isProcessed;
   bool _hovered = false; //true when hovered, set by application
   bool _visible = true; //whether to show the widget (and its children)
   std::recursive_mutex _childLock;
   ReyEngine::Pos<double> _renderOffset; //used for different rendering modes. does not offset position.

   Handled _process_unhandled_input(InputEvent&); //pass input to children if they want it and then process it for ourselves if necessary
   Handled _process_unhandled_editor_input(InputEvent&); //pass input to children if they want it and then process it for ourselves if necessary ONLY FOR EDITOR RELATED THINGS (grab handles mostly)
   InputFilter inputFilter = InputFilter::INPUT_FILTER_PASS_AND_PROCESS;
};