#include "Application.h"
#include "Window.h"
#include "Layout.h"
#include "Button.h"
#include "LineEdit.h"
#include "MenuBar.h"
#include "ScrollArea.h"
#include "ColorPicker.h"
#include "Tree.h"

using namespace std;
using namespace ReyEngine;

//displays a single named color belonging to a theme
class ThemeMember : public Widget {
public:
   REYENGINE_OBJECT(ThemeMember)
   EVENT(EventSelected, 561234987003){}};
   static constexpr float HEIGHT = 30;
   ThemeMember(const std::string& text = "", ColorRGBA* color = nullptr)
   : _text(text)
   , _color(color)
   {}
   void setColorSource(const std::string& text, ColorRGBA* color){_text = text; _color = color;}
   [[nodiscard]] std::string getText() const {return _text;}
   [[nodiscard]] ColorRGBA* getColorSource() {return _color;}
   void setSelected(bool selected){_selected = selected;}
   [[nodiscard]] bool isSelected() const {return _selected;}
protected:
   Handled _unhandled_input(const InputEvent& e) override {
      if (auto mouseEvent = e.isMouse()){
         if (e.isEvent<InputEventMouseButton>()){
            auto& btnEvent = e.toEvent<InputEventMouseButton>();
            if (btnEvent.isDown && btnEvent.button == InputInterface::MouseButton::LEFT && mouseEvent.value()->isInside()){
               EventSelected event(this);
               publish(event);
               return this;
            }
         }
      }
      return nullptr; //everything else (including right clicks) falls through to the explorer
   }
   void render2D(RenderContext&) const override {
      if (!_color) return;
      drawRectangle(getSizeRect(), *_color);
      drawText(_text, {0, 0}, theme->font, _color->getReadableTextColor(), theme->font->size, theme->font->spacing);
      if (_selected) drawRectangleLines(getSizeRect(), 3, Colors::red);
   }
private:
   std::string _text;
   ColorRGBA* _color = nullptr; //points into the theme, so edits to the theme show up live
   bool _selected = false;
};

class ThemeExplorer : public Widget {
public:
   REYENGINE_OBJECT(ThemeExplorer)
   EVENT_ARGS(EventSelectionChanged, 561234987004, const std::shared_ptr<ThemeMember>& member)
   , member(member)
   {}
      const std::shared_ptr<ThemeMember> member; //null when the selection was cleared
   };
   [[nodiscard]] std::shared_ptr<ThemeMember> getSelected() const {return _selectedMember;}
protected:
   void _init() override {
      const std::pair<const char*, ColorRGBA*> members[] = {
         {"Background: colorPrimary",   &theme->background.colorPrimary},
         {"Background: colorSecondary", &theme->background.colorSecondary},
         {"Background: colorTertiary",  &theme->background.colorTertiary},
         {"Background: colorHighlight", &theme->background.colorHighlight},
         {"Background: colorActive1",   &theme->background.colorActive1},
         {"Background: colorActive2",   &theme->background.colorActive2},
         {"Background: colorDisabled",  &theme->background.colorDisabled},
         {"Foreground: colorPrimary",   &theme->foreground.colorPrimary},
         {"Foreground: colorSecondary", &theme->foreground.colorSecondary},
         {"Foreground: colorTertiary",  &theme->foreground.colorTertiary},
         {"Foreground: colorHighlight", &theme->foreground.colorHighlight},
         {"Foreground: colorActive1",   &theme->foreground.colorActive1},
         {"Foreground: colorActive2",   &theme->foreground.colorActive2},
         {"Foreground: colorDisabled",  &theme->foreground.colorDisabled},
      };
      int i = 0;
      for (const auto& [text, color] : members){
         auto member = make_child<ThemeMember>("themeMember" + std::to_string(i++), text, color);
         subscribe<ThemeMember::EventSelected>(member, [this, member](const ThemeMember::EventSelected&){_select(member);});
         _members.push_back(member);
      }
      _layoutMembers();
      //size ourselves to our content so the enclosing scroll area can scroll it
      setMinHeight(_members.size() * (ThemeMember::HEIGHT + RECT_GAP));
      //track the scroll area's width (the scroll area does not size its content - our height stays our own)
      setAnchoring(Anchor::TOP_WIDTH);
   }
   void _on_rect_changed() override {
      if (!_has_inited) return;
      _layoutMembers();
   }
   Handled _unhandled_input(const InputEvent& e) override {
      //right clicking anywhere on the explorer clears the selection
      if (auto mouseEvent = e.isMouse()){
         if (e.isEvent<InputEventMouseButton>()){
            auto& btnEvent = e.toEvent<InputEventMouseButton>();
            if (btnEvent.isDown && btnEvent.button == InputInterface::MouseButton::RIGHT && mouseEvent.value()->isInside()){
               _select(nullptr);
               return this;
            }
         }
      }
      return nullptr;
   }
private:
   void _select(const std::shared_ptr<ThemeMember>& member){
      if (_selectedMember) _selectedMember->setSelected(false);
      _selectedMember = member;
      if (_selectedMember) _selectedMember->setSelected(true);
      EventSelectionChanged event(this, _selectedMember);
      publish(event);
   }
   std::shared_ptr<ThemeMember> _selectedMember;
   static constexpr float RECT_GAP = 2.0;
   void _layoutMembers(){
      float y = 0;
      for (auto& member : _members){
         member->setRect(0, y, getWidth(), ThemeMember::HEIGHT);
         y += ThemeMember::HEIGHT + RECT_GAP;
      }
   }
   std::vector<std::shared_ptr<ThemeMember>> _members;
};

int main() {
//   ArgParse args;
//   args.defineArg(RuntimeArg("--fontPath", "fontPath", 0, RuntimeArg::ArgType::FLAG));
//   args.parseArgs(argc, argv);

   constexpr float ROW_HEIGHT = 30;

   auto& window = Application::createWindowPrototype("UITest", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   auto mainHLayout = make_child<Layout>(root, "mainVLayout", Layout::LayoutDir::HORIZONTAL);
   mainHLayout->getTheme().layoutMargin = 2;
   mainHLayout->getTheme().widgetPadding = {2,2,2,2};
   mainHLayout->setAnchoring(ReyEngine::Anchor::FILL);
   auto vlayoutl = make_child<Layout>(mainHLayout, "vlayoutl", Layout::LayoutDir::VERTICAL);
   auto vlayoutm1 = make_child<Layout>(mainHLayout, "vlayoutm1", Layout::LayoutDir::VERTICAL);
   auto vlayoutm2 = make_child<Layout>(mainHLayout, "vlayoutm2", Layout::LayoutDir::VERTICAL);
   auto vlayoutr = make_child<Layout>(mainHLayout, "vlayoutr", Layout::LayoutDir::VERTICAL);

   //add each widget type
   auto menuBar = make_child<MenuBar>(vlayoutm1, "menubar");
   auto fileMenu = menuBar->createDropDown("File");
   fileMenu->push_back("this", "is", "some", "items");

   auto scrollArea = vlayoutl->make_child<ScrollArea>("scrollArea");
   auto themeExplorer = scrollArea->make_child<ThemeExplorer>("themeExplorer");
   auto colorPicker = vlayoutl->make_child<ColorPicker>("colorPicker");


   //widgets
   auto pushbutton = make_child<PushButton>(vlayoutm1, "pushbutton");
   auto toggleButton = make_child<ToggleButton>(vlayoutm1, "toggleButton");
   auto lineEditHLayout = make_child<Layout>(vlayoutm1, "lineEditHLayout", Layout::LayoutDir::HORIZONTAL);
   auto lineEdit = make_child<LineEdit>(lineEditHLayout, "lineedit", "LineEdit");
   auto checkbox = make_child<CheckBox>(lineEditHLayout, "checkbox", "Enabled");
   checkbox->setChecked(true);

   for (auto& child : vlayoutm1->getChildren()){
      child->as<Widget>().value()->setMaxHeight(ROW_HEIGHT);
   }

   //extra functionality for widgets
   auto cbLineEditDisabled = [lineEdit](const CheckBox::ButtonToggleEvent& e){
      lineEdit->setEnabled(e.button().getDown());
   };
   lineEdit->subscribe<CheckBox::ButtonToggleEvent>(checkbox, cbLineEditDisabled);

   //selecting a theme member loads its color into the color picker (without publishing,
   // so loading a color doesn't immediately write it back)
   colorPicker->subscribe<ThemeExplorer::EventSelectionChanged>(themeExplorer, [colorPicker](const ThemeExplorer::EventSelectionChanged& e){
      if (e.member && e.member->getColorSource()){
         colorPicker->setColor(*e.member->getColorSource(), false);
      }
   });
   //changing the color in the picker writes through to the selected style role's color,
   // which every widget sharing the theme reflects on the next frame
   themeExplorer->subscribe<ColorPicker::EventColorChanged>(colorPicker, [themeExplorer](const ColorPicker::EventColorChanged& e){
      if (auto selected = themeExplorer->getSelected()){
         if (auto color = selected->getColorSource()){
            *color = e.color;
         }
      }
   });


   auto nowSteady = [](){return std::chrono::steady_clock::now();};

   //tree test
   auto populate = vlayoutr->make_child<PushButton>("populate");
   populate->setMaxHeight(30);
   constexpr size_t ITEM_COUNT = 100000;
   auto treeTest = vlayoutr->make_child<Tree>("scrollARea");
   auto treeRoot = treeTest->setRoot(Tree::createItem("root"));

   auto work = [&](const auto& e) {
      for (int i=0; i<ITEM_COUNT; i++) {
         auto start = nowSteady();
         auto txt = "item" + to_string(i);
         {
            auto item = Tree::createItem(txt);
            treeRoot->push_back(std::move(item));
         }
         auto dt = nowSteady() - start;
         // Logger::info() << "Adding item " << txt << " took "  << std::chrono::duration_cast<std::chrono::microseconds>(dt).count() << " us" << endl;
      }
   };

   populate->subscribe<PushButton::ButtonPressEvent>(populate, work);



   //exec
   window.exec();

}