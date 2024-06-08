#pragma once
#include <memory>
#include <filesystem>
#include <queue>
#include "Window.h"
#include "Logger.h"
#include "DrawInterface.h"
#include "Platform.h"
#include "FileSystem.h"

namespace ReyEngine{
   class Application
   {
   public:
      enum class Platform{WINDOWS, LINUX};
      static Application& instance(){
         static Application instance;
         return instance;
      }
   private:
      Application();
   public:
      enum class ExitReason{
         CLEAN = 0,
         INVALID_ARGS = 1,
         INVALID_SCENE_FILE_FORMAT,
      };
      using UniqueValue = uint64_t;
      Application(Application const&)    = delete;
      void operator=(Application const&) = delete;

      std::shared_ptr<Window> createWindow(const std::string& title, int width, int height, const std::vector<ReyEngine::Window::Flags>& flags, int targetFPS=60);
      const std::shared_ptr<Window>& getWindow(int windowIndex){return _windows.at(windowIndex);}
      size_t windowCount(){return _windows.size();}

      static Logger& printDebug(){return Application::instance()._debug_logger;}
      static Logger& printInfo(){return Application::instance()._info_logger;}
      static Logger& printWarn(){return Application::instance()._warn_logger;}
      static Logger& printError(){return Application::instance()._error_logger;}

      static void exitError(std::string msg, ExitReason rsn){printError() << msg << std::endl; ::exit((int)rsn);}
      static void exit(ExitReason rsn){::exit((int)rsn);}

      static void registerForApplicationReady(std::shared_ptr<BaseWidget>&); //somethings require initwindow to have been called - so we can let the application know we want to be called when application is ready.
      static void registerForApplicationReady(std::function<void()>); //somethings require initwindow to have been called - so we can let the application know we want to be called when application is ready.
      static void registerForEnterTree(std::shared_ptr<BaseWidget>&, std::shared_ptr<BaseWidget>&); //widgets can't use shared_from_this in ctor so we need a place that gets called once on tree enter that can do it.
      static bool isReady(){return instance()._is_ready;}
      static constexpr Platform getPlatform(){return PLATFORM;}
      static UniqueValue generateUniqueValue(){return instance()._nextUniqueValue++;}
   protected:
      uint64_t getNewRid(){return ++newRid;}
      static void processEnterTree();
      static void ready();

   private:
      UniqueValue _nextUniqueValue = 0;
      #ifdef REYENGINE_PLATFORM_WINDOWS
         static constexpr Platform PLATFORM = Platform::WINDOWS;
      #elif defined(REYENGINE_PLATFORM_LINUX)
         static constexpr Platform PLATFORM = Platform::LINUX;
      #endif
      bool _is_ready = false;
      std::vector<std::shared_ptr<Window>> _windows;
      uint64_t newRid;
      Logger _debug_logger;
      Logger _info_logger;
      Logger _warn_logger;
      Logger _error_logger;
      std::unordered_set<std::shared_ptr<BaseWidget>> _applicationReadyList; //list of widgets that want to be notified when the application is fully initialized
      std::vector<std::function<void()>> _initListArbCallback; //list of arbitrary callbacks that serve the same purpose as the above

      //init list
      using InitPair = std::pair<std::shared_ptr<BaseWidget>, std::shared_ptr<BaseWidget>>;
      std::queue<InitPair> _initQueue;
      friend class BaseWidget;
      friend class Component;
      friend class Window;
   };
}