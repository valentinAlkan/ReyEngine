#pragma once
#include <memory>
#include "Window.h"

class Application
{
public:
   static Application& instance(){
      static Application instance;
      return instance;
   }
private:
   Application(){}
public:
   Application(Application const&)         = delete;
   void operator=(Application const&) = delete;

   std::optional<std::shared_ptr<Window>> createWindow(const std::string& title, int width, int height, std::shared_ptr<BaseWidget> root, const std::vector<Window::Flags>& flags, int targetFPS=60);
   const std::shared_ptr<Window>& getWindow(){return _window;}

protected:
   uint64_t getNewRid(){return ++newRid;}

private:
   std::shared_ptr<Window> _window;
   uint64_t newRid;
   friend class BaseWidget;
};