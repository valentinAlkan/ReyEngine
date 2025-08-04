#pragma once

#include <vector>
#include <string>
#include "ReyEngine.h"

//just initializes important stuff that must be initialized, so that we can create a root to pass to window if we want.
namespace ReyEngine{
   class Window;
   class Application;
   namespace Internal {
      class WindowPrototype {
      public:
         Window& createWindow();
      protected:
         WindowPrototype(const std::string& title, int width, int height, const std::vector<WindowFlags>& flags,
                         int targetFPS);

         const std::string title;
         const int width;
         const int height;
         const std::vector<WindowFlags>& flags;
         const int targetFPS;

         bool isEditor() { return _isEditor; }

      private:
         void use();

         bool _usedUp = false; //can only generate one window per prototype
         bool _isEditor = false;

         friend class ReyEngine::Application;
      };
   }
}