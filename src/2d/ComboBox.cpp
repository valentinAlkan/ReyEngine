#include "ComboBox.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::render() const {
   //draw the background
   drawRectangle(getRect().toSizeRect(), theme->background.colorPrimary);

   auto& font = theme->font;

   auto textheight = font.value.size;
   //available vertical height
   auto availableHeight= getRect().height;
   auto textPosV = (int)((availableHeight - textheight) / 2);

   //draw current item
   if (fields.size()){
      drawText(fields.value.at(currentIndex).text, {1, textPosV}, font);
   }

   //draw if the menu is open
   if (_isOpen){
      drawRectangle(_selectionMenuRect, Colors::gray);
      for (int i=0; i < fields.size(); i++){
         auto& field = fields.value.at(i);
         auto& rect = _selectionMenuItemRects.at(i);
         if (field.highlighted){
            drawRectangle(rect, theme->highlight);
         }
         drawText(field.text, rect.pos(), font);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled ComboBox::_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
   auto closeMenu = [this]() {
      setModal(false);
      _isOpen = false;
      publish<EventComboBoxMenuClosed>(EventComboBoxMenuClosed(toEventPublisher()));
   };

   auto openMenu = [this]() {
      setModal(true);
      auto textSize = theme->font.value.size;
      //build the menu rect
      _selectionMenuRect = Rect<int>(0, getHeight(), getWidth(), textSize * fields.size());
      //build the item rects
      _selectionMenuItemRects.clear();
      for (int i=0; i < fields.size(); i++) {
         fieldSelectionRectHeight = theme->font.value.size;
         auto itemWidth= getRect().width;
         auto itemY= getRect().height + (i * fieldSelectionRectHeight);
         _selectionMenuItemRects.emplace_back(0, itemY, itemWidth, fieldSelectionRectHeight);
      }
   };

   //only accept inputs that are inside the main rect unless we are modal
   if (!mouse->isInside && !isModal()) return false;
   //open the menu
   if (mouse){
      switch (event.eventId){
         case InputEventMouseMotion::getUniqueEventId():{
            auto mmEvent = event.toEventType<InputEventMouseMotion>();
            if (_isOpen){
               //highlight the field
               auto indexAt = getIndexAt(mouse.value().localPos);
               if (indexAt) {
                  for (int i=0; i<fields.size();i++) {
                     auto& field = fields.get(i);
                     if (indexAt) {
                        field.highlighted = indexAt.value() == i;
                        if (field.highlighted){
                           EventComboBoxItemHovered hoverEvent(toEventPublisher(), i, field);
                           publish(hoverEvent);
                        }
                     }
                  }
                  return true;
               }
            }
            break;}
         case InputEventMouseButton::getUniqueEventId():{
            auto mbEvent = event.toEventType<InputEventMouseButton>();
            if (mbEvent.isDown) return false;
            if (mouse.value().isInside){
               //toggle menu visibility the menu
               _isOpen = !_isOpen;
               if (_isOpen){
                  //declare widget modal
                  openMenu();
                  publish<EventComboBoxMenuOpened>(EventComboBoxMenuOpened(toEventPublisher()));
               } else {
                  closeMenu();
               }
               return true;
            } else {
               //if the mouse was outside the widget's own rect, but the widget is receiving modal input
               if (_selectionMenuRect.isInside(mouse.value().localPos)) {
                  //the mouse is inside the selectionmenu rect - treat it as item selection input
                  auto indexAt = getIndexAt(mouse.value().localPos);
                  if (indexAt) {
                     //select the item at hte mouse position
                     currentIndex = indexAt.value();
                     EventComboBoxItemSelected selectEvent(toEventPublisher(), indexAt.value(), getField(indexAt.value()));
                     publish(selectEvent);
                  }
               }
               //close the menu
               closeMenu();
               return true;
            }
            break;}
      }
   }
return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
ComboBoxDataField& ComboBox::addItem(const std::string &s) {
   auto& field = fields.append();
   field.text = s;

   //recalculate selection menu
   auto textSize = theme->font.value.size;
   return field;
}
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::setItems(const std::vector<std::string>& v) {
   for (auto& s : v){
      addItem(s);
   }
}
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::clear() {
   fields.clear();
   _selectionMenuRect = Rect<int>(0, theme->font.value.size, getWidth(), 5);
   _selectionMenuItemRects.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::eraseItem(int index) {
   fields.erase(index);
}
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::setCurrentIndex(int index) {
   currentIndex = index;
}

/////////////////////////////////////////////////////////////////////////////////////////
ComboBoxDataField &ComboBox::getLastField() {
   return fields.value.at(fields.value.size() - 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
int ComboBox::getCurrentIndex() const {
   return currentIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string ComboBox::text() const {
   return fields.value.at(currentIndex).text;
}

/////////////////////////////////////////////////////////////////////////////////////////
ComboBoxDataField& ComboBox::getCurrentField() {
   return fields.value.at((int)currentIndex);
}


/////////////////////////////////////////////////////////////////////////////////////////
ComboBoxDataField &ComboBox::getField(int index) {
   return fields.value.at(index);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<int> ComboBox::getIndexAt(Pos<int> pos) {
   //get the item in the dropdown list at the specified location
   for (int i=0; i<fields.size();i++){
      auto& selectionRect = _selectionMenuItemRects.at(i);
      if (selectionRect.isInside(pos)){
         return i;
      }
   }
   return nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::_on_modality_lost() {
   _isOpen = false;
}