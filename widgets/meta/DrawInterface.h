#pragma once
#include "raylib.h"
#include <stdexcept>
#include "StringTools.h"
#include <string>

#define NOT_IMPLEMENTED throw std::runtime_error("Not implemented!")

namespace GFCSDraw {
   template <typename T>
   struct Vec2 {
      inline Vec2(): x(0), y(0){}
      inline Vec2(const T _x, const T _y) : x(_x), y(_y){}
      inline Vec2(const Vector2& v)     : x((T)v.x), y((T)v.y){}
      inline Vec2(const Vec2<int>& v)   : x((T)v.x), y((T)v.y){}
      inline Vec2(const Vec2<float>& v) : x((T)v.x), y((T)v.y){}
      inline Vec2(const Vec2<double>& v): x((T)v.x), y((T)v.y){}
      inline explicit operator bool() const {return x || y;}
      inline Vec2 operator+(const Vec2& rhs) const {Vec2<T> val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Vec2 operator-(const Vec2& rhs) const {Vec2<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Vec2& operator+=(const Vec2& rhs){x += rhs.x; y += rhs.y; return *this;}
      inline Vec2& operator-=(const Vec2& rhs){x -= rhs.x; y -= rhs.y; return *this;}
      inline Vec2& operator*=(const Vec2& rhs){x *= rhs.x; y *= rhs.y; return *this;}
      inline Vec2& operator/=(const Vec2& rhs){x /= rhs.x; y /= rhs.y; return *this;}
      inline Vec2 midpoint(){return {x/2, y/2};}
      inline double pct(double input){return (input-x)/(y-x);} //given an input value, what percentage of the range is it from 0 to 1?
      inline double lerp(double lerpVal){return lerpVal * (y-x) + x;} //given a value from 0 to 1, what is the value of the range that corresponds to it?
      inline Vec2 lerp(Vec2 otherPoint, double xprm){return {xprm, y+(((xprm-x)*(otherPoint.y-y))/(otherPoint.x-x))};}
      inline T clamp(T value){if (value < x) return x; if (value > y) return y; return value;}
      [[nodiscard]] inline std::string toString() const {return "{" + std::to_string(x) + ", " + std::to_string(y) + "}";}
      inline static void fromString(const std::string& s){
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
         GFCSDraw::Vec2<T> retval;
         retval.x = std::stoi(split[0]);
         retval.y = std::stoi(split[1]);
         return retval;
      }
      friend std::ostream& operator<<(std::ostream& os, const Vec2<T>& v){os << v.toString();return os;}
      T x;
      T y;
   };

   template <typename T>
   struct Line {
      inline Line(): a(0,0), b(0,0){}
      inline Line(Vec2<T> a, Vec2<T> b): a(a), b(b){}
      inline Line(const T x1, const T y1, const T x2, const T y2): Line({x1, y1}, {x2, y2}){}
      inline Line midpoint(){return {(a.x+b.x)/2, (a.y+b.y)/2};}
      inline Vec2<T> lerp(double xprm){return a.lerp(b, xprm);}
      inline double distance(){NOT_IMPLEMENTED;}
      Vec2<T> a;
      Vec2<T> b;
   };

   template <typename T>
   struct Rect {
      inline Rect(): x(0), y(0), width(0), height(0){}
      inline Rect(const T x, const T y) : x(x), y(y){}
      inline Rect(const T x, const T y, const T width, const T height) : x(x), y(y), width(width), height(height){}
      inline explicit Rect(const Rectangle& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline Rect(const Rect<int>& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline Rect(const Rect<float>& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline Rect(const Rect<double>& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline explicit Rect(const Vec2<T>& v): x((T)v.x), y((T)v.y){}
      inline Rect(const Vec2<T>& pos, const Vec2<T>& size): x((T)pos.x), y((T)pos.y), width((T)size.x), height((T)size.y){}
      inline Rect operator+(const Vec2<T>& rhs) const {Rect<T> val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Rect operator-(const Vec2<T>& rhs) const {Rect<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Rect& operator+=(const Vec2<T>& rhs){x += rhs.x; y += rhs.y; return *this;}
      inline Rect& operator-=(const Vec2<T>& rhs){x -= rhs.x; y -= rhs.y; return *this;}
      inline Rect& operator*=(const Vec2<T>& rhs){x *= rhs.x; y *= rhs.y; return *this;}
      inline Rect& operator/=(const Vec2<T>& rhs){x /= rhs.x; y /= rhs.y; return *this;}
      inline Rect operator+(const Rect<T>& rhs) const {Rect<T> val = *this; val.x += rhs.x; val.y += rhs.y; val.width += rhs.width; val.height += rhs.height; return val;}
      inline Rect operator-(const Rect<T>& rhs) const {Rect<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; val.width -= rhs.width; val.height -= rhs.height; return val;}
      inline Rect& operator+=(const Rect<T>& rhs){x += rhs.x; y += rhs.y; width += rhs.width; height += rhs.height; return *this;}
      inline Rect& operator-=(const Rect<T>& rhs){x -= rhs.x; y -= rhs.y; width -= rhs.width; height -= rhs.height; return *this;}
      inline Rect& operator*=(const Rect<T>& rhs){x *= rhs.x; y *= rhs.y; width *= rhs.width; height *= rhs.height; return *this;}
      inline Rect& operator/=(const Rect<T>& rhs){x /= rhs.x; y /= rhs.y; width /= rhs.width; height /= rhs.height; return *this;}
      inline bool isInside(const Vec2<T>& point){
         return (point.x > x && point.x < x + width) &&
               (point.y > y && point.y < y + height);
      }
      [[nodiscard]] inline Vec2<T> center() const {return {(x+width)/2, (y+height)/2};}
      inline void setCenter(const Vec2<T>& center) {x = center.x-width/2; y=center.y-height/2;}
      [[nodiscard]] inline std::string toString() const {
         return "{" + std::to_string(x) + ", " + std::to_string(y) + ", " +
         std::to_string(width) + ", " + std::to_string(height) + "}";
      }
      inline static GFCSDraw::Rect<T> fromString(const std::string& s){
         std::string sanitized;
         for (const auto& c : s){
            if (::isdigit(c) || c == '-' || c==',' || c=='.'){
               sanitized += c;
            }
         }
         auto split = string_tools::split(sanitized, ",");
         if (split.size() != 4){
            return GFCSDraw::Rect<T>();
         }

         GFCSDraw::Rect<T> retval;
         retval.x = std::stoi(split[0]);
         retval.y = std::stoi(split[1]);
         retval.width = std::stoi(split[2]);
         retval.height = std::stoi(split[3]);
         return retval;
      }
      friend std::ostream& operator<<(std::ostream& os, const Rect<T>& r){
         os << r.toString();
         return os;
      }
      [[nodiscard]] inline Vec2<T> pos() const {return {x, y};}
      [[nodiscard]] inline Vec2<T> size() const {return {width, height};}
      [[nodiscard]] inline Rect<T> toSizeRect() const {return {0,0,width, height};}

      T x;
      T y;
      T width;
      T height;
   };

   Vec2<double> getScreenCenter();
   Vec2<double> getScreenSize();
   void drawText(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color);
   void drawTextCentered(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color);
   void drawTextRelative(const std::string& text, const GFCSDraw::Vec2<int>& relPos, int fontSize, Color color);
   void drawRectangle(const GFCSDraw::Rect<int>&, Color color);
   void drawRectangleRounded(const GFCSDraw::Rect<float>&, float roundness, int segments, Color color);
   void drawRectangleRoundedLines(const GFCSDraw::Rect<float>&, float roundness, int segments, float lineThick, Color color);
   void drawRectangleGradientV(const GFCSDraw::Rect<int>&, Color color1, Color color2);
   inline float getFrameDelta(){return GetFrameTime();}

   inline Font getDefaultFont(){return GetFontDefault();}
   inline Vec2<int> measureText(Font font, const char *text, float fontSize, float spacing){return MeasureTextEx(font, text, fontSize, spacing);}

   class RenderTarget{
      public:
         RenderTarget(const Vec2<int>& size);
         ~RenderTarget(){
            if (_texLoaded) {
               UnloadRenderTexture(_tex);
            }
         }
         void resize(const Vec2<int>& newSize);
         void beginRenderMode(){BeginTextureMode(_tex);}
         void endRenderMode(){EndTextureMode();}
         void clear(Color color=WHITE){ClearBackground(color);}
         void render(Vec2<float> pos) const{
            // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
            DrawTextureRec(_tex.texture, {0, 0, (float)_tex.texture.width, (float)-_tex.texture.height }, {pos.x, pos.y}, {255,255,255,210});
         }
      protected:
         bool _texLoaded = false;
         RenderTexture2D _tex;
         Vec2<float> _size;
      };

   //Everything drawn to a scissor target will be invisible
   class ScissorTarget {
   public:
      void start(Rect<double> scissorArea){BeginScissorMode((int)scissorArea.x, (int)scissorArea.y, (int)scissorArea.width, (int)scissorArea.height);}
      void stop(){EndScissorMode();}
   };
}

namespace InputInterface{
   using KeyCode = int;
   enum class MouseButton{
      MOUSE_BUTTON_NONE = -1,
      MOUSE_BUTTON_LEFT = MOUSE_BUTTON_LEFT,
      MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_RIGHT,
      MOUSE_BUTTON_BACK = MOUSE_BUTTON_BACK,
      MOUSE_BUTTON_EXTRA = MOUSE_BUTTON_EXTRA,
      MOUSE_BUTTON_FORWARD = MOUSE_BUTTON_FORWARD,
      MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_MIDDLE,
      MOUSE_BUTTON_SIDE = MOUSE_BUTTON_SIDE,
   };

   //array of all mouse buttons
   static constexpr MouseButton MouseButtons[] = {
         MouseButton::MOUSE_BUTTON_LEFT,
         MouseButton::MOUSE_BUTTON_RIGHT,
         MouseButton::MOUSE_BUTTON_MIDDLE,
         MouseButton::MOUSE_BUTTON_BACK,
         MouseButton::MOUSE_BUTTON_FORWARD,
         MouseButton::MOUSE_BUTTON_SIDE,
         MouseButton::MOUSE_BUTTON_EXTRA,
   };

   inline float getMouseWheelMove(){return GetMouseWheelMove();} //returns largest of x or y
   inline GFCSDraw::Vec2<float> getMouseWheelMoveV(){return GetMouseWheelMoveV();} //returns both x and y

   inline bool isKeyPressed(KeyCode key){IsKeyPressed(key);}
   inline bool isKeyDown(KeyCode key){return IsKeyDown(key);}
   inline bool isKeyReleased(KeyCode key){return IsKeyReleased(key);}
   inline bool isKeyUp(KeyCode key){return IsKeyUp(key);}
   inline void setExitKey(KeyCode key){return SetExitKey(key);}
   inline KeyCode getKeyPressed(){return GetKeyPressed();}
   inline KeyCode getCharPressed(){return GetCharPressed();}
   inline bool isMouseButtonPressed(MouseButton btn){return IsMouseButtonPressed(static_cast<int>(btn));}
   inline bool isMouseButtonDown(MouseButton btn){return IsMouseButtonDown(static_cast<int>(btn));}
   inline bool isMouseButtonUp(MouseButton btn){return IsMouseButtonUp(static_cast<int>(btn));}
   inline bool isMouseButtonReleased(MouseButton btn){return IsMouseButtonReleased(static_cast<int>(btn));}
   inline GFCSDraw::Vec2<int> getMousePos(){return GetMousePosition();}
   inline GFCSDraw::Vec2<int> getMouseDelta(){return GetMouseDelta();}
}