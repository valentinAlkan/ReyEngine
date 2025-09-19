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
   _btnOk->setEnabled(false);

   _btnBack = make_child<PushButton>(navBar->getNode(), "btnBack", "<-");
   _btnForward = make_child<PushButton>(navBar->getNode(), "btnForward", "->");
   _btnUp = make_child<PushButton>(navBar->getNode(), "btnUp", "^");
   _addrBar = make_child<AddrBar>(navBar->getNode(), "addrBar", _dir);
   navBar->layoutRatios = {1,1,1,20};

   _systemBrowserTree = make_child<Tree>(_systemBrowserScrollArea->getNode(), "systemBrowserTree");
   auto label = make_child<Label>(_systemBrowserScrollArea->getNode(), "label", "this right here is some text yall");

   _addrBar->subscribe<AddrBar::EventAddrEntered>(_addrBar, [&](const AddrBar::EventAddrEntered& e){_on_addr_entered(e);});

   setMinSize(200,200);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::refreshDirectoryContents() {
   vector<Directory> dirs;
   vector<File> files;
   _directoryTree->setRoot(_directoryTree->createItem(_dir.str(), std::make_optional(std::pair<std::string, Path>(VAR_PATH, _dir))));
   auto parentItem = _directoryTree->getRoot().value();
   auto itemPath = parentItem->getMetaData<Path>(VAR_PATH);
   if (!itemPath->isDirectory()) return;
   for (const auto& entry : Directory(itemPath.value()).listContents()) {
      auto p = std::make_optional(std::pair<std::string, Path>(VAR_PATH, entry));
      auto text = entry.tail();
      if (entry.isDirectory()) text += FILESYSTEM_PATH_SEP;
      parentItem->push_back(_directoryTree->createItem(text, p));
   }
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
/////////////////////////////////////////////////////////////////////////////////////////
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