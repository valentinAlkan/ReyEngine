#pragma once
#include <vector>
#include <functional>
#include <BaseWidget.h>
#include <memory>
#include <mutex>
#include <optional>

class Window {
public:
//   using ProcessListIterator = std::unordered_set<std::shared_ptr<BaseWidget>>::iterator;
   enum Flags{RESIZE};
   virtual void exec();
   virtual ~Window();
   const std::shared_ptr<BaseWidget>& getRootWidget(){return _root;}
   bool isProcessed(std::shared_ptr<BaseWidget>);
   bool setProcess(bool, std::shared_ptr<BaseWidget>); //returns whether operation was successful. Returns false if widget already being processed or is not found.
   static GFCSDraw::Vec2<int> getMousePos(); //returns global mouse position
protected:
   Window(const std::string& title, int width, int height, std::shared_ptr<BaseWidget> root, const std::vector<Flags>& flags, int targetFPS=60);
private:
   std::shared_ptr<BaseWidget> _root; //the root widget to draw
   class ProcessList {
   public:
      std::optional<std::shared_ptr<BaseWidget>> add(std::shared_ptr<BaseWidget> widget);
      std::optional<std::shared_ptr<BaseWidget>> remove(std::shared_ptr<BaseWidget> widget);
      std::unordered_set<std::shared_ptr<BaseWidget>>& getList(){return _list;}
   private:
      std::unordered_set<std::shared_ptr<BaseWidget>> _list; //list of widgets that require processing. No specific order.
      std::mutex _mtx;
   } _processList;

   friend class Application;
};