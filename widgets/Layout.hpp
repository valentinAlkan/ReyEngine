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
      const EnumPair<LayoutDir, 2>& getDict() const override {return dict;}
      static constexpr EnumPair<LayoutDir, 2> dict = {
            ENUM_PAIR_DECLARE(LayoutDir, VERTICAL),
            ENUM_PAIR_DECLARE(LayoutDir, HORIZONTAL),
      };
      void registerProperties() override {}
   };
   /////////////////////////////////////////////////////////////////////////////////////////
protected:
   Layout(const std::string &name, const std::string &typeName, const ReyEngine::Rect<float> &r, LayoutDir layoutDir)
   : Control(name, typeName, r)
   , childScales("layoutRatios")
   , dir(layoutDir)
   {
      isLayout = true;
   }
   void _register_parent_properties() override {
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

   void renderEnd() override{
      //debug
      _drawRectangleLines({0,0,_rect.get().width, _rect.get().height}, 2.0, COLORS::black);
   }
   void arrangeChildren(){
      //how much space we have to allocate
      auto totalSpace = dir == LayoutDir::HORIZONTAL ? getWidth() : getHeight();
      auto pos = ReyEngine::Pos<int>(0, 0);
      //how much space we will allocate to each child
      unsigned int childIndex = 0;
      auto calcRatio = [this](int startIndex) -> float{
         float sum = 0;
         for (int i=startIndex;i<childScales.size();i++){
            sum += this->childScales.get(i);
         }
         return childScales.value[startIndex] / sum;
      };

      auto sizeLeft = totalSpace;
      for (auto& child: getChildren()) {
         int allowedSpace = (int)(sizeLeft * calcRatio(childIndex));
         auto newSize = dir == LayoutDir::HORIZONTAL ? ReyEngine::Rect<int>(pos, {allowedSpace, _rect.value.height}) : ReyEngine::Rect<int>(pos, {_rect.value.width, allowedSpace});
         auto minSize = dir == LayoutDir::HORIZONTAL ? child->getMinSize().x : child->getMinSize().y;
         auto maxSize = dir == LayoutDir::HORIZONTAL ? child->getMaxSize().x : child->getMaxSize().y;
         //enforce min/max bounds
         int consumedSpace;
         switch(dir) {
            case LayoutDir::HORIZONTAL:
               newSize.width = math_tools::clamp(minSize, maxSize, newSize.width);
               pos.x += newSize.width;
               consumedSpace = newSize.width;
               break;
            case LayoutDir::VERTICAL:
               newSize.height = math_tools::clamp(minSize, maxSize, newSize.height);
               pos.y += newSize.height;
               consumedSpace = newSize.height;
               break;
         }
         //apply transformations
         child->setRect(newSize);
         childIndex++;
         //recalculate size each if we didn't use all available space
         sizeLeft -= consumedSpace;
//         childrenLeft--;
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
   VLayout(const std::string& instanceName, const ReyEngine::Rect<int>& r)
   : Layout(instanceName, _get_static_constexpr_typename(), r, LayoutDir::VERTICAL)
   {}
   static constexpr char TYPE_NAME[] = "VLayout";
   static std::shared_ptr<BaseWidget> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
      const ReyEngine::Rect<float> &r = {0, 0, 0, 0};
      auto retval = std::make_shared<VLayout>(instanceName, r);
      retval->BaseWidget::_deserialize(properties);
   return retval;
}
protected:
   std::string _get_static_constexpr_typename() override{ return TYPE_NAME;}
   VLayout(const std::string& name, const std::string& typeName, CTOR_RECT): Layout(name, typeName, r, LayoutDir::VERTICAL){}
};

/////////////////////////////////////////////////////////////////////////////////////////
class HLayout : public Layout {
public:
   HLayout(const std::string& instanceName, const ReyEngine::Rect<int>& r)
   : Layout(instanceName, _get_static_constexpr_typename(), r, Layout::LayoutDir::HORIZONTAL)
   {}
   static std::shared_ptr<BaseWidget> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
      const ReyEngine::Rect<float> &r = {0, 0, 0, 0};
      auto retval = std::make_shared<HLayout>(instanceName, r);
      retval->BaseWidget::_deserialize(properties);
   return retval;
}
   static constexpr char TYPE_NAME[] = "HLayout";
protected:
   std::string _get_static_constexpr_typename() override{ return TYPE_NAME;}
   HLayout(const std::string& name, const std::string& typeName, CTOR_RECT): Layout(name, typeName, r, LayoutDir::VERTICAL){}
};