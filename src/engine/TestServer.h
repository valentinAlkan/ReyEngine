#pragma once
#include <atomic>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include "ReyEngine.h"

namespace ReyEngine {
   /// Lets a test drive a running window over a local socket: press keys, move and click the mouse, wait for frames,
   /// take screenshots, and run commands the game adds itself (to report its state, say). Debug only: it listens on
   /// 127.0.0.1 and is never started unless asked (see Window::startTestServer).
   ///
   /// The protocol is plain text, one command per line, so `nc localhost <port>` works. Every command gets exactly one
   /// reply: any number of lines of output, then a line that is either "ok" or "error <why>". Commands run one after
   /// another, in order, and a command that spans frames (wait, tap, click) holds back the ones after it until it's
   /// done. Send "help" for the list.
   ///
   /// Input goes in through raylib's automation events, which write the same input state real devices do, so the
   /// engine and the game see it exactly as if it were typed or clicked. It's applied at the top of each frame, after
   /// raylib has polled the real devices and before the window reads input. A real mouse moved over the window still
   /// moves the pointer, so don't use the mouse while a test runs.
   ///
   /// A socket thread only reads lines and queues them; everything else, game commands included, runs on the game
   /// thread, inside beginFrame/endFrame.
   class TestServer {
   public:
      /// runs on the game thread. Takes the words after the command's name; returns the reply's output (may be empty).
      /// Throw to reply with an error.
      using Command = std::function<std::string(const std::vector<std::string>& args)>;
      TestServer() = default;
      ~TestServer();
      TestServer(const TestServer&) = delete;
      TestServer& operator=(const TestServer&) = delete;
      /// listen on 127.0.0.1:port. Returns false (and logs why) if it can't.
      bool start(uint16_t port);
      /// add a command, or replace one of the built-ins. The help line is shown by "help".
      void addCommand(const std::string& name, const std::string& help, Command command);
      /// called by the window at the top of each frame: run queued commands until one has to wait for a later frame
      void beginFrame(uint64_t frame, const Size<int>& windowSize);
      /// called by the window once the frame is rendered, with what it rendered: takes any screenshot asked for
      void endFrame(const RenderTarget& frameImage);
   private:
      struct Step {
         enum class Kind {LINE, INPUT, WAIT, SCREENSHOT, REPLY, DISCONNECT};
         Kind kind;
         int client = -1;           //the socket it came from, and where any reply goes
         std::string text;          //LINE: the command; SCREENSHOT: the path; REPLY: the output
         int event[3] = {0, 0, 0};  //INPUT: raylib automation event type and params
         uint64_t frames = 0;       //WAIT: frames to let pass
      };
      void listen();               //the socket thread
      void queue(Step step);       //from the socket thread
      /// turn a command line into the steps that carry it out, replying at the end
      void expand(const Step& line, std::deque<Step>& steps);
      void inject(const Step& step);
      void releaseAll();           //let go of every key and button a test is holding
      void reply(int client, const std::string& text);
      [[nodiscard]] std::string help() const;

      std::atomic<bool> _stop = false;
      int _listenSocket = -1;
      std::thread _thread;
      std::mutex _mutex;
      std::deque<Step> _incoming; //from the socket thread, guarded by _mutex

      // game thread only from here
      std::deque<Step> _steps;
      uint64_t _frame = 0;
      uint64_t _waitUntil = 0;
      Size<int> _windowSize;
      std::map<std::string, std::pair<std::string, Command>> _commands; //name -> help, command
      std::set<int> _heldKeys;
      std::set<int> _heldButtons;
      std::optional<Step> _screenshot; //asked for this frame
   };
}
