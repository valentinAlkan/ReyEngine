#pragma once
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stack>
#include <unordered_set>
#include "Scene.h"
#include "Event.h"

//implement to enable dragndrop
namespace ReyEngine{
   namespace Internal{
      template <typename T>
      class TypeContainer;
      class WindowPrototype;
   }
   class UnhandledMouseInput;
   class InputEvent;
   class Draggable{
   public:
      Draggable(const std::string& id, std::shared_ptr<BaseWidget> preview) : id(id), preview(preview){}
      Pos<int> startPos;
      const std::string id;
      std::optional<std::shared_ptr<BaseWidget>> preview;
   };

   class Canvas;
   class Window
   : public Internal::TypeContainer<BaseWidget> {
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
      ~Window() override;

      REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(Window)
      template <typename T> void addChild(std::shared_ptr<TypeContainer<T>> child) = delete;
      bool isProcessed(const std::shared_ptr<BaseWidget>&) const;
      bool isEditor(){return _isEditor;}
      bool setProcess(bool, std::shared_ptr<BaseWidget>); //returns whether operation was successful. Returns false if widget already being processed or is not found.
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
      inline uint64_t getFrameCounter(){return _frameCounter;}
      std::optional<std::shared_ptr<Draggable>>getDragNDrop(){if (_dragNDrop) return _dragNDrop; return std::nullopt;}
      //hover
      void clearHover();
      void setHover(std::shared_ptr<BaseWidget>&);
   
      void pushRenderTarget(RenderTarget&);
      void popRenderTarget();
      std::optional<std::weak_ptr<BaseWidget>> getHovered();
      /// Generates an inputEvent with the window as the publisher
      /// \tparam T
      /// \return
      template <typename T>
      std::unique_ptr<T> generateInputEvent(){
         static_assert(std::is_base_of_v<InputEvent, T>);
         return std::make_unique<T>(toEventPublisher());
      }
      template <typename T>
      void mouseInput(std::unique_ptr<T>& event){
         static_assert(std::is_base_of_v<InputEvent, T>);
         std::unique_lock<std::mutex> sl(_inputMtx);
         _inputQueueMouse.push(std::move(event));
      }
      inline void keyInput(InputInterface::KeyCode){};
      int getFPS() const {return GetFPS();}
   protected:
      Window(const std::string& title, int width, int height, const std::vector<Flags>& flags, int targetFPS);
      void initialize(std::optional<std::shared_ptr<Canvas>> root);
      static constexpr size_t INPUT_COUNT_LIMIT = 256;
   private:
      void processUnhandledInput(InputEvent&, std::optional<UnhandledMouseInput>);
      std::weak_ptr<BaseWidget> _hovered; //the currently hovered widget
      bool _isEditor = false; //enables other features
      std::optional<std::shared_ptr<Draggable>> _dragNDrop; //the widget currently being drag n dropped
      bool _isDragging = false; //true when we start a dragndrop
      uint64_t _frameCounter=0;
      int targetFPS;
      std::chrono::milliseconds _keyDownRepeatDelay = std::chrono::milliseconds(500); //how long a key must be held down before it counts as a repeat
      std::chrono::milliseconds _keyDownRepeatRate = std::chrono::milliseconds(25); //how long must pass before each key repeat event is sent
      const int startingWidth;
      const int startingHeight;
      std::stack<RenderTarget*> renderStack;
      std::queue<std::unique_ptr<InputEvent>> _inputQueueMouse; //a place to hold programatically generated input
      std::queue<std::unique_ptr<InputEvent>> _inputQueueKey; //a place to hold programatically generated input
      std::mutex _inputMtx;
      /////////////////////
      /////////////////////
      class ProcessList {
      public:
         ~ProcessList(){clear();}
         std::optional<std::shared_ptr<BaseWidget>> add(std::shared_ptr<BaseWidget>& widget);
         std::optional<std::shared_ptr<BaseWidget>> remove(std::shared_ptr<BaseWidget>& widget);
         std::optional<std::shared_ptr<BaseWidget>> find(const std::shared_ptr<BaseWidget>& widget) const;
         void processAll(double dt);
         void clear(){
             std::unique_lock<std::mutex> sl(_mtx);
             _list.clear();
         }
      private:
         std::unordered_set<std::shared_ptr<BaseWidget>> _list; //list of widgets that require processing. No specific order.
         std::mutex _mtx;
      } _processList;
      /////////////////////
      /////////////////////
      friend class Application;
   };

    //just initializes important stuff that must be initialized, so that we can create a root to pass to window if we want.
    namespace Internal {
        class WindowPrototype {
        public:
            Window& createWindow();
            Window& createWindow(std::shared_ptr<Canvas>& root);
        protected:
            WindowPrototype(const std::string &title, int width, int height, const std::vector<Window::Flags> &flags, int targetFPS);
            const std::string title;
            const int width;
            const int height;
            const std::vector<Window::Flags> &flags;
            const int targetFPS;
            bool isEditor(){return _isEditor;}
        private:
            void use();
            bool _usedUp = false; //can only generate one window per prototype
            bool _isEditor = false;
            friend class ReyEngine::Application;
        };
    }
}