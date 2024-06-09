#include "Thread.h"
#include "Logger.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
Thread::Thread(const std::string& threadName)
: threadName(threadName)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
Thread::~Thread(){
   stop();
   if (_t.joinable()){
      _t.join();
   }
   Logger::info() << "Thread " << threadName << " exited with return value " << _retval << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Thread::start() {
   _t = std::thread(&Thread::run, this);
   Logger::info() << "Thread " << threadName << " started!" << endl;
}
/////////////////////////////////////////////////////////////////////////////////////////
void Thread::stop() {
   _shutdown = true;
}