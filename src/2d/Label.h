#pragma once
#include "Control.h"
#include "StringTools.h"
#include <iomanip>
#include <sstream>
#include "Theme.h"

namespace ReyEngine{
   class Label : public Widget {
   public:
      REYENGINE_OBJECT(Label)
      Label(const std::string& text)
      : text(text)
      {
         if (auto expandOpt = needsExpand()){
            applyRect({getPos(), expandOpt.value()});
            minSize = expandOpt.value();
         }
         theme->background.fill = Style::Fill::NONE;
      }
      void render2D() const override{
         //todo: scissor text
         auto& outline = theme->background.outline;
         auto& background = theme->background;
//         auto& foreground = theme->foreground;
         switch (background.fill){
            case Style::Fill::SOLID:
               drawRectangle(getRect().toSizeRect(), background.colorPrimary);
            default:
               break;
         }
         switch(outline){
            case Style::Outline::LINE:
               drawRectangleLines(getRect().toSizeRect(), theme->outline.linethick, theme->outline.colorPrimary);
               break;
   //         case Style::Outline::SHADOW:
   //            break;
            default:
               break;
         }

          drawText(text, {0, 0}, theme->font);
      };

      void clear(){text.clear();}
      void setText(const std::string_view newText){setText(std::string(newText));}
      void setText(const std::string& newText){
         text = newText;
         auto expandOpt = needsExpand();
         if (expandOpt) {
            setMinSize(expandOpt.value());
         }
      }
      void appendText(const std::string& newText){
         text += newText;
         if (!isLocked) {
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
      std::string getText(){return text;}

   protected:
      inline ReyEngine::Rect<double> calculateBoundingRect(){
         auto textSize = measureText();
         auto newSize = textSize.clamp(minSize, maxSize);
         return {{0, 0}, newSize};
      }

      inline std::optional<ReyEngine::Size<double>> needsExpand(){
         auto boundingBox = calculateBoundingRect();
         auto thisBox = getRect();
         if (boundingBox.width > thisBox.width || boundingBox.height > thisBox.height) {
            return boundingBox.size();
         }
         return std::nullopt;
      };
      inline ReyEngine::Size<double> measureText() const {return theme->font.measure(text);}
      std::string text;
   };
}