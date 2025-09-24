#pragma once
#include "Widget.h"

namespace ReyEngine{
   class MenuBar;
   class DropDownMenu;
   namespace Internal{
      struct MenuInterface{
         struct MenuEntry {
            MenuEntry(MenuInterface* interface, std::shared_ptr<ReyTexture> icon, const std::string& text)
            : _interface(interface)
            , _text(text)
            , _icon(icon)
            {}
            void setText(const std::string&);
            void setIcon(std::shared_ptr<ReyTexture>&);
            const std::string& getText() const {return _text;}
            const std::shared_ptr<ReyTexture>&  getIcon() const {return _icon;}
         protected:
            std::string _text;
            std::shared_ptr<ReyTexture> _icon;
            MenuInterface* _interface;
            Rect<float> _area; //a physical area to represent where this option is - can be used for input detection
            friend class ReyEngine::MenuBar;
            friend class ReyEngine::DropDownMenu;
            friend class ReyEngine::Internal::MenuInterface;
         };
         MenuInterface(std::vector<std::unique_ptr<MenuEntry>>&& entries): _entries(std::move(entries)){}
         MenuInterface(){};
         MenuEntry* push_back(std::unique_ptr<MenuEntry>&&);
         void push_back(const std::vector<std::string>&);
         MenuEntry* at(size_t index){return _entries.at(index).get();};
         std::optional<MenuEntry*> at(Pos<float>&);
         std::optional<MenuEntry*> at(Pos<float>&& p){return at(p);}
         MenuEntry* front(){return _entries.front().get();}
         void erase(int index){
            auto it = _entries.begin();
            _entries.erase(it);
         }
         MenuEntry take(int index){
            auto it = _entries.begin() + index;
            auto retval = std::move(*it);
            _entries.erase(it);
            return *retval;
         }
      protected:
         virtual void _on_change() = 0;
         std::optional<MenuEntry*> _activeEntry; //can be used to determine which event is currently active
         std::vector<std::unique_ptr<MenuEntry>> _entries;
      };
   }
   class DropDownMenu : public ReyEngine::Widget, public Internal::MenuInterface {
   public:
      DropDownMenu(std::vector<std::unique_ptr<MenuEntry>>&& items): Internal::MenuInterface(std::move(items)){_on_change();}
      DropDownMenu(): Internal::MenuInterface(){}
      EVENT_ARGS(EventItemHovered, 98123475981, const MenuEntry* item), item(item){}
         const MenuEntry* item;
      };
      EVENT_ARGS(EventItemSelected, 981234759822, const MenuEntry* item), item(item){}
         const MenuEntry* item;
      };
      void open(){setVisible(true);setModal(true);}
      void close(){setVisible(false);setModal(false);}
   protected:
      Widget* _unhandled_input(const InputEvent&) override;
      void render2D() const override;
      void _init() override;
      void _on_change() override;
   private:
      static constexpr ColorRGBA GRADIENT_1 = {220,205,216,255};
      static constexpr ColorRGBA GRADIENT_2 = {200,200,200,255};
   };
   class MenuBar : public ReyEngine::Widget, public Internal::MenuInterface {
   public:
      REYENGINE_OBJECT(MenuBar);
      MenuBar(std::vector<std::unique_ptr<MenuEntry>>& items): Internal::MenuInterface(std::move(items)){_on_change();}
      MenuBar(): Internal::MenuInterface(){}
      std::shared_ptr<DropDownMenu> createDropDown(MenuEntry*);
      void removeDropDownMenu(MenuEntry);
   protected:
      void render2D() const override;
      void _init() override;
      void _on_change() override;
      Widget* _unhandled_input(const InputEvent&) override;
      std::optional<DropDownMenu*> getDropDown(const std::string& menu);
      void showDropDown(const std::string& menu, const Pos<float>&);
      bool _itemDown = false;
      DropDownMenu* _lastDrop = nullptr;
   private:
      static constexpr ColorRGBA GRADIENT_1 = {180,180,180,255};
      static constexpr ColorRGBA GRADIENT_2 = {200,200,200,255};
      static constexpr char DROP_DOWN_PREFIX[] = "DROPDOWN_";
   };
}