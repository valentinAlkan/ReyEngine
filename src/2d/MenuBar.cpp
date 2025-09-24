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
void Internal::MenuInterface::push_back(const std::vector<std::string>& entries) {
   for (const auto& entry : entries) {
      _entries.emplace_back(std::make_unique<MenuEntry>(this, nullptr, entry));
   }
   _on_change();
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
   if (_activeEntry){
      drawRectangleGradientH(_activeEntry.value()->_area, GRADIENT_1, GRADIENT_2);
   }
   drawRectangleLines(getSizeRect(), 1.0, Colors::black);
   for (const auto& entry : _entries){
      drawText(entry->_text, entry->_area.copy().pushY(2).pos(), theme->font);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void DropDownMenu::_on_change() {
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
   setMinSize(maxWidth, r.y);
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
         _activeEntry = at(mhEvent.mouse.getLocalPos());
         return this;}
      case InputEventMouseButton::ID:{
         auto& mbEvent = e.toEvent<InputEventMouseButton>();
         _itemDown = _activeEntry ? mbEvent.isDown : false;
         if (!mbEvent.isDown) {
            for (auto &entry: _entries) {
               if (entry->_area.contains(mbEvent.mouse.getLocalPos())) {
                  showDropDown(entry->_text, entry->_area.bottomLeft());
               }
            }
         }
         return this;}
   }
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void MenuBar::render2D() const {
   if (_activeEntry){
      auto color1 = _itemDown ? GRADIENT_1 : GRADIENT_2;
      auto color2 = _itemDown ? GRADIENT_2 : GRADIENT_1;
      drawRectangleGradientH(_activeEntry.value()->_area, color1, color2);
   }
   drawRectangleLines(getSizeRect(), 1.0, Colors::black);
   for (const auto& entry : _entries){
      drawText(entry->_text, entry->_area.copy().pushX(5).pos(), theme->font);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<DropDownMenu> MenuBar::createDropDown(ReyEngine::Internal::MenuInterface::MenuEntry* entry) {
   auto dropDown = make_child<DropDownMenu>(getNode(), DROP_DOWN_PREFIX + entry->_text);
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