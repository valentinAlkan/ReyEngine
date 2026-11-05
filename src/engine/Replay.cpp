#include "Replay.h"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <climits>
#include <format>
#include <sstream>
#include "TestServer.h"
#include "InputManager.h"

using namespace std;
using namespace ReyEngine;

namespace {
   // raylib's AutomationEventType is private to rcore.c, so its values are copied here (as in TestServer.cpp). They
   // must match the enum there (thirdParty/raylib/src/rcore.c).
   constexpr int INPUT_KEY_UP = 1;
   constexpr int INPUT_KEY_DOWN = 2;
   constexpr int INPUT_MOUSE_BUTTON_UP = 5;
   constexpr int INPUT_MOUSE_BUTTON_DOWN = 6;
   constexpr int INPUT_MOUSE_POSITION = 7;
   constexpr int INPUT_MOUSE_WHEEL_MOTION = 8;
   constexpr int INPUT_TOUCH_UP = 14;
   constexpr int INPUT_TOUCH_DOWN = 15;

   constexpr int KEYS = 512;   //raylib's MAX_KEYBOARD_KEYS
   constexpr int BUTTONS = 7;  //raylib's mouse buttons, left to back

   void play(int type, int a, int b = 0) {
      AutomationEvent event{};
      event.type = type;
      event.params[0] = a;
      event.params[1] = b;
      PlayAutomationEvent(event);
   }

   template <typename T>
   T parse(string_view text) {
      T value{};
      auto [end, error] = from_chars(text.data(), text.data() + text.size(), value);
      if (error != errc() || end != text.data() + text.size()) throw runtime_error("not a number: " + string(text));
      return value;
   }

   /// "<a><separator><b>"
   template <typename T>
   pair<T, T> parsePair(string_view text, char separator) {
      auto at = text.find(separator);
      if (at == string_view::npos) throw runtime_error("not a pair: " + string(text));
      return {parse<T>(text.substr(0, at)), parse<T>(text.substr(at + 1))};
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Replay::~Replay() {
   stop();
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Replay::record(const filesystem::path& path) {
   stop();
   error_code error;
   if (path.has_parent_path()) filesystem::create_directories(path.parent_path(), error);
   _file.open(path, ios::trunc | ios::binary); //the same file on every platform, without Windows' CRs
   if (!_file) {
      Logger::error() << "Replay: can't write " << path.string() << endl;
      return false;
   }
   _file << "reyreplay " << VERSION << "\n";
   _path = path;
   _frame = 0;
   _checked.clear();
   // nothing known yet, so the first frame writes it all
   _state = {{-1, -1}, {INT_MIN, INT_MIN}, {}, {}};
   Logger::info() << "Replay: recording to " << path.string() << endl;
   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Replay::play(const filesystem::path& path) {
   stop();
   ifstream file(path);
   if (!file) {
      Logger::error() << "Replay: can't read " << path.string() << endl;
      return false;
   }
   vector<Frame> frames;
   try {
      string line;
      // one written in text mode on Windows ends its lines with CRs
      auto next = [&]{
         if (!getline(file, line)) return false;
         if (!line.empty() && line.back() == '\r') line.pop_back();
         return true;
      };
      next();
      if (line != "reyreplay " + to_string(VERSION)) throw runtime_error("not a replay (version " + to_string(VERSION) + ")");
      while (next()) {
         if (line.empty()) continue;
         istringstream words(line);
         string word;
         words >> word;
         if (parse<uint64_t>(word) != frames.size()) throw runtime_error("frame " + word + " out of order");
         Frame frame;
         words >> word;
         frame.dt = parse<float>(word);
         while (words >> word) {
            string_view rest = string_view(word).substr(1);
            switch (word[0]) {
               case 's': {
                  auto [w, h] = parsePair<int>(rest, 'x');
                  frame.size = Size<int>(w, h);
               } break;
               case 'm': {
                  auto [x, y] = parsePair<int>(rest, ',');
                  frame.mouse = Pos<int>(x, y);
               } break;
               case 'w': {
                  auto [x, y] = parsePair<float>(rest, ',');
                  frame.wheel = {x, y};
               } break;
               case 'k': (rest[0] == '+' ? frame.keysDown : frame.keysUp).push_back(parse<int>(rest.substr(1))); break;
               case 'b': (rest[0] == '+' ? frame.buttonsDown : frame.buttonsUp).push_back(parse<int>(rest.substr(1))); break;
               case 'c': {
                  auto at = rest.find('=');
                  if (at == string_view::npos) throw runtime_error("bad check: " + word);
                  frame.checks.emplace_back(string(rest.substr(0, at)), string(rest.substr(at + 1)));
               } break;
               case 't': {
                  for (size_t at = 0; at < rest.size();) {
                     auto comma = rest.find(',', at);
                     if (comma == string_view::npos) comma = rest.size();
                     frame.chars.push_back(parse<int>(rest.substr(at, comma - at)));
                     at = comma + 1;
                  }
               } break;
               case 'n': break; //a note: only for reading
               default: throw runtime_error("don't know " + word);
            }
         }
         frames.push_back(std::move(frame));
      }
   } catch (const exception& e) {
      Logger::error() << "Replay: can't play " << path.string() << " (line " << frames.size() + 2 << "): " << e.what() << endl;
      return false;
   }
   _frames = std::move(frames);
   _path = path;
   _frame = 0;
   _checked.clear();
   _state = {};
   _playing = true;
   _finished = false;
   applySpeed();
   _diverged.reset();
   _mismatches = 0;
   // the window as it was from the start, so the first frame looks the same
   if (!_frames.empty() && _frames.front().size) SetWindowSize(_frames.front().size->x, _frames.front().size->y);
   Logger::info() << "Replay: playing " << path.string() << ", " << _frames.size() << " frames" << endl;
   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::stop() {
   if (_file.is_open()) {
      _file.close();
      Logger::info() << "Replay: recorded " << _frame << " frames to " << _path.string() << endl;
   }
   if (_playing) finish();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::addCheck(const string& name, Check check) {
   _checks.emplace_back(name, std::move(check));
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::addNote(const string& name, Check note) {
   _notes.emplace_back(name, std::move(note));
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Replay::saveCopy(const filesystem::path& path) {
   if (!isRecording()) {
      Logger::error() << "Replay: not recording, so nothing to save" << endl;
      return false;
   }
   _file.flush();
   error_code error;
   if (path.has_parent_path()) filesystem::create_directories(path.parent_path(), error);
   filesystem::copy_file(_path, path, filesystem::copy_options::overwrite_existing, error);
   if (error) {
      Logger::error() << "Replay: can't copy to " << path.string() << ": " << error.message() << endl;
      return false;
   }
   Logger::info() << "Replay: saved " << _frame << " frames to " << path.string() << endl;
   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
string Replay::status() const {
   string text;
   if (isRecording()) text = format("recording frame {} to {}", _frame, _path.string());
   else if (holding()) text = format("held at frame {} of {}", _frame, _frames.size());
   else if (_playing) text = format("playing frame {} of {}", _frame, _frames.size());
   else if (_finished) text = format("finished, {} frames", _frames.size());
   else return "idle";
   if (_playing || _finished) {
      if (_playing && _speed != 1) text += _speed == 0 ? " at full speed" : format(" at {}x", _speed);
      if (_diverged) text += format("; differs from frame {} ({}), on {} frame{}", _diverged.value(), _divergence, _mismatches, _mismatches == 1 ? "" : "s");
      else text += "; checks match";
   }
   return text;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::addTestCommands(TestServer& server) {
   server.addCommand("replay", "replay [status | hold <frame> | step [frames] | run | speed <times|max> | save <path>]: the replay being recorded or played",
      [this](const vector<string>& args) -> string {
         string what = args.empty() ? "status" : args[0];
         if (what == "hold" && args.size() == 2) holdAt(parse<uint64_t>(args[1]));
         else if (what == "step") holdAt(_frame + (args.size() == 2 ? parse<uint64_t>(args[1]) : 1));
         else if (what == "run") run();
         else if (what == "speed" && args.size() == 2) setSpeed(args[1] == "max" ? 0 : parse<float>(args[1]));
         else if (what == "save" && args.size() == 2) {
            if (!saveCopy(args[1])) throw runtime_error("can't save to " + args[1]);
         } else if (what != "status") throw runtime_error("replay status | hold <frame> | step [frames] | run | speed <times|max> | save <path>");
         return status() + "\n";
      });
}

/////////////////////////////////////////////////////////////////////////////////////////
Size<float> Replay::beginFrame(const Size<float>& windowSize) {
   if (_recordNext) {
      record(*_recordNext);
      _recordNext.reset();
   }
   if (isRecording()) {
      _line = format("{} {}", _frame, GetFrameTime());
      Size<int> size((int)windowSize.x, (int)windowSize.y);
      if (size != _state.size) {
         _line += format(" s{}x{}", size.x, size.y);
         _state.size = size;
      }
      return windowSize;
   }
   if (!_playing) return windowSize;
   if (holding()) {
      Internal::frameDeltaOverridden = true;
      Internal::frameDeltaOverride = 0;
      return {(float)_state.size.x, (float)_state.size.y};
   }
   if (_frame >= _frames.size()) {
      finish();
      return windowSize;
   }
   // the window sets its own frame rate as it starts, after play was called
   if (_frame == 0) applySpeed();
   auto& frame = _frames[_frame];
   Internal::frameDeltaOverridden = true;
   Internal::frameDeltaOverride = frame.dt;
   if (frame.size) _state.size = frame.size.value();
   return {(float)_state.size.x, (float)_state.size.y};
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::input() {
   if (isRecording()) {
      for (int key = 1; key < KEYS; key++) {
         bool down = IsKeyDown(key);
         if (down == _state.keys.contains(key)) continue;
         _line += format(" k{}{}", down ? '+' : '-', key);
         if (down) _state.keys.insert(key);
         else _state.keys.erase(key);
      }
      for (int button = 0; button < BUTTONS; button++) {
         bool down = IsMouseButtonDown(button);
         if (down == _state.buttons.contains(button)) continue;
         _line += format(" b{}{}", down ? '+' : '-', button);
         if (down) _state.buttons.insert(button);
         else _state.buttons.erase(button);
      }
      auto position = GetMousePosition();
      Pos<int> mouse((int)position.x, (int)position.y);
      if (mouse != _state.mouse) {
         _line += format(" m{},{}", mouse.x, mouse.y);
         _state.mouse = mouse;
      }
      auto wheel = GetMouseWheelMoveV();
      if (wheel.x != 0 || wheel.y != 0) _line += format(" w{},{}", wheel.x, wheel.y);
      // what was typed: written down, then put back for the window to take
      auto typed = InputManager::takeTypedChars();
      for (size_t i = 0; i < typed.size(); i++) _line += format("{}{}", i == 0 ? " t" : ",", typed[i]);
      for (int codepoint : typed) InputManager::injectChar(codepoint);
      return;
   }
   if (!_playing) return;
   // the real keys, before the recording's replace them: just pressed (down now, but not as last left)
   if (IsKeyPressed(KEY_F6)) faster();
   if (IsKeyPressed(KEY_F5)) slower();
   if (IsKeyPressed(KEY_F7)) {
      if (holding()) run();
      else holdAt(_frame);
   }
   // what's typed for real is dropped: the recording's typing stands in for it
   InputManager::takeTypedChars();
   if (holding()) {
      apply(_state, {});
      return;
   }
   auto& frame = _frames[_frame];
   for (int codepoint : frame.chars) InputManager::injectChar(codepoint);
   for (int key : frame.keysDown) _state.keys.insert(key);
   for (int key : frame.keysUp) _state.keys.erase(key);
   for (int button : frame.buttonsDown) _state.buttons.insert(button);
   for (int button : frame.buttonsUp) _state.buttons.erase(button);
   if (frame.mouse) _state.mouse = frame.mouse.value();
   apply(_state, frame.wheel);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::endFrame() {
   if (isRecording()) {
      for (auto& [name, check] : _checks) {
         auto value = check();
         auto& last = _checked[name];
         if (value == last) continue;
         _line += format(" c{}={}", name, value);
         last = value;
      }
      for (auto& [name, note] : _notes) {
         auto value = note();
         auto& last = _noted[name];
         if (value == last) continue;
         _line += format(" n{}={}", name, value);
         last = value;
      }
      _file << _line << "\n";
      if (_frame % FLUSH_FRAMES == 0) _file.flush();
      _frame++;
      return;
   }
   if (!_playing || holding()) return;
   for (auto& [name, value] : _frames[_frame].checks) _checked[name] = value;
   bool differs = false;
   for (auto& [name, check] : _checks) {
      auto expected = _checked.find(name);
      if (expected == _checked.end()) continue;
      auto value = check();
      if (value == expected->second) continue;
      if (!_diverged) {
         _diverged = _frame;
         _divergence = format("{} was {}, now {}", name, expected->second, value);
         Logger::warn() << "Replay: differs from the recording at frame " << _frame << ": " << name << " was " << expected->second << ", now " << value << endl;
      }
      differs = true;
   }
   if (differs) _mismatches++;
   _frame++;
   // the window's next size, asked for now so it's there when the frame starts (the canvas gets it regardless)
   if (_frame < _frames.size() && _frames[_frame].size) SetWindowSize(_frames[_frame].size->x, _frames[_frame].size->y);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::apply(const State& state, const Vec2<float>& wheel) {
   for (int key = 1; key < KEYS; key++) {
      bool down = state.keys.contains(key);
      if (IsKeyDown(key) != down) ::play(down ? INPUT_KEY_DOWN : INPUT_KEY_UP, key);
   }
   for (int button = 0; button < BUTTONS; button++) {
      bool down = state.buttons.contains(button);
      if (IsMouseButtonDown(button) == down) continue;
      ::play(down ? INPUT_MOUSE_BUTTON_DOWN : INPUT_MOUSE_BUTTON_UP, button);
      // raylib counts touches as mouse buttons too (see TestServer::inject)
      ::play(down ? INPUT_TOUCH_DOWN : INPUT_TOUCH_UP, button);
   }
   ::play(INPUT_MOUSE_POSITION, state.mouse.x, state.mouse.y);
   ::play(INPUT_MOUSE_WHEEL_MOTION, (int)wheel.x, (int)wheel.y);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::finish() {
   // let go of everything the recording was holding, and give time back to the clock
   apply({_state.size, _state.mouse, {}, {}}, {});
   Internal::frameDeltaOverridden = false;
   _playing = false;
   _finished = true;
   applySpeed();
   Logger::info() << "Replay: finished at frame " << _frame << " (" << status() << ")" << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::setSpeed(float speed) {
   _speed = std::max(speed, 0.f);
   applySpeed();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::faster() {
   // full speed (0) is the last, and stays
   auto at = ranges::find(SPEEDS, _speed);
   if (at != SPEEDS.end() && at + 1 != SPEEDS.end()) setSpeed(*(at + 1));
   else if (at == SPEEDS.end()) setSpeed(*ranges::find_if(SPEEDS, [this](float s){return s == 0 || s > _speed;}));
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::slower() {
   auto at = ranges::find(SPEEDS, _speed);
   if (at == SPEEDS.end()) at = ranges::find_if(SPEEDS, [this](float s){return s == 0 || s > _speed;});
   if (at != SPEEDS.begin()) setSpeed(*(at - 1));
}

/////////////////////////////////////////////////////////////////////////////////////////
void Replay::applySpeed() {
   float speed = _playing ? _speed : 1;
   // the frame rate it would take; 0 for no limit
   int fps = speed == 0 || _windowFps == 0 ? 0 : std::max(1, (int)std::lround(_windowFps * speed));
   bool fast = speed == 0 || speed > 1;
   if (fast && !_vsyncOff && IsWindowState(FLAG_VSYNC_HINT)) {
      ClearWindowState(FLAG_VSYNC_HINT);
      _vsyncOff = true;
   } else if (!fast && _vsyncOff) {
      SetWindowState(FLAG_VSYNC_HINT);
      _vsyncOff = false;
   }
   SetTargetFPS(fps);
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Replay::shouldRender() {
   bool fast = _playing && !holding() && (_speed == 0 || _speed > 1);
   double now = GetTime();
   if (fast && _speed == 0 && now - _renderedAt < RENDER_SECONDS) return false;
   if (fast && _speed > 1 && _frame < _renderedFrame + (uint64_t)std::ceil(_speed)) return false;
   _renderedAt = now;
   _renderedFrame = _frame;
   return true;
}
