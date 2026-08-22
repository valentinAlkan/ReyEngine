#pragma once

#include <vector>
#include <string>
#include <memory>
#include "ReyEngine.h"

//just initializes important stuff that must be initialized, so that we can create a root to pass to window if we want.
namespace ReyEngine{
   class Window;
   class Application;
   class Canvas;
   namespace Internal::Tree {
      class TypeNode;
   }
   namespace Internal {
      class WindowPrototype {
      public:
         ~WindowPrototype(); //defined in the .cpp, where TypeNode is complete
         Window& createWindow();
         void setRoot(std::unique_ptr<Tree::TypeNode> newRoot);
      protected:
         WindowPrototype(const std::string& title, int width, int height, const std::vector<WindowFlags>& flags,int targetFPS);
         const std::string title;
         const int width;
         const int height;
         const std::vector<WindowFlags>& flags;
         const int targetFPS;
         bool isEditor() { return _isEditor; }
         std::unique_ptr<Tree::TypeNode> root; //optional canvas that the user can pass in to override the default canvas
      private:
         void use();
         bool _usedUp = false; //can only generate one window per prototype
         bool _isEditor = false;

         friend class ReyEngine::Application;
      };
   }
}