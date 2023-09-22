#pragma once

#include <string>
#include <vector>
#include <optional>
#include <map>

struct RuntimeArg{
   enum class ArgType{
      DEDUCE,  //if arg has leading dash and one char, or two leading dashes, then will be be a flag - otherwise positional if appears before another flag or optional, otherwise optional
      POSITIONAL,
      OPTIONAL,
      FLAG  //Flags will not consume args
   };
   enum class ConsumeType{
      CONSUME_COUNT,
      CONSUME_ALL,
   };

   enum class ValueType{
      INT,
      FLOAT,
      STRING
   };
   RuntimeArg(
         const std::string& name,
         std::string docString="",
         ArgType argType=ArgType::DEDUCE,
         ValueType valueType = ValueType::STRING,
         ConsumeType consumeType=ConsumeType::CONSUME_COUNT,
         int varCount=1,
         std::string dest="",
         std::string metavar=""
         );
   static std::optional<std::string> flagParse(std::string rawArg);
private:
   std::string sanitizeName(const std::string& arg);
   std::string _rawValue; //raw
   std::string _docString;
   ArgType     _argType;
   ValueType   _valueType;
   ConsumeType _consume;
   int         _varCount;
   std::string _dest;
   std::string _metavar;
   std::string _value;
   std::vector<std::string> _params; //any additional parameters the arg consumed
};


class ArgParse {
public:
   ArgParse(){};
   void parseArgs(int argc, char** argv);
   std::optional<RuntimeArg> getArg(const std::string& name);
   void defineArg(RuntimeArg arg);
private:
   std::string _arg0;
   size_t _argc;
   std::vector<std::string> _rawArgs;
   std::map<std::string, RuntimeArg> _definedArgs;
};