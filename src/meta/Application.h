#pragma once
#include <memory>
#include <filesystem>
#include <queue>
#include "Window.h"
#include "Logger.h"
#include "DrawInterface.h"
#include "Platform.h"
#include "FileSystem.h"
#include "Component.h"
#include <thread>

namespace ReyEngine{
   namespace Internal{
      class Component;
   }
   class Application
   {
   public:
      enum class Platform{WINDOWS, LINUX};
      static Application& instance(){
         static Application instance;
         return instance;
      }
   private:
      Application();
   public:
      ~Application(){
         Logger::debug() << "Deleting Application" << std::endl;
      }
      enum class ExitReason{
         CLEAN = 0,
         INVALID_ARGS = 1,
         INVALID_SCENE_FILE_FORMAT,
      };
      using UniqueValue = uint64_t;
      Application(Application const&)    = delete;
      void operator=(Application const&) = delete;

      static Internal::WindowPrototype createWindowPrototype(const std::string& title, int width, int height, const std::vector<ReyEngine::Window::Flags>& flags, int targetFPS=60);
      static Window& getWindow(int windowIndex){return *(instance()._windows.at(windowIndex));}
      static size_t windowCount(){return instance()._windows.size();}

      static void exitError(std::string msg, ExitReason rsn){Logger::error() << msg << std::endl; ::exit((int)rsn);}
      static void exit(ExitReason rsn){::exit((int)rsn);}

      static void registerForApplicationReady(std::shared_ptr<Internal::Component>&); //somethings require initwindow to have been called - so we can let the application know we want to be called when application is ready.
      static void registerForApplicationReady(std::function<void()>); //somethings require initwindow to have been called - so we can let the application know we want to be called when application is ready.
      static bool isReady(){return instance()._is_ready;}
      static std::unique_lock<std::mutex> getLock(); //use this to syncrhonize with the engine
      static constexpr Platform getPlatform(){return PLATFORM;}
      static UniqueValue generateUniqueValue(){return instance()._nextUniqueValue++;}
      static double generateRandom(double low, double high);
      static long double secondsSinceInit();
   protected:
      Window& createWindow(Internal::WindowPrototype&, std::optional<std::shared_ptr<Canvas>>);
      static uint64_t getNewRid(){return ++instance().newRid;}
      static void ready();
      std::chrono::time_point<std::chrono::steady_clock> _startTime;

   private:
      UniqueValue _nextUniqueValue = 0;
      #ifdef REYENGINE_PLATFORM_WINDOWS
         static constexpr Platform PLATFORM = Platform::WINDOWS;
      #elif defined(REYENGINE_PLATFORM_LINUX)
         static constexpr Platform PLATFORM = Platform::LINUX;
      #endif
      bool _is_ready = false;

     //for now, only one. Note: Windows MUST be shared_ptrs, as they are TypeContainers, which leverages shared_from_this,
     // which requires that a shared_ptr exist before being called. HOWEVER, we do not hand out the shared_ptrs, only references to
     // the underlying window, in order to maintain strict ownership.
      std::vector<std::shared_ptr<Window>> _windows;
      uint64_t newRid;
      std::mutex _busy; //the main mutex that determines if the engine is busy or not
      std::unordered_set<std::shared_ptr<Internal::Component>> _applicationReadyList; //list of widgets that want to be notified when the application is fully initialized
      std::vector<std::function<void()>> _initListArbCallback; //list of arbitrary callbacks that serve the same purpose as the above

      //init list
      using InitPair = std::pair<std::shared_ptr<Internal::Component>, std::shared_ptr<Internal::Component>>;
      std::queue<InitPair> _initQueue;
      friend class Internal::Component;
      friend class Internal::WindowPrototype;
      friend class Window;
   };
}