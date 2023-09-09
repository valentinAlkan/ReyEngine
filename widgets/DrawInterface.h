#pragma once
#include "raylib.h"
#include <string>

namespace GFCSDraw {
      template <typename T>
      struct Vec2 {
         Vec2(): x(0), y(0){}
         Vec2(const T _x, const T _y) : x(_x), y(_y){}
         Vec2(const Vector2& v)     : x((T)v.x), y((T)v.y){}
         Vec2(const Vec2<int>& v)   : x((T)v.x), y((T)v.y){}
         Vec2(const Vec2<float>& v) : x((T)v.x), y((T)v.y){}
         Vec2(const Vec2<double>& v): x((T)v.x), y((T)v.y){}
         Vec2& operator+=(const Vec2& rhs){x += rhs.x; y += rhs.y; return *this;}
         Vec2& operator-=(const Vec2& rhs){x -= rhs.x; y -= rhs.y; return *this;}
         Vec2& operator*=(const Vec2& rhs){x *= rhs.x; y *= rhs.y; return *this;}
         Vec2& operator/=(const Vec2& rhs){x /= rhs.x; y /= rhs.y; return *this;}
         T x;
         T y;
      };

      template <typename T>
      struct Rect {
         Rect(): x(0), y(0), width(0), height(0){}
         Rect(const T x, const T y) : x(x), y(y){}
         Rect(const Rectangle& r): x((T)x), y((T)y), width((T)width), height((T)height){}
         Rect(const Vec2<int>& v): x((T)x), y((T)y){}
         Rect(const Vec2<float>& v): x((T)x), y((T)y){}
         Rect(const Vec2<double>& v): x((T)x), y((T)y){}
         T x;
         T y;
         T width;
         T height;
      };
}

GFCSDraw::Vec2<double> getScreenCenter();
void drawTextCentered(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color);
void drawTextRelative(const std::string& text, const GFCSDraw::Vec2<int>& relPos, int fontSize, Color color);