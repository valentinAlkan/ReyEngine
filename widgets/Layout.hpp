#pragma once
#include "Control.hpp"
#include "MathTools.h"

/////////////////////////////////////////////////////////////////////////////////////////
class Layout : public Control {
public:
   /////////////////////////////////////////////////////////////////////////////////////////
   enum class LayoutDir{HORIZONTAL, VERTICAL};
   /////////////////////////////////////////////////////////////////////////////////////////
   struct LayoutProperty : public EnumProperty<LayoutDir, 2>{
      LayoutProperty(const std::string& instanceName,  LayoutDir defaultvalue)
            : EnumProperty<LayoutDir, 2>(instanceName, defaultvalue){}
      const EnumPair<LayoutDir, 2>& getDict() override {return dict;}
      static constexpr EnumPair<LayoutDir, 2> dict = {
            ENUM_PAIR_DECLARE(LayoutDir, VERTICAL),
            ENUM_PAIR_DECLARE(LayoutDir, HORIZONTAL),
      };
      void registerProperties() override {}
   };
   /////////////////////////////////////////////////////////////////////////////////////////
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
      //sum of each childs scale values
      float totalScale = childScales.sum();
      //how much space we have to allocate
      auto totalSpace = dir == LayoutDir::HORIZONTAL ? getWidth() : getHeight();
      auto sizeEach = (float)totalSpace / (float) childCount;
      unsigned long i = 0;
      auto pos = GFCSDraw::Pos<int>(0, 0);
      //how much space we will allocate to each child
      for (auto& child: getChildren()) {
         auto thisRatio = childScales.value[i];
         int thisSize = (int)(sizeEach * thisRatio);
         auto newSize = dir == LayoutDir::HORIZONTAL ? GFCSDraw::Rect<int>(pos, {thisSize, _rect.value.height}) : GFCSDraw::Rect<int>(pos, {_rect.value.width, thisSize});
         auto minSize = dir == LayoutDir::HORIZONTAL ? child->getMinSize().x : child->getMinSize().y;
         auto maxSize = dir == LayoutDir::HORIZONTAL ? child->getMaxSize().x : child->getMaxSize().y;
         //enforce min/max bounds
         switch(dir) {
            case LayoutDir::HORIZONTAL:
               newSize.width = math_tools::clamp(minSize, maxSize, newSize.width);
               break;
            case LayoutDir::VERTICAL:
               newSize.height = math_tools::clamp(minSize, maxSize, newSize.height);
               break;
         }
         //apply transformations
         child->setRect(newSize);
         switch(dir){
            case LayoutDir::HORIZONTAL:
               pos.x += thisSize;
               break;
            case LayoutDir::VERTICAL:
               pos.y += thisSize;
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
   VLayout(const std::string& instanceName, const GFCSDraw::Rect<int>& r)
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
   HLayout(const std::string& instanceName, const GFCSDraw::Rect<int>& r)
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