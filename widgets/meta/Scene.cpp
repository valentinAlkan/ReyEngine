#include "Scene.h"
#include "FileSystem.h"
#include <iostream>
#include <sstream>
#include "Application.h"
#include "StringTools.h"
#include <stack>
#include <utility>

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
std::shared_ptr<Scene> SceneFileParser::Parser::parse() {
   //parse the tree
   SceneFileParser::TreeStruct tree;
   SceneFileParser::DescStruct desc;
   SceneFileParser::ParseStruct* openStruct = nullptr;
   std::string currentLine;
   bool lineContinuation = false;
   for (const auto& c : _rawData){
      currentLine.push_back(c);
      auto doOpen = [&](SceneFileParser::ParseStruct& strct){
         if (openStruct) openStruct->currentState = ParseStruct::ParseState::CLOSED;
         openStruct = &strct;
         openStruct->currentState = ParseStruct::ParseState::OPEN;
      };

      //open a new struct (and potentially close an open struct)
      if(currentLine == TOKEN_TREE_START){
         doOpen(tree);
      } else if (currentLine == TOKEN_DESC_START){
         doOpen(desc);
      } else if (c == TOKEN_LINE_CONTINUATION) {
         lineContinuation = true;
      } else if (c == TOKEN_NEWLINE && !lineContinuation) {
         lineContinuation = false;
         openStruct->addLine(string_tools::rstrip(currentLine));
         currentLine.clear();
      }

   }
   openStruct->currentState = ParseStruct::ParseState::CLOSED;
   bool treeFound = tree.currentState != ParseStruct::ParseState::NOT_FOUND;
   bool descFound = desc.currentState != ParseStruct::ParseState::NOT_FOUND;
   if (!treeFound || !descFound){
      stringstream ss;
      ss << "Scene file parsing failed - " << (treeFound ? TOKEN_TREE_START : TOKEN_DESC_START) << " not found!";
      Application::exitError(ss.str(), Application::ExitReason::INVALID_SCENE_FILE_FORMAT);
   }

   //once we've added all the lines to the correct parser, we can process the structs
   return desc.parse(tree.parse());
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<TreeObject> SceneFileParser::TreeStruct::parse(){
   //make sure there is actual data to parse (first line will be "[Tree]")
   if (_lines.size() < 2){
      setError(ParseStruct::ParseError::NO_DATA, "NO_DATA");
   }
   //convert lines to struct type
   // baseIndentCharCount * currentIndentLevel = leading spaces for each element (or child element). It is an error if
   //  this convention is violated.
   int baseIndentCharCount = 0;
   int currentIndentLevel = 0;

   //the stack to contain the current parent
   stack<shared_ptr<TreeObject>> parentStack;

   //make sure the line isn't blank and has no leading whitespace
   auto rootName = _lines[1];
   if (allWhite(rootName)){
      setError(ParseStruct::ParseError::BLANK_LINE, "BLANK_LINE");
   }
   if (string_tools::countwhitel(rootName)){
      setError(ParseStruct::ParseError::ROOT_LEADING_WHITESPACE, "ROOT_LEADING_WHITESPACE");
   }
   //create the root
   auto root = TreeObject::make_shared(string_tools::rstrip(rootName));
   parentStack.push(root);

   //parse the first two lines so we can get the root name and the indent type
   if (_lines.size() > 2){
      //get the indent char count
      baseIndentCharCount = string_tools::countwhitel(_lines[2]);
      if (!baseIndentCharCount){
         //no spaces on second line, or the second line is not what we expected.
         setError(ParseStruct::ParseError::INVALID_CONTENTS, "INVALID_CONTENTS");
      }
   } else {
      //no children, only root
      return root;
   }

   //parse children
   //start at second line, if applicable
   for (size_t i=2; i<_lines.size(); i++){
      auto line = _lines[i];
      auto whiteCount = string_tools::lcount(line, ' ');
      if (line.empty() || line.size() == whiteCount){
         //empty or all whitespace
         continue;
      }

      //indent consistency check
      auto newIndentLevel = whiteCount / baseIndentCharCount;
      auto rem = whiteCount % baseIndentCharCount;
      if (rem != 0) {
         setError(ParseStruct::ParseError::INVALID_INDENT_TYPE, "INVALID_INDENT_TYPE");
      }

      if (newIndentLevel == currentIndentLevel){
         //the top of the stack does not have children
         parentStack.pop();
      } else if (newIndentLevel < currentIndentLevel){
         //currentObj has no more children. determine how many indent levels we have dropped,
         auto popCount = currentIndentLevel - newIndentLevel;
         if (popCount > parentStack.size()){
            //dropped too many indent levels somehow - is this even possible? come to a halt
            throw std::runtime_error("Dropped too many indent levels");
         }
         // pop that many objects, plus one
         for (int j=0; j<popCount + 1; j++){
            parentStack.pop();
         }
      } else if (newIndentLevel > currentIndentLevel + 1) {
         throw std::runtime_error("newIndentLevel > currentIndentLevel + 1");
      }
      //note - we do not need to handle newIndentLevel == currentIndentLevel + 1 because we just
      // add a child to the object at the top of the stack - which we do anyway.

      auto newObj = TreeObject::make_shared(string_tools::lrstrip(line));
      parentStack.top()->addChild(newObj);
      parentStack.push(newObj);
      Application::printDebug() << "Scene: TreeObject " << newObj->instanceName << " added to parent " << newObj->parent.lock()->instanceName << endl;

      currentIndentLevel = newIndentLevel;
   }
   return root;
}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> SceneFileParser::DescStruct::parse(std::shared_ptr<TreeObject> root){
   //convert desc lines to descriptions
   //drop first line ([Desc])
   if (_lines.size() < 2){
      setError(ParseStruct::ParseError::NO_DATA, "NO_DATA");
   }
   map<string, DescObject> objects;
   std::string currentObjectName;

   for (size_t i=1; i<_lines.size(); i++){
      auto line = _lines[i];
      //see if its a declaration line
      if (currentObjectName.empty()){
         //is delcaration line
         auto split = string_tools::split(line, TOKEN_OBJECT_DECLARATION);
         //todo: enforce whitespace requirements and check data validity
         currentObjectName = split[0];
         auto typeName = string_tools::split(split[1], ":")[0];
         auto pair = std::pair<string, DescObject>(currentObjectName, DescObject(currentObjectName, typeName));
         objects.emplace(pair);
      } else {
         //otherwise we are declaring properties to the most recent object
         //iterate over the line from the start, since the data section could be arbitrarily long
         string soFar;
         string instanceName;
         string typeName;
         PropertyPrototype* protoperty = nullptr; //pronounced pro-top-urty
         //todo: this could be a lot faster if we didn't allocate new strings and do copies.
         for (size_t j=0; j<line.size(); j++){
            auto c = line[j];
            //todo: use substr find
            if (c != PropertyMeta::SEP){
               soFar+=c;
            } else {
               if (instanceName.empty()){
                  instanceName = string_tools::lrstrip(soFar);
                  auto it = objects.find(currentObjectName);
                  auto& descobj = it->second;
                  descobj.properties[instanceName] = PropertyPrototype();
                  protoperty = &descobj.properties[instanceName];
                  protoperty->instanceName = instanceName;
               } else if (typeName.empty()) {
                  typeName = soFar;
                  protoperty->typeName = typeName;
               } else if (protoperty->data.empty()){
                  //this is where things get tricky - property data can be arbitrarily long and include arbitrary symbols
                  // so we need to advance the line pointer and consume as many chars as we need
                  size_t count = stoi(soFar); //should throw an error if invalid
                  //pull in that many bytes, and go to the next line if we have to
                  soFar.clear();
                  j++; //advance char since next char will be a separator
                  while (count > 0){
                     if (j >= line.size()){
                        //go to next line
                        i++;
                        line = _lines[i];
                     }
                     protoperty->data += line[j];
                     count--;
                  }
                  break;
               }
               soFar.clear();
            }
         }
      }
   }

   // We now have a map of desc objects. Go through the tree and pick out which one is which
   auto findDesc = [&](string typeName) -> DescObject*{
      auto found = objects.find(typeName);
      if (found == objects.end()){
         return nullptr;
      }
      return &found->second;
   };

   //recursive deserializer function
   std::function<void(shared_ptr<TreeObject>)> deserialize = [&](const shared_ptr<TreeObject>& obj){
      auto desc = findDesc(obj->instanceName);
      if (!desc) {
         throw std::runtime_error("Object type " + desc->instanceName + " has no matching Description!");
      }//Deserialize the type
      obj->widget = TypeManager::deserialize(desc->typeName, desc->instanceName, desc->properties);
      for (auto& child : obj->children){
         deserialize(child);
         obj->widget->addChild(child->widget);
      }
   };

   deserialize(std::move(root));
   //declare the scene
   auto scene = make_shared<Scene>(root->widget);
   return scene;
}

