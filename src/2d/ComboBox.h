#pragma once
#include "Widget.h"

namespace ReyEngine {

   struct ComboBoxData{};

   template <typename T>
   struct ComboBoxDataField {
      ComboBoxDataField(const std::string& text, const T& data)
      : text(text)
      , checkable(true)
      , checked(true)
      , data(data)
      {}
      std::string text;
      bool checkable;
      bool checked;
      bool highlighted = false;
      const T data;
   };

   template <typename T=int>
   class ComboBox : public Widget {
      static constexpr float DROP_HANDLE_WIDTH = 20;
   public:
      EVENT_ARGS(EventComboBoxItemSelected, 654654654, size_t itemIndex, const ComboBoxDataField<T>* field)
         , itemIndex(itemIndex)
         , field(field)
         {}
         const size_t itemIndex;
         const ComboBoxDataField<T>* field;
      };

      EVENT_ARGS(EventComboBoxItemHovered, 654654655, size_t itemIndex, const ComboBoxDataField<T>* field)
         , field(field)
         , itemIndex(itemIndex)
         {}
         const size_t itemIndex;
         const ComboBoxDataField<T>* field;
      };
      EVENT(EventComboBoxMenuClosed, 654654656){}};
      EVENT(EventComboBoxMenuOpened, 654654657){}};

      REYENGINE_OBJECT(ComboBox)
      ComboBox() {
         acceptsHover = true;
         setMaxSize(std::numeric_limits<float>::max(), theme->font.size);
      }
      constexpr ComboBox(const auto& items)
      : ComboBox()
      {
         setItems(items);
      }
      template <typename I>
      constexpr void setItems(const I& items, size_t _currentIndex=0) {
         for (auto& s : items){
            addItem(s);
         }
         setCurrentIndex(_currentIndex);
      }
      void clear() {
         fields.clear();
         _selectionMenuRect = Rect<float>(0, theme->font.size, getWidth(), 5);
         _selectionMenuItemRects.clear();
      }
      void eraseItem(size_t index) {
         if (index < fields.size()) {
            fields.erase(fields.begin() + (int64_t) index);
         }
      }
      void setCurrentIndex(size_t index) {
         if (index < fields.size()) {
            currentIndex = index;
         }
      }
      inline ComboBoxDataField<T>* addItem(const std::string& item) {
         static_assert(std::is_arithmetic_v<T>, "When using default addItem, T must be arithmetic in nature (int/uint/size_t, etc). "
                                                "Otherwise you must explicitly specify the stored value using one of the "
                                                "other addItem functions, since we cannot meaningfully infer what value T "
                                                "should map to in the default case");
         //guess what the next data value should be. Increment last field's value.
         if (auto hasField = getLastField()) {
            return addItem(item, hasField.value()->data + 1);
         }
         //try setting a zero value if there are no fields.
         return addItem(item, 0);
      }
      inline ComboBoxDataField<T>* addItem(const std::pair<std::string, T>& item) {return addItem(item.first, item.second);}
      inline ComboBoxDataField<T>* addItem(const std::string& s, const T& data){
         auto added = fields.emplace_back(std::make_unique<ComboBoxDataField<T>>(s, data)).get();
         if (!currentIndex) currentIndex = 0;
         return added;
      }
      inline void addItems(const std::vector<std::string> v) {
         for (const auto& item : v) {
            addItem(item);
         }
      }
      inline void addItems(const auto& kvp_iterable) {
         for (const auto& [key, value]: kvp_iterable) {
            addItem(key, value);
         }
      }
      [[nodiscard]] inline std::optional<size_t> getCurrentIndex() const {return currentIndex;}
      [[nodiscard]] inline size_t getItemCount() const {return fields.size();}
      [[nodiscard]] std::optional<std::string> text() const {
         if (!currentIndex) return {};
         return fields.at(currentIndex.value())->text;
      }
      [[nodiscard]] std::optional<ComboBoxDataField<T>*> getLastField() const {
         if (fields.empty()) return {};
         return fields.back().get();
      }
      [[nodiscard]] std::optional<ComboBoxDataField<T>*> getCurrentField() const {
         if (!currentIndex) return {};
         return fields.at(currentIndex.value()).get();
      }
      [[nodiscard]] std::optional<ComboBoxDataField<T>*> getField(size_t index) const{
         if (index >= fields.size()) return {};
         return fields.at(index).get();
      }
      void render2D() const override{
         //draw the background
         drawRectangle(getSizeRect(), theme->background.colorPrimary);
         drawRectangleLines(getSizeRect(), 1.0, theme->background.colorSecondary);

         //draw the dropdown arrow
         //configure the drop handle
         auto dropRect = getSizeRect().chopLeft(getWidth()-DROP_HANDLE_WIDTH);
         if (_isOpen) {
            //color in rect if its open
            drawRectangle(dropRect, theme->background.colorActive);
            drawRectangleLines(dropRect, 1.0, theme->background.colorSecondary);
         }
         //draw drop arrow
         auto center = dropRect.center();
         auto line = center.to(dropRect.top().midpoint()).scale(80);
//         line += Pos<float>(0, -3);
         static constexpr auto rotate_amt = Degrees(60);
         drawLine(line.copy().rotate(center, rotate_amt), 2.0, theme->background.colorSecondary);
         drawLine(line.rotate(center, -rotate_amt), 2.0, theme->background.colorSecondary);


         auto& font = theme->font;

         auto textheight = font.size;
         //available vertical height
         auto availableHeight= getRect().height;
         auto textPosV = (availableHeight - textheight) / 2;

         //draw current item
         if (auto hasField = getCurrentField()){
            drawText(hasField.value()->text, {1, textPosV}, font);
         }

         //draw if the menu is open
         if (_isOpen){
            drawRectangle(_selectionMenuRect, Colors::gray);
            for (int i=0; i < fields.size(); i++){
               auto& field = fields.at(i);
               auto& rect = _selectionMenuItemRects.at(i);
               if (field->highlighted){
                  drawRectangle(rect, theme->highlight.colorPrimary);
               }
               drawText(field->text, rect.pos(), font);
            }
         }
      }

   protected:
      Widget* _unhandled_input(const InputEvent& event) override {
         auto closeMenu = [this]() {
            setModal(false);
            _isOpen = false;
            publish<EventComboBoxMenuClosed>(EventComboBoxMenuClosed(this));
         };

         auto openMenu = [this]() {
            setModal(true);
            auto textSize = theme->font.size;
            //build the menu rect
            _selectionMenuRect = Rect<float>(0, getHeight(), getWidth(), textSize * fields.size());
            //build the item rects
            _selectionMenuItemRects.clear();
            for (int i = 0; i < fields.size(); i++) {
               fieldSelectionRectHeight = theme->font.size;
               auto itemWidth = getRect().width;
               auto itemY = getRect().height + (i * fieldSelectionRectHeight);
               _selectionMenuItemRects.emplace_back(0, itemY, itemWidth, fieldSelectionRectHeight);
            }
         };

         if (auto isMouse = event.isMouse()) {
            auto& mouse = isMouse.value();
            //only accept inputs that are inside the main rect unless we are modal
            if (!mouse->isInside() && !isModal()) return nullptr;
            //open the menu
            if (mouse) {
               switch (event.eventId) {
                  case InputEventMouseMotion::getUniqueEventId(): {
                     if (_isOpen) {
                        //highlight the field
                        auto indexAt = getIndexAt(mouse->getLocalPos());
                        if (indexAt) {
                           for (int i = 0; i < fields.size(); i++) {
                              auto& field = fields.at(i);
                              if (indexAt) {
                                 field->highlighted = indexAt.value() == i;
                                 if (field->highlighted) {
                                    EventComboBoxItemHovered hoverEvent(this, i, field.get());
                                    publish(hoverEvent);
                                 }
                              }
                           }
                           return this;
                        }
                     }
                     break;
                  }
                  case InputEventMouseButton::getUniqueEventId(): {
                     auto& mbEvent = event.toEvent<InputEventMouseButton>();
                     if (mbEvent.isDown) return nullptr;
                     if (mouse->isInside()) {
                        //toggle menu visibility
                        _isOpen = !_isOpen;
                        if (_isOpen) {
                           //declare widget modal
                           openMenu();
                           publish<EventComboBoxMenuOpened>(EventComboBoxMenuOpened(this));
                        } else {
                           closeMenu();
                        }
                        return this;
                     } else {
                        //if the mouse was outside the widget's own rect, but the widget is receiving modal input
                        if (_selectionMenuRect.contains(mouse->getLocalPos())) {
                           //the mouse is inside the selectionmenu rect - treat it as item selection input
                           auto indexAt = getIndexAt(mouse->getLocalPos());
                           if (indexAt) {
                              //select the item at hte mouse position
                              currentIndex = indexAt.value();
                              EventComboBoxItemSelected selectEvent(this, indexAt.value(),
                                                                    getField(indexAt.value()).value());
                              publish(selectEvent);
                           }
                        }
                        //close the menu
                        closeMenu();
                        return this;
                     }
                     break;
                  }
                  case InputEventMouseWheel::getUniqueEventId():
                     auto& mwEvent = event.toEvent<InputEventMouseWheel>();
//               mwEvent.wheelMove.pct()
                     break;
               }
            }
         }
         return nullptr;
      }
      [[nodiscard]] std::optional<size_t> getIndexAt(const Pos<float>& pos) const {
         //get the item in the dropdown list at the specified location
         for (int i = 0; i < fields.size(); i++) {
            auto& selectionRect = _selectionMenuItemRects.at(i);
            if (selectionRect.contains(pos)) {
               return i;
            }
         }
         return {};
      }
      void _on_modality_lost() override { _isOpen = false;}
      std::string toolTip;
      std::vector<std::unique_ptr<ComboBoxDataField<T>>> fields; //dont want addresses to change
      std::optional<size_t> currentIndex;

   private:
      float fieldSelectionRectHeight{};
      bool _isOpen = false;
      Rect<int> _selectionMenuRect;
      std::vector<Rect<float>> _selectionMenuItemRects;

   };
}