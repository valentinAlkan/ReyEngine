#include "Table.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void Internal::TableViewWidget::render2D() const {
    //draw table lines
    std::vector<const std::vector<Line<float>>*> _lines;
    if (_showHorizontalSeparators) _lines.push_back(&_hlines);
    if (_showVeritcalSeparators) _lines.push_back(&_vlines);
    for (const auto& lines : _lines){
        for (const auto& line : *lines) {
            drawLine(line, 1.0, theme->background.colorSecondary);
        }
    }
    for (const auto& [x, items] : _data) {
        for (const auto& [y, item] : items) {
            if (item->text.empty()) continue;
            drawText(item->text, item->textPos, {item->overrideTheme ? item->overrideTheme->font : _table.getTheme().font});
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
Rect<float> Internal::TableViewWidget::getCellRect(const Pos<size_t>& cellPos) const {
    return Rect<float>(cellPos.x * _columnWidth, cellPos.y * _rowHeight, _columnWidth, _rowHeight);
}

/////////////////////////////////////////////////////////////////////////////////////////
Rect<float> Internal::TableViewWidget::getCellRect(const Pos<float>& mousePos) const {
    size_t x = mousePos.x / _columnWidth;
    size_t y = mousePos.y / _rowHeight;
    return getCellRect(Pos<size_t>(x,y));
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<size_t> Internal::TableViewWidget::getCellRectCoords(const Pos<float>& mousePos) const {
    auto cellRect = getCellRect(mousePos);
    return Pos<size_t>(cellRect.x / _columnWidth, cellRect.y/_rowHeight);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Internal::TableViewWidget::_recalculate() {
    setSize(_columnCount * _columnWidth, _rowCount * _rowHeight);
    _hlines.clear();
    _vlines.clear();
    for (int i = 0; i < _columnCount + 1; i++) {
        auto x = _columnWidth * i;
        auto a = Pos<float>(x, 0);
        auto b = Pos<float>(x, getHeight());
        _vlines.emplace_back(a, b);
    }
    for (int i = 0; i < _rowCount + 1; i++) {
        auto y = _rowHeight * i;
        auto a = Pos<float>(0, y);
        auto b = Pos<float>(getWidth(), y);
        _hlines.emplace_back(a, b);
    }
    for (const auto& [x, items] : _data) {
        for (const auto& [y, item] : items) {
            if (item->text.empty()) continue;
            item->textPos = getCellRect(Pos<size_t>(x, y)).embiggen(-2).alignText(item->text, _table.fontAlignment, *_table.getTheme().font);
         }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Table::_unhandled_input(const InputEvent& e) {
    if (e.isMouse()) {
        switch (e.eventId) {
        case InputEventMouseButton::ID: {
            auto& mbEvent = e.toEvent<InputEventMouseButton>();
            Logger::info() << std::format("Clicked on cell {}", _view->getCellRectCoords(mbEvent.mouse.getLocalPos()).toString()) << endl;
        }
        }
    }
    return nullptr;
}
