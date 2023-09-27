#pragma once
#include "BaseWidget.h"
#include "Property.h"
#include <map>

// A pre-made tree of widgets
class Scene;
namespace SceneFileParser{
   //a potential widget
   struct TreeObject {
      TreeObject(std::string name, std::shared_ptr<TreeObject>): name(name), parent(parent){
         if (parent){
            parent.value()->children.push_back(this);
         }
      }
      const std::string name;
      std::optional<std::shared_ptr<TreeObject>> parent;
      std::vector<std::shared_ptr<TreeObject>> children;
   };

   struct DescObject {
      DescObject(std::string name, std::string typeName)
      : name(name)
      , typeName(typeName)
      {}
      const std::string name;
      const std::string typeName;
      std::map<std::string, BaseProperty> properties;
      void parse();
   };

   //represents what the parser is doing with the text
   struct ParseStruct {
      enum class ParseState{NOT_FOUND, OPEN, CLOSED, ERROR};
      enum class StructType {TREE, DESC};
      enum class ParseError{INVALID_INDENT_TYPE, INVALID_INDENT_LEVEL};
      ParseError error;

      ParseStruct(StructType type): type(type){}
      virtual std::optional<ParseError> parse() = 0;
      void addLine(const std::string& line){
         _lines.push_back(line);
      };
      ParseError setError(ParseError e){
         error = e;
         currentState = ParseStruct::ParseState::ERROR;
         return e;
      };

      const StructType type;
      ParseState currentState = ParseState::NOT_FOUND;
      std::vector<std::string> _lines;
   };

   struct TreeStruct : public ParseStruct{
      TreeStruct(): ParseStruct(ParseStruct::StructType::TREE){};
      std::optional<ParseError> parse() override;
   };
   struct DescStruct : public ParseStruct{
      DescStruct(): ParseStruct(ParseStruct::StructType::DESC){};
      std::optional<ParseError> parse() override;
   };

   class Parser{
   public:
      Parser(const std::vector<char>&);
      std::optional<std::shared_ptr<Scene>> parse();
   private:
      std::vector<std::string> _tree;
      std::vector<std::string> _desc;
      const std::vector<char>& _rawData;
      
      
   };

   static constexpr char TOKEN_TREE_START[] = "[Tree]";
   static constexpr char TOKEN_DESC_START[] = "[Desc]";
   static constexpr char TOKEN_NEWLINE = '\n';
   static constexpr char TOKEN_LINE_CONTINUATION = '\\';
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