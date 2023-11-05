#pragma once
#include "Control.hpp"

/////////////////////////////////////////////////////////////////////////////////////////
class Layout : public Control {
public:
   enum class LayoutDir{HORIZONTAL, VERTICAL};
protected:
   Layout(const std::string &name, const std::string &typeName, const GFCSDraw::Rect<float> &r, LayoutDir layoutDir)
   : Control(name, typeName, r)
   , childScales("layoutRatios")
   , dir(layoutDir)
   {
      isLayout = true;
   }
   void _register_parent_properties() override{
      Control::_register_parent_properties();
      Control::registerProperties();
   }
   void _on_child_added(std::shared_ptr<BaseWidget>& child) override {
      Application::printDebug() << child->getName() << " added to layout " << getName() << std::endl;
      arrangeChildren();
   }
   void _on_child_added_immediate(std::shared_ptr<BaseWidget>& child) override {
      childScales.ref().push_back(1.0);
   }
   void _on_rect_changed() override {
      arrangeChildren();
   }
   void arrangeChildren(){
      auto& children = getChildren();
      if (children.empty()) return; //early return
      size_t childCount = children.size();
      //each childs scale value
      float totalScale = childScales.sum();
      //todo: ratios
      bool canExpand = isInLayout; //if we're not in a layout we can expand ourselves to fit our children
      switch (dir) {
         case LayoutDir::HORIZONTAL: {
            //how much space we have to allocate
            auto totalSpace = getWidth();
            //how much space we will allocate to each child
            auto sizeEach = totalSpace / (float) childCount;
            unsigned long i = 0;
            auto pos = GFCSDraw::Pos<int>(0, 0);
            for (auto &child: getChildren()) {
               auto thisRatio = childScales.value[i];
               int thisSize = (int)sizeEach * thisRatio;
               child->setRect({pos, {thisSize, _rect.value.height}});
               pos.x += thisSize;
            }
            break;
         }
         case LayoutDir::VERTICAL: {
            //how much space we have to allocate
            auto totalSpace = getHeight();
            //how much space we will allocate to each child
            auto sizeEach = totalSpace / (float)childCount;
            unsigned long i=0;
            auto pos = GFCSDraw::Pos<int>(0,0);
            for (auto& child : getChildren()){
               auto thisRatio = childScales.value[i];
               int thisSize = sizeEach * thisRatio;
               child->setRect({pos, {_rect.value.width, thisSize}});
               pos.y += thisSize;
            }
            break;
         }
      }
   }
public:
   FloatListProperty childScales;
protected:
   const LayoutDir dir;
};

/////////////////////////////////////////////////////////////////////////////////////////
class VLayout : public Layout {
public:
   VLayout(const std::string& instanceName, const GFCSDraw::Rect<float>& r)
   : Layout(instanceName, _get_static_constexpr_typename(), r, Layout::LayoutDir::VERTICAL)
   {}
   static constexpr char TYPE_NAME[] = "VLayout";
   static std::shared_ptr<BaseWidget> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
      const GFCSDraw::Rect<float> &r = {0, 0, 0, 0};
      auto retval = std::make_shared<VLayout>(instanceName, r);
      retval->BaseWidget::_deserialize(properties);
   return retval;
}
protected:
   std::string _get_static_constexpr_typename() override{ return TYPE_NAME;}
};

/////////////////////////////////////////////////////////////////////////////////////////
class HLayout : public Layout {
public:
   HLayout(const std::string& instanceName, const GFCSDraw::Rect<float>& r)
   : Layout(instanceName, _get_static_constexpr_typename(), r, Layout::LayoutDir::HORIZONTAL)
   {}
   static std::shared_ptr<BaseWidget> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
      const GFCSDraw::Rect<float> &r = {0, 0, 0, 0};
      auto retval = std::make_shared<HLayout>(instanceName, r);
      retval->BaseWidget::_deserialize(properties);
   return retval;
}
   static constexpr char TYPE_NAME[] = "HLayout";
protected:
   std::string _get_static_constexpr_typename() override{ return TYPE_NAME;}
};