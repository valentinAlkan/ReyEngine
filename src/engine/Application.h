#pragma once
#include <memory>
#include <filesystem>
#include <queue>
#include <thread>
#include <random>
#include "Logger.h"
#include "ReyEngine.h"
#include "Platform.h"
#include "FileSystem.h"
#include "UniqueValue.h"

namespace ReyEngine{
   namespace Internal{
      class WindowPrototype;
   }
   class Window;
   class Canvas;
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
         Logger::debug() << "Deleting Application2" << std::endl;
      }
      enum class ExitReason{
         CLEAN = 0,
         INVALID_ARGS = 1,
         INVALID_SCENE_FILE_FORMAT,
      };
      Application(Application const&)    = delete;
      void operator=(Application const&) = delete;

      static std::unique_ptr<Internal::WindowPrototype> createWindowPrototype(const std::string& title, int width, int height, const std::vector<ReyEngine::WindowFlags>& flags, int targetFPS=60);
      static Window& getWindow(int windowIndex){return *(instance()._windows.at(windowIndex));}
      static size_t windowCount(){return instance()._windows.size();}

      static void exitError(std::string msg, ExitReason rsn){Logger::error() << msg << std::endl; ::exit((int)rsn);}
      static void exit(ExitReason rsn){::exit((int)rsn);}

//      static void registerForApplication2Ready(std::shared_ptr<Internal::Component>&); //somethings require initwindow to have been called - so we can let the Application2 know we want to be called when Application2 is ready.
//      static void registerForApplication2Ready(std::function<void()>); //somethings require initwindow to have been called - so we can let the Application2 know we want to be called when Application2 is ready.
      static bool isReady(){return instance()._is_ready;}
      static std::unique_lock<std::mutex> getLock(); //use this to syncrhonize with the engine
      static constexpr Platform getPlatform(){return PLATFORM;}
      static UniqueValue generateUniqueValue(){return instance().uniqueGenerator.makeNew();}
      template <typename T=double>
      static T generateRandom(T low, T high){
         std::random_device rd;
         std::mt19937 gen(rd());
         std::uniform_real_distribution<> dis(low, high);
         return dis(gen);
      };
      static long double secondsSinceInit();
   protected:
      Window& createWindow(Internal::WindowPrototype&, std::optional<std::shared_ptr<Canvas>>);
      static uint64_t getNewRid(){return ++instance().newRid;}
      static void ready();
      std::chrono::time_point<std::chrono::steady_clock> _startTime;

   private:
      Internal::UniqueGenerator uniqueGenerator;
#ifdef REYENGINE_PLATFORM_WINDOWS
      static constexpr Platform PLATFORM = Platform::WINDOWS;
#elif defined(REYENGINE_PLATFORM_LINUX)
      static constexpr Platform PLATFORM = Platform::LINUX;
#endif
      bool _is_ready = false;

      //for now, only one.
      std::vector<Window*> _windows;
      uint64_t newRid;
      std::mutex _busy; //the main mutex that determines if the engine is busy or not
//      std::vector<std::function<void()>> _Application2ReadyList; //list of widgets that want to be notified when the Application2 is fully initialized
//      std::vector<std::function<void()>> _initListArbCallback; //list of arbitrary callbacks that serve the same purpose as the above

      //init list
//      using InitPair = std::pair<std>, std::shared_ptr<Internal::Component>>;
//      std::queue<InitPair> _initQueue;
//      friend class Internal::Component;
      friend class Internal::WindowPrototype;
      friend class Window;
   };
}