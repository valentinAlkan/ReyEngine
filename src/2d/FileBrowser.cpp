#include "FileBrowser.h"
#include "MiscTools.h"
#include "Label.h"

using namespace std;
using namespace ReyEngine;
using namespace FileSystem;

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::render2D() const {
   drawRectangleLines(getSizeRect(), 2.0, Colors::black);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_rect_changed(){
   _layout->setRect(getSizeRect().embiggen(-20));
   _systemBrowserTree->setWidth(_systemBrowserScrollArea->getWidth());

}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_init() {
   _layout = make_child<Layout>(getNode(), "layout", Layout::LayoutDir::VERTICAL);
   auto navBar = make_child<Layout>(_layout->getNode(), "navBar", Layout::LayoutDir::HORIZONTAL);
   auto browser = make_child<Layout>(_layout->getNode(), "browser", Layout::LayoutDir::HORIZONTAL);
   _systemBrowserScrollArea = make_child<ScrollArea>(browser->getNode(), "systemBrowserScrollArea");
   _directoryScrollArea = make_child<ScrollArea>(browser->getNode(), "directoryScrollArea");

   auto footer = make_child<Layout>(_layout->getNode(), "footer", Layout::LayoutDir::HORIZONTAL);

   navBar->setMaxHeight(30);
   footer->setMaxHeight(60);

   _layout->layoutRatios = {1, 4, 1};
   browser->layoutRatios = {1, 4};
   _directoryTree = make_child<Tree>(_directoryScrollArea->getNode(), "directoryTree");
   _directoryTree->setAllowSelect(true);
   _directoryTree->setHideRoot(true);
//   _directoryTree

   auto footerL = make_child<Layout>(footer->getNode(), "footerL", Layout::LayoutDir::HORIZONTAL);
   auto footerR = make_child<Layout>(footer->getNode(), "footerR", Layout::LayoutDir::HORIZONTAL);

   _filterText = make_child<LineEdit>(footerL->getNode(), "_filterText");
   _filterType = make_child<ComboBox<string>>(footerL->getNode(), "_filterType");

   _btnOk = make_child<PushButton>(footerR->getNode(), "btnOk", "Ok");
   _btnCancel = make_child<PushButton>(footerR->getNode(), "btnCancel", "Cancel");
   _btnOk->subscribe<Tree::EventItemSelected>(_directoryTree, [this](const Tree::EventItemSelected& e){_on_item_selected(e);});
   _btnOk->subscribe<Tree::EventItemDeselected>(_directoryTree, [this](const Tree::EventItemDeselected& e){_on_item_deselected(e);});
   _directoryTree->subscribe<Tree::EventItemDoubleClicked>(_directoryTree, [this](const Tree::EventItemDoubleClicked& e){_on_item_doubleClicked(e);});
   _btnOk->setEnabled(false);

   _btnBack = make_child<PushButton>(navBar->getNode(), "btnBack", "<-");
   _btnFwd = make_child<PushButton>(navBar->getNode(), "btnForward", "->");
   _btnUp = make_child<PushButton>(navBar->getNode(), "btnUp", "^");
   _addrBar = make_child<AddrBar>(navBar->getNode(), "addrBar", _dir);
   navBar->layoutRatios = {1,1,1,20};

   _systemBrowserTree = make_child<Tree>(_systemBrowserScrollArea->getNode(), "systemBrowserTree");
   auto label = make_child<Label>(_systemBrowserScrollArea->getNode(), "label", "this right here is some text yall");

   _addrBar->subscribe<AddrBar::EventAddrEntered>(_addrBar, [&](const AddrBar::EventAddrEntered& e){_on_addr_entered(e);});
   _btnUp->subscribe<PushButton::ButtonPressEvent>(_btnUp, [&](const PushButton::ButtonPressEvent& e){ _on_up(e);});
   _btnFwd->subscribe<PushButton::ButtonPressEvent>(_btnFwd, [&](const PushButton::ButtonPressEvent& e){ _on_fwd(e);});
   _btnBack->subscribe<PushButton::ButtonPressEvent>(_btnBack, [&](const PushButton::ButtonPressEvent& e){ _on_back(e);});

   setMinSize(200,200);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::refreshDirectoryContents() {
   vector<Directory> dirs;
   vector<File> files;
   _directoryTree->setRoot(_directoryTree->createItem(_dir.str(), std::make_optional(std::pair<std::string, Path>(VAR_PATH, _dir))));
   auto parentItem = _directoryTree->getRoot().value();
   auto itemPath = parentItem->getMetaData<Path>(VAR_PATH);
   if (!itemPath) throw std::runtime_error("Directory entry " + parentItem->getText() + " did not have a directory associated with it");
   for (const auto& entry : Directory(itemPath.value()).listContents()) {
      auto p = std::make_optional(std::pair<std::string, Path>(VAR_PATH, entry));
      if (entry.isDirectory()){
         dirs.emplace_back(entry);
      } else if (entry.isRegularFile()){
         files.emplace_back(entry);
      }
   }
   for (const auto& dir : dirs){
      parentItem->push_back(_directoryTree->createItem(dir.tail() + FILESYSTEM_PATH_SEP));
      parentItem->back()->setMetaData<Path>(VAR_PATH, dir);
   }
   for (const auto& file : files){
      parentItem->push_back(_directoryTree->createItem(file.tail()));
      parentItem->back()->setMetaData<Path>(VAR_PATH, file);
   }
   _directoryTree->setSize(_directoryTree->measureContents());
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_item_selected(const Tree::EventItemSelected& event) {
   Logger::info() << "Selected item " << *event.item << endl;
   _btnOk->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_item_deselected(const Tree::EventItemDeselected& event) {
   Logger::info() << "Deselected item " << *event.item << endl;
   _btnOk->setEnabled(false);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_item_doubleClicked(const Tree::EventItemDoubleClicked& e) {
   if (auto metaData = e.item->getMetaData<Path>(VAR_PATH)){
      auto dir = metaData.value();
      if (dir.isDirectory()){
         setCurrentDirectory(dir);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_addr_entered(const AddrBar::EventAddrEntered& e) {
   Logger::info() << "Addr entered : " << e.addr << endl;
   //validate the addr
   auto newDir = FileSystem::Directory(e.addr);
   if (newDir.exists() && newDir.isDirectory()) {
      setCurrentDirectory(newDir);
   } else {
      setCurrentDirectory(_dir);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_up(const PushButton::ButtonPressEvent&) {
   if (auto parent = _dir.getParent()){
      setCurrentDirectory(parent.value());
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_back(const PushButton::ButtonPressEvent&) {
   if (auto back = _history.back()){
      _setCurrentDirectory(back.value());
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_fwd(const PushButton::ButtonPressEvent&) {
   if (auto fwd = _history.fwd()){
      _setCurrentDirectory(fwd.value());
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::AddrBar::_init() {
   subscribe<LineEdit::EventLineEditTextEntered>(this, [&](const LineEdit::EventLineEditTextEntered& e){
      EventAddrEntered event(this, getText());
      publish(event);
   });
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::AddrBar::setDir(FileSystem::Directory& dir) {
   _currentDir = dir;
   setText(_currentDir.str());
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::AddrBar::render2D() const {
   LineEdit::render2D();
   if (_hoveredDirToken){
      auto& token = _hoveredDirToken.value();
      drawRectangleLines(token->highlightRect, 1.0, Colors::lightGray);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* FileBrowser::AddrBar::_unhandled_input(const ReyEngine::InputEvent& event) {
   auto handled = LineEdit::_unhandled_input(event);
   if (handled) return handled;

   switch (event.eventId){
      case InputEventKey::ID:
         auto keyEvent = event.toEvent<InputEventKey>();
         if (!keyEvent.isDown) return nullptr;
         switch (keyEvent.key){
            case InputInterface::KeyCode::KEY_ENTER:{
               EventAddrEntered addrEntered(this, getText());
               publish(addrEntered);
               return this;}
            default:
               break;
         }
   }
   //clicking on a directory token highlights it
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::History::add(const FileSystem::Directory& dir) {
   if (_ptr != _dirs.size()) {
      //overwrite at ptr+1;
      auto iter = _dirs.begin() + (long long)_ptr + 1;
      //erase everything after that point
      _dirs.erase(iter, _dirs.end());
   }
   _dirs.push_back(dir);
   _ptr=_dirs.size()-1;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<FileSystem::Directory> FileBrowser::History::back() {
   if (!_dirs.empty() && _ptr <= _dirs.size()){
      if (_ptr >= 1){
         _ptr--;
      }
      return _dirs.at(_ptr);
   }
   return {};
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<FileSystem::Directory> FileBrowser::History::fwd() {
   if (hasFwd()){
      return _dirs.at(++_ptr);
   }
   return {};
}

/////////////////////////////////////////////////////////////////////////////////////////
bool FileBrowser::History::hasBack() {
   return !_dirs.empty() && _ptr < _dirs.size() && _ptr > 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool FileBrowser::History::hasFwd() {
   return !_dirs.empty() && _ptr < _dirs.size() - 1; // never test size_t against -1
}
