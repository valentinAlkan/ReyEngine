#include "BaseWidget.h"

class Table : public BaseWidget {
public:
   class TableItem{};

   REYENGINE_OBJECT(Table, BaseWidget){}
protected:
   void render() const;
   void addItem();
};
