#pragma once

#include "Tree.h"
#include "FileSystem.h"
#include "Button.h"
#include "LineEdit.h"  
#include "ComboBox.h"
#include "ScrollArea.h"
#include "History.h"

namespace ReyEngine {
   class FileBrowser : public Widget {
   public:
      enum class SelectMode {SINGLE_FILE, SINGLE_DIR, MULTIPLE_DIR, MULTIPLE_FILE, ANY_SINGLE, ANY_MULTIPLE, NONE};
      EVENT(EventCancelled, 287346538943581){}};
      EVENT_ARGS(EventOk, 287346538943582, TreeItem* selectedItem), _selectedItem(selectedItem)
         {}
         TreeItem* _selectedItem;
      };
      FileBrowser()
      : _dir("/")
      {
      }
      //updates history
      inline void setCurrentDirectory(auto arg){
         auto newDir = FileSystem::Directory(arg);
         if (!newDir.exists()) return;
         if (newDir != _dir) {
            _history.add(newDir);
         }
         _setCurrentDirectory(newDir);
      }
      inline void setDirectoryAndClearHistory(auto arg) {
         _setCurrentDirectory(FileSystem::Directory(arg));
         _history.clear();
      }
      auto getCurrentDirectory() const {return _dir;}
      void open();
      void close();
      std::vector<FileSystem::Directory> getSystemDirs();
      std::optional<FileSystem::Path> extractPathFromItem(const TreeItem*) const;
      auto getFileTypesFilter(){return _fileTypesFilter;}
      void addFileTypesFilter(const std::string& ext){_fileTypesFilter.push_back(ext);}
      void clearFileTypesFilter(){_fileTypesFilter.clear();}
   protected:
      class AddrBar;
      static constexpr char VAR_PATH[] = "PATH";
      //does not update history
      void _setCurrentDirectory(const FileSystem::Directory& newDir) {
         if (!newDir.exists()) return;
         _dir = newDir;
         refreshDirectoryContents();
         _addrBar->setDir(_dir);
         _directoryTree->setSize(_directoryTree->measureContents());
         _systemBrowserTree->setSize(_systemBrowserTree->measureContents());
         _btnFwd->setEnabled(_history.hasFwd());
         _btnBack->setEnabled(_history.hasBack());
         auto parentDir = _dir.getParentDirectory();
         _btnUp->setEnabled(parentDir.has_value() && parentDir != _dir);
         //highlight system dir when we are in that directory
         for (const auto& item : _systemBrowserTree->getRoot().value()->getChildren()) {
            if (auto path = item->getMetaData<FileSystem::Path>(VAR_PATH)) {
               if (path->isDirectory() && path == _dir) {
                  _systemBrowserTree->setSelected(item.get());
                  break;
               }
            }
         }
      }
      bool _modMultiSelectRange = false;
      bool _modMultiSelect = false;
      void refreshDirectoryContents();
      void render2D() const override;
      void _init() override;
      void _on_rect_changed() override;
      FileSystem::Directory _dir;
      std::shared_ptr<Layout> _layout;
      std::shared_ptr<ScrollArea> _systemBrowserScrollArea;
      std::shared_ptr<Tree> _systemBrowserTree;
      std::shared_ptr<ScrollArea> _directoryScrollArea;
      std::shared_ptr<Tree> _directoryTree;
      std::shared_ptr<PushButton> _btnOk;
      std::shared_ptr<PushButton> _btnCancel;
      std::shared_ptr<PushButton> _btnBack;
      std::shared_ptr<PushButton> _btnFwd;
      std::shared_ptr<PushButton> _btnUp;
      std::shared_ptr<AddrBar> _addrBar;
      std::shared_ptr<LineEdit> _filterText;
//      std::shared_ptr<ComboBox<std::string>> _filterType;
      std::vector<std::string> _fileTypesFilter;

//      class History {
//      public:
//         void add(const FileSystem::Directory&); //overwrites next item after current pointer (if any)
//         std::optional<FileSystem::Directory> back();
//         std::optional<FileSystem::Directory> fwd();
//         bool hasFwd();
//         bool hasBack();
//         void clear();
//         std::vector<FileSystem::Directory> _dirs;
//         size_t _ptr = 0;
//      } _history;
      History<FileSystem::Directory> _history;
      class AddrBar : public LineEdit {
      protected:
         struct DirectoryToken;
      public:
         AddrBar(FileSystem::Directory& dir): _currentDir(dir){setDir(dir);}
         void _init();
         void setDir(FileSystem::Directory& dir);
         EVENT_ARGS(EventTokenClicked, 287346538947589, DirectoryToken* token), token(token){}
            DirectoryToken* token;
         };
         EVENT_ARGS(EventAddrEntered, 287346538947590, std::string addr), addr(addr){}
            std::string addr;
         };
      protected:
         Widget* _unhandled_input(const InputEvent&) override;
         void render2D() const override;
         std::vector<DirectoryToken> _dirTokens;
         std::optional<DirectoryToken*> _hoveredDirToken;
         FileSystem::Directory _currentDir;

         struct DirectoryToken {
            std::string text;
            int index;
            Rect<float> highlightRect;
            bool hovered;
         };
      };
      void _on_addr_entered(const AddrBar::EventAddrEntered&);
      void _on_up(const PushButton::ButtonPressEvent&);
      void _on_back(const PushButton::ButtonPressEvent&);
      void _on_fwd(const PushButton::ButtonPressEvent&);
      void _on_ok(const PushButton::ButtonPressEvent&);
      void _on_cancel(const PushButton::ButtonPressEvent&);
      void _on_directory_item_doubleClicked(const Tree::EventItemDoubleClicked&);
      void _on_directory_item_selected(const Tree::EventItemSelected&);
      void _on_directory_item_deselected(const Tree::EventItemDeselected&);
      void _on_system_item_doubleClicked(const Tree::EventItemDoubleClicked&);
      void _on_system_item_selected(const Tree::EventItemSelected&);
      void _on_system_item_deselected(const Tree::EventItemDeselected&);
   };
}
