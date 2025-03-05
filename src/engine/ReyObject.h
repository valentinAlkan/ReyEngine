#pragma once
#include "TypeTree.h"
#include "Event.h"
#include <memory>
#include "Application.h"

namespace ReyEngine{
   using TypeNode = Internal::Tree::TypeNode;
   using TreeStorable = Internal::Tree::TreeStorable;
   using TypeTag = Internal::Tree::TypeTag;
   using Handled = bool;
   #define TYPENAME(CLASSNAME) \
   static constexpr char TYPE_NAME[] = #CLASSNAME; \

   #define REYENGINE_OBJECT(CLASSNAME) \
   TYPENAME(CLASSNAME) \
   std::string getTypeName() override {return TYPE_NAME;}  \

   //satisfies typewrappable
   namespace Internal{
      struct ReyObject : public TreeStorable
      {
         ReyObject(){
            uniqueValue = Application::generateUniqueValue();
         }
         TYPENAME(ReyObject)
         virtual std::string getTypeName() {return TYPE_NAME;}
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
         inline bool operator==(const ReyObject& rhs){return uniqueValue == rhs.uniqueValue;}
         inline bool operator!=(const ReyObject& rhs){return uniqueValue == rhs.uniqueValue;}
      private:
         UniqueValue uniqueValue;
      };
   }
}