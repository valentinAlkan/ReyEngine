#include "Window.h"
#include "Label.h"
#include "Table.h"

using namespace ReyEngine;
using namespace std;

int main(){
   auto& window = Application::createWindowPrototype("window", 800, 600, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   auto table = make_child<Table>(root, "Table");
   table->setAnchoring(Anchor::FILL);

   table->setData<std::string>({1,1}, "tableData", "tableData", "tableData");
   table->setColumnCount(0);
   table->setRowCount(0);

   table->setData({3,4}, "sometext");

   window.exec();
   return 0;
}