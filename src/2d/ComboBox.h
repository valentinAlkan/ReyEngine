#pragma once
#include "BaseWidget.h"

namespace ReyEngine {

   struct ComboBoxDataField
   {
      ComboBoxDataField()
      : text("text")
      , checkable("checkable")
      , checked("checked")
      {}
      StringProperty text;
      BoolProperty checkable;
      BoolProperty checked;
      bool highlighted = false;
   };
   
struct ComboBoxDataFieldList : public ListProperty<ComboBoxDataField>{
      using ListProperty<ComboBoxDataField>::operator=;
      ComboBoxDataFieldList(const std::string& instanceName): ListProperty<ComboBoxDataField>(instanceName){}
      ComboBoxDataField stringToElement(const std::string& element) const override {throw std::runtime_error("ComboBoxDataField stringToElement not implemented");}
      std::string elementToString(const ComboBoxDataField& t)       const override {throw std::runtime_error("ComboBoxDataField elementToString not implemented");}
   };


   class ComboBox : public BaseWidget {
   REYENGINE_OBJECT(ComboBox, BaseWidget)
   , PROPERTY_DECLARE(toolTip)
   , PROPERTY_DECLARE(data)
   , PROPERTY_DECLARE(currentIndex, 0)
   {
      setAcceptsHover(true);
   }

   public:
      ComboBoxDataField& addItem(const std::string&);
      void setItems(const std::vector<std::string>&);
      void clear();
      void eraseItem(int index);
      void setCurrentIndex(int index);
      int getCurrentIndex() const;
      inline int getItemCount() const {return data.size();}
      std::string text() const;
      ComboBoxDataField& getCurrentField();
      ComboBoxDataField& getField(int index);
      void render() const override;
      void registerProperties() override{registerProperty(data);};
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;

   protected:
      std::optional<int> getIndexAt(Pos<int>);
      StringProperty toolTip;
      ComboBoxDataFieldList data;
      IntProperty currentIndex;

   private:
      bool _isOpen = false;
      Rect<int> _selectionMenuRect;
   };
}