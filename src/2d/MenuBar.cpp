#include "MenuBar.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void Internal::MenuInterface::MenuEntry::setText(const std::string& text) {
   _text = text;
   _interface->_on_change();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Internal::MenuInterface::MenuEntry::setIcon(std::shared_ptr<ReyTexture>& icon) {
   _icon = icon;
   _interface->_on_change();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Internal::MenuInterface::MenuEntry* Internal::MenuInterface::push_back(std::unique_ptr<MenuInterface::MenuEntry>&& entry) {
   _entries.push_back(std::move(entry));
   _on_change();
   return _entries.back().get();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<Internal::MenuInterface::MenuEntry*> Internal::MenuInterface::at(Pos<float>& p) {
   for (const auto& entry : _entries){
      if (entry->_area.contains(p)){return entry.get();}
   }
   return {};
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void DropDownMenu::_init() {
   setMinWidth(200);
}

/////////////////////////////////////////////////////////////////////////////////////////
void DropDownMenu::render2D() const {
   drawRectangle(getSizeRect(), theme->background.colorPrimary);
   if (_activeEntry){
      drawRectangle(_activeEntry.value()->_area, theme->background.colorHighlight);
   }
   drawRectangleLines(getSizeRect(), 1.0, theme->background.colorSecondary);
   for (const auto& entry : _entries){
      drawText(entry->_text, entry->_area.copy().pushX(2).pushY(2).pos(), theme->font);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void DropDownMenu::_on_change() {
   setMinSize(_calculateSize());
}

/////////////////////////////////////////////////////////////////////////////////////////
Size<float> DropDownMenu::_calculateSize() {
   static constexpr float SPACING = 4;
   Rect<float> r;
   float maxWidth = 0;
   float yPos = 0;
   for (auto& entry : _entries){
      auto textSize = measureText(entry->_text, theme->font);
      r.width = getWidth();
      r.height = textSize.y + SPACING;
      entry->_area = r;
      r.y = yPos += r.height;
      maxWidth = std::max(maxWidth, r.width);
   }
   return {maxWidth, r.y};
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget *DropDownMenu::_unhandled_input(const ReyEngine::InputEvent& e) {
   switch (e.eventId){
      case InputEventMouseMotion::ID:{
         auto& mhEvent = e.toEvent<InputEventMouseMotion>();
         _activeEntry = at(mhEvent.mouse.getLocalPos());
         return this;}
      case InputEventMouseButton::ID:{
         auto& mbEvent = e.toEvent<InputEventMouseButton>();
         if (!mbEvent.isDown) {
            if (!mbEvent.mouse.isInside()) {
               setModal(false);
               setVisible(false);
               return this;
            }
            for (auto &entry: _entries) {
               if (entry->_area.contains(mbEvent.mouse.getLocalPos())) {
                  close();
                  publish(EventItemSelected(this, entry.get()));
               }
            }
         }
         return this;}
   }
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void MenuBar::_init() {
   setMinHeight(22);
   setAcceptsHover(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
void MenuBar::_on_change() {
   _lastDrop = nullptr;
   static constexpr float SPACING = 15;
   Rect<float> r = getSizeRect();
   for (auto& entry : _entries){
      r.width =  measureText(entry->_text, theme->font).x + SPACING;
      entry->_area = r;
      r.x += r.width;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* MenuBar::_unhandled_input(const ReyEngine::InputEvent& e) {
   switch (e.eventId){
      case InputEventMouseMotion::ID:{
         auto& mhEvent = e.toEvent<InputEventMouseMotion>();
         if (isFocused() || mhEvent.mouse.isInside()) {
            _activeEntry = at(mhEvent.mouse.getLocalPos());
            setFocused(_activeEntry.has_value());
         } else {
            _activeEntry.reset();
            return nullptr;
         }
         return this;}
      case InputEventMouseButton::ID:{
         auto& mbEvent = e.toEvent<InputEventMouseButton>();
         _itemDown = _activeEntry ? mbEvent.isDown : false;
         if (!mbEvent.isDown) {
            for (auto &entry: _entries) {
               if (entry->_area.contains(mbEvent.mouse.getLocalPos())) {
                  showDropDown(entry->_text, entry->_area.bottomLeft());
                  return this;
               }
            }
         }
      }
   }
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void MenuBar::render2D() const {
   if (_activeEntry){
      drawRectangle(_activeEntry.value()->_area, theme->background.colorHighlight);
   }
   drawRectangleLines(getSizeRect(), 1.0, theme->background.colorSecondary);
   for (const auto& entry : _entries){
      drawText(entry->_text, entry->_area.copy().pushX(5).pushY(2).pos(), theme->font);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<DropDownMenu> MenuBar::createDropDown(const std::string& name) {
   auto menuEntry = push_back(name);
   auto dropDown = make_child<DropDownMenu>(this, DROP_DOWN_PREFIX + name);
   dropDown->setVisible(false);
   return dropDown;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<DropDownMenu*> MenuBar::getDropDown(const std::string& menu) {
   for (const auto& child : getChildren()){
      if (child->getName() == DROP_DOWN_PREFIX + menu) return child->as<DropDownMenu>().value();
   }
   return {};
}

/////////////////////////////////////////////////////////////////////////////////////////
void MenuBar::showDropDown(const std::string& menu, const Pos<float>& pos) {
   if (auto dropDownOpt = getDropDown(menu)){
      auto dropDown = dropDownOpt.value();
      dropDown->open();
      dropDown->setPosition(pos);
   } else {
      Logger::warn() << "No menu entry for " << menu << endl;
   }
}