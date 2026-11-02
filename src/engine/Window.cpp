#include "Window.h"
#include <chrono>
#include <iostream>
#include "Application.h"
#include "InputManager.h"
#include "Canvas.h"
#include "TestServer.h"
#include "Replay.h"
#include "rlgl.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
using sc = chrono::steady_clock;

constexpr bool PRINT_MOUSEUP = false;
constexpr bool PRINT_MOUSEDOWN = false;
constexpr bool PRINT_HOVER = false;
constexpr bool PRINT_MOTION = false;
constexpr bool PRINT_TOOLTIP = false;
constexpr bool PRINT_TOOLTIP_CANCEL = false;
constexpr bool PRINT_WHEEL = false;
constexpr bool PRINT_CHAR = false;
constexpr bool PRINT_KEYUP = false;
constexpr bool PRINT_KEYDOWN = false;
constexpr bool PRINT_KEYREPEAT = false;

constexpr std::chrono::milliseconds TOOLTIP_DELAY = 500ms;
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Window::Window(const std::string &title, int width, int height, const std::vector<WindowFlags> &flags, int targetFPS)
: _targetFPS(targetFPS)
{

}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::initialize(std::optional<std::unique_ptr<TypeNode>> optRoot){
   std::unique_ptr<TypeNode> node;
   std::shared_ptr<Canvas> canvas;
   if (optRoot) {
      node = std::move(optRoot.value());
      canvas = node->ref<Canvas>();
      if (!canvas) throw std::runtime_error("Window failed to initialize canvas");
   } else {
      auto pair = make_node<Canvas>("root");
      canvas = pair.first;
      node = std::move(pair.second);
   }
   _root = std::move(node);
   _root->_window = this;
   _root->_deferredCallList = &_deferredCallList;
   _canvas = canvas.get();
   canvas->setSize(getSize());
   canvas->__on_added_to_tree();
   //The root never passes through TypeNode::addChild, so it would otherwise never be inited.
   canvas->__initIfNeeded();
   SetExitKey(KEY_NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::exec(){
   auto canvas = _root->ref<Canvas>();
   Size<float> size;
   Pos<float> position;
   WindowSpace<Pos<float>> mousePos = {{std::numeric_limits<float>::min(), std::numeric_limits<float>::min()}};
   Vec2<float> mouseDelta;
   bool checkedToolTip = false;
   if (_targetFPS) SetTargetFPS(_targetFPS);
   //double clicks are timed on the frames' time steps, not the clock, so a replay (played back at any speed) sees
   //just the ones there were. A press is the second of a double click if it's the same button as the last press, soon
   //after it and close by; the same goes for releases. The click after a double click starts afresh.
   double clickClock = 0;
   struct LastClick {
      InputInterface::MouseButton button = InputInterface::MouseButton::NONE;
      Pos<float> pos;
      double time = 0;
   };
   std::optional<LastClick> lastDown, lastUp;
   auto isDoubleClick = [&](std::optional<LastClick>& last, InputInterface::MouseButton button, const Pos<float>& pos) {
      double threshold = std::chrono::duration<double>(_doubleClickThreshold).count();
      bool isDouble = last && last->button == button && clickClock - last->time < threshold &&
                      std::hypot(pos.x - last->pos.x, pos.y - last->pos.y) <= _doubleClickReach;
      if (isDouble) last.reset();
      else last = LastClick{button, pos, clickClock};
      return isDouble;
   };
   std::chrono::steady_clock::time_point mousePosChangeTime;
   publish(WindowExecEvent(this));
   using FrameClock = std::chrono::steady_clock;
   auto ms = [](FrameClock::time_point from, FrameClock::time_point to){return std::chrono::duration<double, std::milli>(to - from).count();};
   while (!WindowShouldClose()){
      auto frameStart = FrameClock::now();
      //see if the window size has changed (a replay being played back says what it is)
      Size<float> newSize = _replay ? _replay->beginFrame(getSize()) : getSize();
      if (newSize != size) {
         WindowResizeEvent event(this, getSize());
         size = newSize;
         publish(event);
         //root canvas always same size as window
         canvas->setSize(size);
      }

      // see if the window has moved
      Pos<R_FLOAT> newPos = Pos<R_FLOAT>(getPosition());
      if (newPos != position) {
         WindowMoveEvent event(this, Pos<R_FLOAT>(getPosition()));
         position = newPos;
         event.position = newPos;
         publish(event);
      }

      clickClock += getFrameDelta();

      // input from a test, if one is connected: after raylib has read the real devices, before we do
      if (_testServer) _testServer->beginFrame(_frameCounter, Size<int>((int)size.x, (int)size.y));
      // and recorded, or replaced by the recording
      if (_replay) _replay->input();

      // programatically generated inputs
      while(!_inputQueueKey.empty()){
         auto event = std::move(_inputQueueKey.front());
         _inputQueueKey.pop();
         __process_unhandled_input(*event);
      }

      while(!_inputQueueMouse.empty()){
         auto event = std::move(_inputQueueMouse.front());
         _inputQueueMouse.pop();
         __process_unhandled_input(*event);
      }

      // collect char input (up to limit)
      // only downs for chars - no ups. Use keys for uppies.
      for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
         auto charDown = InputManager::instance().getCharPressed();
         if (charDown) {
            InputEventChar event(this);
            event.ch = charDown;
            auto handledBy = __process_unhandled_input(InputEvent(event));
            if constexpr (PRINT_CHAR) if (handledBy) Logger::info() << "Char handled by " << handledBy.handler->getName() << endl;
         } else {
            break;
         }
      }

      //collect key input (up to limit)
      //do ups first so we don't process up and down on same frame
      for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
         auto keyUp = InputManager::instance().getKeyReleased();
         if (keyUp != InputInterface::KeyCode::KEY_NULL) {
            InputEventKey event(this);
            event.key = keyUp;
            event.isDown = false;
            event.isRepeat = false;
            auto handledBy = __process_unhandled_input(InputEvent(event));
            if constexpr (PRINT_KEYUP) if (handledBy) Logger::info() << "Key up handled by " << handledBy.handler->getName() << endl;
         } else {
            break;
         }
      }

      //REPEATS
      {
         auto now = sc::now();
         static auto keyDownTimestamp = now;
         auto lastKey = InputManager::getLastKeyPressed();
         if (InputInterface::isKeyDown(lastKey)) {
            static auto keyRepeatTimestamp = now;
            if (now - keyDownTimestamp > _keyDownRepeatDelay) {
               //start sending repeats
               if (now - keyRepeatTimestamp > _keyDownRepeatRate) {
                  keyRepeatTimestamp = sc::now();
                  InputEventKey event(this);
                  event.key = lastKey;
                  event.isDown = true;
                  event.isRepeat = true;
                  auto handledBy = __process_unhandled_input(event);
                  if constexpr (PRINT_KEYREPEAT) if (handledBy) Logger::info() << "Key repeat handled by " << handledBy.handler->getName() << endl;
               }
            }
         }

         //DOWNS
         for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
            auto keyDown = InputManager::instance().getKeyPressed();
            if (keyDown != InputInterface::KeyCode::KEY_NULL) {
               keyDownTimestamp = sc::now();
               InputEventKey event(this);
               event.key = keyDown;
               event.isDown = true;
               event.isRepeat = false;
               auto handledBy = __process_unhandled_input(event);
               if constexpr (PRINT_KEYDOWN) if (handledBy) Logger::info() << "Key down handled by " << handledBy.handler->getName() << endl;
            } else {
               break;
            }
         }
      }

      //now do mouse input
      //UPS
      mouseDelta = InputManager::getMouseDelta();
      mousePos = InputManager::getMousePos();
      for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
         auto btnUp = InputManager::instance().getMouseButtonReleased();
         if (btnUp != InputInterface::MouseButton::NONE) {
            bool isDouble = isDoubleClick(lastUp, btnUp, mousePos.get());
            InputEventMouseButton event(this, mousePos.get(), btnUp, false, isDouble);
            auto handledBy = __process_unhandled_input(event);
            if constexpr (PRINT_MOUSEUP) if (handledBy) Logger::info() << "MouseUp handled by " << handledBy.handler->getName() << endl;
         } else {
            break;
         }
      }

      //DOWNS
      for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
         auto btnDown = InputManager::instance().getMouseButtonPressed();
         if (btnDown != InputInterface::MouseButton::NONE) {
            bool isDouble = isDoubleClick(lastDown, btnDown, mousePos.get());
            InputEventMouseButton event(this, mousePos.get(), btnDown, true, isDouble);
            auto handledBy = __process_unhandled_input(event);
            if constexpr (PRINT_MOUSEDOWN) if (handledBy) Logger::info() << "MouseDown handled by " << handledBy.handler->getName() << endl;
         } else {
            break;
         }
      }

      {
         auto wheel = InputManager::getMouseWheel();
         if (wheel) {
            InputEventMouseWheel event(this, mousePos.get(), wheel);
            auto handledBy = __process_unhandled_input(event);
            if constexpr (PRINT_WHEEL) if (handledBy) Logger::info() << "Mouse wheel handled by " << handledBy.handler->getName() << endl;
         }
      }


      //check the mouse delta compared to last frame
      if (mouseDelta) {
         checkedToolTip = false;
         mousePosChangeTime = std::chrono::steady_clock::now();
         InputEventMouseHover hoverEvent(this, mousePos.get());
         InputEventMouseMotion motionEvent(this, mousePos.get(), mouseDelta);
         InputEventMouseToolTip toolTipCancel(this, mousePos.get(), true); //cancel open tooltips as soon as there's mouse movement

         auto handledBy = __process_unhandled_input(motionEvent);
         if constexpr (PRINT_MOTION) if (handledBy) Logger::info() << "Motion handled by " << handledBy.handler->getName() << endl;
         handledBy = __process_unhandled_input(hoverEvent);
         if constexpr (PRINT_HOVER) if (handledBy) Logger::info() << "Hover handled by " << handledBy.handler->getName() << endl;
         handledBy = __process_unhandled_input(toolTipCancel);
         if constexpr (PRINT_TOOLTIP_CANCEL) if (handledBy) Logger::info() << "Tooltip Cancel handled by " << handledBy.handler->getName() << endl;
      }

      //check for tooltips
      if (!checkedToolTip && canvas->getRect().contains(mousePos.get()) && std::chrono::steady_clock::now() - mousePosChangeTime > TOOLTIP_DELAY){
         InputEventMouseToolTip tooltipEvent(this, mousePos.get());
         checkedToolTip = true;
         auto handledBy = __process_unhandled_input(tooltipEvent);
         if constexpr (PRINT_TOOLTIP) if (handledBy) Logger::info() << "Tooltip handled by " << handledBy.handler->getName() << endl;
      }

      //process timers and call their callbacks
      //SystemTime::processTimers();

      //process logic
      auto updateStart = FrameClock::now();
      ProcessList<Tree::Processable>::processAll(getFrameDelta());
      //process and reap easings
      //an easing's callbackAfter may remove or destroy others on the list, so check each one is still there
      for (auto reap : ProcessList<Easing>::processAllDone(getFrameDelta())){
         if (ProcessList<Easing>::contains(reap) && reap->done()) reap->easable()->finishEasing(reap);
      }
      Easable::emptyGraveyard();

      //call deferred calls
      _deferredCallList.executeAllAndClear();
      if (_replay) _replay->endFrame();

      //clean up orphaned textures
      if (ReyTexture::OrphanedTextureCache::instance().size()) {
         ReyTexture::OrphanedTextureCache::instance().clear();
      }

      //render the scene to our internal buffer. A replay playing back fast skips most frames, leaving the window as it
      //was, but for reading the input, which EndDrawing would have done.
      bool render = !_replay || _replay->shouldRender();
      auto drawStart = FrameClock::now();
      if (render) {
         //scoped so the destructor fires
         RenderContext renderContext(_canvas->_renderTarget);
         canvas->renderProcess(renderContext);
      }
      if (_testServer) _testServer->endFrame(_canvas->readRenderTarget());

      //paint the scene on the window
      auto presentStart = FrameClock::now();
      if (render) {
         BeginDrawing();
         auto rect = _canvas->getSizeRect();
         BeginBlendMode(BLEND_ALPHA);
         drawRenderTargetRect(_canvas->readRenderTarget(), rect, rect, Colors::none);
         EndBlendMode();
         EndDrawing();
      } else {
         PollInputEvents();
      }
      auto frameEnd = FrameClock::now();
      _frameTimes = {ms(frameStart, updateStart), ms(updateStart, drawStart), ms(drawStart, presentStart), ms(presentStart, frameEnd), ms(frameStart, frameEnd)};
      _frameCounter++;
   } //next frame

   //window shutdowm
   if (_replay) _replay->stop();
   auto lock = Application::getLock();
   _isClosing = true;
   ProcessList<Tree::Processable>::clear();
   ProcessList<Easing>::clear();
   if (IsAudioDeviceReady()) CloseAudioDevice();
   if (Application::windowCount() == 1){
      Application::exit(Application::ExitReason::CLEAN);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
TestServer* Window::startTestServer(uint16_t port) {
   _testServer = std::make_unique<TestServer>();
   if (!_testServer->start(port)) return nullptr;
   return _testServer.get();
}

/////////////////////////////////////////////////////////////////////////////////////////
Replay& Window::getReplay() {
   if (!_replay) _replay = std::make_unique<Replay>(_targetFPS);
   return *_replay;
}

/////////////////////////////////////////////////////////////////////////////////////////
Window::~Window(){
   Logger::debug() << "Deleting Window" << endl;
}

///////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Canvas> Window::getCanvas() {
   return _root->ref<Canvas>();
}

/////////////////////////////////////////////////////////////////////////////////////////////
Handled Window::__process_unhandled_input(const InputEvent& event) const {
   // Logger::debug() << "---------------------" << endl;
   return _canvas->processInput(event);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void findCanvas(std::vector<Canvas*>& graph, TypeNode* thisNode) {
   if (auto isCanvas = thisNode->as<Canvas>()) {
      graph.push_back(isCanvas.value());
   }
   for (auto& node : thisNode->getChildren()) {
      findCanvas(graph, node);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void Window::_on_tree_updated() {

}