#pragma once
#include "DrawInterface.h"
#include <vector>
#include <functional>
#include <BaseWidget.h>
#include <memory>

class Window {
public:
   enum Flags{RESIZE};
   virtual void exec();
   virtual ~Window();
   const std::shared_ptr<BaseWidget>& getRootWidget(){return _root;}
protected:
   Window(const std::string& title, int width, int height, std::shared_ptr<BaseWidget> root, const std::vector<Flags>& flags, int targetFPS=60);
private:
   std::shared_ptr<BaseWidget> _root; //the root widget to draw
   friend class Application;
};