#pragma once
#include "BaseWidget.h"

// A pre-made tree of widgets
class Scene{
public:
   Scene(std::shared_ptr<BaseWidget> root);
   static std::shared_ptr<Scene> parseSceneFile();
   std::string toString();
   static std::shared_ptr<Scene> fromString();
private:
   std::shared_ptr<BaseWidget> _root;
   std::string sceneName;
   friend class Window;
};


//comments not supported at this time
// Blank or missing widget property fields indicate defaults

/*
 * Scene Format:
 * -----------------
 * [Tree]
 * Root
 *    Child1
 *    Child2
 *       GrandChild1
 *       GrandChild2
 *    Child3
 *       GrandChild3
 *
 *
 * [Desc]
 * Root - BaseWidget:
 *    Property0|type: Value
 *    Property1|type: Value
 *    Property2|type: Value
 * Child1 - BaseWidget:
 *    Property0|type: Value
 *    Property1|type: Value
 *    Property2|type: Value
 * Child2 - BaseWidget:
 * Child3 - BaseWidget:
 * GrandChild1 - BaseWidget:
 * GrandChild2 - BaseWidget:
 * GrandChild3 - BaseWidget:
 * -----------------
 */