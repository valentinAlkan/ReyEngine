#include "WindowPrototype.h"
#include "Application.h"

using namespace ReyEngine;
using namespace Internal;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
WindowPrototype::WindowPrototype(const std::string &title, int width, int height, const std::vector<WindowFlags> &flags, int targetFPS)
      : title(title)
      , width(width)
      , height(height)
      , flags(flags)
      , targetFPS(targetFPS)
{
   for (const auto& flag : flags){
      switch (flag){
         case WindowFlags::RESIZE:
            SetConfigFlags(FLAG_WINDOW_RESIZABLE);
            break;
         case WindowFlags::IS_EDITOR:
            _isEditor = true;
            break;
      }
   }

   SetTraceLogLevel(LOG_NONE);
   SetConfigFlags(FLAG_VSYNC_HINT);
   InitWindow(width, height, title.c_str());
   Application::instance()._is_ready = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
Window& WindowPrototype::createWindow() {
   use();
   return Application::instance().createWindow(*this, nullopt);
}

/////////////////////////////////////////////////////////////////////////////////////////
void WindowPrototype::use() {
   if (_usedUp) {
      throw std::runtime_error(
            "WindowPrototype2 for window " + title + " {" + std::to_string(width) + ":" + std::to_string(height) + " already used!");
   }
   _usedUp = true;
}