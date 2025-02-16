#pragma once
#include "BaseWidget.h"

namespace ReyEngine {

   struct ComboBoxData{};

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
      std::shared_ptr<ComboBoxData> data;
      bool highlighted = false;
   };
   
   struct ComboBoxDataFieldList : public ListProperty<ComboBoxDataField>{
      using ListProperty<ComboBoxDataField>::operator=;
      ComboBoxDataFieldList(const std::string& instanceName): ListProperty<ComboBoxDataField>(instanceName){}
      ComboBoxDataField stringToElement(const std::string& element) const override {throw std::runtime_error("ComboBoxDataField stringToElement not implemented");}
      std::string elementToString(const ComboBoxDataField& t)       const override {throw std::runtime_error("ComboBoxDataField elementToString not implemented");}
   };


   class ComboBox : public BaseWidget {
   REYENGINE_OBJECT_BUILD_ONLY(ComboBox, BaseWidget)
   , PROPERTY_DECLARE(toolTip)
   , PROPERTY_DECLARE(fields)
   , PROPERTY_DECLARE(currentIndex, 0)
   {
      setAcceptsHover(true);
   }

   public:
      struct EventComboBoxItemSelected : public Event<EventComboBoxItemSelected>{
         EVENT_CTOR_SIMPLE(EventComboBoxItemSelected, Event<EventComboBoxItemSelected>, int index, const ComboBoxDataField& field)
         , field(field)
         , index(index)
         {}
         const int index;
         const ComboBoxDataField& field;
      };

      struct EventComboBoxItemHovered : public Event<EventComboBoxItemHovered>{
         EVENT_CTOR_SIMPLE(EventComboBoxItemHovered, Event<EventComboBoxItemHovered>, int index, const ComboBoxDataField& field)
         , field(field)
         , index(index)
         {}
         const int index;
         const ComboBoxDataField& field;
      };

      struct EventComboBoxMenuClosed : public Event<EventComboBoxMenuClosed>{EVENT_CTOR_SIMPLE(EventComboBoxMenuClosed, Event<EventComboBoxMenuClosed>){}};
      struct EventComboBoxMenuOpened : public Event<EventComboBoxMenuOpened>{EVENT_CTOR_SIMPLE(EventComboBoxMenuOpened, Event<EventComboBoxMenuOpened>){}};

      REYENGINE_DEFAULT_BUILD(ComboBox)
      ComboBoxDataField& addItem(const std::string&);
      void setItems(const std::vector<std::string>&);
      void clear();
      void eraseItem(int index);
      void setCurrentIndex(int index);
      ComboBoxDataField& getLastField();
      int getCurrentIndex() const;
      inline int getItemCount() const {return fields.size();}
      std::string text() const;
      ComboBoxDataField& getCurrentField();
      ComboBoxDataField& getField(int index);
      void render2D() const override;
      void registerProperties() override{registerProperty(fields);};

   protected:
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      std::optional<int> getIndexAt(Pos<int>);
      void _on_modality_lost() override;
      StringProperty toolTip;
      ComboBoxDataFieldList fields;
      IntProperty currentIndex;

   private:
      int fieldSelectionRectHeight;
      bool _isOpen = false;
      Rect<int> _selectionMenuRect;
      std::vector<Rect<int>> _selectionMenuItemRects;

   };
}