#include "WindowPrototype.h"
#include "Application.h"
#include "TypeTree.h"
#include "Logger.h"

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
         case WindowFlags::AA_4X:
            //4x antialiasing, by 2x2 supersampling: every Canvas allocates its render target at
            //twice the linear size and the final blit averages each 2x2 block back down. This
            //has to happen here, before any RenderTarget exists, since they read the default
            //when they're constructed.
            RenderTarget::setDefaultRenderScale(2.0f);
            //MSAA is only a hint and only multisamples the *default* framebuffer, which sees
            //nothing but the one quad each canvas is blitted through - so it does nothing for
            //widgets today. Set anyway: it costs nothing and covers anything drawn straight to
            //the window.
            SetConfigFlags(FLAG_MSAA_4X_HINT);
            break;
      }
   }

   SetTraceLogLevel(LOG_NONE);
   SetConfigFlags(FLAG_VSYNC_HINT);
   InitWindow(width, height, title.c_str());
   //sound, if there's a device to play it on (without one, sounds just don't play)
   InitAudioDevice();
   if (!IsAudioDeviceReady()) Logger::warn() << "No audio device: sounds won't play" << std::endl;
   Application::instance()._is_ready = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
WindowPrototype::~WindowPrototype() = default;

/////////////////////////////////////////////////////////////////////////////////////////
void WindowPrototype::setRoot(std::unique_ptr<Tree::TypeNode> newRoot){
   root = std::move(newRoot);
}


/////////////////////////////////////////////////////////////////////////////////////////
Window& WindowPrototype::createWindow() {
   use();
   return Application::instance().createWindow(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////
void WindowPrototype::use() {
   if (_usedUp) {
      throw std::runtime_error("WindowPrototype for window " + title + " {" + std::to_string(width) + ":" + std::to_string(height) + " already used!");}
   _usedUp = true;
}