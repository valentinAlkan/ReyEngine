#pragma once
#include "DrawInterface.h"
#include "SharedFromThis.h"
#include "Event.h"
#include "Property.h"
#include "InputManager.h"
#include "TypeManager.h"
#include <mutex>
#include <string>
#include <memory>

using Handled = bool;

#define CTOR_RECT const ReyEngine::Rect<float>& r
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(TYPENAME) \
static constexpr char TYPE_NAME[] = #TYPENAME;               \
std::string _get_static_constexpr_typename() override {return TYPE_NAME;}
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_NODE_SERIALIZER(CLASSNAME, PARENT_CLASSNAME) \
   public:                                           \
   static std::shared_ptr<Node> deserialize(const std::string& instanceName, PropertyPrototypeMap& properties) { \
   auto retval = std::make_shared<CLASSNAME>(instanceName); \
   retval->Node::_deserialize(properties);        \
   return retval;}                                       \
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME) \
   CLASSNAME(const std::string& name, const std::string& typeName, CTOR_RECT): PARENT_CLASSNAME(name, typeName)
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_DEFAULT_CTOR(CLASSNAME) \
   CLASSNAME(const std::string& name, CTOR_RECT): CLASSNAME(name, _get_static_constexpr_typename()){}
/////////////////////////////////////////////////////////////////////////////////////////
#define REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME) \
protected:                                                \
   void _register_parent_properties() override{           \
      PARENT_CLASSNAME::_register_parent_properties();    \
      PARENT_CLASSNAME::registerProperties();             \
   }

#define REYENGINE_NODE(CLASSNAME, PARENT_CLASSNAME)  \
public:                                                   \
   REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(CLASSNAME)  \
   REYENGINE_NODE_SERIALIZER(CLASSNAME, PARENT_CLASSNAME)       \
   REYENGINE_DEFAULT_CTOR(CLASSNAME)                       \
   REYENGINE_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME)  \
   REYENGINE_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME)


class Scene;
class Draggable;
class  Node
: public inheritable_enable_shared_from_this<Node>
, public EventSubscriber
, public EventPublisher
, public PropertyContainer
{
   using ChildIndex = unsigned long;
   using NodePtr = std::shared_ptr<Node>;
   using ChildMap = std::map<std::string, std::pair<ChildIndex, NodePtr>>;
   using ChildOrder = std::vector<NodePtr>;
public:
   static constexpr char TYPE_NAME[] = "Node";
   Node(const std::string& name, std::string  typeName);
   ~Node();
   uint64_t getRid() const {return _rid;}
   std::string getName() const {return _name;}

   bool setName(const std::string& name, bool append_index=false);
   bool setIndex(unsigned int newIndex);
   std::string getTypeName(){return _typeName;}

   std::weak_ptr<Node> getParent(){return _parent;}
   const ChildOrder& getChildren() const {return _childrenOrdered;}
   std::optional<NodePtr> getChild(const std::string& name);
   bool hasChild(const std::string& name);

   template <typename T>
   std::shared_ptr<T> toType(){
      static_assert(std::is_base_of_v<Node, T>);
      auto me = toNode();
      return std::static_pointer_cast<T>(me);
   }

   void setProcess(bool process);
   NodePtr setFree(); //request to remove this widget from the tree at next available opportunity. Does not immediately delete it
   // if you need your object to stop being processed immediately, use setFreeImmediately(), which could pause to syncrhonize threads.
   NodePtr setFreeImmediately(); // Pauses other threads and immediately removes objects from the tree.

   bool isRoot();

   std::optional<NodePtr> addChild(NodePtr);
   std::optional<NodePtr> removeChild(const std::string& name, bool quiet = false); //quiet silences the output if child is not found.
   void removeAllChildren(); //removes all children and DOES NOT RETURN THEM!

   bool operator==(const NodePtr& other) const {if (other){return other->getRid()==_rid;}return false;}
   bool operator==(const Node& other) const{return other._rid == _rid;}

   template <typename T> bool is_base_of(){return std::is_base_of_v<Node, T>;}
   static void registerType(const std::string& typeName, const std::string& parentType, bool isVirtual, Deserializer fx){TypeManager::registerType(typeName, parentType, isVirtual, fx);}
   std::string serialize();
protected:
   std::shared_ptr<Node> toNode(){return inheritable_enable_shared_from_this<Node>::downcasted_shared_from_this<Node>();}
   virtual void _on_application_ready(){}; //called when the main loop is starting, or immediately if that's already happened
   virtual void _init(){}; //run ONCE PER OBJECt when it enters tree for first time.
   virtual void _on_rect_changed(){} //called when the rect is manipulated
   virtual void _on_mouse_enter(){};
   virtual void _on_mouse_exit(){};
   virtual void _on_child_added_immediate(NodePtr&){} //Called immediately upon a call to addChild - DANGER: widget is not actually a child yet! It is (probably) a very bad idea to do much at all here. Make sure you know what you're doing.
   virtual void _on_child_added(NodePtr&){} // called at the beginning of the next frame after a child is added. Child is now owned by us. Safe to manipulate child.
   virtual void _on_enter_tree(){} //called every time a widget enters the tree
   virtual void _on_exit_tree(){}
   virtual void _on_child_removed(){}

   //override and setProcess(true) to allow processing
   virtual void _process(float dt){};
   void registerProperties() override;
   void _deserialize(PropertyPrototypeMap&);
   virtual void _register_parent_properties(){};
   void _is_extendable(){static_assert(true);}
   virtual std::string _get_static_constexpr_typename(){return TYPE_NAME;}
   bool _has_inited = false; //set true THE FIRST TIME a widget enters the tree. Can do constructors of children and other stuff requiring shared_from_this();

   //input
   bool _acceptsInput = false;
   bool _acceptsHover = false;
   virtual Handled _unhandled_input(InputEvent&){return false;}
   virtual std::optional<std::shared_ptr<Draggable>> _on_drag_start(ReyEngine::Pos<int> globalPos){return std::nullopt;} //override and return something to implement drag and drop
   virtual Handled _on_drag_drop(std::shared_ptr<Draggable>){return false;}

   bool _is2d = false;
   bool _is3d = false;

private:
   void rename(NodePtr& child, const std::string& newName);
   uint64_t _rid; //unique identifier
   const std::string _typeName; //can't just use static constexpr TYPE_NAME since we need to know what the type is if using type-erasure
   std::string _name;
   BoolProperty _isProcessed;
   std::weak_ptr<Node> _parent;
   ///If this node is the root of a scene, then the rest of the scene data is here.
   std::optional<std::shared_ptr<Scene>> _scene;
   bool _request_delete = false; //true when we want to remove this object from the tree
   std::recursive_mutex _childLock;
   bool _scheduled_for_deletion = false; // true when the widget has been scheduled for deletion but is not yet deleted.

   ChildMap _children;
   ChildOrder _childrenOrdered;
   friend class Window;
   friend class Application;
};