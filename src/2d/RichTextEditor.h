#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "MenuBar.h"
#include "ScrollView.h"
#include "TextEditHandler.h"
#include "TextRenderView.h"

namespace ReyEngine {
   // An editable TextRenderView. Reuses TextRenderView's TextRenderModel<TrString>
   // as the backing store, so edits stay localization-aware (the TrString's
   // language/key are preserved). All editing behavior (caret/selection, undo/redo,
   // clipboard, keyboard+mouse input) lives in the composed TextEditHandler; this
   // class owns what's geometric: visual-line layout (word wrap), vertical
   // scrolling, the context menu widget, and rendering. It answers the handler's
   // geometry questions through the (privately inherited) Host interface.
   class RichTextEditor : public TextRenderView, private TextEditHandler::Host {
   public:
      REYENGINE_OBJECT(RichTextEditor)
      EVENT(EventTextChanged, 6546547654354){}};
      RichTextEditor(auto&&... args)
      : TextRenderView(std::forward<decltype(args)>(args)...)
      , _edit(*this, getModel())
      {}
      ~RichTextEditor() override = default;
      void setText(const std::string& text); //hides TextRenderModel::setText; preserves language/key
      [[nodiscard]] std::string getString() const {return getText().str();}
      [[nodiscard]] bool isEditing() const {return _edit.isEditing();}
      void clear(){setText("");}
      void undo(){_edit.undo();} //reverse the most recent edit
      void redo(){_edit.redo();} //re-apply the most recently undone edit
      [[nodiscard]] bool canUndo() const {return _edit.canUndo();}
      [[nodiscard]] bool canRedo() const {return _edit.canRedo();}
      //word wrap: when enabled, logical lines too wide for the widget are broken across
      //multiple visual rows at word boundaries. The stored text is unchanged; only layout differs.
      void setWordWrapEnabled(bool enabled){_wordWrap = enabled;}
      [[nodiscard]] bool getWordWrapEnabled() const {return _wordWrap;}
   protected:
      void _init() override;
      void _process(float dt) override; //per-frame: keep the scroll limits and bar in sync with content
      void render2D(RenderContext&) const override;
      void _on_focus_gained() override;
      void _on_focus_lost() override;
      void _on_rect_changed() override; //reposition the scrollbar gutter + re-clamp scroll limits on resize
      Handled _unhandled_input(const InputEvent&) override;
   private:
      void _on_text_changed() override; //shared model changed (here or via another view): invalidate cache, clamp caret, re-layout
      void _assignString(const std::string&); //writes back to the model and publishes
      [[nodiscard]] float lineHeight() const;

      // TextEditHandler::Host: geometry/state answers for the editing logic
      [[nodiscard]] size_t caretFromMouse(const Pos<float>& localPos) const override;
      [[nodiscard]] size_t caretVertical(size_t caret, int dir) const override; //dir: -1 up, +1 down
      [[nodiscard]] size_t rowStart(size_t caret) const override; //start of the caret's visual row
      [[nodiscard]] size_t rowEnd(size_t caret) const override;   //end of the caret's visual row
      void ensureCaretVisible() override; //scroll vertically so the caret's row sits in the viewport
      [[nodiscard]] EngineFrameCount editFrameCount() const override {return getEngineFrameCount();}
      [[nodiscard]] bool editEnabled() const override {return getIsEnabled();}
      [[nodiscard]] bool editFocused() const override {return isFocused();}
      void editSetFocused(bool focused) override {setFocused(focused);}
      void onEdited() override {publish(EventTextChanged(this));} //editor-level event for external listeners
      void onContextMenu(const Pos<float>& localPos) override {openContextMenu(localPos);}

      // A visual row of on-screen text: the byte range [start, end) of `text` drawn on
      // one line. Excludes any trailing '\n'. With word wrap off there is one VisualLine
      // per logical line; with it on, wide logical lines split into several.
      struct VisualLine { size_t start; size_t end; };
      [[nodiscard]] std::vector<VisualLine> computeVisualLines(const std::string& text) const;
      //cached wrapper around computeVisualLines: recomputes only when the text, width,
      //wrap flag or font changes, so layout isn't rebuilt every frame.
      [[nodiscard]] const std::vector<VisualLine>& visualLines() const;
      [[nodiscard]] float wrapWidth() const;     //usable text width inside the margins (minus scrollbar gutter)
      [[nodiscard]] float viewportWidth() const; //widget width minus the scrollbar gutter when the bar is shown
      [[nodiscard]] float contentHeight() const; //total height of all visual rows plus top/bottom margins
      void updateScrollLayout();                 //refresh scroll limits + bar from current content/viewport
      //find the visual row containing `caret` and the byte column within it
      static void rowColForCaret(const std::vector<VisualLine>& lines, size_t caret, size_t& row, size_t& col);

      //right-click context menu: rebuilt on each open so Copy/Paste only appear when applicable
      void openContextMenu(const Pos<float>& localPos);
      void drawSelection(const std::vector<VisualLine>& lines, float lineHeight) const; //highlight rects, called from render2D

      static constexpr float TEXT_MARGIN = 4.0f;
      TextEditHandler _edit;   //caret/selection/undo/clipboard + edit input (see class comment)
      bool _wordWrap = false;  //wrap wide logical lines across visual rows

      // vertical scrolling. _scroll owns the offset/limits and keeps _vScrollBar in sync;
      // the editor offsets its own text drawing and translates input by _scroll.offsetY().
      ScrollView _scroll;
      std::shared_ptr<Slider> _vScrollBar;
      std::shared_ptr<DropDownMenu> _contextMenu; //right-click menu (copy/paste/select all)
      static constexpr float SCROLLBAR_WIDTH = 14.0f;
      uint64_t _textVersion = 1;    //bumped on every content change; keys the layout cache

      // layout cache (mutable: filled lazily from const render/query paths). Rebuilt only
      // when one of the keys below no longer matches the current text/width/wrap/font.
      mutable std::vector<VisualLine> _layoutCache;
      mutable bool _layoutValid = false;
      mutable uint64_t _layoutTextVersion = 0;
      mutable float _layoutWidth = -1.0f;
      mutable bool _layoutWrap = false;
      mutable float _layoutFontSize = -1.0f;
      mutable float _layoutFontSpacing = -1.0f;
   };
}
