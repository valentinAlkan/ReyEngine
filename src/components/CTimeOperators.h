#pragma once
#include <ctime>

#ifdef REY_ADD_TIMESPEC_OPERATORS
inline timespec operator+(const timespec& t1, const timespec& t2) {
   timespec result;
   result.tv_sec = t1.tv_sec + t2.tv_sec;
   result.tv_nsec = t1.tv_nsec + t2.tv_nsec;
   // Handle nanoseconds overflow
   if (result.tv_nsec >= 1000000000) {
      result.tv_sec += 1;
      result.tv_nsec -= 1000000000;
   }
   return result;
}

inline timespec operator-(const timespec& t1, const timespec& t2) {
   timespec result;
   result.tv_sec = t1.tv_sec - t2.tv_sec;
   result.tv_nsec = t1.tv_nsec - t2.tv_nsec;
   if (result.tv_nsec < 0) {
      result.tv_sec -= 1;
      result.tv_nsec += 1000000000;
   }
   return result;
}

inline bool operator>(const timespec& t1, const timespec& t2) {
   if (t1.tv_sec > t2.tv_sec) return true;
   return t1.tv_sec == t2.tv_sec && t1.tv_nsec > t2.tv_nsec;
}

inline bool operator<(const timespec& t1, const timespec& t2) {
   if (t1.tv_sec < t2.tv_sec) return true;
   return t1.tv_sec == t2.tv_sec && t1.tv_nsec < t2.tv_nsec;
}

inline bool operator==(const timespec& t1, const timespec& t2) {
   return t1.tv_sec == t2.tv_sec && t1.tv_nsec == t2.tv_nsec;
}
inline bool operator>=(const timespec& t1, const timespec& t2) {return t1 > t2 || t1 == t2;}
inline bool operator<=(const timespec& t1, const timespec& t2) {return t1 < t2 || t1 == t2;}
#endif