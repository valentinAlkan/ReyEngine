#pragma once
#include "BaseWidget.h"

// A pre-made tree of widgets
namespace SceneFileParser{
   class Parser{
   public:
      Parser(const std::vector<std::byte>&);
      std::optional<std::shared_ptr<Scene>> parse();
   private:
      std::vector<std::string> _tree;
      std::vector<std::string> _desc;
      const std::vector<std::byte>& _rawData;
   };
};



class Scene{
public:
   Scene(std::shared_ptr<BaseWidget> root);
   std::string toString();
   static std::optional<std::shared_ptr<Scene>> fromString();
   static std::optional<std::shared_ptr<Scene>> fromFile(const std::string& filePath);
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