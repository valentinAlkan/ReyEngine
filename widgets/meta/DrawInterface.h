#pragma once
#include "raylib.h"
#include <stdexcept>
#include "StringTools.h"
#include <string>
#include <array>
#include <iostream>
#define NOT_IMPLEMENTED throw std::runtime_error("Not implemented!")

namespace GFCSDraw {
   template <typename T> struct Size;
   template <typename T> struct Pos;
   template <typename T>
   struct Vec {
      explicit Vec(size_t size): size(size){}
      inline static std::vector<T> fromString(size_t size, const std::string& s) {
         std::string sanitized;
         for (const auto &c: s) {
            if (::isdigit(c) || c == '-' || c == ',' || c == '.') {
               sanitized += c;
            }
         }
         auto split = string_tools::split(sanitized, ",");
         if (split.size() != size) {
            return;
         }

         std::vector<double> retval;
         for (size_t i = 0; i < size; i++) {
            retval.push_back(std::stod(split[i]));
         }
         return retval;
      }
      [[nodiscard]] inline std::string toString() const {
         auto elements = getElements();
         std::string retval = "{";
         for (const auto& element : elements){
            retval += std::to_string(element);
            retval += ", ";
         }
         //remeove trailing comma
         retval.pop_back();
         retval.pop_back();
         retval += "}";
         return retval;
      }
      [[nodiscard]] virtual std::vector<T> getElements() const = 0;
      const size_t size;
   };


   template <typename T>
   struct Vec2 : protected Vec<T> {
      inline Vec2(): Vec<T>(2), x(0), y(0){}
      inline Vec2(const T& x, const T& y) : Vec<T>(2), x(x), y(y){}
      inline Vec2(const Vector2& v)     : Vec<T>(2), x((T)v.x), y((T)v.y){}
      inline Vec2(const Vec2<int>& v)   : Vec<T>(2), x((T)v.x), y((T)v.y){}
      inline Vec2(const Vec2<float>& v) : Vec<T>(2), x((T)v.x), y((T)v.y){}
      inline Vec2(const Vec2<double>& v): Vec<T>(2), x((T)v.x), y((T)v.y){}
      inline explicit operator bool() const {return x || y;}
      inline Vec2 operator+(const Vec2& rhs) const {Vec2<T> val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Vec2 operator-(const Vec2& rhs) const {Vec2<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Vec2& operator+=(const Vec2& rhs){x += rhs.x; y += rhs.y; return *this;}
      inline Vec2& operator-=(const Vec2& rhs){x -= rhs.x; y -= rhs.y; return *this;}
      inline Vec2& operator*=(const Vec2& rhs){x *= rhs.x; y *= rhs.y; return *this;}
      inline Vec2& operator/=(const Vec2& rhs){x /= rhs.x; y /= rhs.y; return *this;}
      inline Vec2& operator=(const Vec2& rhs){x = rhs.x; y=rhs.y; return *this;}
      inline Vec2& operator-(){x = -x; y =-y; return *this;}
      inline void operator=(Size<T>&) = delete;
      inline void operator=(Pos<T>&) = delete;
      inline Vec2 midpoint(){return {x/2, y / 2};}
      inline void min(Vec2<T> other){if (this->x > other.x) this->x = other.x; if (this->y > other.y) this->y = other.y;}
      inline void max(Vec2<T> other){if (this->x < other.x) this->x = other.x; if (this->y < other.y) this->y = other.y;}
      inline double pct(double input){return (input-x)/(y - x);} //given an input value, what percentage of the range is it from 0 to 1?
      inline double lerp(double lerpVal){return lerpVal * (y - x) + x;} //given a value from 0 to 1, what is the value of the range that corresponds to it?
      inline Vec2 lerp(Vec2 otherPoint, double xprm){return {xprm, y + (((xprm - x) * (otherPoint.y - y)) / (otherPoint.x - x))};}
      inline T clamp(T value){if (value < x) return x; if (value > y) return y; return value;}
      inline static void fromString(const std::string& s){return Vec<T>::fromString(2, s);};
      std::ostream& operator<<(std::ostream& os) const {os << Vec<T>::toString(); return os;}
      friend std::ostream& operator<<(std::ostream& os, Vec2<T> v) {os << v.toString(); return os;}
      T x;
      T y;
   protected:
      [[nodiscard]] inline std::vector<T> getElements() const override {return {x,y};}
   };

   template <typename T>
   struct Vec3 : protected Vec<T> {
      inline Vec3(): Vec<T>(3), x(0), y(0){}
       inline Vec3(const T& _x, const T& y, const T& _z) : Vec<T>(3), x(_x), y(y),z(_z) {}
      inline explicit Vec3(const Vector3& v)     : Vec<T>(3), x((T)v.x), y((T)v.y), z((T)v.z){}
      inline explicit Vec3(const Vec3<int>& v)   : Vec<T>(3), x((T)v.x), y((T)v.y), z((T)v.z){}
      inline explicit Vec3(const Vec3<float>& v) : Vec<T>(3),  x((T)v.x), y((T)v.y), z((T)v.z){}
      inline explicit Vec3(const Vec3<double>& v): Vec<T>(3),  x((T)v.x), y((T)v.y), z((T)v.z){}
      inline Vec3& operator=(const Vec3& rhs){x = rhs.x; y=rhs.y; z=rhs.z; return *this;}
      inline Vec3& operator-(){x = -x; y =-y; z = -z; return *this}
      inline static void fromString(const std::string& s){return Vec<T>::fromString(3, s);};
      [[nodiscard]] inline std::vector<T> getElements() const override {return {x,y,z};}
      friend std::ostream& operator<<(std::ostream& os, Vec3<T> v) {os << v.toString(); return os;}
      T x;
      T y;
      T z;
   };

   template <typename T>
   class Range : private Vec3<T> {
   public:
      //Vec3 but x represents min, z represents max, and y represents value; Can lerp and set pct.
      //Vec3 shall be considered x=min, y=max, z=default value
      inline Range(): Vec3<T>::Vec3(){}
      inline Range(const T& min, const T& max, const T& defaultValue) : Vec3<T>::Vec3(min, max, defaultValue){}
      inline explicit Range(const Vector3& v)     : Vec3<T>::Vec3(v){}
      inline explicit Range(const Vec3<int>& v)   : Vec3<T>::Vec3(v){}
      inline explicit Range(const Vec3<float>& v) : Vec3<T>::Vec3(v){}
      inline explicit Range(const Vec3<double>& v): Vec3<T>::Vec3(v){}
      inline double getpct(){return getRange().pct(getValue());};
      inline void setLerp(double pct){setValue(getRange().lerp(pct));}
      inline T getMin() const {return Vec3<T>::x;}
      inline T getMax() const {return Vec3<T>::y;}
      inline T getValue() const {return Vec3<T>::z;}
      inline T setMin(T value){return Vec3<T>::x = value;}
      inline T setMax(T value){return Vec3<T>::y = value;}
      inline T setValue(T value){return Vec3<T>::z = value;}
      inline Vec2<T> getRange(){return Vec2<T>(Vec3<T>::x, Vec3<T>::y);}
      inline void setRange(Vec2<T> newRange){ setMin(newRange.x); setMax(newRange.y);}
      inline void setRange(T min, T max){ setMin(min); setMax(max);}
      inline static void fromString(const std::string& s){return Vec3<T>::fromString();};
      inline std::vector<T> getElements() const override {return Vec3<T>::getElements();}
      friend std::ostream& operator<<(std::ostream& os, Range<T> r) {os << r.toString(); return os;}
   };

   template <typename T>
   struct Line {
      inline Line(): a(0,0), b(0,0){}
      inline Line(Vec2<T> a, Vec2<T> b): a(a), b(b){}
      inline Line(const T x1, const T y1, const T x2, const T y2): Line({x1, y1}, {x2, y2}){}
      inline Line midpoint(){return {(a.x+b.x)/2, (a.y + b.y) / 2};}
      inline Vec2<T> lerp(double xprm){return a.lerp(b, xprm);}
      inline double distance(){NOT_IMPLEMENTED;}
      Vec2<T> a;
      Vec2<T> b;
   };

   template <typename T>
   struct Pos : public Vec2<T>{
      inline Pos(): Vec2<T>(){}
      inline Pos(const T& x, const T& y) : Vec2<T>(x, y){}
      inline Pos(const Vector2& v)     : Vec2<T>(v){}
      inline Pos(const Vec2<int>& v)   : Vec2<T>(v){}
      inline Pos(const Vec2<double>& v): Vec2<T>(v){}
      inline Pos(const Vec2<float>& v) : Vec2<T>(v){}

      inline Pos operator+(const Pos& rhs) const {auto val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Pos operator-(const Pos& rhs) const {auto val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Pos& operator+=(const Pos& rhs){this->x += rhs.x; this->y += rhs.y; return *this;}
      inline Pos& operator-=(const Pos& rhs){this->x -= rhs.x; this->y -= rhs.y; return *this;}

      inline void operator=(Size<T>&) = delete;
      inline void operator=(Pos<int>& other){x = other.x; y = other.y;};
   };

   template <typename T>
   struct Size : public Vec2<T>{
      inline Size(): Vec2<T>(){}
      inline Size(const T& x, const T& y) : Vec2<T>(x, y){}
      inline Size(const Vector2& v)     : Vec2<T>(v.x,v.y){}
      inline Size(const Vec2<int>& v)   : Vec2<T>(v.x,v.y){}
      inline void operator=(Pos<T>&) = delete;
      inline Size operator+(const Size& rhs) const {auto val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Size operator-(const Size& rhs) const {auto val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Size& operator+=(const Size& rhs){this->x += rhs.x; this->y += rhs.y; return *this;}
      inline Size& operator-=(const Size& rhs){this->x -= rhs.x; this->y -= rhs.y; return *this;}
   };

   template <typename T>
   struct Rect {
      inline Rect(): x(0), y(0), width(0), height(0){}
      inline Rect(const T x, const T y, const T width, const T height) : x(x), y(y), width(width), height(height){}
      inline explicit Rect(const Rectangle& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline Rect(const Rect<int>& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline Rect(const Rect<float>& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline Rect(const Rect<double>& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline explicit Rect(const Vec2<T>&) = delete;
      inline explicit Rect(const Pos<T>& v): x((T)v.x), y((T)v.y){}
      inline explicit Rect(const Size<T>& v): width((T)v.x), height((T)v.y){}
      inline Rect(const Pos<T>& pos, const Size<T>& size): x((T)pos.x), y((T)pos.y), width((T)size.x), height((T)size.y){}
      inline Rect operator+(const Pos<T>& rhs) const {Rect<T> val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Rect operator-(const Pos<T>& rhs) const {Rect<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Rect& operator+=(const Pos<T>& rhs){x += rhs.x; y += rhs.y; return *this;}
      inline Rect& operator-=(const Pos<T>& rhs){x -= rhs.x; y -= rhs.y; return *this;}
      inline Rect& operator*=(const Pos<T>& rhs){x *= rhs.x; y *= rhs.y; return *this;}
      inline Rect& operator/=(const Pos<T>& rhs){x /= rhs.x; y /= rhs.y; return *this;}
      inline Rect operator+(const Size<T>& rhs) const {Rect<T> val = *this; val.width += rhs.x; val.height += rhs.y; return val;}
      inline Rect operator-(const Size<T>& rhs) const {Rect<T> val = *this; val.width -= rhs.x; val.height -= rhs.y; return val;}
      inline Rect& operator+=(const Size<T>& rhs){width += rhs.width; height += rhs.height; return *this;}
      inline Rect& operator-=(const Size<T>& rhs){width -= rhs.width; height -= rhs.height; return *this;}
      inline Rect& operator*=(const Size<T>& rhs){width *= rhs.width; height *= rhs.height; return *this;}
      inline Rect& operator/=(const Size<T>& rhs){width /= rhs.width; height /= rhs.height; return *this;}
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
      inline void setCenter(const Vec2<T>& center) {x = center.x-width/2; y= center.y - height / 2;}
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
      [[nodiscard]] inline Pos<T> pos() const {return {x, y};}
      [[nodiscard]] inline Size<T> size() const {return {width, height};}
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
            DrawTextureRec(_tex.texture, {0, 0, (float)_tex.texture.width, (float)-_tex.texture.height }, {pos.x, pos.y}, {255, 255, 255, 210});
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