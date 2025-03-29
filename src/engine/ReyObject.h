#pragma once
#include "TypeTree.h"
#include "Event.h"
#include <memory>
#include "Application.h"

namespace ReyEngine{
   // promote node making functions to ReyEngine namespace
   template <typename T>
   using MakeNodeReturnType = Internal::Tree::MakeNodeReturnType<T>;
   template<typename T, typename InstanceName, typename... Args>
   Internal::Tree::MakeNodeReturnType<T> make_node(InstanceName&& instanceName, Args&&... args){
      return Internal::Tree::_make_node<T>(instanceName, std::forward<Args>(args)...);
   }
   template<typename T, typename InstanceName>
   Internal::Tree::MakeNodeReturnType<T> make_node(InstanceName&& instanceName, std::unique_ptr<T>&& ptr){
      return Internal::Tree::_make_node(instanceName, std::move(ptr));
   }
   using TypeNode = Internal::Tree::TypeNode;
   using TreeStorable = Internal::Tree::TreeStorable;
   using TypeTag = Internal::Tree::TypeTag;
   using Handled = bool;
   #define TYPENAME(CLASSNAME) \
   static constexpr char TYPE_NAME[] = #CLASSNAME; \

   #define REYENGINE_OBJECT(CLASSNAME) \
   TYPENAME(CLASSNAME) \
   [[nodiscard]] std::string getTypeName() const override {return TYPE_NAME;}
   //satisfies typewrappable
   namespace Internal{
      struct ReyObject : public TreeStorable
      {
         ReyObject(){
            uniqueValue = Application::generateUniqueValue();
         }
         TYPENAME(ReyObject)
         [[nodiscard]] virtual std::string getTypeName() const {return TYPE_NAME;}
         std::string getName() const {return _node ? _node->name : "";}
         std::vector<TypeNode*> getChildren(){return _node ? _node->getChildren() : std::vector<TypeNode*>();}
         const std::vector<TypeNode*> getChildren() const {return const_cast<ReyObject*>(this)->getChildren();}
         std::optional<TypeNode*> getChild(const std::string& name){return _node ? _node->getChild(name) : std::nullopt;}

         template <typename T>
         std::vector<T*> getChildrenAs(){
            std::vector<T*> retval;
            for (auto child : getChildren()){
               if (auto isT = child->as<T>()){
                  retval.push_back(isT.value());
               }
            }
            return retval;
         }

         template <typename T>
         std::vector<const T*> getChildrenAs() const {
            std::vector<T*> retval;
            for (auto child : getChildren()){
               if (auto isT = child->as<T>()){
                  retval.push_back(child);
               }
            }
            return retval;
         }

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
         virtual inline bool operator==(const ReyObject& rhs){return uniqueValue == rhs.uniqueValue;}
      private:
         UniqueValue uniqueValue;
      };
   }
}