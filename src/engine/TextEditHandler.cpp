#include "TextEditHandler.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
static bool isWordSpace(char c) {
   return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t TextEditHandler::nextCharBoundary(const std::string& text, size_t i) {
   if (i >= text.size()) return text.size();
   ++i; //skip the lead byte, then any UTF-8 continuation bytes (10xxxxxx)
   while (i < text.size() && (static_cast<unsigned char>(text[i]) & 0xC0) == 0x80) ++i;
   return i;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t TextEditHandler::prevCharBoundary(const std::string& text, size_t i) {
   if (i == 0) return 0;
   --i; //step back over the trailing continuation bytes to the lead byte
   while (i > 0 && (static_cast<unsigned char>(text[i]) & 0xC0) == 0x80) --i;
   return i;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t TextEditHandler::nextWordBoundary(const std::string& text, size_t i) {
   while (i < text.size() && isWordSpace(text[i])) ++i;  //skip whitespace
   while (i < text.size() && !isWordSpace(text[i])) ++i; //skip the word
   return i;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t TextEditHandler::prevWordBoundary(const std::string& text, size_t i) {
   while (i > 0 && isWordSpace(text[i - 1])) --i;  //skip whitespace
   while (i > 0 && !isWordSpace(text[i - 1])) --i; //skip the word
   return i;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string TextEditHandler::normalizeNewlines(std::string text) {
   std::string out;
   out.reserve(text.size());
   for (size_t i = 0; i < text.size(); ++i) {
      if (text[i] == '\r') {
         out.push_back('\n');
         if (i + 1 < text.size() && text[i + 1] == '\n') ++i; //collapse a \r\n pair
      } else {
         out.push_back(text[i]);
      }
   }
   return out;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool TextEditHandler::caretVisible() const {
   auto elapsed = _host.editFrameCount() - _caretBlinkBase;
   return elapsed % 60 < 30;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::afterEdit() {
   resetCaretBlink();  //any edit (incl. delete-forward) keeps the caret visible
   _host.onEdited();   //let the widget publish its own text-changed event
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::externalTextChanged() {
   //a remote edit can shrink the buffer out from under our caret/selection; clamp them back in
   const size_t size = text().size();
   if (_caret > size) _caret = size;
   if (_selectionAnchor > size) _selectionAnchor = size;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::textReset() {
   if (_caret > text().size()) _caret = text().size();
   clearSelection(); //drop any stale selection that could point past the new text
   resetHistory();   //wholesale replacement isn't an undoable edit; old actions' offsets are now invalid
   resetCaretBlink();
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::pushAction(std::shared_ptr<EditAction> action, bool mergeable) {
   action->redo(*this);  //apply the edit immediately
   //a brand-new edit invalidates any redo branch hanging off the cursor
   if (_undoCursor < _undo.size()) _undo.erase(_undo.begin() + (long)_undoCursor, _undo.end());
   //fold continuous typing/deleting into the previous step when allowed
   if (mergeable && _coalesce && !_undo.empty() && _undo.back()->tryMerge(*action)) {
      //absorbed into _undo.back(); nothing appended, cursor already == size
   } else {
      _undo.push_back(std::move(action));
      _undoCursor = _undo.size();
   }
   _coalesce = mergeable; //only a mergeable edit lets the *next* one coalesce into it
   _didEdit = true;       //tell handleInput this caret move came from an edit, not navigation
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::undo() {
   if (_undoCursor == 0) return;
   _undo[--_undoCursor]->undo(*this);
   breakUndoMerge(); //an undo ends any in-progress typing run
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::redo() {
   if (_undoCursor >= _undo.size()) return;
   _undo[_undoCursor++]->redo(*this);
   breakUndoMerge();
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::replaceSelectionWith(const TrString& text, bool mergeable) {
   if (hasSelection()) {
      //overwrite the highlight as one undo step: remove the selection, then insert
      auto comp = std::make_shared<CompositeAction>();
      comp->actions.push_back(std::make_shared<RemoveTextAction>(selMin(), TrString(getSelectedText())));
      comp->actions.push_back(std::make_shared<InsertTextAction>(selMin(), text));
      pushAction(comp, false); //a replace is always a discrete step, never coalesced
   } else {
      pushAction(std::make_shared<InsertTextAction>(_caret, text), mergeable);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string TextEditHandler::getSelectedText() const {
   if (!hasSelection()) return "";
   return text().substr(selMin(), selMax() - selMin());
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::deleteSelection() {
   if (!hasSelection()) return;
   //a selection delete is its own discrete undo step; RemoveTextAction::redo
   //erases the run, collapses the caret to selMin and clears the selection
   pushAction(std::make_shared<RemoveTextAction>(selMin(), TrString(getSelectedText())), false);
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::copySelection() const {
   if (hasSelection()) SetClipboardText(getSelectedText().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::pasteClipboard() {
   const char* clip = GetClipboardText();
   if (clip && clip[0] != '\0') {
      std::string pasted = normalizeNewlines(clip); //strip \r so it won't render as '?'
      if (_singleLine) { //single-line: newlines can never enter the buffer
         pasted.erase(std::remove(pasted.begin(), pasted.end(), '\n'), pasted.end());
         if (pasted.empty()) return;
      }
      replaceSelectionWith(TrString(pasted), /*mergeable*/false); //paste is one discrete step
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
bool TextEditHandler::handleInput(const InputEvent& event) {
   size_t caretBefore = _caret;
   _didEdit = false;
   bool handled = processEdit(event);
   if (_caret != caretBefore) {
      resetCaretBlink();                   //caret moved -> show it immediately
      if (!_didEdit) breakUndoMerge();     //moved by navigation, not an edit -> end the typing run
   }
   if (_caret != caretBefore || _didEdit) _host.ensureCaretVisible(); //keep the caret on-screen after edits/moves
   return handled;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool TextEditHandler::processEdit(const InputEvent& event) {
   if (auto isMouse = event.isMouse()) {
      auto& mouse = isMouse.value();
      switch (event.eventId) {
         case InputEventMouseButton::ID: {
            const auto& mouseEvent = event.toEvent<InputEventMouseButton>();
            if (mouseEvent.isDown) {
               if (_host.editEnabled() && mouse->isInside()) {
                  if (!_host.editFocused()) _host.editSetFocused(true);
                  //right-click: handled on release so the matching button-up doesn't
                  //immediately register as a click-outside and close the menu. Consume the
                  //press and leave the caret/selection alone.
                  if (mouseEvent.button == InputInterface::MouseButton::RIGHT) return true;
                  _caret = _host.caretFromMouse(mouse->getLocalPos());
                  clearSelection();      //new click starts a fresh selection at the caret
                  _isDragging = true;
                  return true;
               } else if (_host.editFocused()) {
                  _host.editSetFocused(false); //pressed elsewhere -> stop editing
                  _isDragging = false;
               }
               break; //not consuming: let the click reach whatever was pressed
            }
            //right-click release inside the widget: the widget may open a context menu
            if (mouseEvent.button == InputInterface::MouseButton::RIGHT &&
                _host.editEnabled() && mouse->isInside()) {
               _host.onContextMenu(mouse->getLocalPos());
               return true;
            }
            //button released: end the drag but keep focus, even if released outside the widget
            //(otherwise dragging a selection off the widget would defocus and break editing)
            if (_isDragging) {
               _isDragging = false;
               return true;
            }
            break;
         }
         case InputEventMouseMotion::ID: {
            if (_isDragging && _host.editFocused()) {
               //extend selection: move the caret, leave the anchor put
               _caret = _host.caretFromMouse(mouse->getLocalPos());
               return true;
            }
            break;
         }
      }
   }

   if (!_editing || !_host.editFocused()) return false;

   switch (event.eventId) {
      case InputEventChar::ID: {
         const auto& charEvent = event.toEvent<InputEventChar>();
         //drop control characters (incl. \r and \n); newlines are inserted via the Enter key,
         //and other control codepoints have no glyph and would render as '?'
         if (charEvent.ch < 0x20 || charEvent.ch == 0x7F) break;
         //charEvent.ch is a Unicode codepoint; encode it to UTF-8 before storing
         int byteCount = 0;
         const char* utf8 = CodepointToUTF8(charEvent.ch, &byteCount);
         if (byteCount <= 0) return true;
         //typing replaces any selection then inserts; plain typing is mergeable so
         //a run of keystrokes collapses into a single undo step
         replaceSelectionWith(TrString(std::string(utf8, (size_t)byteCount)), /*mergeable*/true);
         return true;
      }
      case InputEventKey::ID: {
         const auto& keyEvent = event.toEvent<InputEventKey>();
         if (!keyEvent.isDown) break;
         bool ctrlHeld = InputInterface::isKeyDown(InputInterface::KeyCode::KEY_LEFT_CONTROL) ||
                         InputInterface::isKeyDown(InputInterface::KeyCode::KEY_RIGHT_CONTROL);
         bool shiftHeld = InputInterface::isKeyDown(InputInterface::KeyCode::KEY_LEFT_SHIFT) ||
                          InputInterface::isKeyDown(InputInterface::KeyCode::KEY_RIGHT_SHIFT);
         switch (keyEvent.key) {
            default: break;
            case InputInterface::KeyCode::KEY_A:
               if (ctrlHeld) selectAll();
               return true;
            case InputInterface::KeyCode::KEY_C:
               if (ctrlHeld) copySelection();
               return true;
            case InputInterface::KeyCode::KEY_Z:
               if (ctrlHeld) { if (shiftHeld) redo(); else undo(); } //Ctrl+Z undo, Ctrl+Shift+Z redo
               return true;
            case InputInterface::KeyCode::KEY_Y:
               if (ctrlHeld) redo(); //Ctrl+Y redo (Windows-style)
               return true;
            case InputInterface::KeyCode::KEY_X:
               if (ctrlHeld && hasSelection()) {
                  SetClipboardText(getSelectedText().c_str());
                  deleteSelection();
               }
               return true;
            case InputInterface::KeyCode::KEY_V:
               if (ctrlHeld) {
                  pasteClipboard();
                  return true;
               }
               break;
            case InputInterface::KeyCode::KEY_ENTER:
            case InputInterface::KeyCode::KEY_KP_ENTER:
               if (_singleLine) {
                  _host.onSubmit(); //e.g. LineEdit publishes its text-entered event
               } else {
                  replaceSelectionWith(TrString("\n"), /*mergeable*/false); //a newline is an undo boundary
               }
               return true;
            case InputInterface::KeyCode::KEY_BACKSPACE:
               if (hasSelection()) {
                  deleteSelection();
               } else if (_caret > 0) {
                  const auto& t = text();
                  size_t prev = prevCharBoundary(t, _caret); //erase the whole codepoint
                  pushAction(std::make_shared<RemoveTextAction>(prev, TrString(t.substr(prev, _caret - prev))), /*mergeable*/true);
               }
               return true;
            case InputInterface::KeyCode::KEY_DELETE:
               if (hasSelection()) {
                  deleteSelection();
               } else if (_caret < text().size()) {
                  const auto& t = text();
                  size_t next = nextCharBoundary(t, _caret); //erase the whole codepoint
                  pushAction(std::make_shared<RemoveTextAction>(_caret, TrString(t.substr(_caret, next - _caret))), /*mergeable*/true);
               }
               return true;
            case InputInterface::KeyCode::KEY_LEFT:
               if (ctrlHeld) {
                  _caret = prevWordBoundary(text(), _caret); //jump a word
               } else if (!shiftHeld && hasSelection()) {
                  _caret = selMin();            //collapse selection to its left edge
               } else if (_caret > 0) {
                  _caret = prevCharBoundary(text(), _caret);
               }
               if (!shiftHeld) clearSelection();
               return true;
            case InputInterface::KeyCode::KEY_RIGHT:
               if (ctrlHeld) {
                  _caret = nextWordBoundary(text(), _caret); //jump a word
               } else if (!shiftHeld && hasSelection()) {
                  _caret = selMax();            //collapse selection to its right edge
               } else if (_caret < text().size()) {
                  _caret = nextCharBoundary(text(), _caret);
               }
               if (!shiftHeld) clearSelection();
               return true;
            case InputInterface::KeyCode::KEY_UP:
               //collapse to the top of the selection first, then move up from there
               if (!shiftHeld && hasSelection()) _caret = selMin();
               _caret = _host.caretVertical(_caret, -1);
               if (!shiftHeld) clearSelection();
               return true;
            case InputInterface::KeyCode::KEY_DOWN:
               //collapse to the bottom of the selection first, then move down from there
               if (!shiftHeld && hasSelection()) _caret = selMax();
               _caret = _host.caretVertical(_caret, 1);
               if (!shiftHeld) clearSelection();
               return true;
            case InputInterface::KeyCode::KEY_HOME: {
               const auto& t = text();
               size_t start = _host.rowStart(_caret);
               size_t end = _host.rowEnd(_caret); //bound to the visual row, not the logical line
               //first non-whitespace char on the visual row
               size_t firstNonWs = start;
               while (firstNonWs < end && isWordSpace(t[firstNonWs])) ++firstNonWs;
               //already at column 0 with leading whitespace -> jump to first non-whitespace,
               //otherwise go to the start of the row
               if (_caret == start && firstNonWs > start && firstNonWs < end) {
                  _caret = firstNonWs;
               } else {
                  _caret = start;
               }
               if (!shiftHeld) clearSelection();
               return true;
            }
            case InputInterface::KeyCode::KEY_END:
               _caret = _host.rowEnd(_caret); //end of the visual row
               if (!shiftHeld) clearSelection();
               return true;
         }
      }
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::InsertTextAction::redo(TextEditHandler& h) {
   //mutate through the shared model: it publishes EventTextChanged to every view
   h._model->insertText(index, text);  //put the run back at the recorded offset
   h._caret = index + text.size();     //caret lands just after the inserted run
   h.clearSelection();
   h.afterEdit();
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::InsertTextAction::undo(TextEditHandler& h) {
   h._model->removeText(index, text.size()); //remove exactly what redo() inserted
   h._caret = index;                   //caret returns to where the run started
   h.clearSelection();
   h.afterEdit();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::RemoveTextAction::redo(TextEditHandler& h) {
   h._model->removeText(index, text.size()); //delete the recorded run
   h._caret = index;                   //caret collapses to the deletion point
   h.clearSelection();
   h.afterEdit();
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::RemoveTextAction::undo(TextEditHandler& h) {
   h._model->insertText(index, text);  //restore exactly what redo() removed
   h._caret = index + text.size();     //caret lands just after the restored run
   h.clearSelection();
   h.afterEdit();
}

/////////////////////////////////////////////////////////////////////////////////////////
bool TextEditHandler::InsertTextAction::tryMerge(const EditAction& next) {
   auto* o = dynamic_cast<const InsertTextAction*>(&next);
   if (!o) return false;                              //only merge insert-into-insert
   if (o->index != index + text.size()) return false; //must continue right where we left off
   //stop a merge at line boundaries so undo lands sensibly per line
   if (!text.empty() && text.back() == '\n') return false;
   if (!o->text.empty() && o->text.front() == '\n') return false;
   text += o->text;                                   //absorb the newer run
   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool TextEditHandler::RemoveTextAction::tryMerge(const EditAction& next) {
   auto* o = dynamic_cast<const RemoveTextAction*>(&next);
   if (!o) return false;                              //only merge remove-into-remove
   //forward-delete: each press removes at the same offset, so runs are consecutive
   if (o->index == index) {
      if (!text.empty() && text.back() == '\n') return false;
      if (!o->text.empty() && o->text.front() == '\n') return false;
      text += o->text;
      return true;
   }
   //backspace: each press removes just before the previous one, ending where we began
   if (o->index + o->text.size() == index) {
      if (!o->text.empty() && o->text.back() == '\n') return false;
      if (!text.empty() && text.front() == '\n') return false;
      text = o->text + text;                          //prepend the earlier-in-buffer run
      index = o->index;                               //deletion now starts further left
      return true;
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::CompositeAction::redo(TextEditHandler& h) {
   for (auto& a : actions) a->redo(h);                //replay front-to-back
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextEditHandler::CompositeAction::undo(TextEditHandler& h) {
   for (auto it = actions.rbegin(); it != actions.rend(); ++it) (*it)->undo(h); //reverse order
}
