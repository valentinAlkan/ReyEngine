#pragma once
#include "raylib.h"
//#include "Application.h"
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
         inline Vec2 operator+(const Vec2& rhs) const {Vec2<T> val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
         inline Vec2 operator-(const Vec2& rhs) const {Vec2<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
         inline Vec2& operator+=(const Vec2& rhs){x += rhs.x; y += rhs.y; return *this;}
         inline Vec2& operator-=(const Vec2& rhs){x -= rhs.x; y -= rhs.y; return *this;}
         inline Vec2& operator*=(const Vec2& rhs){x *= rhs.x; y *= rhs.y; return *this;}
         inline Vec2& operator/=(const Vec2& rhs){x /= rhs.x; y /= rhs.y; return *this;}
         inline Vec2 midpoint(){return {x/2, y/2};}
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
         [[nodiscard]] inline Vec2<T> center() const {return {(x+width)/2, (y+height)/2};}
         [[nodiscard]] inline std::string toString() const {
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
         Vec2<T> pos() const {return {x, y};}
         Vec2<T> size() const {return {width, height};}
         Rect<T> toSizeRect() const {return {0,0,width, height};}

         T x;
         T y;
         T width;
         T height;
      };

      Vec2<double> getScreenCenter();
      void drawText(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color);
      void drawTextCentered(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color);
      void drawTextRelative(const std::string& text, const GFCSDraw::Vec2<int>& relPos, int fontSize, Color color);
      void drawRectangle(const GFCSDraw::Rect<int>&, Color color);
      void drawRectangleRounded(const GFCSDraw::Rect<float>&, float roundness, int segments, Color color);
      void drawRectangleRoundedLines(const GFCSDraw::Rect<float>&, float roundness, int segments, float lineThick, Color color);
      void drawRectangleGradientV(const GFCSDraw::Rect<int>&, Color color1, Color color2);
      inline Vec2<int> getMousePos(){return GetMousePosition();}
      inline float getFrameDelta(){return GetFrameTime();}


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
            DrawTextureRec(_tex.texture, {0, 0, (float)_tex.texture.width, (float)-_tex.texture.height }, {pos.x, pos.y}, WHITE);
         }
      protected:
         bool _texLoaded = false;
         RenderTexture2D _tex;
         Vec2<float> _size;
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
}