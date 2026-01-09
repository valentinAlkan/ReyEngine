#pragma once
//carefully swiped from easings.net
#include <StrongUnits.h>
#include <cmath>
#include <chrono>
#include "ProcessList.h"

namespace ReyEngine {
   #ifndef EASINGS_FLOAT_TYPE
   #define EASINGS_FLOAT_TYPE double
   //the reason this is a double is that we would lose integer precision after about 3 and some change days
   // running at 60 frames per second if it were a float. With a double, that time frame becomes a little
   // over 4 million years. It is somewhat unlikely that anyone would leave a system running for that long.
   #endif

   namespace Easings {
      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction ease1(Fraction x) {return 1;}
      static constexpr inline Fraction ease0(Fraction x) {return 0;}
      static constexpr inline Fraction easeLinear(Fraction x) {return x;}
      static constexpr inline Fraction easeInSine(Fraction x) {
         return 1 - std::cos((x.get() * M_PI) / 2);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeOutSine(Fraction x) {
         return std::sin((x.get() * M_PI) / 2);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInOutSine(Fraction x) {
         return -(std::cos(M_PI * x.get()) - 1) / 2;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInQuad(Fraction x) {
         return x.get() * x;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeOutQuad(Fraction x) {
         return 1 - (1 - x.get()) * (1 - x);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInOutQuad(Fraction x) {
         return x.get() < 0.5 ? 2 * x.get() * x.get() : 1 - std::pow(-2 * x.get() + 2, 2) / 2;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInCubic(Fraction x) {
         return x.get() * x.get() * x;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeOutCubic(Fraction x) {
         return 1 - std::pow(1 - x.get(), 3);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInOutCubic(Fraction x) {
         return x.get() < 0.5 ? 4 * x.get() * x.get() * x.get() : 1 - std::pow(-2 * x.get() + 2, 3) / 2;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInQuart(Fraction x) {
         return x.get() * x.get() * x.get() * x;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeOutQuart(Fraction x) {
         return 1 - std::pow(1 - x.get(), 4);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInOutQuart(Fraction x) {
         return x.get() < 0.5 ? 8 * x.get() * x.get() * x.get() * x.get() : 1 - std::pow(-2 * x.get() + 2, 4) / 2;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInQuint(Fraction x) {
         return x.get() * x.get() * x.get() * x.get() * x;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeOutQuint(Fraction x) {
         return 1 - std::pow(1 - x.get(), 5);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInOutQuint(Fraction x) {
         return x.get() < 0.5 ? 16 * x.get() * x.get() * x.get() * x.get() * x.get() : 1 -
                                                                                       std::pow(-2 * x.get() + 2, 5) /
                                                                                       2;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInExpo(Fraction x) {
         return x.get() == 0 ? 0 : std::pow(2, 10 * x.get() - 10);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeOutExpo(Fraction x) {
         return x.get() == 1 ? 1 : 1 - std::pow(2, -10 * x.get());
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInOutExpo(Fraction x) {
         return x.get() == 0
                ? 0
                : x.get() == 1
                  ? 1
                  : x.get() < 0.5 ? std::pow(2, 20 * x.get() - 10) / 2
                                  : (2 - std::pow(2, -20 * x.get() + 10)) / 2;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInCirc(Fraction x) {
         return 1 - std::sqrt(1 - std::pow(x.get(), 2));
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeOutCirc(Fraction x) {
         return std::sqrt(1 - std::pow(x.get() - 1, 2));
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInOutCirc(Fraction x) {
         return x.get() < 0.5
                ? (1 - std::sqrt(1 - std::pow(2 * x.get(), 2))) / 2
                : (std::sqrt(1 - std::pow(-2 * x.get() + 2, 2)) + 1) / 2;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInBack(Fraction x) {
         const EASINGS_FLOAT_TYPE c1 = 1.70158;
         const EASINGS_FLOAT_TYPE c3 = c1 + 1;
         return c3 * x.get() * x.get() * x.get() - c1 * x.get() * x;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeOutBack(Fraction x) {
         const EASINGS_FLOAT_TYPE c1 = 1.70158;
         const EASINGS_FLOAT_TYPE c3 = c1 + 1;
         return 1 + c3 * std::pow(x.get() - 1, 3) + c1 * std::pow(x.get() - 1, 2);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInOutBack(Fraction x) {
         const EASINGS_FLOAT_TYPE c1 = 1.70158;
         const EASINGS_FLOAT_TYPE c2 = c1 * 1.525;
         return x.get() < 0.5
                ? (std::pow(2 * x.get(), 2) * ((c2 + 1) * 2 * x.get() - c2)) / 2
                : (std::pow(2 * x.get() - 2, 2) * ((c2 + 1) * (x.get() * 2 - 2) + c2) + 2) / 2;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInElastic(Fraction x) {
         const EASINGS_FLOAT_TYPE c4 = (2 * M_PI) / 3;
         return x.get() == 0
                ? 0
                : x.get() == 1
                  ? 1
                  : -std::pow(2, 10 * x.get() - 10) * std::sin((x.get() * 10 - 10.75) * c4);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeOutElastic(Fraction x) {
         const EASINGS_FLOAT_TYPE c4 = (2 * M_PI) / 3;
         return x.get() == 0
                ? 0
                : x.get() == 1
                  ? 1
                  : std::pow(2, -10 * x.get()) * std::sin((x.get() * 10 - 0.75) * c4) + 1;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInOutElastic(Fraction x) {
         const EASINGS_FLOAT_TYPE c5 = (2 * M_PI) / 4.5;
         return x.get() == 0
                ? 0
                : x.get() == 1
                  ? 1
                  : x.get() < 0.5
                    ? -(std::pow(2, 20 * x.get() - 10) * std::sin((20 * x.get() - 11.125) * c5)) / 2
                    : (std::pow(2, -20 * x.get() + 10) * std::sin((20 * x.get() - 11.125) * c5)) / 2 + 1;
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeOutBounce(Fraction x) {
         const EASINGS_FLOAT_TYPE n1 = 7.5625;
         const EASINGS_FLOAT_TYPE d1 = 2.75;

         if (x.get() < 1 / d1) {
            return n1 * x.get() * x;
         } else if (x.get() < 2 / d1) {
            x.get() -= 1.5 / d1;
            return n1 * x.get() * x.get() + 0.75;
         } else if (x.get() < 2.5 / d1) {
            x.get() -= 2.25 / d1;
            return n1 * x.get() * x.get() + 0.9375;
         } else {
            x.get() -= 2.625 / d1;
            return n1 * x.get() * x.get() + 0.984375;
         }
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInBounce(Fraction x) {
         return 1 - easeOutBounce(1 - x);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      static constexpr inline Fraction easeInOutBounce(Fraction x) {
         return x.get() < 0.5
                ? (1 - easeOutBounce(1 - 2 * x)) / 2
                : (1 + easeOutBounce(2 * x.get() - 1)) / 2;
      }
   }

   using EasingFunctor = std::function<Fraction(Fraction)>;
   using EasingCallback = std::function<void(Fraction)>;
   struct Easable;
   // an easing function is any function that takes an input between 0 and 1 and returns an output that typically
   // represents a value between 0 and 1 (min and max) although the output value is not strictly enforced since
   // overshoot is allowed
   struct Easing {
      Easing(EasingFunctor&& easing,
             std::chrono::milliseconds duration, const EasingCallback& callbackDuring, const std::function<void()>& callbackAfter=nullptr)
      : _duration(duration)
      , _callbackDuring(callbackDuring)
      , _callbackAfter(callbackAfter)
      , _startTime(std::chrono::steady_clock::now())
      , functor(std::move(easing))
      {}
      ~Easing(){
         if (_callbackAfter) _callbackAfter();
      }
      bool _process(float _){
         (void)_;//needed to conform with the process list api
         auto now = std::chrono::steady_clock::now();
         if (_startTime > now) return false; //only process easings that have a start time in the past
         auto msDuration = _duration.count();
         auto dt = std::chrono::duration_cast<std::chrono::milliseconds>((now - _startTime)).count();
         input = (double)dt / (double)msDuration;
         if (input>1) input = 1; //clamp output
         output = functor(input);
         if (_callbackDuring) _callbackDuring(output); //alert the easable that the easing has been processed
         return input == 1;
      }
      [[nodiscard]] bool done() const {return input >= 1.0;}
      [[nodiscard]] Easable* easable() const {return _easable;}
      [[nodiscard]] std::chrono::steady_clock::time_point endTime() const {return _startTime + _duration;}
   protected:
      Easable* _easable;
   private:
      std::chrono::steady_clock::time_point _startTime;
      std::chrono::milliseconds _duration;
      EasingCallback _callbackDuring;
      std::function<void()> _callbackAfter;
      const EasingFunctor functor;
      Fraction input;
      Fraction output;
      friend struct Easable;
   };

   struct Easable  {
      virtual ~Easable(){
         for (auto& easing : _easings){
            removeEasing(easing.get());
         }
      };
      template <typename... Args>
      Easing* addEasing(Args&&... args){
         _wantsEase = true;
         _easings.push_back(make_unique<Easing>(std::forward<Args>(args)...));
         auto retval = _easings.back().get();
         ProcessList<Easing>::add(retval, _isEased);
         retval->_easable = this;
         return retval;
      }

      void chainEasings(std::vector<std::unique_ptr<Easing>>&& easings){
         _wantsEase = true;
         Easing* last = _easings.empty() ? nullptr : _easings.back().get();
         for (auto& easing : easings){
            _easings.push_back(std::move(easing));
            auto back = _easings.back().get();
            ProcessList<Easing>::add(back, _isEased);
            back->_easable = this;
            if (last){
               back->_startTime = last->endTime();
            }
            last = back;
         }
      }
      void removeEasing(Easing* easing) {
         ProcessList<Easing>::remove(easing, _isEased);
         for (auto it = _easings.begin(); it != _easings.end(); ++it){
            if (easing == it->get()){
               //store the easing, so we can destroy it outside the erase function
               auto ptr = std::move(*it);
               _easings.erase(it);
               return;
            }
         }
      }
      void removeAllEasings(){
         ProcessList<Easing>::removeAll(_isEased);
         _easings.clear();
      }
      [[nodiscard]] bool isEased() const {return _isEased;}
   protected:
      bool _wantsEase = false;
      bool _isEased = false;
      std::vector<std::unique_ptr<Easing>> _easings;
   };
   static std::unique_ptr<ProcessList<Easing>> _easingList;
}