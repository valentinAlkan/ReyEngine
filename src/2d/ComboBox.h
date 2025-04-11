#pragma once
#include "Widget.h"

namespace ReyEngine {

   struct ComboBoxData{};

   struct ComboBoxDataField {
      ComboBoxDataField(const std::string& text)
      : text(text)
      , checkable(true)
      , checked(true)
      {}
      std::string text;
      bool checkable;
      bool checked;
      bool highlighted = false;
   };
   
   class ComboBox : public Widget {
   public:
      REYENGINE_OBJECT(ComboBox)
      ComboBox() {
         acceptsHover = true;
         setMaxSize({std::numeric_limits<float>::max(), theme->font.size});
      }
      EVENT_ARGS(EventComboBoxItemSelected, 654654654, size_t itemIndex, const ComboBoxDataField* field)
         , itemIndex(itemIndex)
         , field(field)
         {}
         const size_t itemIndex;
         const ComboBoxDataField* field;
      };

      EVENT_ARGS(EventComboBoxItemHovered, 654654655, size_t itemIndex, const ComboBoxDataField* field)
         , field(field)
         , itemIndex(itemIndex)
         {}
         const size_t itemIndex;
         const ComboBoxDataField* field;
      };

      EVENT(EventComboBoxMenuClosed, 654654656){}};
      EVENT(EventComboBoxMenuOpened, 654654657){}};

      void setItems(const std::vector<std::string>&, size_t currentIndex=0);
      void clear();
      void eraseItem(size_t index);
      void setCurrentIndex(size_t index);
      inline ComboBoxDataField* addItem(const std::string& s){
         auto added = fields.emplace_back(std::make_unique<ComboBoxDataField>(s)).get();
         if (!currentIndex) currentIndex = 0;
         return added;
      }
      inline void addItems(const std::vector<std::string>& v) {for (const auto& s: v) {addItem(s);}}
      [[nodiscard]] inline std::optional<size_t> getCurrentIndex() const {return currentIndex;}
      [[nodiscard]] inline size_t getItemCount() const {return fields.size();}
      [[nodiscard]] std::optional<std::string> text() const;
      [[nodiscard]] std::optional<ComboBoxDataField*> getLastField() const;
      [[nodiscard]] std::optional<ComboBoxDataField*> getCurrentField() const;
      [[nodiscard]] std::optional<ComboBoxDataField*> getField(size_t index) const;
      void render2D() const override;

   protected:
      Widget* _unhandled_input(const InputEvent&) override;
      [[nodiscard]] std::optional<size_t> getIndexAt(const Pos<float>&) const;
      void _on_modality_lost() override;
      std::string toolTip;
      std::vector<std::unique_ptr<ComboBoxDataField>> fields; //dont want addresses to change
      std::optional<size_t> currentIndex;

   private:
      float fieldSelectionRectHeight{};
      bool _isOpen = false;
      Rect<int> _selectionMenuRect;
      std::vector<Rect<float>> _selectionMenuItemRects;

   };
}