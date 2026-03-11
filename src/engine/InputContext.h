// #pragma once
// #include "Event.h"
//
// namespace ReyEngine {
//     class ContextHandler;
//     class Widget;
//     class InputEvent;
//     class InputContext {
//     public:
//         std::pair<std::unique_ptr<InputContext>, std::unique_ptr<ContextHandler>> makeContextHandler(Widget* handler) {
//             auto inputContext = std::unique_ptr<InputContext>(new InputContext(handler));
//             auto contextHandler = std::unique_ptr<ContextHandler>(inputContext.get());
//             return {inputContext, contextHandler};
//         }
//         virtual ~InputContext() = default;
//         virtual Widget* handleInput(const InputEvent& event);
//     protected:
//         InputContext(Widget* handler)
//         : _handler(handler)
//         {}
//         Widget* _handler;
//     friend class ContextHandler;
//     };
//
//     class ContextHandler {
//     public:
//         std::unique_ptr<ContextHandler> makeContextHandler(InputContext* handler) {
//             return std::unique_ptr<ContextHandler>(new ContextHandler(handler));
//         }
//         ~ContextHandler() {
//             //disconnect
//             _inputContext->_handler = nullptr;
//         }
//     protected:
//         ContextHandler(InputContext* inputContext)
//         : _inputContext(inputContext)
//         {}
//     private:
//         InputContext* _inputContext;
//     };
// }