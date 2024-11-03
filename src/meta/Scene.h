#pragma once
#include "Component.h"
#include "Property.h"
#include <map>
#include "StringTools.h"
#include "TypeManager.h"

namespace ReyEngine{
   // A pre-made tree of widgets
   class Scene;
   namespace SceneFileParser{
      //a potential widget
      struct TreeObject : std::enable_shared_from_this<TreeObject>{
      private:
         TreeObject(const std::string& name): instanceName(name){}
      public:
         static std::shared_ptr<TreeObject> make_shared(std::string name){
            return std::shared_ptr<TreeObject>(new TreeObject(name));
         }
         void addChild(std::shared_ptr<TreeObject> child){
            if (child){
               children.push_back(child);
               child->parent = shared_from_this();
            } else {
               throw std::runtime_error("Child is null!");
            }
         }
         const std::string instanceName;
         std::weak_ptr<TreeObject> parent;
         std::vector<std::shared_ptr<TreeObject>> children;
         std::shared_ptr<Internal::Component> component;

      };

      struct DescObject {
         DescObject(std::string instanceName, std::string typeName)
         : instanceName(instanceName)
         , typeName(typeName)
         {}
         const std::string instanceName;
         const std::string typeName;
         PropertyPrototypeMap properties;
      };

      //represents what the parser is doing with the text
      struct ParseStruct {
         enum class ParseState{NOT_FOUND, OPEN, CLOSED, ERROR};
         enum class StructType {TREE, DESC, META};
         enum class ParseError{MISSING_DATA, BLANK_LINE, ROOT_LEADING_WHITESPACE, INVALID_CONTENTS, INVALID_INDENT_TYPE, INVALID_INDENT_LEVEL};
         ParseError error;

         ParseStruct(StructType type): type(type){}
         void addLine(const std::string& line){
            _lines.push_back(line);
         };
         void setError(ParseError e, const std::string& msg){
            error = e;
            currentState = ParseStruct::ParseState::ERROR;
            //for now, throw exception
            throw std::runtime_error("Parse error: " + msg);
         };

         const StructType type;
         ParseState currentState = ParseState::NOT_FOUND;
         std::vector<std::string> _lines;
         virtual std::string toString(std::shared_ptr<Internal::Component> root) = 0;
      };

      struct TreeStruct : public ParseStruct{
         TreeStruct(): ParseStruct(ParseStruct::StructType::TREE){};
         std::shared_ptr<TreeObject> parse();
         std::string toString(std::shared_ptr<Internal::Component> root) override;
      };

      struct DescStruct : public ParseStruct{
         DescStruct(): ParseStruct(ParseStruct::StructType::DESC){};
         std::shared_ptr<Scene> parse(std::shared_ptr<TreeObject> root);
         std::string toString(std::shared_ptr<Internal::Component> root) override;
      };

      struct MetaStruct : public ParseStruct{
         MetaStruct(): ParseStruct(ParseStruct::StructType::META){}
         std::shared_ptr<Scene> parse(std::shared_ptr<Scene> scene);
         std::string toString(std::shared_ptr<Internal::Component> root) override;
      };

      class Parser{
      public:
         Parser(const std::vector<char>&);
         std::shared_ptr<Scene> parse();
      private:
         std::vector<std::string> _tree;
         std::vector<std::string> _desc;
         const std::vector<char>& _rawData;


      };

      static constexpr char TOKEN_META_START[] = "[Meta]";
      static constexpr char TOKEN_TREE_START[] = "[Tree]";
      static constexpr char TOKEN_DESC_START[] = "[Desc]";
      static constexpr char TOKEN_NEWLINE = '\n';
      static constexpr char TOKEN_LINE_CONTINUATION = '\\';
      static constexpr char TOKEN_OBJECT_DECLARATION[] = " - ";
      static constexpr char TOKEN_OBJECT_TERMINATOR = ';';
      static constexpr char TOKEN_PROPERTY_LIST = ':';
      static constexpr char TOKEN_PROPERTY_SEP = ':';
      static std::string_view TOKEN_PROPERTY_INDENT = "   ";

      static constexpr char KEY_SCENE_NAME[] = "SceneName";
   };



   class Scene{
   public:
      Scene(std::shared_ptr<Internal::Component> root);
      std::string toString();
      static std::optional<std::shared_ptr<Scene>> fromString();
      static std::optional<std::shared_ptr<Scene>> fromFile(const FileSystem::File& file);
      std::shared_ptr<Internal::Component> getRoot(){return _root;}
   protected:
      std::string name;
   private:
      std::shared_ptr<Internal::Component> _root;
      std::string sceneName;
      friend struct SceneFileParser::MetaStruct;
      friend class Window;
   };
}