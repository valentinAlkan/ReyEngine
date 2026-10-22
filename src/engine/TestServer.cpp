#include "TestServer.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include "Logger.h"
#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

using namespace std;
using namespace ReyEngine;

namespace {
   // raylib's AutomationEventType is private to rcore.c, so its values are copied here. They must match the enum
   // there (thirdParty/raylib/src/rcore.c).
   constexpr int INPUT_KEY_UP = 1;
   constexpr int INPUT_KEY_DOWN = 2;
   constexpr int INPUT_MOUSE_BUTTON_UP = 5;
   constexpr int INPUT_MOUSE_BUTTON_DOWN = 6;
   constexpr int INPUT_MOUSE_POSITION = 7;
   constexpr int INPUT_MOUSE_WHEEL_MOTION = 8;
   constexpr int INPUT_TOUCH_UP = 14;
   constexpr int INPUT_TOUCH_DOWN = 15;

   constexpr int POLL_MS = 100; //how often the socket thread checks whether it should stop

   string upper(string s){
      ranges::transform(s, s.begin(), [](unsigned char c){return toupper(c);});
      return s;
   }

   int parseInt(const string& s){
      size_t used = 0;
      int value = 0;
      try {value = stoi(s, &used);} catch (const exception&) {used = 0;}
      if (used != s.size()) throw runtime_error("not a number: " + s);
      return value;
   }

   /// a key by name (W, SPACE, F1, LEFT_SHIFT...; SHIFT, CTRL and ALT mean the left ones), or by raylib key code
   int parseKey(const string& name){
      static const map<string, int> named = {
         {"SPACE", KEY_SPACE}, {"ESCAPE", KEY_ESCAPE}, {"ESC", KEY_ESCAPE}, {"ENTER", KEY_ENTER}, {"TAB", KEY_TAB},
         {"BACKSPACE", KEY_BACKSPACE}, {"INSERT", KEY_INSERT}, {"DELETE", KEY_DELETE},
         {"RIGHT", KEY_RIGHT}, {"LEFT", KEY_LEFT}, {"DOWN", KEY_DOWN}, {"UP", KEY_UP},
         {"PAGE_UP", KEY_PAGE_UP}, {"PAGE_DOWN", KEY_PAGE_DOWN}, {"HOME", KEY_HOME}, {"END", KEY_END},
         {"SHIFT", KEY_LEFT_SHIFT}, {"LEFT_SHIFT", KEY_LEFT_SHIFT}, {"RIGHT_SHIFT", KEY_RIGHT_SHIFT},
         {"CTRL", KEY_LEFT_CONTROL}, {"LEFT_CTRL", KEY_LEFT_CONTROL}, {"RIGHT_CTRL", KEY_RIGHT_CONTROL},
         {"ALT", KEY_LEFT_ALT}, {"LEFT_ALT", KEY_LEFT_ALT}, {"RIGHT_ALT", KEY_RIGHT_ALT},
         {"MINUS", KEY_MINUS}, {"EQUAL", KEY_EQUAL}, {"COMMA", KEY_COMMA}, {"PERIOD", KEY_PERIOD}, {"SLASH", KEY_SLASH},
         {"GRAVE", KEY_GRAVE},
      };
      auto key = upper(name);
      if (key.size() == 1 && isalnum((unsigned char)key[0])) return key[0]; //raylib's codes for letters and digits are their ASCII
      if (auto it = named.find(key); it != named.end()) return it->second;
      if (key.size() >= 2 && key[0] == 'F' && isdigit((unsigned char)key[1])) {
         int n = parseInt(key.substr(1));
         if (n >= 1 && n <= 12) return KEY_F1 + n - 1;
      }
      if (isdigit((unsigned char)key[0])) return parseInt(key);
      throw runtime_error("unknown key: " + name);
   }

   int parseButton(const string& name){
      auto button = upper(name);
      if (button == "LEFT") return MOUSE_BUTTON_LEFT;
      if (button == "RIGHT") return MOUSE_BUTTON_RIGHT;
      if (button == "MIDDLE") return MOUSE_BUTTON_MIDDLE;
      throw runtime_error("unknown mouse button: " + name);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
TestServer::~TestServer() {
   _stop = true;
   if (_thread.joinable()) _thread.join();
#ifndef _WIN32
   if (_listenSocket >= 0) close(_listenSocket);
   // sockets of clients still connected are closed when their disconnect is handled, which won't happen now
   set<int> clients;
   for (auto& step : _steps) if (step.client >= 0) clients.insert(step.client);
   for (auto& step : _incoming) if (step.client >= 0) clients.insert(step.client);
   for (int client : clients) close(client);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
bool TestServer::start(uint16_t port) {
#ifdef _WIN32
   Logger::error() << "TestServer: not supported on Windows yet" << endl;
   return false;
#else
   _listenSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (_listenSocket < 0) {
      Logger::error() << "TestServer: can't create a socket" << endl;
      return false;
   }
   int yes = 1;
   setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
   sockaddr_in address{};
   address.sin_family = AF_INET;
   address.sin_port = htons(port);
   address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); //local only
   if (bind(_listenSocket, (sockaddr*)&address, sizeof(address)) < 0 || ::listen(_listenSocket, 1) < 0) {
      Logger::error() << "TestServer: can't listen on 127.0.0.1:" << port << endl;
      close(_listenSocket);
      _listenSocket = -1;
      return false;
   }
   _thread = thread([this]{listen();});
   Logger::info() << "TestServer: listening on 127.0.0.1:" << port << endl;
   return true;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
void TestServer::addCommand(const string& name, const string& help, Command command) {
   _commands[name] = {help, std::move(command)};
}

/////////////////////////////////////////////////////////////////////////////////////////
void TestServer::listen() {
#ifndef _WIN32
   // one client at a time. Each line becomes a step; the game thread does the rest, including closing the client's
   // socket once it has handled everything the client sent.
   while (!_stop) {
      pollfd listening{_listenSocket, POLLIN, 0};
      if (poll(&listening, 1, POLL_MS) <= 0) continue;
      int client = accept(_listenSocket, nullptr, nullptr);
      if (client < 0) continue;
      string buffer;
      char chunk[4096];
      while (!_stop) {
         pollfd reading{client, POLLIN, 0};
         if (poll(&reading, 1, POLL_MS) <= 0) continue;
         auto got = recv(client, chunk, sizeof(chunk), 0);
         if (got <= 0) break; //closed, or gone
         buffer.append(chunk, got);
         for (size_t end; (end = buffer.find('\n')) != string::npos; buffer.erase(0, end + 1)) {
            auto line = buffer.substr(0, end);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            queue(Step{Step::Kind::LINE, client, line});
         }
      }
      queue(Step{Step::Kind::DISCONNECT, client});
   }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
void TestServer::queue(Step step) {
   lock_guard lock(_mutex);
   _incoming.push_back(std::move(step));
}

/////////////////////////////////////////////////////////////////////////////////////////
void TestServer::beginFrame(uint64_t frame, const Size<int>& windowSize) {
   _frame = frame;
   _windowSize = windowSize;
   {
      lock_guard lock(_mutex);
      while (!_incoming.empty()) {
         _steps.push_back(std::move(_incoming.front()));
         _incoming.pop_front();
      }
   }
   while (!_steps.empty() && _frame >= _waitUntil && !_screenshot) {
      auto step = std::move(_steps.front());
      _steps.pop_front();
      switch (step.kind) {
         case Step::Kind::LINE: {
            deque<Step> expanded;
            try {
               expand(step, expanded);
            } catch (const exception& e) {
               reply(step.client, string("error ") + e.what() + "\n");
               break;
            }
            _steps.insert(_steps.begin(), expanded.begin(), expanded.end());
            break;
         }
         case Step::Kind::INPUT: inject(step); break;
         case Step::Kind::WAIT: _waitUntil = _frame + step.frames; break;
         case Step::Kind::SCREENSHOT: _screenshot = std::move(step); break; //taken, and replied to, in endFrame
         case Step::Kind::REPLY: reply(step.client, step.text); break;
         case Step::Kind::DISCONNECT:
            releaseAll();
#ifndef _WIN32
            close(step.client);
#endif
            break;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void TestServer::endFrame(const RenderTarget& frameImage) {
   if (!_screenshot) return;
   // render textures are stored upside down
   Image image = LoadImageFromTexture(frameImage.getTexture());
   ImageFlipVertical(&image);
   bool saved = ExportImage(image, _screenshot->text.c_str());
   UnloadImage(image);
   reply(_screenshot->client, saved ? "ok\n" : "error can't write " + _screenshot->text + "\n");
   _screenshot.reset();
}

/////////////////////////////////////////////////////////////////////////////////////////
void TestServer::expand(const Step& line, deque<Step>& steps) {
   istringstream words(line.text);
   vector<string> args;
   for (string word; words >> word;) args.push_back(word);
   if (args.empty()) return; //blank lines get no reply
   auto name = args[0];
   args.erase(args.begin());
   int client = line.client;
   auto input = [&](int type, int a = 0, int b = 0){
      Step step{Step::Kind::INPUT, client};
      step.event[0] = type; step.event[1] = a; step.event[2] = b;
      steps.push_back(step);
   };
   auto wait = [&](uint64_t frames){
      Step step{Step::Kind::WAIT, client};
      step.frames = frames;
      steps.push_back(step);
   };
   auto done = [&](string output = ""){
      if (!output.empty() && output.back() != '\n') output += '\n';
      steps.push_back(Step{Step::Kind::REPLY, client, output + "ok\n"});
   };
   auto need = [&](size_t count, const char* usage){
      if (args.size() != count) throw runtime_error(string("usage: ") + usage);
   };

   // game commands come first, so a game can replace a built-in
   if (auto it = _commands.find(name); it != _commands.end()) {
      done(it->second.second(args));
   } else if (name == "help") {
      done(help());
   } else if (name == "frame") {
      done(to_string(_frame));
   } else if (name == "window") {
      done(to_string(_windowSize.x) + " " + to_string(_windowSize.y));
   } else if (name == "wait") {
      need(1, "wait <frames>");
      int frames = parseInt(args[0]);
      if (frames < 0) throw runtime_error("can't wait a negative number of frames");
      wait(frames);
      done();
   } else if (name == "key") {
      need(2, "key down|up|tap <key>");
      int key = parseKey(args[1]);
      auto action = args[0];
      if (action == "down") input(INPUT_KEY_DOWN, key);
      else if (action == "up") input(INPUT_KEY_UP, key);
      else if (action == "tap") {
         // held for a frame, so code that checks whether a key is down each frame sees it
         input(INPUT_KEY_DOWN, key);
         wait(1);
         input(INPUT_KEY_UP, key);
      } else throw runtime_error("usage: key down|up|tap <key>");
      done();
   } else if (name == "mouse") {
      if (args.empty()) throw runtime_error("usage: mouse move|down|up|click|wheel ...");
      auto action = args[0];
      if (action == "move") {
         need(3, "mouse move <x> <y>");
         input(INPUT_MOUSE_POSITION, parseInt(args[1]), parseInt(args[2]));
      } else if (action == "down" || action == "up" || action == "click") {
         if (args.size() != 2 && args.size() != 4) throw runtime_error("usage: mouse " + action + " left|right|middle [<x> <y>]");
         int button = parseButton(args[1]);
         if (args.size() == 4) {
            // move a frame ahead, so the window has seen the pointer arrive before the button changes
            input(INPUT_MOUSE_POSITION, parseInt(args[2]), parseInt(args[3]));
            wait(1);
         }
         if (action != "up") input(INPUT_MOUSE_BUTTON_DOWN, button);
         if (action == "click") wait(1);
         if (action != "down") input(INPUT_MOUSE_BUTTON_UP, button);
      } else if (action == "wheel") {
         need(2, "mouse wheel <notches>");
         input(INPUT_MOUSE_WHEEL_MOTION, 0, parseInt(args[1]));
      } else throw runtime_error("usage: mouse move|down|up|click|wheel ...");
      done();
   } else if (name == "release") {
      releaseAll();
      done();
   } else if (name == "screenshot") {
      need(1, "screenshot <path.png>");
      steps.push_back(Step{Step::Kind::SCREENSHOT, client, args[0]}); //replies itself
   } else {
      throw runtime_error("unknown command: " + name + " (try help)");
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void TestServer::inject(const Step& step) {
   int type = step.event[0];
   int a = step.event[1];
   switch (type) {
      case INPUT_KEY_DOWN: _heldKeys.insert(a); break;
      case INPUT_KEY_UP: _heldKeys.erase(a); break;
      case INPUT_MOUSE_BUTTON_DOWN: _heldButtons.insert(a); break;
      case INPUT_MOUSE_BUTTON_UP: _heldButtons.erase(a); break;
   }
   AutomationEvent event{};
   event.type = type;
   event.params[0] = a;
   event.params[1] = step.event[2];
   PlayAutomationEvent(event);
   // raylib counts touches as mouse buttons, and its own mouse callback sets the touch with the same number to match.
   // Without it, IsMouseButtonUp says a held button is up, and the engine sees a fresh press every frame.
   if (type == INPUT_MOUSE_BUTTON_DOWN || type == INPUT_MOUSE_BUTTON_UP) {
      event.type = type == INPUT_MOUSE_BUTTON_DOWN ? INPUT_TOUCH_DOWN : INPUT_TOUCH_UP;
      PlayAutomationEvent(event);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void TestServer::releaseAll() {
   for (int key : set(_heldKeys)) inject(Step{Step::Kind::INPUT, -1, "", {INPUT_KEY_UP, key, 0}});
   for (int button : set(_heldButtons)) inject(Step{Step::Kind::INPUT, -1, "", {INPUT_MOUSE_BUTTON_UP, button, 0}});
}

/////////////////////////////////////////////////////////////////////////////////////////
void TestServer::reply(int client, const string& text) {
#ifndef _WIN32
   // the client may have gone already; MSG_NOSIGNAL keeps that from killing the game with SIGPIPE
   send(client, text.data(), text.size(), MSG_NOSIGNAL);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
string TestServer::help() const {
   string text =
      "every command replies with any output, then \"ok\" or \"error <why>\"\n"
      "key down|up|tap <key>                  keys by name: W, SPACE, F1, SHIFT, LEFT_SHIFT, ESCAPE, UP...\n"
      "mouse move <x> <y>                     window pixels, from the top left\n"
      "mouse down|up|click left|right|middle [<x> <y>]\n"
      "mouse wheel <notches>                  positive scrolls up\n"
      "wait <frames>\n"
      "screenshot <path.png>                  what the window draws, after the current frame\n"
      "release                                let go of every key and button still held\n"
      "frame                                  the window's frame count\n"
      "window                                 the window's size in pixels\n";
   for (auto& [name, command] : _commands) text += name + " " + command.first + "\n";
   return text;
}
