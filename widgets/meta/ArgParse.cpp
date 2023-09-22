#include "ArgParse.h"
#include "Application.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
RuntimeArg::RuntimeArg(const std::string &arg, std::string docString, ArgType argType, ValueType valueType,
                       ConsumeType consumeType, int varCount, std::string dest, std::string metavar)
: _rawValue(arg)
, _docString(docString)
, _argType(argType)
, _valueType(valueType)
, _consume(consumeType)
, _varCount(varCount)
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


   _dest = dest.empty() ? arg : dest;
   _metavar = metavar.empty() ? arg : metavar;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string RuntimeArg::sanitizeName(const std::string& arg) {
   string sanitizedName;
   if (arg.empty()) return "";
   if (::isdigit(arg.front())){
      throw std::runtime_error("Arg names cannot start with numbers!");
   }
   for (const auto& c : arg){
      if (!::isalnum(c)){
         sanitizedName.push_back('_');
      }
   }
}


/////////////////////////////////////////////////////////////////////////////////////////
void ArgParse::defineArg(RuntimeArg arg) {

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

   bool openArg = false; //whether we are currently parsing a multi-value arg or not
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

      //try to find the corresponding arg in the _definedArgs map
      auto optArg = getArg(rawArg);
      if (!optArg){
         //raise error
         stringstream msg;
         msg << "Unrecognized " << (argType == RuntimeArg::ArgType::FLAG ? "flag" : "positional argument") << " <" << rawArg << ">";
         Application::printError() << msg.str() << endl;
         exit(1);
      }
   }

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
optional<RuntimeArg> ArgParse::getArg(const std::string& name) {
   auto it = _definedArgs.find(name);
   if (it == _definedArgs.end())
   {
      return nullopt;
   }
   return it->second;
}