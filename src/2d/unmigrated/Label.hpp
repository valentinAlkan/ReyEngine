#pragma once
#include "Control.h"
#include "StringTools.h"
#include <iomanip>
#include <sstream>

namespace ReyEngine{
   class Label : public Widget {
   public:
      Label(const std::string& text){
         auto expandOpt = needsExpand();
         if (expandOpt){
            applyRect({getPos(), expandOpt.value()});
            minSize = expandOpt.value();
         }
         theme->background = Style::Fill::NONE;
      }
      void render2D() const override{
         //todo: scissor text
         auto& outline = theme->outline;
         auto& background = theme->background;
//         auto& foreground = theme->foreground;
         switch (background.value){
            case Style::Fill::SOLID:
               drawRectangle(getRect().toSizeRect(), background.colorPrimary.value);
            default:
               break;
         }
         switch(outline.value){
            case Style::Outline::LINE:
               drawRectangleLines(getRect().toSizeRect(), outline.thickness.value, outline.color.value);
               break;
   //         case Style::Outline::SHADOW:
   //            break;
            default:
               break;
         }
          drawText(text.value, {0, 0}, theme->font.value);
      };
      void registerProperties() override{
         registerProperty(text);
      };
      void clear(){text.value.clear();}
      void setText(const std::string& newText){
         text.set(newText);
         auto expandOpt = needsExpand();
         if (expandOpt) {
            setMinSize(expandOpt.value());
         }
      }
      void appendText(const std::string& newText){
         text.value += newText;
         if (!isInLayout) {
            auto expandOpt = needsExpand();
            if (expandOpt) {
                setMinSize(expandOpt.value());
            }
         }
      }
      //precision refers to how many decimal places should appear
      void setText(double newText, int precision){
         auto textRepr = std::to_string(newText);
         auto digitsCount = string_tools::decimalCount(textRepr);
         auto integerCount = digitsCount.value().first;
         auto decimalCount = digitsCount.value().second;
         setText(std::string(textRepr, 0, integerCount + 1 + ((int)decimalCount > precision ? precision : decimalCount)));
      }
      void setText(int newText){
         setText(std::to_string(newText));
      }
      std::string getText(){return text.value;}

   protected:
      inline ReyEngine::Rect<double> calculateBoundingRect(){
         auto textSize = measureText();
         auto newSize = getClampedSize(textSize);
         if (newSize.x > getSize().x || newSize.y > getSize().y){
            return {{0, 0}, newSize};
         }
         return getRect().toSizeRect();
      }

      inline std::optional<ReyEngine::Size<double>> needsExpand(){
         auto boundingBox = calculateBoundingRect();
         auto thisBox = getRect();
         if (boundingBox.width > thisBox.width || boundingBox.height > thisBox.height) {
            return boundingBox.size();
         }
         return std::nullopt;
      };
      inline ReyEngine::Size<double> measureText() const {return theme->font.value.measure(text.value);}
      std::string text;
   };
}