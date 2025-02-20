#pragma once
#include "TypeTree.h"

using TypeNode = ReyEngine::Internal::Tree::TypeNode;
using TreeStorable = ReyEngine::Internal::Tree::TreeStorable;

#define TYPENAME(CLASSNAME) \
static constexpr char TYPE_NAME[] = #CLASSNAME; \

#define REYENGINE_OBJECT(CLASSNAME, PARENT_CLASSNAME) \
TYPENAME(CLASSNAME) \
virtual std::string getTypeName(){return TYPE_NAME;}  \

//satisfies typewrappable
struct EngineObject : public TreeStorable
{
   TYPENAME(EngineObject)
   //reserved for internal functionality.
   //   GOOD FREND FOR IESVS SAKE FORBEARE
   //   TO DIGG THE DVST ENCLOASED HEARE
   //   BLESE BE Ye MAN Yt SPARES THES NODES
   //   AND CVRST BE HE Yt MOVES MY CODES
   // Seriously. don't override these unless you
   // really know what you're doing.
   void __init() override {_init();}
   void __on_added_to_tree() override {_on_added_to_tree();}
   void __on_child_added_to_tree(TypeNode *n) override { _on_child_added_to_tree(n);}
   void __on_descendant_added_to_tree(TypeNode *n) override { _on_descendant_added_to_tree(n);}
   void __on_child_removed_from_tree(TypeNode* n) override { _on_child_removed_from_tree(n);}
   void __on_descendant_removed_from_tree(TypeNode* n) override { _on_descendant_removed_from_tree(n);}
   // these are the functions you should override for your nodes
   virtual void _init(){}; //added to tree for first time
   virtual void _on_added_to_tree(){}; //happens every time the node is added to the tree
   virtual void _on_child_added_to_tree(TypeNode *n) {};
   virtual void _on_descendant_added_to_tree(TypeNode *n) {};
   virtual void _on_child_removed_from_tree(TypeNode*){};
   virtual void _on_descendant_removed_from_tree(TypeNode*){};
};

struct Drawable2D;
struct Renderer2D : public EngineObject {
   REYENGINE_OBJECT(Renderer2D, EngineObject)
   Renderer2D(){}
   virtual ~Renderer2D() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
   void _on_child_added_to_tree(TypeNode* child) override;
   std::vector<Drawable2D*> drawOrder;
};


struct Canvas : public Renderer2D {
   REYENGINE_OBJECT(Canvas, Renderer2D)
   Canvas(const std::string& blah){}
   ~Canvas() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
   std::string someOtherData;
};

struct Drawable2D : public Renderer2D {
   REYENGINE_OBJECT(Drawable2D, Renderer2D)
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
   Sprite(const std::string& texPath): texPath(texPath){}
   ~Sprite() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
   std::string texPath;
};

struct AnimatedSprite : public Sprite {
   REYENGINE_OBJECT(AnimatedSprite, Sprite)
   ~AnimatedSprite() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
};

struct CompletelyUnrelatedType {
   TYPENAME(CompletelyUnrelatedType)
   ~CompletelyUnrelatedType() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
};
