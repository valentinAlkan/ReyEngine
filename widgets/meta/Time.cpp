#include "Time.h"
#include <sstream>
#include <iomanip>

using namespace std;
using namespace chrono;

/////////////////////////////////////////////////////////////////////////////////////////
std::chrono::time_point<std::chrono::high_resolution_clock> Time::nowHiRes() {
   return high_resolution_clock::now();
}
/////////////////////////////////////////////////////////////////////////////////////////
std::chrono::time_point<std::chrono::steady_clock> Time::nowSteady() {
   return steady_clock::now();
}
/////////////////////////////////////////////////////////////////////////////////////////
std::chrono::time_point<std::chrono::system_clock> Time::nowSystem() {
   return system_clock::now();
}

/////////////////////////////////////////////////////////////////////////////////////////
uint64_t Time::getCurrentFrameCount() {
   return Time::instance()._frame_count;
};

/////////////////////////////////////////////////////////////////////////////////////////
uint64_t Time::getSecondsPassed() {
   auto delta = nowHiRes() - Time::instance()._startTime;
   return duration_cast<seconds>(delta).count();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string Time::nowSystemString() {
   return serializeTimePoint(nowSystem(), "UTC: %Y-%m-%d %H:%M:%S");
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string Time::serializeTimePoint(const std::chrono::system_clock::time_point &time, const std::string &format) {
   std::time_t tt = std::chrono::system_clock::to_time_t(time);
   std::tm tm = *std::gmtime(&tt); //GMT (UTC)
   //std::tm tm = *std::localtime(&tt); //Locale time-zone, usually UTC by default.
   std::stringstream ss;
   ss << std::put_time( &tm, format.c_str() );
   return ss.str();
}