#pragma once
#include "TypeTree.h"

using TypeNode = ReyEngine::Internal::Tree::TypeNode;
using TreeCallable = ReyEngine::Internal::Tree::TreeCallable;

#define TYPENAME(CLASSNAME) \
static constexpr char TYPE_NAME[] = #CLASSNAME; \

#define REYENGINE_OBJECT(CLASSNAME, PARENT_CLASSNAME) \
TYPENAME(CLASSNAME) \
virtual std::string getTypeName(){return TYPE_NAME;}  \
//virtual void __on_added_to_tree(TypeNode* node) {      \
//   PARENT_CLASSNAME::__on_added_to_tree(node);   \
//   CLASSNAME::_on_added_to_tree(node);           \
//}                                                     \
//virtual void __on_child_added_to_tree(TypeNode* node) {\
//   PARENT_CLASSNAME::__on_child_added_to_tree(node);   \
//   CLASSNAME::_on_child_added_to_tree(node);           \
//}

//satisfies typewrappable
struct EngineObject : public TreeCallable
{
   TYPENAME(EngineObject)
//   void __on_added_to_tree(TypeNode* node){_on_added_to_tree(node);}
//   void __on_child_added_to_tree(TypeNode* node){ _on_child_added_to_tree(node);}
//   void _on_added_to_tree(TypeNode*){
//      std::cout << TYPE_NAME << " added to tree" << std::endl;
//   };
//   void _on_child_added_to_tree(TypeNode*){};
};

struct Renderer2D : public EngineObject {
   REYENGINE_OBJECT(Renderer2D, EngineObject)
   Renderer2D(){}
   virtual ~Renderer2D() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
   void _on_added_to_tree(TypeNode*){
      std::cout << TYPE_NAME << " added to tree" << std::endl;
   };
   void _on_child_added_to_tree(TypeNode* child);
};


struct Canvas : public Renderer2D {
   REYENGINE_OBJECT(Canvas, Renderer2D)
   Canvas(const std::string& blah){}
   ~Canvas() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
   std::string someOtherData;
};

struct Drawable2D : public Renderer2D {
   REYENGINE_OBJECT(Drawable2D, Renderer2D)
   Drawable2D(){}
   ~Drawable2D() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
   std::string someData;
};


struct Component : public EngineObject {
   REYENGINE_OBJECT(Component, EngineObject)
   Component(): componentData("Default args"){}
   ~Component() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
   std::string componentData;
};


struct Sprite : public Drawable2D {
   REYENGINE_OBJECT(Sprite, Drawable2D)
   Sprite(): Drawable2D(){}
   ~Sprite() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
};

struct CompletelyUnrelatedType {
   TYPENAME(CompletelyUnrelatedType)
   ~CompletelyUnrelatedType() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
};
