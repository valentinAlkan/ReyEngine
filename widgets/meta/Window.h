#pragma once
#include <vector>
#include <functional>
#include <BaseWidget.h>
#include <memory>
#include <mutex>
#include <optional>
#include "Scene.h"
#include "Event.h"

//implement to enable dragndrop
class Draggable{
public:
   Draggable(const std::string& id, std::shared_ptr<BaseWidget> preview) : id(id), preview(preview){}
   std::optional<std::shared_ptr<BaseWidget>> preview;
   const std::string id;
   GFCSDraw::Pos<int> startPos;
};

class Window : public EventPublisher {
public:
   struct WindowResizeEvent : public Event<WindowResizeEvent> {
      EVENT_CTOR_SIMPLE(WindowResizeEvent, Event<WindowResizeEvent>, GFCSDraw::Pos<int> newSize), size(newSize){
//         std::cout << "Window resize is event id " << getUniqueEventId() << std::endl;
      }
      GFCSDraw::Size<int> size;
   };
   struct WindowMoveEvent : public Event<WindowMoveEvent> {
      EVENT_CTOR_SIMPLE(WindowMoveEvent, Event<WindowMoveEvent>){
//         std::cout << "Window move is event id " << getUniqueEventId() << std::endl;
      }
      GFCSDraw::Pos<int> position;
   };
   enum Flags{RESIZE, IS_EDITOR};
   virtual void exec();
   virtual ~Window();
   bool isProcessed(const std::shared_ptr<BaseWidget>&) const;
   bool setProcess(bool, std::shared_ptr<BaseWidget>); //returns whether operation was successful. Returns false if widget already being processed or is not found.
   void setRoot(std::shared_ptr<BaseWidget>&);
   static GFCSDraw::Pos<int> getMousePos(); //returns global mouse position
   static GFCSDraw::Vec2<double> getMousePct(); //returns global mouse position as a percentage of the window size from 0 to 1
   const std::shared_ptr<BaseWidget>& getRootWidget(){return _root;}
   std::optional<std::shared_ptr<BaseWidget>> getWidgetAt(GFCSDraw::Pos<int>);
   GFCSDraw::Size<int> getSize(){return GFCSDraw::getWindowSize();}
   void setSize(GFCSDraw::Size<int> newSize){GFCSDraw::setWindowSize(newSize);}
   GFCSDraw::Pos<int> getPosition(){return GFCSDraw::getWindowPosition();}
   void setPosition(GFCSDraw::Pos<int> newPos){GFCSDraw::setWindowPosition(newPos);}
   void maximize(){GFCSDraw::maximizeWindow();}
   void minimize(){GFCSDraw::minimizeWindow();}
   std::optional<std::shared_ptr<Draggable>>getDragNDrop(){if (_dragNDrop) return _dragNDrop; return std::nullopt;}
protected:
   Window(const std::string& title, int width, int height, const std::vector<Flags>& flags, int targetFPS=60);
   static constexpr size_t INPUT_COUNT_LIMIT = 256;
private:
   std::shared_ptr<BaseWidget> _root; //the scene to draw
   bool _isEditor = false; //enables other features
   std::optional<std::shared_ptr<Draggable>> _dragNDrop; //the widget currently being drag n dropped
   bool _isDragging = false; //true when we start a dragndrop
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