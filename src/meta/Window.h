#pragma once
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include "Scene.h"
#include "Event.h"

//implement to enable dragndrop
namespace ReyEngine{
   class Draggable{
   public:
      Draggable(const std::string& id, std::shared_ptr<BaseWidget> preview) : id(id), preview(preview){}
      Pos<int> startPos;
      const std::string id;
      std::optional<std::shared_ptr<BaseWidget>> preview;
   };
   
   class Canvas;
   class Window : public EventPublisher {
   public:
      struct WindowResizeEvent : public Event<WindowResizeEvent> {
         EVENT_CTOR_SIMPLE(WindowResizeEvent, Event<WindowResizeEvent>, Pos<int> newSize), size(newSize){
   //         std::cout << "Window resize is event id " << getUniqueEventId() << std::endl;
         }
         Size<int> size;
      };
      struct WindowMoveEvent : public Event<WindowMoveEvent> {
         EVENT_CTOR_SIMPLE(WindowMoveEvent, Event<WindowMoveEvent>){
   //         std::cout << "Window move is event id " << getUniqueEventId() << std::endl;
         }
         Pos<int> position;
      };
      enum Flags{RESIZE, IS_EDITOR};
      virtual void exec();
      virtual ~Window();
      bool isProcessed(const std::shared_ptr<BaseWidget>&) const;
      bool isEditor(){return _isEditor;}
      bool setProcess(bool, std::shared_ptr<BaseWidget>); //returns whether operation was successful. Returns false if widget already being processed or is not found.
      void setCanvas(std::shared_ptr<Canvas>&);
      static Pos<int> getMousePos(); //returns global mouse position
      static Vec2<double> getMousePct(); //returns global mouse position as a percentage of the window size from 0 to 1
      const std::shared_ptr<Canvas>& getCanvas() const {return _root;}
      Size<int> getSize(){return getWindowSize();}
      void setSize(Size<int> newSize){setWindowSize(newSize);}
      Pos<int> getPosition(){return getWindowPosition();}
      void setPosition(Pos<int> newPos){setWindowPosition(newPos);}
      void maximize(){maximizeWindow();}
      void minimize(){minimizeWindow();}
      inline uint64_t getFrameCounter(){return _frameCounter;}
      std::optional<std::shared_ptr<Draggable>>getDragNDrop(){if (_dragNDrop) return _dragNDrop; return std::nullopt;}
      //hover
      void clearHover();
      void setHover(std::shared_ptr<BaseWidget>&);
   
      void pushRenderTarget(RenderTarget&);
      void popRenderTarget();
      std::optional<std::weak_ptr<BaseWidget>> getHovered();
   protected:
      Window(const std::string& title, int width, int height, const std::vector<Flags>& flags, int targetFPS=60, std::optional<std::shared_ptr<Canvas>> root=std::nullopt);
      static constexpr size_t INPUT_COUNT_LIMIT = 256;
   private:
      void makeRoot(std::shared_ptr<Canvas>& newRoot, const Size<int>&); //internal. Makes a new canvas into the root. Drops old one.
      void processUnhandledInput(InputEvent&, std::optional<UnhandledMouseInput>);
      std::shared_ptr<Canvas> _root; //the scene to draw
      std::weak_ptr<BaseWidget> _hovered; //the currently hovered widget
      bool _isEditor = false; //enables other features
      std::optional<std::shared_ptr<Draggable>> _dragNDrop; //the widget currently being drag n dropped
      bool _isDragging = false; //true when we start a dragndrop
      uint64_t _frameCounter=0;
      int targetFPS;
      std::chrono::milliseconds _keyDownRepeatDelay = std::chrono::milliseconds(500); //how long a key must be held down before it counts as a repeat
      std::chrono::milliseconds _keyDownRepeatRate = std::chrono::milliseconds(25); //how long must pass before each key repeat event is sent
   
      std::stack<RenderTarget*> renderStack;
      /////////////////////
      /////////////////////
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
      /////////////////////
      /////////////////////
   
      friend class Application;
   };
}