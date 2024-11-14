#include "BaseWidget.h"

using namespace std;
using namespace ReyEngine;

class BuildableObject : public BaseWidget{
private:
   REYENGINE_OBJECT_BUILD_ONLY(BuildableObject, BaseWidget){}
public:
   REYENGINE_DEFAULT_BUILD(BuildableObject)
   void render() const override{};
};




int main(){
   return 0;
}