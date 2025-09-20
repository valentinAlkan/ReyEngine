#include "Tree.h"
#include "FileSystem.h"
#include "Button.h"
#include "LineEdit.h"  
#include "ComboBox.h"
#include "ScrollArea.h"

namespace ReyEngine {
   class FileBrowser : public Widget {
   public:
      FileBrowser()
      : _dir(CrossPlatform::getUserLocalConfigDirApp())
      {
         _history.add(_dir);
      }
      //updates history
      template <typename... Args>
      inline void setCurrentDirectory(Args... args){
         auto newDir = FileSystem::Directory(std::forward<Args>(args)...);
         Logger::debug() << newDir << std::endl;
         Logger::debug() << _dir << std::endl;
         if (newDir != _dir) {
            _history.add(newDir);
         }
         _setCurrentDirectory(newDir);
      }
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
         auto parentDir = _dir.getParent();
         _btnUp->setEnabled(parentDir.has_value() && parentDir != _dir);
//         Logger::info() << "Setting new dir to " << newDir.str() << std::endl;
//         Logger::info() << "--------------" << std::endl;
//         int i = 0;
//         for (const auto& dir: _history._dirs) {
//            std::stringstream msg;
//            if (_history._ptr == i++) {
//               msg << " * ";
//            }
//            msg << dir.str();
//            Logger::info() << msg.str() << std::endl;
//         }
      }
      void refreshDirectoryContents();
      void render2D() const override;
      void _init() override;
      void _on_rect_changed() override;
      void _on_item_selected(const Tree::EventItemSelected&);
      void _on_item_deselected(const Tree::EventItemDeselected&);
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
      std::shared_ptr<ComboBox<std::string>> _filterType;

      class History {
      public:
         void add(const FileSystem::Directory&); //overwrites next item after current pointer (if any)
         std::optional<FileSystem::Directory> back();
         std::optional<FileSystem::Directory> fwd();
         bool hasFwd();
         bool hasBack();
         std::vector<FileSystem::Directory> _dirs;
         size_t _ptr = 0;
      } _history;
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
      void _on_item_doubleClicked(const Tree::EventItemDoubleClicked&);
   };
}
