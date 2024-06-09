#pragma once
#include <thread>
#include <string>

namespace ReyEngine {
   class Thread {
   public:
      Thread(const std::string& threadName);
      virtual ~Thread();
      virtual void run() = 0;
      void start();
      void stop();
      bool shutdownRequested(){return _shutdown;}
   protected:
      void setReturnValue(int retval){ _retval = retval;}
   private:
      bool _shutdown = false;
      bool _restartable = true;
      bool _running = false;
      std::thread _t;
      const std::string threadName;
      int _retval = 0;
   };
}
