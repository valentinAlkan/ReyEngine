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
      {}
      template <typename... Args>
      inline void setCurrentDirectory(Args... args){
         _dir = FileSystem::Directory(std::forward<Args>(args)...);
         refreshDirectoryContents();
         _addrBar->setDir(_dir);
      }
   protected:
      class AddrBar;
      static constexpr char VAR_PATH[] = "PATH";
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
      std::shared_ptr<PushButton> _btnForward;
      std::shared_ptr<PushButton> _btnUp;
      std::shared_ptr<AddrBar> _addrBar;
      std::shared_ptr<LineEdit> _filterText;
      std::shared_ptr<ComboBox<std::string>> _filterType;

      class AddrBar : public LineEdit {
      protected:
         struct DirectoryToken;
      public:
         AddrBar(FileSystem::Directory& dir): _currentDir(dir){setDir(dir);}
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
   };
}
