#include "Scene.h"
#include "FileSystem.h"
#include <iostream>
#include "Application.h"
#include "StringTools.h"
#include <stack>
#include <utility>

using namespace std;
using namespace ReyEngine;
using namespace SceneFileParser;

/////////////////////////////////////////////////////////////////////////////////////////
Scene::Scene(std::shared_ptr<BaseWidget> root)
: _root(root){

}

/////////////////////////////////////////////////////////////////////////////////////////
optional<shared_ptr<Scene>> Scene::fromFile(const std::string& filePath) {
   auto bytes = FileSystem::readFile(filePath);
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
   //create a scene file
   MetaStruct meta;
   TreeStruct tree;
   DescStruct desc;
   return meta.toString(_root) + tree.toString(_root) + desc.toString(_root);
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
   SceneFileParser::MetaStruct meta;
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

      //todo: newlines in arbitrary property data? how to handle
      //open a new struct (and potentially close an open struct)
      if(currentLine == TOKEN_TREE_START) {
         doOpen(tree);
      } else if (currentLine == TOKEN_META_START){
         doOpen(meta);
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
   //if we dont have an eof newline then we need to add the last line
   auto rem = string_tools::rstrip(currentLine);
   if (!rem.empty() && string_tools::countwhite(rem) != rem.size()){
      openStruct->addLine(rem);
   }
   openStruct->currentState = ParseStruct::ParseState::CLOSED;
   bool treeFound = tree.currentState != ParseStruct::ParseState::NOT_FOUND;
   bool descFound = desc.currentState != ParseStruct::ParseState::NOT_FOUND;
   bool metaFound = meta.currentState != ParseStruct::ParseState::NOT_FOUND;
   if (!treeFound || !descFound || !metaFound){
      stringstream ss;
      ss << "Scene file parsing failed - " << (treeFound ? (descFound ? TOKEN_META_START : TOKEN_DESC_START) : TOKEN_TREE_START) << " not found!";
      Application::exitError(ss.str(), Application::ExitReason::INVALID_SCENE_FILE_FORMAT);
   }

   //once we've added all the lines to the correct parser, we can process the structs
   return meta.parse(desc.parse(tree.parse()));
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<TreeObject> SceneFileParser::TreeStruct::parse(){
   //make sure there is actual data to parse (first line will be "[Tree]")
   if (_lines.size() < 2){
      setError(ParseStruct::ParseError::MISSING_DATA, "NO_DATA");
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
std::string SceneFileParser::TreeStruct::toString(std::shared_ptr<BaseWidget> root) {
   return "";
}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> SceneFileParser::DescStruct::parse(std::shared_ptr<TreeObject> root){
   //convert desc lines to descriptions
   //drop first line ([Desc])
   if (_lines.size() < 2){
      setError(ParseStruct::ParseError::MISSING_DATA, "NO_DATA");
   }
   map<string, DescObject> objects;
   std::string currentObjectName;
   std::stack<PropertyPrototype*> propertiesStack;

   for (size_t i=1; i<_lines.size(); i++){
      auto line = _lines[i];
      //see if its a declaration line
      if (string_tools::countwhitel(line)){
         //line has leading whitespace, and so is a property
      } else {
         //line has no leading whitespace, and so is a declaration
         currentObjectName.clear();
         propertiesStack = {};
      }
      if (currentObjectName.empty()){
         //is declaration line
         auto split = string_tools::split(line, TOKEN_OBJECT_DECLARATION);
         //todo: enforce whitespace requirements and check data validity
         currentObjectName = split[0];
         auto typeName = string_tools::split(split[1], ":")[0];
         auto pair = std::pair<string, DescObject>(currentObjectName, DescObject(currentObjectName, typeName));
         objects.emplace(pair);
      } else {
         //otherwise we are declaring properties to the most recent object/property/subproperty
         // iterate over the line from the start, since the data section could be arbitrarily long
         //Check indent level to match with correct property
         auto indentCount = string_tools::countwhitel(line);
         if (indentCount % TOKEN_PROPERTY_INDENT.size() != 0){
            throw std::runtime_error("Scene file parsing: Indent level mismatch in line " + to_string(i) + ": " + line);
         }
         //pop off elements for indent count
         auto indentLevel = indentCount / TOKEN_PROPERTY_INDENT.size();
         auto popCount = propertiesStack.size() - (indentLevel - 1);
         for(size_t p=0; p<popCount; p++){
            propertiesStack.pop();
         }
         string soFar;
         string instanceName;
         string typeName;
         //todo: this could be a lot faster if we didn't allocate new strings and do copies.
         for (size_t j=0; j<line.size(); j++){
            auto c = line[j];
            //todo: use substr find
            if (c != PropertyMeta::SEP){
               soFar+=c;
            } else {
               if (instanceName.empty()){
                  //parse property name
                  instanceName = string_tools::lrstrip(soFar);
                  auto it = objects.find(currentObjectName);
                  auto& descobj = it->second;
                  if (propertiesStack.empty()){
                     //object property
                     descobj.properties[instanceName] = PropertyPrototype();
                     propertiesStack.push(&descobj.properties[instanceName]);
                  } else {
                     //subproperty
                     propertiesStack.top()->subproperties[instanceName];
                     propertiesStack.push(&propertiesStack.top()->subproperties[instanceName]);
                  }
                  propertiesStack.top()->instanceName = instanceName;
               } else if (typeName.empty()) {
                  //parse property type
                  typeName = soFar;
                  propertiesStack.top()->typeName = typeName;
               } else if (propertiesStack.top()->data.empty()){
                  //parse property data
                  //this is where things get tricky - property data can be arbitrarily long and include arbitrary symbols
                  // so we need to advance the line pointer and consume as many chars as we need
                  size_t count = stoi(soFar); //should throw an error if invalid
                  //pull in that many bytes, and go to the next line if we have to
                  while (count > 0){
                     j++;
                     if (j >= line.size()){
                        //go to next line
                        i++;
                        line = _lines[i];
                     }
                     propertiesStack.top()->data += line[j];
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
      }
      //Deserialize the type
      obj->widget = TypeManager::deserialize(desc->typeName, desc->instanceName, desc->properties);
      //deserialize children
      for (auto& child : obj->children){
         deserialize(child);
         obj->widget->addChild(child->widget);
      }
   };

   deserialize(root);
   //declare the scene
   auto scene = make_shared<Scene>(root->widget);
   return scene;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string SceneFileParser::DescStruct::toString(std::shared_ptr<BaseWidget> root) {
   stringstream ss;
   ss << TOKEN_DESC_START;
   map<string, std::string> descs;
   auto& widget = root;
   while (widget){
      descs[widget->getName()] = widget->serialize();
   }
   return "";
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> SceneFileParser::MetaStruct::parse(std::shared_ptr<Scene> scene) {
   for (const auto& line : _lines){
      if(line == TOKEN_META_START) continue;
      auto split = string_tools::split(line, ":");
      if (split.size() != 2){
         throw std::runtime_error("Key value pair requires two values to unpack!");
      }
      auto key = string_tools::lrstrip(split[0]);
      auto value = string_tools::lrstrip(split[1]);
      if (key==KEY_SCENE_NAME){
         scene->name = value;
      }
   }
   return scene;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string SceneFileParser::MetaStruct::toString(std::shared_ptr<BaseWidget> root) {
   return "";
}