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
      Label(const std::string& text = "")
            : _text(text)
      {}
      void render2D() const override{
//         ScopeScissor scissor(getSizeRect());
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
         if (_wrap) {
            float yPos = 0;
            if (_wrappedText.empty()) return;

            // Use the height of a sample string to determine line height
            float lineHeight = theme->font->measure(" ").y;

            for(const auto& line : _wrappedText){
               drawText(line, {0, yPos}, theme->font);
               yPos += lineHeight + Y_GAP_PXL;
            }
         } else {
            drawText(_displayText, {0, 0}, theme->font);
         }
      };

      void setMaxChars(size_t charCount){_maxCharCount = charCount;  setText(_text);}
      void clear(){setText("");}
      void setText(const char* newText){setText(std::string(newText));}
      void setText(const std::string_view newText){setText(std::string(newText));}
      void setText(const std::string& newText){
         _text = newText;
         _displayText = _text.substr(0, _maxCharCount);
         _wrappedText.clear();

         if (_wrap) {
            auto boxWidth = getSize().x;
            // Can't wrap if there's no width to wrap into, or no text.
            if (boxWidth > 0 && !_text.empty()) {
               std::string remainingText = _text;
               while(!remainingText.empty()){
                  size_t breakIndex = 0; // The point in the string where we will break the line.
                  size_t lastGoodIndex = 0; // The last index that we know for sure fits.
                  size_t lastSpaceIndex = std::string::npos; // The index of the last space found.

                  // Find the longest possible line by checking character by character
                  for(size_t i = 1; i <= remainingText.length(); ++i){
                     auto substr = remainingText.substr(0, i);
                     if (theme->font->measure(substr).x > boxWidth){
                        // This substring is too long, so the break must have happened before
                        breakIndex = lastGoodIndex;
                        break;
                     }
                     if (remainingText[i-1] == ' ') {
                        lastSpaceIndex = i - 1;
                     }
                     lastGoodIndex = i;
                     if (i == remainingText.length()){
                        breakIndex = i; // The whole remaining text fits
                     }
                  }

                  if (breakIndex == 0 && remainingText.length() > 0) {
                     // This happens if not even one character fits.
                     // Force a break at the first character to prevent an infinite loop.
                     breakIndex = 1;
                  }

                  // Now decide where to actually cut the line
                  size_t cutIndex = breakIndex;
                  // If we are not at the end of the text and we found a space on the line, prefer that.
                  if(breakIndex < remainingText.length() && lastSpaceIndex != std::string::npos && lastSpaceIndex > 0){
                     cutIndex = lastSpaceIndex;
                  }

                  _wrappedText.push_back(remainingText.substr(0, cutIndex));

                  // Trim leading spaces from the start of the next line
                  size_t nextStart = cutIndex;
                  while(nextStart < remainingText.length() && remainingText[nextStart] == ' '){
                     nextStart++;
                  }
                  remainingText = remainingText.substr(nextStart);
               }
            }

            // Calculate the required height and update the widget's size directly.
            float requiredHeight = 0;
            if (!_wrappedText.empty()){
               float lineHeight = theme->font->measure(" ").y;
               requiredHeight = _wrappedText.size() * (lineHeight + Y_GAP_PXL) - Y_GAP_PXL; // No gap after the last line
            }
            // Set the size directly, preserving the current width and updating the height.
            // This prevents the widget from changing its width, only its height.
            Logger::info() << "New required height is " << requiredHeight << std::endl;
            setSize(getSize().x, requiredHeight);

         } else {
            auto textSize = measureText();
            setMinSize(textSize);
            setSize(0,0); // Trigger auto-resize based on minSize
         }
      }
      void prependText(const std::string& newText){setText(newText + getText());}
      void appendText(const std::string& newText){setText(getText() + newText);}
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
      [[nodiscard]] std::string getText() const {return _text;}
      void setWrap(bool newWrap) {
         if (_wrap == newWrap) return;
         _wrap = newWrap;
         setText(_text); // Recalculate size and layout with the new wrap setting
      }
   protected:
      void _init() override {
         if (_text.empty()) _text = getName();
         setText(_text);
         theme->background.fill = Style::Fill::NONE;
      }

      inline ReyEngine::Size<R_FLOAT> measureText() const {return theme->font->measure(_displayText);}
      std::string _text;
      std::string _displayText;
      std::vector<std::string> _wrappedText;
      bool _wrap = false; //enables wrapping aka multi-line mode
      size_t _maxCharCount = std::numeric_limits<size_t>::max();
      static constexpr float Y_GAP_PXL = 2; //the vertical gap between lines
   };
}
