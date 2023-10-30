#include "ArgParse.h"
#include "Application.h"
#include "StringTools.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
RuntimeArg::RuntimeArg(const std::string &arg, std::string docString, int paramCount, ArgType argType,
                       ValueType valueType, ConsumeType consumeType, std::string dest, std::string metavar)
: _rawValue(arg)
, _sanitizedName(sanitizeName(arg))
, _paramCount(paramCount)
, _docString(docString)
, _argType(argType)
, _valueType(valueType)
, _consume(consumeType)
, _invoked(false)
{
   _value = "";
   if (argType == ArgType::DEDUCE){
      _argType = RuntimeArg::ArgType::POSITIONAL;
      //determine type of arg
      auto optFlag = RuntimeArg::flagParse(_rawValue);
      if (optFlag){
         _argType = RuntimeArg::ArgType::FLAG;
      }
   }


   _dest = dest.empty() ? _sanitizedName : dest;
   _metavar = metavar.empty() ? _sanitizedName : metavar;
}

/////////////////////////////////////////////////////////////////////////////////////////
RuntimeArg::RuntimeArg(const RuntimeArg& other)
{
   _rawValue = other._rawValue;
   _sanitizedName = other._sanitizedName;
   _docString = other._docString;
   _argType = other._argType;
   _valueType = other._valueType;
   _consume = other._consume;
   _paramCount = other._paramCount;
   _dest = other._dest;
   _metavar = other._metavar;
   _value = other._value;
   _invoked = other._invoked;
   for (const auto& param : _params){
      _params.push_back(param);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string RuntimeArg::sanitizeName(const std::string& arg){
   string sanitizedName;
   if (arg.empty()) return "";
    if (::isdigit(arg.front())){
      Application::printError() << "Invalid argument name: " << arg << endl;
      Application::exitError("Arg names cannot start with numbers!", Application::ExitReason::INVALID_ARGS);
   }

   string t = string_tools::lstrip(arg, '-');
   for (const auto& c : t){
      if (!::isalnum(c)){
         sanitizedName.push_back('_');
      } else {
         sanitizedName.push_back(c);
      }
   }
   return sanitizedName;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RuntimeArg::setValue(const std::string &value) {
   _rawValue = value;
   _invoked = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ArgParse::defineArg(const RuntimeArg& arg) {
   //make sure the arg doesn't already exist
   auto found = _definedArgs.find(arg.name());
   if (found != _definedArgs.end()){
      stringstream msg;
      msg << "Arg name " << arg._rawValue << " already defined as " << arg.name() << "!" << endl;
      Application::printError() << msg.str() << endl;
      throw std::runtime_error(msg.str());
   }
   auto pair = make_pair(arg.name(), arg);
   _definedArgs.insert(pair);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ArgParse::parseArgs(int argc, char **argv) {
   if (argc == 0){
      Application::printError() << "No first arg! por que????" << endl;
      exit(1);
   }
   _arg0 = argv[0];
   _argc = argc;
   if (argc >= SIZE_MAX) exit(1);
   for (size_t i=0; i<argc; i++){
      _rawArgs.emplace_back(argv[i]);
   }

   RuntimeArg* openArg = nullptr;
   size_t openParamCount = 0;
   for (auto rawArg: _rawArgs){
      if (rawArg == _arg0){
         continue;
      }
      RuntimeArg::ArgType argType = RuntimeArg::ArgType::POSITIONAL;
      //determine type of arg
      auto optFlag = RuntimeArg::flagParse(rawArg);
      if (optFlag){
         argType = RuntimeArg::ArgType::FLAG;
      }

      //parse the next arg as a parameter if we have an open arg, or close an open arg where appropriate
      bool isFlag = argType == RuntimeArg::ArgType::FLAG;
      if (openArg) {
         if (openArg->_paramCount > openParamCount){
            if (!isFlag) {
               //parse paramaters of an existing arg
               openArg->_params.push_back(rawArg);
               openParamCount++;
               continue;
            } else {
               //raise error
               stringstream msg;
               msg << "Expected parameter #" << openParamCount << " to arg \"" << openArg->name() << "\" but got flag \"" << rawArg << "\" instead!" << endl;
               Application::printError() << msg.str() << endl;
               exit(1);
            }
         } else {
            //close the arg
            openArg = nullptr;
         }
      }

      //parse a new arg
      auto saniName = RuntimeArg::sanitizeName(rawArg);
      //try to find the corresponding arg in the _definedArgs map
      auto it = _definedArgs.find(saniName);
      if (it == _definedArgs.end()) {
         //raise error
         stringstream msg;
         msg << "Unrecognized " << (argType == RuntimeArg::ArgType::FLAG ? "flag" : "argument") << " \""
             << rawArg << "\" using name \"" << saniName << "\"";
         Application::printError() << msg.str() << endl;
         exit(1);
      }
      openArg = &it->second;
      openArg->setValue(rawArg);
      openParamCount = 0;
   }
   if (openParamCount != openArg->_paramCount) throw std::runtime_error("Arg count mismatch!");
}
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::string> RuntimeArg::flagParse(std::string rawArg) {
   string tArg = rawArg;
   //make sure the length requirements are met
   if (
         (rawArg.find("--") == 0 && rawArg.length() > 3) ||
         (rawArg.find('-') == 0 && rawArg.length() == 2)
         ) {
      //remove leading dashes
      while (!tArg.empty() && tArg.front() == '-') {
         tArg = tArg.substr(1, tArg.size() - 1);
      }
      return tArg;
   }
   return nullopt;
}
/////////////////////////////////////////////////////////////////////////////////////////
optional<RuntimeArg*> ArgParse::getArg(const std::string& name) {
   auto it = _definedArgs.find(RuntimeArg::sanitizeName(name));
   if (it == _definedArgs.end())
   {
      return nullopt;
   }
   if (!it->second._invoked){
      return nullopt;
   }
   return &it->second;
}