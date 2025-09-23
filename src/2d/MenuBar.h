#pragma once
#include "Widget.h"

namespace ReyEngine{
   using MenuEntry = std::pair<std::shared_ptr<ReyTexture>, std::string>;
   namespace Internal{
      struct MenuInterface{
         MenuInterface(std::vector<MenuEntry>& entries){}
         MenuInterface();
         void push_back(const MenuEntry&);
         void push_back(const std::string&);
      protected:
         std::vector<MenuEntry> _entries;
      };
   }
   class DropDownMenu : public ReyEngine::Widget {
   public:
      DropDownMenu(const std::vector<MenuEntry> &){}
      EVENT_ARGS(EventItemHovered, 98123475981, const std::string &item), item(item){}
         const std::string item;
      };
      EVENT_ARGS(EventItemSelected, 981234759822, const std::string &item), item(item){}
         const std::string item;
      };
      void render2D() const override;
   };
   class MenuBar : public ReyEngine::Widget {
   public:
      REYENGINE_OBJECT(MenuBar);
      MenuBar(std::vector<std::string> item){}
   protected:


   };
}