#include "XML.h"
#include <iostream>
using namespace std;
using namespace ReyEngine;
using namespace rapidxml;
using namespace XML;

std::unique_ptr<Parser> Parser::_self;

/////////////////////////////////////////////////////////////////////////////////////////
Parser& Parser::instance() {
   if (!_self){
      _self.reset(new Parser);
   }
   return *_self;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Document> Parser::loadFile(const FileSystem::File& filePath) {
   shared_ptr<Document> retval;
   retval.reset(new Document(filePath));
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
Document::Document(const FileSystem::File& filePath)
: file(filePath.abs().c_str())
{
   //load file
   _doc.parse<0>(file.data());
}
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Element> Document::getRoot() {
   shared_ptr<Element> element;
   element.reset(new Element(shared_from_this(), _doc.first_node()));
   return element;
}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Element::Element(std::shared_ptr<Document> doc, rapidxml::xml_node<>* node): XMLBase(doc, node){}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Element>> Element::firstChild(const std::string &matches, bool caseSensitive) const {
   auto retval = _xmlobj->first_node(matches.empty() ? nullptr : matches.c_str(), matches.size(), caseSensitive);
   if (retval){
      return shared_ptr<Element>(new Element(_doc, retval));
   }
   return nullopt;
}
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Element>> Element::lastChild(const std::string &matches, bool caseSensitive) const {
   auto retval = _xmlobj->last_node(matches.empty() ? nullptr : matches.c_str(), matches.size(), caseSensitive);
   if (retval){
      return shared_ptr<Element>(new Element(_doc, retval));
   }
   return nullopt;
}
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Element>> Element::nextSibling(const std::string &matches, bool caseSensitive) const {
   auto retval = _xmlobj->next_sibling(matches.empty() ? nullptr : matches.c_str(), matches.size(), caseSensitive);
   if (retval){
      return shared_ptr<Element>(new Element(_doc, retval));
   }
   return nullopt;
}
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Element>> Element::previousSibling(const std::string &matches, bool caseSensitive) const {
   auto retval = _xmlobj->previous_sibling(matches.empty() ? nullptr : matches.c_str(), matches.size(), caseSensitive);
   if (retval){
      return shared_ptr<Element>(new Element(_doc, retval));
   }
   return nullopt;
}
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Attribute>> Element::firstAttr(const std::string &matches, bool caseSensitive) const {
   auto retval = _xmlobj->first_attribute(matches.empty() ? nullptr : matches.empty() ? 0 : matches.c_str(), matches.size(), caseSensitive);
   if (retval){
      return shared_ptr<Attribute>(new Attribute(_doc, retval));
   }
   return nullopt;
}
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Attribute>> Element::lastAttr(const std::string &matches, bool caseSensitive) const {
   auto retval = _xmlobj->last_attribute(matches.empty() ? nullptr : matches.c_str(), matches.size(), caseSensitive);
   if (retval){
      return shared_ptr<Attribute>(new Attribute(_doc, retval));
   }
   return nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Attribute::Attribute(std::shared_ptr<Document> doc, rapidxml::xml_attribute<>* attr): XMLBase(doc, attr){}
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Attribute>> Attribute::next(const std::string &matches, bool caseSensitive) {
   auto retval = _xmlobj->next_attribute(matches.empty() ? nullptr : matches.c_str(), matches.size(), caseSensitive);
   if (retval){
      return shared_ptr<Attribute>(new Attribute(_doc, retval));
   }
   return nullopt;
}
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Attribute>> Attribute::previous(const std::string &matches, bool caseSensitive) {
   auto retval = _xmlobj->previous_attribute(matches.empty() ? nullptr : matches.c_str(), matches.size(), caseSensitive);
   if (retval){
      return shared_ptr<Attribute>(new Attribute(_doc, retval));
   }
   return nullopt;
}