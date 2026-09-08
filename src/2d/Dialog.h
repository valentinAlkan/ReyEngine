#pragma once
#include "Panel.h"

namespace ReyEngine {
   enum class DialogOptions{CANCEL, OK, CLOSE, YES, NO};

   namespace __ {
      constexpr auto toString(DialogOptions option){
         switch (option) {
            case DialogOptions::OK: return "Ok";
            case DialogOptions::CANCEL: return "Cancel";
            case DialogOptions::CLOSE: return "Close";
            case DialogOptions::NO: return "No";
            case DialogOptions::YES: return "Yes";
            default: return "<invalid>";
         }
      }
   }

   template <size_t N, typename OptMetaType=void>
   class Dialog : public ReyEngine::Panel {
   public:
      REYENGINE_OBJECT(Dialog);
      Dialog(std::array<DialogOptions, N> options, const std::string& message="", Layout::LayoutDir layoutDirection = Layout::LayoutDir::HORIZONTAL)
      : options(options)
      , layoutDirection(layoutDirection)
      , message(message)
      {
         if (layoutDirection != Layout::LayoutDir::HORIZONTAL && layoutDirection != Layout::LayoutDir::VERTICAL) {
            Logger::error() << "Dialogs do not support this layout direction. Defaulting to Horizontal" << std::endl;
         }
         _visible = false; //no events
      }

      // Overload taking a caller-supplied display label per option (in the same order as `options`).
      // The `DialogOptions` values still drive the published DialogCloseEvent, so labels are purely
      // cosmetic - an empty label falls back to __::toString(option).
      Dialog(std::array<DialogOptions, N> options, std::array<std::string, N> labels, const std::string& message="", Layout::LayoutDir layoutDirection = Layout::LayoutDir::HORIZONTAL)
      : options(options)
      , labels(labels)
      , layoutDirection(layoutDirection)
      , message(message)
      {
         if (layoutDirection != Layout::LayoutDir::HORIZONTAL && layoutDirection != Layout::LayoutDir::VERTICAL) {
            Logger::error() << "Dialogs do not support this layout direction. Defaulting to Horizontal" << std::endl;
         }
         _visible = false; //no events
      }

      EVENT_ARGS(DialogOpenEvent, 6454983, const Dialog& dialog)
         , dialog(dialog)
         {}
         const Dialog& dialog;
      };

      EVENT_ARGS(DialogCloseEvent, 6454984, const std::string& option, DialogOptions value)
         , value(value)
         , asString(__::toString(value))
         {}
         const DialogOptions value;
         const std::string asString;
      };

      void show(){ setVisible(true);} //setVisible calls onvis change
      void hide(){ setVisible(false);} //setVisible calls onvis change
      void setMessage(const std::string& m){message = m;}

   protected:
      void _init() override{
         setSize(320, 240); //todo: find minimum size

         // The Panel renders a 35px title bar across the top and draws its children on top of
         // it, so lay everything out in the content area below the header. This mirrors the
         // _anchorArea that Panel::_on_rect_changed computes:
         //     getSizeRect().splitAtVPos(header).second.embiggen(-8)
         // but is computed directly so it is correct even before the panel receives its final rect.
         static constexpr float HEADER_HEIGHT_PXL = 35.f; // keep in sync with Panel::_on_rect_changed
         const float margin = 8.f;
         const float gap    = 6.f;
         const Rect<float> size = getSizeRect();
         const Rect<float> content(size.x + margin, size.y + HEADER_HEIGHT_PXL + margin,
                                   size.width - 2.f * margin,
                                   size.height - HEADER_HEIGHT_PXL - 2.f * margin);

         // Display text per option: the caller-supplied label if present, else the enum name.
         auto labelFor = [&](size_t idx) -> std::string {
            const std::string& l = labels[idx];
            return l.empty() ? std::string(__::toString(options[idx])) : l;
         };

         // Uniform button height (one text line + padding), measured from the label text.
         static constexpr float BTN_PAD = 10.f; // matches Button::setMinSize's text+10 padding
         float btnH = 10.f;
         for (size_t i = 0; i < N; i++){
            float h = measureText(labelFor(i), theme->font).y + BTN_PAD;
            if (h > btnH) btnH = h;
         }

         // Reserve the button band along the bottom of the content area; the message fills the rest.
         const float buttonRegionSize = (layoutDirection == Layout::LayoutDir::VERTICAL)
                                        ? (N * btnH + (N - 1) * gap)
                                        : btnH;
         const Rect<float> buttonRegion(content.x, content.y + content.height - buttonRegionSize,
                                        content.width, buttonRegionSize);
         const Rect<float> messageRegion(content.x, content.y,
                                         content.width,
                                         content.height - buttonRegionSize - gap);

         // add the label first so its text (draws at local 0,0) sits inside the message area.
         // Give it the message width BEFORE enabling wrap: Label::setWrap->setText wraps into
         // getSize().x, so the width has to be established first or nothing wraps.
         {
            auto [label, node] = make_node<Label>("MessageLabel", message);
            label->setRect(messageRegion);
            label->setWrap(true);
            addChild(std::move(node));
         }

         // Each button is sized to its own text (a label can be long), then either stacked in a
         // full-width column or packed into a right-aligned row so the labels never clip.
         std::vector<Rect<float>> rects(N);
         if (layoutDirection == Layout::LayoutDir::VERTICAL){
            std::vector<Percent> percents;
            for (size_t i = 0; i < N; i++) percents.emplace_back(100/N);
            rects = buttonRegion.splitV(percents);
         } else {
            std::vector<float> widths(N);
            float rowWidth = (N - 1) * gap;
            for (size_t i = 0; i < N; i++){
               widths[i] = measureText(labelFor(i), theme->font).x + BTN_PAD;
               rowWidth += widths[i];
            }
            float x = content.x + content.width - rowWidth; // right-align the row within the content
            for (size_t i = 0; i < N; i++){
               rects[i] = {x, buttonRegion.y, widths[i], btnH};
               x += widths[i] + gap;
            }
         }

         size_t i = 0;
         for(const auto& option : options){
            const std::string text = labelFor(i);
            // node name stays enum-derived (a stable id); display text is the (possibly custom) label
            auto [btn, node] = make_node<PushButton>(std::string(__::toString(option)) + "Button");
            btn->setText(text);
            btn->setRect(rects.at(i));
            //add metadata T
            static constexpr std::string_view METADATA_VALUE_NAME = "value";
            btn->setMetaData<DialogOptions>(std::string(METADATA_VALUE_NAME), option);
            auto btnCB = [btn, this](const PushButton::ButtonPressEvent& event){
               auto publisher = event.publisher->as<Button>().value();
               auto option = publisher->getText();
               auto value = publisher->getMetaData<DialogOptions>(std::string(METADATA_VALUE_NAME));
               setVisible(false);
               if (!value) {
                  Logger::error() << "Invalid metadata for dialog option " << option << std::endl;
                  return;
               }
               DialogCloseEvent closeEvent(this, option, value.value());
               publish(closeEvent);
            };
            subscribe<PushButton::ButtonPressEvent>(btn, btnCB);
            addChild(std::move(node));
            ++i;
         }
      }

      void _on_visibility_changed() override {
         if (_visible) {
            DialogOpenEvent event(this, *this);
            publish(event);
         }
      }

   private:
      const std::array<DialogOptions, N> options;
      const std::array<std::string, N> labels{};
      const Layout::LayoutDir layoutDirection;
      std::string message;
   };
}
