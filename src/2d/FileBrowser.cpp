#include "FileBrowser.h"
#include "MiscTools.h"
#include "Label.h"

using namespace std;
using namespace ReyEngine;
using namespace FileSystem;

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::open() {
   setVisible(true);
   setModal(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::close() {
   setVisible(false);
   setModal(false);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<FileSystem::Directory> FileBrowser::getSystemDirs() {
   vector<Directory> retval;
   for (const auto& item : _systemBrowserTree->getRoot().value()->getChildren()){
      if (auto path = item->getMetaData<Path>(VAR_PATH)){
         if (path->isDirectory()){
            retval.push_back(path.value());
         }
      }
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::render2D() const {
   drawRectangle(getSizeRect(), theme->background.colorPrimary);
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
   _systemBrowserTree = make_child<Tree>(_systemBrowserScrollArea->getNode(), "systemBrowserTree");

   auto footer = make_child<Layout>(_layout->getNode(), "footer", Layout::LayoutDir::HORIZONTAL);

   navBar->setMaxHeight(30);
   footer->setMaxHeight(60);

   _layout->layoutRatios = {1, 4, 1};
   browser->layoutRatios = {1, 4};
   _directoryTree = make_child<Tree>(_directoryScrollArea->getNode(), "directoryTree");
   _directoryTree->setAllowSelect(true);
   _directoryTree->setHideRoot(true);

   auto footerL = make_child<Layout>(footer->getNode(), "footerL", Layout::LayoutDir::HORIZONTAL);
   auto footerR = make_child<Layout>(footer->getNode(), "footerR", Layout::LayoutDir::HORIZONTAL);

   _filterText = make_child<LineEdit>(footerL->getNode(), "_filterText");
//   _filterType = make_child<ComboBox<string>>(footerL->getNode(), "_filterType");

   _btnOk = make_child<PushButton>(footerR->getNode(), "btnOk", "Ok");
   _btnCancel = make_child<PushButton>(footerR->getNode(), "btnCancel", "Cancel");
   _directoryTree->subscribe<Tree::EventItemSelected>(_directoryTree, [this](const Tree::EventItemSelected& e){_on_directory_item_selected(e);});
   _directoryTree->subscribe<Tree::EventItemDeselected>(_directoryTree, [this](const Tree::EventItemDeselected& e){_on_directory_item_deselected(e);});
   _directoryTree->subscribe<Tree::EventItemDoubleClicked>(_directoryTree, [this](const Tree::EventItemDoubleClicked& e){_on_directory_item_doubleClicked(e);});
   _systemBrowserTree->subscribe<Tree::EventItemSelected>(_systemBrowserTree, [this](const Tree::EventItemSelected& e){_on_system_item_selected(e);});
   _systemBrowserTree->subscribe<Tree::EventItemDeselected>(_systemBrowserTree, [this](const Tree::EventItemDeselected& e){_on_system_item_deselected(e);});
   _systemBrowserTree->subscribe<Tree::EventItemDoubleClicked>(_systemBrowserTree, [this](const Tree::EventItemDoubleClicked& e){_on_system_item_doubleClicked(e);});
   _btnOk->setEnabled(false);

   _btnBack = make_child<PushButton>(navBar->getNode(), "btnBack", "<-");
   _btnFwd = make_child<PushButton>(navBar->getNode(), "btnForward", "->");
   _btnUp = make_child<PushButton>(navBar->getNode(), "btnUp", "^");
   _addrBar = make_child<AddrBar>(navBar->getNode(), "addrBar", _dir);
   navBar->layoutRatios = {1,1,1,20};

   _addrBar->subscribe<AddrBar::EventAddrEntered>(_addrBar, [&](const AddrBar::EventAddrEntered& e){_on_addr_entered(e);});
   _btnOk->subscribe<PushButton::ButtonPressEvent>(_btnOk, [&](const PushButton::ButtonPressEvent& e){ _on_ok(e);});
   _btnCancel->subscribe<PushButton::ButtonPressEvent>(_btnCancel, [&](const PushButton::ButtonPressEvent& e){ _on_cancel(e);});
   _btnUp->subscribe<PushButton::ButtonPressEvent>(_btnUp, [&](const PushButton::ButtonPressEvent& e){ _on_up(e);});
   _btnFwd->subscribe<PushButton::ButtonPressEvent>(_btnFwd, [&](const PushButton::ButtonPressEvent& e){ _on_fwd(e);});
   _btnBack->subscribe<PushButton::ButtonPressEvent>(_btnBack, [&](const PushButton::ButtonPressEvent& e){ _on_back(e);});

   //populate the system browser tree
   {
      _systemBrowserTree->setHideRoot(true);
      _systemBrowserTree->setAllowSelect(true);
      auto treeRoot = _systemBrowserTree->setRoot("...");
      auto vipDirs = CrossPlatform::getRootFolders();
      std::vector<string> mightExist;
      mightExist.push_back(CrossPlatform::getUserDir());
      mightExist.push_back(CrossPlatform::getUserLocalConfigDir());
      mightExist.push_back(CrossPlatform::getUserLocalConfigDirApp());
      mightExist.push_back(CrossPlatform::getExeDir());
      while (!mightExist.empty()){
         Directory d(mightExist.back());
         if (d.exists()){
            vipDirs.push_back(mightExist.back());
         }
         mightExist.pop_back();
      }
      for (const auto& dir : vipDirs){
         auto text = dir;
         if (dir.size() > 18) text = Directory(dir).tail();
         auto item = treeRoot->push_back(text);
         item->setMetaData<Path>(VAR_PATH, dir);
      }
   }

   //try to set a default directory
   auto defaultDir = Directory(CrossPlatform::getUserLocalConfigDirApp());
   if (defaultDir.exists()) {
      _setCurrentDirectory(defaultDir);
   } else {
      _setCurrentDirectory(Directory(CrossPlatform::getUserDir()));
   }

   setMinSize(800,600);
   setVisible(false);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::refreshDirectoryContents() {
   vector<Directory> dirs;
   vector<File> files;
   _directoryTree->setRoot(_directoryTree->createItem(_dir.str(), std::make_optional(std::pair<std::string, Path>(VAR_PATH, _dir))));
   auto parentItem = _directoryTree->getRoot().value();
   auto itemPath = parentItem->getMetaData<Path>(VAR_PATH);
   if (!itemPath){
      Logger::error() << "Directory entry " + parentItem->getText() + " did not have a directory associated with it";
      return;
   }
   auto contents = Directory(itemPath.value()).listContents();
   Directory::logfsError(Logger::error(), contents.second);
   for (const auto& entry : contents.first) {
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
      //apply type filter
      if (!_fileTypesFilter.empty()) {
         bool showFile = false;
         for (const auto& ext: _fileTypesFilter) {
            if (file.tail().ends_with(ext)) {
               showFile = true;
               break;
            }
         }
         if (!showFile) continue;
      }
      parentItem->push_back(_directoryTree->createItem(file.tail()));
      parentItem->back()->setMetaData<Path>(VAR_PATH, file);
   }
   auto size = _directoryTree->measureContents();
   _directoryTree->setSize(size);
   if (!_fileTypesFilter.empty()) _filterText->setText("*"+string_tools::join(",*", _fileTypesFilter));
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_directory_item_selected(const Tree::EventItemSelected& event) {
   Logger::info() << "Selected item " << *event.item << endl;
   _btnOk->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_directory_item_deselected(const Tree::EventItemDeselected& event) {
   Logger::info() << "Deselected item " << *event.item << endl;
   _btnOk->setEnabled(false);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_directory_item_doubleClicked(const Tree::EventItemDoubleClicked& e) {
   if (auto metaData = e.item->getMetaData<Path>(VAR_PATH)){
      auto dir = metaData.value();
      if (dir.isDirectory()){
         setCurrentDirectory(dir);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_system_item_selected(const Tree::EventItemSelected& event) {
//   Logger::info() << "Selected item " << *event.item << endl;
//   _btnOk->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_system_item_deselected(const Tree::EventItemDeselected& event) {
//   Logger::info() << "Deselected item " << *event.item << endl;
//   _btnOk->setEnabled(false);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_system_item_doubleClicked(const Tree::EventItemDoubleClicked& e) {
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
   if (auto parent = _dir.getParentDirectory()){
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
void FileBrowser::_on_ok(const PushButton::ButtonPressEvent &) {
   close();
   publish(EventOk(this, _directoryTree->getSelected().value()));
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_on_cancel(const PushButton::ButtonPressEvent &) {
   close();
   publish(EventCancelled(this));
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
   setText(_currentDir.canonical());
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
std::optional<FileSystem::Path> FileBrowser::extractPathFromItem(const ReyEngine::TreeItem* item) const {
   if (!item) return {};
   return item->getMetaData<Path>(VAR_PATH);
}