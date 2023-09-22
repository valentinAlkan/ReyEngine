#pragma once
#include <vector>
#include <functional>
#include <BaseWidget.h>
#include <memory>
#include <mutex>
#include <optional>
#include "Scene.h"

class Window {
public:
//   using ProcessListIterator = std::unordered_set<std::shared_ptr<BaseWidget>>::iterator;
   enum Flags{RESIZE};
   virtual void exec();
   virtual ~Window();
   bool isProcessed(const std::shared_ptr<BaseWidget>&) const;
   bool setProcess(bool, std::shared_ptr<BaseWidget>); //returns whether operation was successful. Returns false if widget already being processed or is not found.
   static GFCSDraw::Vec2<int> getMousePos(); //returns global mouse position
   const std::shared_ptr<BaseWidget>& getRootWidget(){return _root;}
protected:
   Window(const std::string& title, int width, int height, std::shared_ptr<BaseWidget> root, const std::vector<Flags>& flags, int targetFPS=60);
private:
   std::shared_ptr<BaseWidget> _root; //the scene to draw

   class ProcessList {
   public:
      std::optional<std::shared_ptr<BaseWidget>> add(std::shared_ptr<BaseWidget> widget);
      std::optional<std::shared_ptr<BaseWidget>> remove(std::shared_ptr<BaseWidget> widget);
      std::optional<std::shared_ptr<BaseWidget>> find(const std::shared_ptr<BaseWidget>& widget) const;
      std::unordered_set<std::shared_ptr<BaseWidget>>& getList(){return _list;}
   private:
      std::unordered_set<std::shared_ptr<BaseWidget>> _list; //list of widgets that require processing. No specific order.
      std::mutex _mtx;
   } _processList;

   friend class Application;
};