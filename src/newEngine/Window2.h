#pragma once
#include "ReyObject.h"
#include "Application2.h"
#include "InputHandler2.h"
#include <unordered_set>

namespace ReyEngine{
   using FrameCount = uint64_t;
   class Canvas;
   class Window2 : public EventPublisher {
   public:

      EVENT_ARGS(WindowResizeEvent, 6565546465, Size<float> newSize)
      , size(newSize)
      {}
         Size<float> size;
      };
      EVENT_ARGS(WindowMoveEvent, 321657798551, Pos<float> newPos)
      , position(newPos)
      {}
         Pos<int> position;
      };
      virtual void exec();
      ~Window2();

//      bool isProcessed(const std::shared_ptr<BaseWidget>&) const;
      bool isEditor(){return _isEditor;}
//      bool setProcess(bool, std::shared_ptr<BaseWidget>); //returns whether operation was successful. Returns false if widget already being processed or is not found.
//      void setCanvas(std::shared_ptr<Canvas>&);
      static WindowSpace<Pos<float>> getMousePos(); //returns global mouse position
      static Vec2<double> getMousePct(); //returns global mouse position as a percentage of the window size from 0 to 1
      std::shared_ptr<Canvas> getCanvas();
      Size<int> getSize(){return getWindowSize();}
      void setSize(Size<int> newSize);
      Pos<int> getPosition(){return getWindowPosition();}
      void setPosition(Pos<int> newPos){setWindowPosition(newPos);}
      void maximize(){maximizeWindow();}
      void minimize(){minimizeWindow();}
      inline uint64_t getFrameCounter() const {return _frameCounter;}
//      std::optional<std::shared_ptr<Draggable>>getDragNDrop(){if (_dragNDrop) return _dragNDrop; return std::nullopt;}
      inline void keyInput(InputInterface::KeyCode){};
      int getFPS() const {return GetFPS();}
   protected:
      Window2(const std::string& title, int width, int height, const std::vector<WindowFlags>& flags, int targetFPS);
      void initialize(std::optional<std::shared_ptr<Canvas>> root);
      static constexpr size_t INPUT_COUNT_LIMIT = 256;
   private:
//      std::weak_ptr<BaseWidget> _hovered; //the currently hovered widget
      bool _isEditor = false; //enables other features
//      std::optional<std::shared_ptr<Draggable>> _dragNDrop; //the widget currently being drag n dropped
//      bool _isDragging = false; //true when we start a dragndrop
      uint64_t _frameCounter=0;
      int targetFPS;
      std::chrono::milliseconds _keyDownRepeatDelay = std::chrono::milliseconds(500); //how long a key must be held down before it counts as a repeat
      std::chrono::milliseconds _keyDownRepeatRate = std::chrono::milliseconds(25); //how long must pass before each key repeat event is sent
      const int startingWidth;
      const int startingHeight;
//      std::stack<RenderTarget*> renderStack;
      std::queue<std::unique_ptr<InputEvent>> _inputQueueMouse; //a place to hold programatically generated input
      std::queue<std::unique_ptr<InputEvent>> _inputQueueKey; //a place to hold programatically generated input
//      std::mutex _inputMtx;
      std::unique_ptr<TypeNode> _root;
      /////////////////////
      /////////////////////
      class ProcessList {
      public:
         ~ProcessList(){clear();}
//         std::optional<std::shared_ptr<BaseWidget>> add(std::shared_ptr<BaseWidget>& widget);
//         std::optional<std::shared_ptr<BaseWidget>> remove(std::shared_ptr<BaseWidget>& widget);
//         std::optional<std::shared_ptr<BaseWidget>> find(const std::shared_ptr<BaseWidget>& widget) const;
         void processAll(double dt);
         void clear(){
            std::unique_lock<std::mutex> sl(_mtx);
            _list.clear();
         }
      private:
         std::unordered_set<TreeStorable*> _list; //list of widgets that require processing. No specific order.
         std::mutex _mtx;
      } _processList;
      /////////////////////
      /////////////////////
      friend class Application2;
   };


   //just initializes important stuff that must be initialized, so that we can create a root to pass to window if we want.
   namespace Internal {
      class WindowPrototype2 {
      public:
         Window2& createWindow();
//         Window2& createWindow(std::shared_ptr<Canvas>& root);
      protected:
         WindowPrototype2(const std::string &title, int width, int height, const std::vector<WindowFlags>& flags, int targetFPS);
         const std::string title;
         const int width;
         const int height;
         const std::vector<WindowFlags> &flags;
         const int targetFPS;
         bool isEditor(){return _isEditor;}
      private:
         void use();
         bool _usedUp = false; //can only generate one window per prototype
         bool _isEditor = false;
         friend class ReyEngine::Application2;
      };
   }
}
