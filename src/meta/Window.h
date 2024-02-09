#pragma once
#include <vector>
#include <functional>
#include <Node.h>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_set>
#include "Scene.h"
#include "Event.h"

//implement to enable dragndrop
class Draggable{
public:
   Draggable(const std::string& id, std::shared_ptr<Node> preview) : id(id), preview(preview){}
   std::optional<std::shared_ptr<Node>> preview;
   const std::string id;
   ReyEngine::Pos<int> startPos;
};

class Window : public EventPublisher {
public:
   struct WindowResizeEvent : public Event<WindowResizeEvent> {
      EVENT_CTOR_SIMPLE(WindowResizeEvent, Event<WindowResizeEvent>, ReyEngine::Pos<int> newSize), size(newSize){
//         std::cout << "Window resize is event id " << getUniqueEventId() << std::endl;
      }
      ReyEngine::Size<int> size;
   };
   struct WindowMoveEvent : public Event<WindowMoveEvent> {
      EVENT_CTOR_SIMPLE(WindowMoveEvent, Event<WindowMoveEvent>){
//         std::cout << "Window move is event id " << getUniqueEventId() << std::endl;
      }
      ReyEngine::Pos<int> position;
   };
   enum Flags{RESIZE, IS_EDITOR};
   virtual void exec();
   virtual ~Window();
   bool isProcessed(const std::shared_ptr<Node>&) const;
   bool isEditor(){return _isEditor;}
   bool setProcess(bool, std::shared_ptr<Node>); //returns whether operation was successful. Returns false if widget already being processed or is not found.
   void setRoot(std::shared_ptr<Node>&);
   static ReyEngine::Pos<int> getMousePos(); //returns global mouse position
   static ReyEngine::Vec2<double> getMousePct(); //returns global mouse position as a percentage of the window size from 0 to 1
   const std::shared_ptr<Node>& getRootWidget(){return _root;}
   ReyEngine::Size<int> getSize(){return ReyEngine::getWindowSize();}
   void setSize(ReyEngine::Size<int> newSize){ReyEngine::setWindowSize(newSize);}
   ReyEngine::Pos<int> getPosition(){return ReyEngine::getWindowPosition();}
   void setPosition(ReyEngine::Pos<int> newPos){ReyEngine::setWindowPosition(newPos);}
   void maximize(){ReyEngine::maximizeWindow();}
   void minimize(){ReyEngine::minimizeWindow();}
   std::optional<std::shared_ptr<Draggable>>getDragNDrop(){if (_dragNDrop) return _dragNDrop; return std::nullopt;}
protected:
   Window(const std::string& title, int width, int height, const std::vector<Flags>& flags, int targetFPS=60);
   static constexpr size_t INPUT_COUNT_LIMIT = 256;
private:
   std::shared_ptr<Node> _root; //the scene to draw
   bool _isEditor = false; //enables other features

   class ProcessList {
   public:
      std::optional<std::shared_ptr<Node>> add(std::shared_ptr<Node> widget);
      std::optional<std::shared_ptr<Node>> remove(std::shared_ptr<Node> widget);
      std::optional<std::shared_ptr<Node>> find(const std::shared_ptr<Node>& widget) const;
      std::unordered_set<std::shared_ptr<Node>>& getList(){return _list;}
   private:
      std::unordered_set<std::shared_ptr<Node>> _list; //list of widgets that require processing. No specific order.
      std::mutex _mtx;
   } _processList;

   friend class Application;
};