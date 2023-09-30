#pragma once
#include "raylib.h"
#include "StringTools.h"
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
         inline std::string toString() const {return "{" + std::to_string(x) + ", " + std::to_string(y) + "}";}
         inline void fromString(const std::string& s){
            std::string sanitized;
            for (const auto& c : s){
               if (::isdigit(c) || c == '-' || c==',' || c=='.'){
                  sanitized += c;
               }
            }
            auto split = string_tools::split(sanitized, ",");
            if (split.size() != 2){
               return;
            }

            x = std::stoi(split[0]);
            y = std::stoi(split[1]);
         }
         friend std::ostream& operator<<(std::ostream& os, const Vec2<T>& v){os << v.toString();return os;}
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
         inline std::string toString() const {
            return "{" + std::to_string(x) + ", " + std::to_string(y) + ", " +
            std::to_string(width) + ", " + std::to_string(height) + "}";
         }
         inline void fromString(const std::string& s){
            std::string sanitized;
            for (const auto& c : s){
               if (::isdigit(c) || c == '-' || c==',' || c=='.'){
                  sanitized += c;
               }
            }
            auto split = string_tools::split(sanitized, ",");
            if (split.size() != 4){
               return;
            }

            x = std::stoi(split[0]);
            y = std::stoi(split[1]);
            width = std::stoi(split[2]);
            height = std::stoi(split[3]);
         }
         friend std::ostream& operator<<(std::ostream& os, const Rect<T>& r){
            os << r.toString();
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