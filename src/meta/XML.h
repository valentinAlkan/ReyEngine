#pragma once
#include "rapidxml.hpp"
#include <memory>
#include "FileSystem.h"
#include "rapidxml_utils.hpp"
#include <iostream>

namespace ReyEngine {
   namespace XML {
      class Document;

      template <typename T>
      class XMLBase{
      public:
         ~XMLBase(){}
         std::string value(){return _xmlobj->value();}
         std::string name(){return _xmlobj->name();}
      protected:
         XMLBase(std::shared_ptr<Document> doc, T* xmlobj): _doc(doc), _xmlobj(xmlobj){};
         std::shared_ptr<Document> _doc; //must stay alive
         T* _xmlobj;
         friend class Document;
      };

      class Attribute : public XMLBase<rapidxml::xml_attribute<>> {
      public:
         std::optional<std::shared_ptr<Attribute>> next(const std::string& matches="", bool caseSensitive=false);
         std::optional<std::shared_ptr<Attribute>> previous(const std::string& matches="", bool caseSensitive=false);
      protected:
         Attribute(std::shared_ptr<Document>, rapidxml::xml_attribute<>*);
         friend class Element;
      };

      class Element : public XMLBase<rapidxml::xml_node<>> {
      public:
         std::optional<std::shared_ptr<Element>> firstChild(const std::string& matches="", bool caseSensitive=false) const;
         std::optional<std::shared_ptr<Element>> lastChild(const std::string& matches="", bool caseSensitive=false) const;
         std::optional<std::shared_ptr<Element>> nextSibling(const std::string& matches="", bool caseSensitive=false) const;
         std::optional<std::shared_ptr<Element>> previousSibling(const std::string& matches="", bool caseSensitive=false) const;
         std::optional<std::shared_ptr<Attribute>> firstAttr(const std::string& matches="", bool caseSensitive=false) const;
         std::optional<std::shared_ptr<Attribute>> lastAttr(const std::string& matches="", bool caseSensitive=false) const;
      protected:
         Element(std::shared_ptr<Document>, rapidxml::xml_node<>*);
         friend class Document;
      };

      class Document : public std::enable_shared_from_this<Document>{
      public:
         ~Document(){}
         std::string getVersion();
         std::shared_ptr<Element> getRoot();
      protected:
         Document(const FileSystem::File& filePath);
      private:
         rapidxml::xml_document<> _doc;
         rapidxml::file<> file;
         friend class Parser;
      };
      class Parser {
         static Parser &instance();
         static std::unique_ptr<Parser> _self;
      public:
         static std::shared_ptr<Document> loadFile(const FileSystem::File&);
      };
   }
}