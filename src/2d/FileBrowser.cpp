#include "FileBrowser.h"
#include "MiscTools.h"


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
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_init() {
   _layout = make_child<Layout>(getNode(), "layout", Layout::LayoutDir::VERTICAL);
   auto navBar = make_child<Layout>(_layout->getNode(), "navBar", Layout::LayoutDir::HORIZONTAL);
   auto browser = make_child<Layout>(_layout->getNode(), "browser", Layout::LayoutDir::HORIZONTAL);
   auto systemBrowser = make_child<Layout>(browser->getNode(), "systemBrowser", Layout::LayoutDir::VERTICAL);
   auto dirTreeBrowser = make_child<Layout>(browser->getNode(), "fileBrowserLayout", Layout::LayoutDir::VERTICAL);
   auto footer = make_child<Layout>(_layout->getNode(), "footer", Layout::LayoutDir::HORIZONTAL);

   navBar->setMaxHeight(30);
   footer->setMaxHeight(60);

   _layout->layoutRatios = {1, 4, 1};
   browser->layoutRatios = {1, 4};
   _tree = make_child<Tree>(dirTreeBrowser->getNode(), "tree");
   _tree->setAllowSelect(true);
   _tree->setHideRoot(true);

   auto footerL = make_child<Layout>(footer->getNode(), "footerL", Layout::LayoutDir::HORIZONTAL);
   auto footerR = make_child<Layout>(footer->getNode(), "footerR", Layout::LayoutDir::HORIZONTAL);

   _filterText = make_child<LineEdit>(footerL->getNode(), "_filterText");
   _filterType = make_child<ComboBox<string>>(footerL->getNode(), "_filterType");

   _btnOk = make_child<PushButton>(footerR->getNode(), "btnOk", "Ok");
   _btnCancel = make_child<PushButton>(footerR->getNode(), "btnCancel", "Cancel");
   _btnOk->subscribe<Tree::EventItemSelected>(_tree, [this](const Tree::EventItemSelected& e){_on_item_selected(e);});
   _btnOk->subscribe<Tree::EventItemDeselected>(_tree, [this](const Tree::EventItemDeselected& e){_on_item_deselected(e);});
   _btnOk->setEnabled(false);

   _btnBack = make_child<PushButton>(navBar->getNode(), "btnBack", "<-");
   _btnForward = make_child<PushButton>(navBar->getNode(), "btnForward", "->");
   _btnUp = make_child<PushButton>(navBar->getNode(), "btnUp", "^");
   _addrBar = make_child<LineEdit>(navBar->getNode(), "addrBar");
   navBar->layoutRatios = {1,1,1,20};

   setMinSize(200,200);
}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::refreshDirectoryContents() {
   Logger::info() << "Folder " << _dir << " contains : " << endl;

   std::function<void(TreeItem*)> populate = [&](TreeItem* parentItem){
      auto itemPath = parentItem->getMetaData<Path>(VAR_PATH);
      if (!itemPath->isDirectory()) return;
      for (const auto& entry : Directory(itemPath.value()).listContents()) {
         Logger::info() << entry << endl;
         auto p = std::make_optional(std::pair<std::string, Path>(VAR_PATH, entry));
         auto text = entry.tail();
         if (entry.isDirectory()) text += FILESYSTEM_PATH_SEP;
         auto child = parentItem->push_back(_tree->createItem(text, p));
         child->setExpandable(entry.isDirectory());
         populate(child);
      }
   };

   _tree->setRoot(_tree->createItem(_dir.str(), std::make_optional(std::pair<std::string, Path>(VAR_PATH, _dir))));
   populate(_tree->getRoot().value());


//      stringstream ss;
//      auto f = [&](){
//         Logger::info() << ss.str();
//      };
//      auto exit = Tools::make_scope_exit(f);
//
//      if (entry.isRegularFile()) {
//         ss << "    File : ";
//      }
//      else if (entry.isDirectory()) {
//         ss << "    Directory : ";
//      }
//      ss << entry.abs() << endl;
//
//   }
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