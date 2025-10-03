#pragma once
#include "TypeTree.h"
#include "Event.h"
#include <memory>
#include "Application.h"

namespace ReyEngine{
   // promote node making functions to ReyEngine namespace
   template<typename T, typename InstanceName, typename... Args>
   Internal::Tree::MakeNodeReturnType<T> make_node(InstanceName&& instanceName, Args&&... args){
      return Internal::Tree::_make_node<T>(instanceName, std::forward<Args>(args)...);
   }

   //accept either TypeNode* directly, or assume it has a getNode() function that returns same
   template<typename T, typename N, typename InstanceName, typename... Args>
   std::shared_ptr<T> make_child(N parent, InstanceName&& instanceName, Args&&... args){
      if constexpr (std::is_same_v<Internal::Tree::TypeNode*, N>) {
         return Internal::Tree::_make_child<T>(parent, instanceName, std::forward<Args>(args)...);
      } else {
         return Internal::Tree::_make_child<T>(parent->getNode(), instanceName, std::forward<Args>(args)...);
      }
   }

   template<typename T, typename N, typename InstanceName, typename... Args>
   std::shared_ptr<T> replace_child(N parent, InstanceName&& instanceName, Args&&... args){
      auto oldChild = parent->removeChild(instanceName, true);
      auto [_newChildPtr, _newChildNode] = Internal::Tree::_make_node<T>(instanceName, std::forward<Args>(args)...);
      std::optional<size_t> index = oldChild.has_value() ? oldChild.value()->getIndex() : std::optional<size_t>();
      (void)parent->addChild(std::move(_newChildNode), index)->getData();
      return _newChildPtr;
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
         TYPENAME(ReyObject)
         [[nodiscard]] virtual std::string getTypeName() const {return TYPE_NAME;}
         [[nodiscard]] std::string getName() const {return _node ? _node->name : "";}
         std::vector<TypeNode*> getChildren(){return _node ? _node->getChildren() : std::vector<TypeNode*>();}
         [[nodiscard]] const std::vector<TypeNode*> getChildren() const {return const_cast<ReyObject*>(this)->getChildren();}
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
         // (for real though, the intention here is internal engine
         // functionality can override them but then they should explicitly
         // call this version of the function so that the calls can be dispatched
         // correctly. The problem if you override them is you might not know
         // all the places they are being overridden, which could cause some
         // calls to be missed if you don't call them explicitly)
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
      protected:
         ReyObject(){
            uniqueValue = Application::generateUniqueValue();
         }
      private:
         UniqueValue uniqueValue;
      };
   }
}