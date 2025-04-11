#include "ComboBox.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::render2D() const {
   //draw the background
   drawRectangle(getRect().toSizeRect(), theme->background.colorPrimary);

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

/////////////////////////////////////////////////////////////////////////////////////////
Widget* ComboBox::_unhandled_input(const InputEvent& event) {
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
      for (int i=0; i < fields.size(); i++) {
         fieldSelectionRectHeight = theme->font.size;
         auto itemWidth= getRect().width;
         auto itemY= getRect().height + (i * fieldSelectionRectHeight);
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
                  if (_selectionMenuRect.isInside(mouse->getLocalPos())) {
                     //the mouse is inside the selectionmenu rect - treat it as item selection input
                     auto indexAt = getIndexAt(mouse->getLocalPos());
                     if (indexAt) {
                        //select the item at hte mouse position
                        currentIndex = indexAt.value();
                        EventComboBoxItemSelected selectEvent(this, indexAt.value(), getField(indexAt.value()).value());
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

/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::setItems(const std::vector<std::string>& v, size_t currentIndex) {
   for (auto& s : v){
      addItem(s);
   }
   setCurrentIndex(currentIndex);
}
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::clear() {
   fields.clear();
   _selectionMenuRect = Rect<float>(0, theme->font.size, getWidth(), 5);
   _selectionMenuItemRects.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::eraseItem(size_t index) {
   if (index < fields.size()) {
      fields.erase(fields.begin() + (int64_t)index);
   }
}
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::setCurrentIndex(size_t index) {
   if (index < fields.size()) {
      currentIndex = index;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<ComboBoxDataField*> ComboBox::getLastField() const {
   if (fields.empty()) return {};
   return fields.back().get();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::string> ComboBox::text() const {
   if (!currentIndex) return {};
   return fields.at(currentIndex.value())->text;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<ComboBoxDataField *> ComboBox::getCurrentField() const {
   if (!currentIndex) return {};
   return fields.at(currentIndex.value()).get();
}


/////////////////////////////////////////////////////////////////////////////////////////
std::optional<ComboBoxDataField*> ComboBox::getField(size_t index) const {
   if (index >= fields.size()) return {};
   return fields.at(index).get();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<size_t> ComboBox::getIndexAt(const Pos<float>& pos) const {
   //get the item in the dropdown list at the specified location
   for (int i=0; i<fields.size();i++){
      auto& selectionRect = _selectionMenuItemRects.at(i);
      if (selectionRect.isInside(pos)){
         return i;
      }
   }
   return {};
}

/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::_on_modality_lost() {
   _isOpen = false;
}