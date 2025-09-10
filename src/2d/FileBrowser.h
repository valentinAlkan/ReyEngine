#include "Tree.h"
#include "FileSystem.h"

namespace ReyEngine {
   class FileBrowser : public Layout {
   public:
      template <typename... Args>
      FileBrowser(Args... args)
      : _dir(std::forward<Args>(args)...)
      , Layout(LayoutDir::VERTICAL)
      {
         refreshDirectoryContents();
      }
      template <typename... Args>
      inline void setCurrentDirectory(Args... args){
         _dir = FileSystem::Directory(std::forward<Args>(args)...);
         refreshDirectoryContents();
      }
   protected:
      void refreshDirectoryContents();
      void render2D() const override;
      void _init() override;
      std::shared_ptr<Tree> _tree;
      FileSystem::Directory _dir;
   };
}
