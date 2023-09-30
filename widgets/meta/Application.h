#pragma once
#include <memory>
#include "Window.h"
#include "Logger.h"

class Application
{
public:
   static Application& instance(){
      static Application instance;
      return instance;
   }
private:
   Application()
   : _debug_logger(std::cout, "debug")
   , _info_logger(std::cout, "info")
   , _warn_logger(std::cout, "WARNING")
   , _error_logger(std::cout, "ERROR")
   {
      //initialize time and log current time
      _info_logger << "Application start!" << std::endl;
      //initialize built-in types
      TypeManager::instance()._registerTypes();
   }
public:
   enum class ExitReason{
      INVALID_ARGS = 1,
      INVALID_SCENE_FILE_FORMAT,
   };

   Application(Application const&)         = delete;
   void operator=(Application const&) = delete;

   std::optional<std::shared_ptr<Window>> createWindow(const std::string& title, int width, int height, std::shared_ptr<BaseWidget> root, const std::vector<Window::Flags>& flags, int targetFPS=60);
   const std::shared_ptr<Window>& getWindow(){return _window;}

   static Logger& printDebug(){return Application::instance()._debug_logger;}
   static Logger& printInfo(){return Application::instance()._info_logger;}
   static Logger& printWarn(){return Application::instance()._warn_logger;}
   static Logger& printError(){return Application::instance()._error_logger;}

   static void exitError(std::string msg, ExitReason rsn){printError() << msg << std::endl; ::exit((int)rsn);}
   static void exit(ExitReason rsn){::exit((int)rsn);}

protected:
   uint64_t getNewRid(){return ++newRid;}

private:
   std::shared_ptr<Window> _window;
   uint64_t newRid;
   Logger _debug_logger;
   Logger _info_logger;
   Logger _warn_logger;
   Logger _error_logger;
   friend class BaseWidget;
};