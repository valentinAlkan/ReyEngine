#pragma once
#include <chrono>
#include <ostream>
#include <iomanip>

template <typename T>
class TimeStamp{
   struct DurationParam{};
public:
   using Duration = NamedType<T, DurationParam, ToSizeT>; //delta between two time points.
   struct TimePoint{
      TimePoint(T value): timeUnit(value){}
      TimePoint(const TimePoint& other): timeUnit(other.timeUnit){}
      TimePoint& operator=(const TimePoint& other){timeUnit = other.timeUnit; return *this;}
      static TimePoint epoch(); //the '0' value of our time system. User supplied.
      [[nodiscard]] Duration sinceEpoch() const {return *this - epoch();}
      [[nodiscard]] bool operator==(const TimePoint& other) const {return timeUnit == other.timeUnit;}
      [[nodiscard]] bool operator!=(const TimePoint& other) const {return timeUnit != other.timeUnit;}
      [[nodiscard]] bool operator>(const TimePoint& other) const {return sinceEpoch() > other.sinceEpoch();}
      [[nodiscard]] bool operator>=(const TimePoint& other) const {return sinceEpoch() >= other.sinceEpoch();}
      [[nodiscard]] bool operator<(const TimePoint& other) const {return sinceEpoch() < other.sinceEpoch();}
      [[nodiscard]] bool operator<=(const TimePoint& other) const {return sinceEpoch() <= other.sinceEpoch();}
      TimePoint operator+(const Duration& duration) const {TimePoint t = *this; t.timeUnit += duration.get(); return t;}
      TimePoint& operator+=(const Duration& duration){timeUnit += duration.get(); return *this;}
      TimePoint operator-(const Duration& duration) const {TimePoint t = *this; t.timeUnit -= duration.get(); return t;}
      TimePoint& operator-=(const Duration& duration){timeUnit -= duration.get(); return *this;}
      Duration operator-(const TimePoint& other) const {TimePoint t = *this; return t.timeUnit - other.timeUnit;}
      [[nodiscard]] TimePoint& getHappenedFirst(TimePoint& other){return *this > other ? *this : other;}
      [[nodiscard]] T count() const {return timeUnit;}
   protected:
      T timeUnit;
   };
};
