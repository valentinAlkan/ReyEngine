#pragma once

#include <string>
#include <vector>
#include <optional>
#include <map>

struct RuntimeArg{
   enum class ArgType{
      DEDUCE,  //if arg has leading dash and one char, or two leading dashes, then will be be a flag - otherwise positional if appears before another flag or optional, otherwise optional
      POSITIONAL, //args is required and an error is thrown if it is not there
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
         int paramCount=1,
         ArgType argType=ArgType::DEDUCE,
         ValueType valueType = ValueType::STRING,
         ConsumeType consumeType=ConsumeType::CONSUME_COUNT,
         std::string dest="",
         std::string metavar=""
         );
   static std::optional<std::string> flagParse(std::string rawArg);
   std::string name() const {return _sanitizedName;}
   const std::vector<std::string>& getParams() const {return _params;}
   RuntimeArg(const RuntimeArg&);
protected:
   static std::string sanitizeName(const std::string& arg);
   void setValue(const std::string& value);
private:
   std::string _sanitizedName;
   std::string _rawValue; //the value as it exists on the commandline, including dashes and everything
   bool _invoked; //whether or not the arg was invoked on the command-line (that is to say, parsed by the parser at runtime)
   unsigned int _invoked_args_count; //how many args were passed to the param. must match expected positional args count
   std::string _docString;
   ArgType     _argType;
   ValueType   _valueType;
   ConsumeType _consume;
   int         _paramCount;
   std::string _dest;
   std::string _metavar;
   std::string _value;
   std::optional<bool> _flagValue;
   std::vector<std::string> _params; //any additional parameters the arg consumed
   friend class ArgParse;
};


class ArgParse {
public:
   ArgParse(){};
   void parseArgs(int argc, char** argv);
   std::string getDocString();
   std::optional<RuntimeArg*> getArg(const std::string& name);
   void defineArg(const RuntimeArg& arg);
private:
   std::string _arg0;
   std::vector<std::string> _rawArgs;
   std::map<std::string, RuntimeArg> _definedArgs;
};