#pragma once
#include "Application.h"

//an object which can render Widgets
class Canvas : public BaseWidget {
public:
   static constexpr char TYPE_NAME[] = "Canvas";
   std::string _get_static_constexpr_typename()
   override{return TYPE_NAME;}
public:
   static std::shared_ptr<Node> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
      const ReyEngine::Rect<float> &r = {0, 0, 0, 0};
      auto retval = std::make_shared<Canvas>(instanceName, r);
      retval->Node::_deserialize(properties);
      return retval;
   }
   Canvas(const std::string &name, const ReyEngine::Rect<float> &r) : Canvas(name, _get_static_constexpr_typename(), r){

   }
protected:
   void _register_parent_properties() override {
         Node::_register_parent_properties();
         Node::registerProperties();
   }

   std::shared_ptr<BaseWidget> _root;
};
