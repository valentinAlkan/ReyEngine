#include "ComboBox.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::render() const {
   //draw the background
   _drawRectangle(_rect.value.toSizeRect(), theme->background.colorPrimary);

   auto& font = theme->font;

   auto textheight = font.value.size;
   //available vertical height
   auto availableHeight = _rect.value.height;
   auto textPosV = (int)((availableHeight - textheight) / 2);

   //draw current item
   if (data.size()){
      _drawText(data.value.at(currentIndex).text, {0,textPosV}, font);
   }

   //draw if the menu is open
   if (_isOpen){
      _drawRectangle(_selectionMenuRect, Colors::blue);
      for (int i=0; i<data.size(); i++){
         auto& field = data.value.at(i);
         _drawText(field.text, {0,textPosV + (i*font.value.size)}, font);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled ComboBox::_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
   auto closeMenu = [this]() {
      setModal(false);
      _isOpen = false;
   };

   auto openMenu = [this]() {
      setModal(true);
      auto textSize = theme->font.value.size;
      _selectionMenuRect = Rect<int>(0, textSize, _rect.value.width, textSize * data.size());
   };

   if (!mouse->isInside && !isModal()) return false;
   cout << "Hello from " << getName() << "@" << mouse->localPos << endl;
   //open the menu
   if (mouse){
      switch (event.eventId){
         case InputEventMouseMotion::getUniqueEventId():{
            auto mmEvent = event.toEventType<InputEventMouseMotion>();
            if (_isOpen){
               //highlight the field
               auto indexAt = getIndexAt(mouse.value().localPos);
               if (indexAt) {
                  cout << getField(indexAt.value()).text.value << endl;
                  for (auto &field: data.value) {
                     field.highlighted = indexAt.value() == currentIndex.value;
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
               } else {
                  closeMenu();
               }
               return true;
            } else {
               //if the mouse was outside the widget's own rect, but the widget is receiving modal input
               if (_selectionMenuRect.isInside(mouse.value().localPos)){
                  //highlight the item at the mouse position
                  auto indexAt = getIndexAt(mouse.value().localPos);
                  if (indexAt) {

                     currentIndex = indexAt.value();
                     return true;
                  } else {
                     //close menu
                     closeMenu();
                     return true;
                  }
               } else {
                  //close the menu
                  closeMenu();
                  return true;
               }
            }
            break;}
      }
   }
return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
ComboBoxDataField& ComboBox::addItem(const std::string &s) {
   auto& field = data.append();
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
   data.clear();
   _selectionMenuRect = Rect<int>(0, theme->font.value.size, _rect.value.width, 5);
}
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::eraseItem(int index) {
   data.erase(index);
}
/////////////////////////////////////////////////////////////////////////////////////////
void ComboBox::setCurrentIndex(int index) {
   currentIndex = index;
}
/////////////////////////////////////////////////////////////////////////////////////////
int ComboBox::getCurrentIndex() const {
   return currentIndex;
}
/////////////////////////////////////////////////////////////////////////////////////////
std::string ComboBox::text() const {
   return data.value.at(currentIndex).text;
}
/////////////////////////////////////////////////////////////////////////////////////////
ComboBoxDataField& ComboBox::getCurrentField() {
   return data.value.at((int)currentIndex);
}
/////////////////////////////////////////////////////////////////////////////////////////
ComboBoxDataField &ComboBox::getField(int index) {
   return data.value.at(index);
}
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<int> ComboBox::getIndexAt(Pos<int> pos) {
   //get the item in the dropdown list at the specified location
   int itemIndex = pos.y / theme->font.value.size - 1;
   if (itemIndex < 0 || itemIndex >= data.size()){
      //selected the combobox itself
      return nullopt;
   }
   return itemIndex;
}