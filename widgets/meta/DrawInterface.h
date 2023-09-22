#pragma once
#include "raylib.h"
#include <string>

namespace GFCSDraw {
      template <typename T>
      struct Vec2 {
         inline Vec2(): x(0), y(0){}
         inline Vec2(const T _x, const T _y) : x(_x), y(_y){}
         inline Vec2(const Vector2& v)     : x((T)v.x), y((T)v.y){}
         inline Vec2(const Vec2<int>& v)   : x((T)v.x), y((T)v.y){}
         inline Vec2(const Vec2<float>& v) : x((T)v.x), y((T)v.y){}
         inline Vec2(const Vec2<double>& v): x((T)v.x), y((T)v.y){}
         inline Vec2& operator+=(const Vec2& rhs){x += rhs.x; y += rhs.y; return *this;}
         inline Vec2& operator-=(const Vec2& rhs){x -= rhs.x; y -= rhs.y; return *this;}
         inline Vec2& operator*=(const Vec2& rhs){x *= rhs.x; y *= rhs.y; return *this;}
         inline Vec2& operator/=(const Vec2& rhs){x /= rhs.x; y /= rhs.y; return *this;}
         friend std::ostream& operator<<(std::ostream& os, const Vec2<T>& v){
            os << "{" + std::to_string(v.x) + ", " + std::to_string(v.y) + "}";
            return os;
         }
         T x;
         T y;
      };

      template <typename T>
      struct Rect {
         inline Rect(): x(0), y(0), width(0), height(0){}
         inline Rect(const T x, const T y) : x(x), y(y){}
         inline Rect(const Rectangle& r): x((T)x), y((T)y), width((T)width), height((T)height){}
         inline Rect(const Vec2<int>& v): x((T)x), y((T)y){}
         inline Rect(const Vec2<float>& v): x((T)x), y((T)y){}
         inline Rect(const Vec2<double>& v): x((T)x), y((T)y){}
         friend std::ostream& operator<<(std::ostream& os, const Rect<T>& r){
            os << "{" + std::to_string(v.x) + ", " + std::to_string(v.y) + "}";
            return os;
         }
         T x;
         T y;
         T width;
         T height;
      };

      GFCSDraw::Vec2<double> getScreenCenter();
      void drawText(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color);
      void drawTextCentered(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color);
      void drawTextRelative(const std::string& text, const GFCSDraw::Vec2<int>& relPos, int fontSize, Color color);
      GFCSDraw::Vec2<int> getMousePos();
      float getFrameDelta();
}