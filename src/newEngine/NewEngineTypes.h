#pragma once
#include "TypeTree.h"
#include "Event2.h"
#include <memory>

namespace ReyEngine{
   using TypeNode = Internal::Tree::TypeNode;
   using TreeStorable = Internal::Tree::TreeStorable;
   using TypeTag = Internal::Tree::TypeTag;
   using Handled = bool;
   #define TYPENAME(CLASSNAME) \
   static constexpr char TYPE_NAME[] = #CLASSNAME; \

   #define REYENGINE_OBJECT(CLASSNAME) \
   TYPENAME(CLASSNAME) \
   virtual std::string getTypeName(){return TYPE_NAME;}  \

   //satisfies typewrappable
   struct ReyObject : public TreeStorable
   {
      TYPENAME(ReyObject)
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

   struct Canvas;
   struct InputHandler : TypeTag {
      virtual Handled __process_unhandled_input() = 0;
      virtual Handled _unhandled_input(){return false;}
   };

   struct Transformable : virtual ReyObject {

   };

   struct Drawable2D : ReyObject {
      REYENGINE_OBJECT(Drawable2D)
      ~Drawable2D() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
      std::string someData;
      Transform xform{};
      Canvas* canvas = nullptr;
   };

   struct Canvas : public ReyObject {
      REYENGINE_OBJECT(Canvas)
      Canvas(const std::string& blah){}
      ~Canvas() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
      void _on_descendant_added_to_tree(TypeNode* child) override;
      ///walk the tree and pin any drawables to us
      void cacheDrawables();
      std::vector<std::pair<Matrix, Drawable2D*>> drawOrder;
   };

   struct Widget
         : public Drawable2D
         , public InputHandler
         , public EventPublisher
         {
      Handled __process_unhandled_input(){std::cout << "Processed input!" << std::endl;};
   };

   struct Sprite : public Widget {
      REYENGINE_OBJECT(Sprite)
      Sprite(const std::string& texPath): texPath(texPath){}
      ~Sprite() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
      std::string texPath;
   };

   struct AnimatedSprite : public Sprite {
      REYENGINE_OBJECT(AnimatedSprite)
      ~AnimatedSprite() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
   };

   struct CompletelyUnrelatedType {
      TYPENAME(CompletelyUnrelatedType)
      ~CompletelyUnrelatedType() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
   };
}