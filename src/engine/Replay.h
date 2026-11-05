#pragma once
#include <array>
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
   /// The game can add notes too: values written whenever they change, like checks, but only for reading afterwards
   /// (how long frames took, say), never compared.
   ///
   /// A playback can run faster or slower than it was recorded (F6 and F5 while it plays, or setSpeed): more or fewer
   /// frames a second, each with its recorded time step, so it plays out just the same. Past the window's own frame
   /// rate, vsync is turned off while it plays, and the scene is drawn only on some frames (see shouldRender): drawing
   /// is often what takes the time, and nothing the game does depends on it.
   ///
   /// A playback can be held at a frame: from there, frames go by with no time passing and the input held as it was,
   /// so everything can be looked at (over the test port, say) until it's run on again. F7 holds it where it is, or runs
   /// it on.
   ///
   /// The file is text, a line a frame: the frame's number and time step, then what changed: "s<w>x<h>" the window's
   /// size, "m<x>,<y>" where the mouse went, "k+<key>" and "k-<key>" a key going down and up (raylib's key codes),
   /// "b+<n>" and "b-<n>" a mouse button, "w<x>,<y>" the wheel, "t<c>,<c>..." characters typed (Unicode codepoints, in
   /// order), "c<name>=<value>" a check's new value, and "n<name>=<value>" a note's. The first
   /// line is "reyreplay <version>". It's written as it goes, so a crash leaves all but the last few frames.
   ///
   /// Play a replay back on a build of the same code, for the same platform: another compiler can round floats
   /// differently, and the run drifts (the checks say where).
   ///
   /// Keys go in by whether they're held, so a key pressed and let go between two frames isn't recorded, and the mouse
   /// goes back in whole pixels (raylib's automation events have no fractions). Typed characters go in as they were
   /// typed (see InputManager::injectChar); while playing back, those typed for real are dropped.
   class Replay {
   public:
      static constexpr int VERSION = 1;
      static constexpr int FLUSH_FRAMES = 30; //written out at least this often while recording
      /// the speeds F5 and F6 step through, as many times as fast as recorded; 0: as fast as it will go
      static constexpr std::array<float, 7> SPEEDS = {0.25f, 0.5f, 1, 2, 4, 8, 0};
      /// a value the game wants to be the same when played back. No spaces.
      using Check = std::function<std::string()>;
      /// for a window that runs at this many frames a second (0: as fast as it can)
      explicit Replay(int windowFps): _windowFps(windowFps){}
      ~Replay();
      Replay(const Replay&) = delete;
      Replay& operator=(const Replay&) = delete;
      /// start recording to this file, replacing it. Returns false (and logs why) if it can't be written.
      bool record(const std::filesystem::path& path);
      /// the same, but from the start of the next frame, so it can be called part way through one: nothing of this one is
      /// written to the file
      void recordNextFrame(const std::filesystem::path& path){_recordNext = path;}
      /// load this file and start playing it back. Returns false (and logs why) if it can't be read.
      bool play(const std::filesystem::path& path);
      /// stop recording (closing the file) or playing back
      void stop();
      [[nodiscard]] bool isRecording() const {return _file.is_open();}
      [[nodiscard]] bool isPlaying() const {return _playing;}
      /// add a check, written while recording and compared while playing back
      void addCheck(const std::string& name, Check check);
      /// add a note, written while recording (whenever it changes) and ignored while playing back. No spaces.
      void addNote(const std::string& name, Check note);
      /// while recording: write out all that's been recorded, and copy the file here. Returns false (and logs why) if
      /// it can't.
      bool saveCopy(const std::filesystem::path& path);
      [[nodiscard]] const std::filesystem::path& getPath() const {return _path;}

      /// how fast to play back, as many times as fast as recorded (0: as fast as it will go). Takes effect while playing.
      void setSpeed(float speed);
      [[nodiscard]] float getSpeed() const {return _speed;}
      /// the next speed up or down in SPEEDS
      void faster();
      void slower();
      /// while playing back: stop before this frame, until run (or holdAt a later one)
      void holdAt(uint64_t frame){_holdAt = frame;}
      void run(){_holdAt.reset();}
      /// the frame being recorded or played next
      [[nodiscard]] uint64_t getFrame() const {return _frame;}
      /// what it's doing, in a line: recording, playing (and how far), held, finished; and where it first differed
      [[nodiscard]] std::string status() const;
      /// "replay": status, hold, step, run, speed, save (see help)
      void addTestCommands(TestServer& server);

      /// called by the window at the top of each frame, with its size. Returns the size to use for it: the recorded
      /// one, while playing back.
      Size<float> beginFrame(const Size<float>& windowSize);
      /// called by the window once raylib (and any test) has put in this frame's input: records it, or replaces it with
      /// the recorded input
      void input();
      /// called by the window once the frame's logic is done: the checks
      void endFrame();
      /// whether the window should draw the scene this frame: always, unless playing back fast. At n times as fast,
      /// once every n frames, so the frames drawn come as often as the window's frame rate (if drawing keeps up); as fast
      /// as it will go, RENDER_SECONDS apart (by the clock: it's only for show)
      [[nodiscard]] bool shouldRender();
      static constexpr double RENDER_SECONDS = 0.1;
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
         std::vector<int> chars; //typed
         std::vector<std::pair<std::string, std::string>> checks;
      };
      [[nodiscard]] bool holding() const {return _playing && _holdAt && _frame >= _holdAt.value();}
      /// make raylib's input what the state says
      void apply(const State& state, const Vec2<float>& wheel);
      void finish();
      /// set the frame rate (and vsync) for the speed while playing back, or put the window's back
      void applySpeed();

      std::vector<std::pair<std::string, Check>> _checks;
      std::vector<std::pair<std::string, Check>> _notes;
      std::map<std::string, std::string> _noted; //each note's last value written
      std::map<std::string, std::string> _checked; //each check's last value, written or expected
      uint64_t _frame = 0;
      std::filesystem::path _path;
      std::optional<std::filesystem::path> _recordNext;
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
      int _windowFps;
      float _speed = 1;
      bool _vsyncOff = false; //turned off for playing back fast
      double _renderedAt = 0; //raylib's clock, when the scene was last drawn
      uint64_t _renderedFrame = 0; //the frame it was last drawn on
   };
}
