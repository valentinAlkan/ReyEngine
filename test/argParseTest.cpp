#include <cstdio>
#include <string>
#include <vector>
#include "ArgParse.h"

using namespace ReyEngine;

namespace {
   int failures = 0;
   void check(bool ok, const char* what){
      if (!ok) {std::fprintf(stderr, "FAILED: %s\n", what); failures++;}
   }
   void parse(ArgParse& args, std::vector<std::string> line){
      std::vector<char*> argv;
      for (auto& arg : line) argv.push_back(arg.data());
      args.parseArgs((int)argv.size(), argv.data());
   }
}

int main(){
   { //an arg given more than once collects all of its parameters
      ArgParse args;
      args.defineArg(RuntimeArg("--set", "doc"));
      parse(args, {"exe", "--set", "a=1", "--set", "b=2"});
      auto arg = args.getArg("set");
      check(arg && arg.value()->getParams() == std::vector<std::string>{"a=1", "b=2"}, "repeated arg");
   }
   { //a copy keeps the parameters
      ArgParse args;
      args.defineArg(RuntimeArg("--set", "doc"));
      parse(args, {"exe", "--set", "a=1"});
      RuntimeArg copy(*args.getArg("set").value());
      check(copy.getParams() == std::vector<std::string>{"a=1"}, "copied params");
   }
   { //a parameter can be spelled the same as argv[0]
      ArgParse args;
      args.defineArg(RuntimeArg("--name", "doc"));
      parse(args, {"exe", "--name", "exe"});
      auto arg = args.getArg("name");
      check(arg && arg.value()->getParams() == std::vector<std::string>{"exe"}, "parameter equal to argv[0]");
   }
   { //args that weren't given aren't there, and flags take no parameters
      ArgParse args;
      args.defineArg(RuntimeArg("--unused", "doc"));
      args.defineArg(RuntimeArg("--flag", "doc", 0));
      args.defineArg(RuntimeArg("--count", "doc"));
      parse(args, {"exe", "--flag", "--count", "3"});
      check(!args.getArg("unused"), "arg not given");
      check(args.getArg("flag") && args.getArg("flag").value()->getParams().empty(), "flag");
      check(args.getArg("count") && args.getArg("count").value()->getParams().size() == 1, "arg after a flag");
   }
   { //the help shows names as they were defined
      ArgParse args;
      args.defineArg(RuntimeArg("--test-port", "the port"));
      check(args.getDocString().find("--test-port : the port") != std::string::npos, "doc string");
      check(args.getArg("test-port") == std::nullopt, "lookup of an arg not given");
   }
   if (!failures) std::printf("ok\n");
   return failures != 0;
}
