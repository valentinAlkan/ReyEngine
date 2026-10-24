#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include "ReyEngine.h"

namespace ReyEngine {
   class TestServer;
   /// Records what a window takes in, frame by frame, so a run can be played back exactly: each frame's time step,
   /// the window's size, the mouse (where it is, its buttons and wheel) and the keys held. Played back, each frame gets
   /// the same input and the same time step it had, so anything driven only by those (and not by the clock, or by
   /// threads racing) does just what it did. The window owns one (see Window::getReplay); it does nothing unless asked
   /// to record or play.
   ///
   /// The game can add checks: values (a checksum of its state, say) that are written whenever they change, and
   /// compared while playing back. The first frame where one differs is logged, and given by status: from there on,
   /// the playback has gone its own way.
   ///
   /// A playback can be held at a frame: from there, frames go by with no time passing and the input held as it was,
   /// so everything can be looked at (over the test port, say) until it's run on again.
   ///
   /// The file is text, a line a frame: the frame's number and time step, then what changed: "s<w>x<h>" the window's
   /// size, "m<x>,<y>" where the mouse went, "k+<key>" and "k-<key>" a key going down and up (raylib's key codes),
   /// "b+<n>" and "b-<n>" a mouse button, "w<x>,<y>" the wheel, and "c<name>=<value>" a check's new value. The first
   /// line is "reyreplay <version>". It's written as it goes, so a crash leaves all but the last few frames.
   ///
   /// Play a replay back on a build of the same code, for the same platform: another compiler can round floats
   /// differently, and the run drifts (the checks say where).
   ///
   /// Keys go in by whether they're held, so a key pressed and let go between two frames isn't recorded, and the mouse
   /// goes back in whole pixels (raylib's automation events have no fractions). Typed characters aren't recorded.
   class Replay {
   public:
      static constexpr int VERSION = 1;
      static constexpr int FLUSH_FRAMES = 30; //written out at least this often while recording
      /// a value the game wants to be the same when played back. No spaces.
      using Check = std::function<std::string()>;
      Replay() = default;
      ~Replay();
      Replay(const Replay&) = delete;
      Replay& operator=(const Replay&) = delete;
      /// start recording to this file, replacing it. Returns false (and logs why) if it can't be written.
      bool record(const std::filesystem::path& path);
      /// load this file and start playing it back. Returns false (and logs why) if it can't be read.
      bool play(const std::filesystem::path& path);
      /// stop recording (closing the file) or playing back
      void stop();
      [[nodiscard]] bool isRecording() const {return _file.is_open();}
      [[nodiscard]] bool isPlaying() const {return _playing;}
      /// add a check, written while recording and compared while playing back
      void addCheck(const std::string& name, Check check);
      /// while recording: write out all that's been recorded, and copy the file here. Returns false (and logs why) if
      /// it can't.
      bool saveCopy(const std::filesystem::path& path);
      [[nodiscard]] const std::filesystem::path& getPath() const {return _path;}

      /// while playing back: stop before this frame, until run (or holdAt a later one)
      void holdAt(uint64_t frame){_holdAt = frame;}
      void run(){_holdAt.reset();}
      /// the frame being recorded or played next
      [[nodiscard]] uint64_t getFrame() const {return _frame;}
      /// what it's doing, in a line: recording, playing (and how far), held, finished; and where it first differed
      [[nodiscard]] std::string status() const;
      /// "replay": status, hold, step, run, save (see help)
      void addTestCommands(TestServer& server);

      /// called by the window at the top of each frame, with its size. Returns the size to use for it: the recorded
      /// one, while playing back.
      Size<float> beginFrame(const Size<float>& windowSize);
      /// called by the window once raylib (and any test) has put in this frame's input: records it, or replaces it with
      /// the recorded input
      void input();
      /// called by the window once the frame's logic is done: the checks
      void endFrame();
   private:
      /// input as it stands
      struct State {
         Size<int> size;
         Pos<int> mouse;
         std::set<int> keys;
         std::set<int> buttons;
      };
      /// one frame of a file being played back
      struct Frame {
         float dt = 0;
         std::optional<Size<int>> size;
         std::optional<Pos<int>> mouse;
         Vec2<float> wheel;
         std::vector<int> keysDown, keysUp, buttonsDown, buttonsUp;
         std::vector<std::pair<std::string, std::string>> checks;
      };
      [[nodiscard]] bool holding() const {return _playing && _holdAt && _frame >= _holdAt.value();}
      /// make raylib's input what the state says
      void apply(const State& state, const Vec2<float>& wheel);
      void finish();

      std::vector<std::pair<std::string, Check>> _checks;
      std::map<std::string, std::string> _checked; //each check's last value, written or expected
      uint64_t _frame = 0;
      std::filesystem::path _path;
      State _state; //recording: as last written; playing back: as the recording has it

      // recording
      std::ofstream _file;
      std::string _line; //this frame's, written at its end

      // playing back
      bool _playing = false;
      bool _finished = false;
      std::vector<Frame> _frames;
      std::optional<uint64_t> _holdAt;
      std::optional<uint64_t> _diverged; //the first frame a check differed on
      std::string _divergence;            //which, and how
      uint64_t _mismatches = 0; //frames where a check differed
   };
}
