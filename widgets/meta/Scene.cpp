#include "Scene.h"
#include "FileSystem.h"
#include <iostream>
#include <sstream>
#include "Application.h"
#include "StringTools.h"

using namespace std;
using namespace SceneFileParser;
using namespace FileSystem;

/////////////////////////////////////////////////////////////////////////////////////////
Scene::Scene(std::shared_ptr<BaseWidget> root)
: _root(std::move(root)){

}

/////////////////////////////////////////////////////////////////////////////////////////
optional<shared_ptr<Scene>> Scene::fromFile(const std::string& filePath) {
   auto bytes = FileSystem::loadFile(filePath);
   if (bytes.empty()){
      return nullptr;
   }

   stringstream ss;
   for (const auto& c : bytes){
      //throw away carriage return
      if (c == '\r'){
         continue;
      }
      ss << (char)c;
   }

   //   cout << ss.str() << endl;

   auto parser = SceneFileParser::Parser(bytes);
   return parser.parse();
}

/////////////////////////////////////////////////////////////////////////////////////////
optional<shared_ptr<Scene>> Scene::fromString() {
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string Scene::toString() {
   return "";
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
SceneFileParser::Parser::Parser(const std::vector<char>& data)
: _rawData(data)
{

}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Scene>> SceneFileParser::Parser::parse() {
   //parse the tree
   SceneFileParser::TreeStruct tree(ParseStruct::StructType::TREE);
   SceneFileParser::DescStruct desc(ParseStruct::StructType::DESC);
   SceneFileParser::ParseStruct* openStruct = tree;
   std::string currentLine;
   bool lineContinuation = false;
   for (const auto& c : _rawData){
      currentLine.push_back(c);
      auto doOpen = [&](SceneFileParser::ParseStruct& strct){
         openStruct->currentState = ParseStruct::ParseState::OPEN;
         openStruct = &strct;
      };

      //open a new struct (and potentially close an open struct)
      if(currentLine == TOKEN_TREE_START){
         doOpen(tree);
      } else if (currentLine == TOKEN_DESC_START){
         doOpen(desc);
      }

      if (c == TOKEN_LINE_CONTINUATION) lineContinuation = true;
      if (c == '\n' && !lineContinuation){
         lineContinuation = false;
         openStruct->addLine(currentLine);
      }

   }
   bool treeFound = tree.currentState == ParseStruct::ParseState::NOT_FOUND;
   bool descFound = desc.currentState == ParseStruct::ParseState::NOT_FOUND;
   if (!treeFound || !descFound){
      stringstream ss;
      ss << "Scene file parsing failed - " << (treeFound ? TOKEN_TREE_START : TOKEN_DESC_START) << " not found!";
      Application::exitError(ss.str(), Application::ExitReason::INVALID_SCENE_FILE_FORMAT);
   }

   //once we've added all the lines to the correct parser, we can process the structs
//   desc.parse
   
   return nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<SceneFileParser::ParseStruct::ParseError> SceneFileParser::TreeStruct::parse(){
   auto setError = [this](ParseError e){
      error = e;
      currentState = ParseStruct::ParseState::ERROR;
      return e;
   };
   //convert lines to struct type
   //indentFactor is the base "unit" of indents.
   // indentFactor * indentLevel = leading spaces for each element (or child element). It is an error if
   //  this convention is violated.
   int indentFactor = 0;
   int indentLevel = 0;
   for (const auto& line : _lines) {
      auto whiteCount = string_tools::lcount(line, ' ');

      if (indentFactor) {
         //check indent char type
         if (whiteCount % indentFactor != 0) {
            return setError(ParseStruct::ParseError::INVALID_INDENT_TYPE);
         }
         //check indent level
         auto newIndentFactor = whiteCount;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<SceneFileParser::ParseStruct::ParseError> SceneFileParser::DescStruct::parse(){
   //convert lines to struct type
}