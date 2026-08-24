#pragma once
#include "InputManager.h"
#include "UniqueValue.h"
#include <memory>
#include <functional>
#include <unordered_map>

namespace ReyEngine {
    class InputMap;
    struct InputAction {
        using InputMetaData =  std::variant<
                InputInterface::KeyCode,
                InputInterface::MouseButton,
                InputInterface::GamePadButton,
                InputInterface::GamePadAxis
        >;

        using InputActionCallbackFx = std::function<Handled(const InputEvent&, const InputAction*)>;

        //RAII token handed back by InputMap::registerCallback. The registration lives exactly
        //as long as this handle: the destructor deregisters, so callers never have to.
        //Move-only - copying would deregister the same callback twice.
        struct InputActionHandle {
            InputActionHandle() = default;
            InputActionHandle(const InputActionHandle&) = delete;
            InputActionHandle& operator=(const InputActionHandle&) = delete;
            InputActionHandle(InputActionHandle&&) noexcept;
            InputActionHandle& operator=(InputActionHandle&&) noexcept;
            ~InputActionHandle();
            //false for a default-constructed handle, a moved-from handle, one that has been
            //reset, or one whose InputMap has already been destroyed.
            explicit operator bool() const {return !_owner.expired();}
            //deregister now instead of waiting for scope exit. Idempotent.
            void reset();
        private:
            friend class InputMap;
            InputActionHandle(const UniqueValue& uv, std::weak_ptr<InputMap*> owner)
            : _uv(uv)
            , _owner(std::move(owner))
            {}
            UniqueValue _uv{};
            std::weak_ptr<InputMap*> _owner; //expires if the InputMap dies before we do
        };

        struct InputActionCallback {
            InputActionCallbackFx fx;
            UniqueValue _uv;
            //Tombstone. Deregistering during dispatch cannot erase the entry out from under
            //the loop, so it is marked dead here and swept up once dispatch unwinds.
            //Dispatchers must skip dead entries.
            bool dead = false;
        };
        using InputIdentifier = std::pair<ReyEngine::EventId, InputMetaData>;
        std::string actionName;
        std::vector<InputIdentifier> identifiers;
        std::vector<InputActionCallback> callbacks;
        bool matches(const ReyEngine::InputEvent& input) const {

            const InputMetaData* matched = nullptr;
            for (const auto& identifier : identifiers) {
                if (identifier.first == input.eventId) {
                    matched = &identifier.second;
                    break;
                }
            }
            //short circuit if the eventid is not matched
            if (!matched) return false;

            //compare the event's payload against the stored metadata. Returns false
            //if the variant is holding a different alternative than M.
            auto matchesMetaData = [&]<typename M>(const M& metaData) {
                const auto* stored = std::get_if<M>(matched);
                return stored && *stored == metaData;
            };

            // if the event types are the same, check the metadata
             switch (input.eventId) {
                case ReyEngine::InputEventMouseButton::ID:
                     return matchesMetaData(input.toEvent<ReyEngine::InputEventMouseButton>().button);
                case ReyEngine::InputEventKey::ID:
                     return matchesMetaData(input.toEvent<ReyEngine::InputEventKey>().key);
                case ReyEngine::InputEventGamePadButton::ID:
                     return matchesMetaData(input.toEvent<ReyEngine::InputEventGamePadButton>().button);
                case ReyEngine::InputEventGamePadAxis::ID:
                     return matchesMetaData(input.toEvent<ReyEngine::InputEventGamePadAxis>().axis);
                }
            return false;
        }

    };

    class InputMap {

    public:
        InputMap();
        ~InputMap();
        //The handles hold a weak reference back to this object, so it must not be copied or
        //moved - the outstanding handles would keep pointing at the original.
        InputMap(const InputMap&) = delete;
        InputMap& operator=(const InputMap&) = delete;
        InputMap(InputMap&&) = delete;
        InputMap& operator=(InputMap&&) = delete;

        //Actions are keyed by name, so adding an action whose name is already present
        //overwrites the existing one. Any callbacks registered against the old action are
        //dropped; their handles notice and become no-ops.
        //Called during dispatch, this is deferred until dispatch unwinds - see DispatchScope.
        void addAction(InputAction&& action);
        //Both overloads are no-ops if the action is not present. Pointers previously handed
        //out by getAction for that action are invalidated. Also deferred during dispatch.
        void removeAction(const std::string& actionName);
        void removeAction(const InputAction* action);

        const InputAction* getAction(const std::string& actionName) const {
            auto it = _actions.find(actionName);
            return it == _actions.end() ? nullptr : &it->second;
        }

        //THE HOT PATH. Every action bound to this event type, or nullptr if none are. One
        //hash lookup per incoming event instead of a walk over every action in the map.
        //The returned vector is guaranteed stable for the life of a DispatchScope, because
        //structural changes made from inside a callback are deferred past it.
        const std::vector<InputAction*>* actionsFor(ReyEngine::EventId eventId) const {
            auto it = _byEvent.find(eventId);
            return it == _byEvent.end() ? nullptr : &it->second;
        }

        //Register fx against the named action. The returned handle owns the registration:
        //when it goes out of scope the callback is removed, so callers never deregister by
        //hand. Evaluates false if actionName does not name a known action, in which case
        //nothing was registered.
        [[nodiscard]] InputAction::InputActionHandle registerCallback(const std::string& actionName, InputAction::InputActionCallbackFx fx);

        //Normally called for you by ~InputActionHandle. Safe to call with a UniqueValue that
        //is no longer registered. During dispatch this only tombstones the entry.
        void deregisterCallback(const UniqueValue& uv);

        //Held by the dispatcher for as long as it is walking actions and their callbacks.
        //While one exists, every mutation is deferred: deregistration only tombstones, and
        //addAction/removeAction are queued. A callback is therefore free to deregister
        //itself, destroy its owner, or rebind an action without invalidating the loop.
        //Everything is applied when the outermost scope unwinds.
        class DispatchScope {
        public:
            explicit DispatchScope(InputMap& map): _map(map) {++_map._dispatchDepth;}
            ~DispatchScope();
            DispatchScope(const DispatchScope&) = delete;
            DispatchScope& operator=(const DispatchScope&) = delete;
        private:
            InputMap& _map;
        };
    private:
        friend class DispatchScope;
        //Erase tombstoned callbacks and run anything queued during dispatch.
        //Only legal when no DispatchScope is active.
        void compact();
        //Add/remove this action's entries in the _byEvent index, one per distinct event
        //type it binds. Never called while a dispatch is in progress.
        void _index(InputAction& action);
        void _unindex(const InputAction* action);

        //Owns the actions. Node-based, so the InputAction* held in _byEvent stay valid as
        //long as that particular action is not erased.
        std::map<std::string, InputAction> _actions;
        //The dispatch index: event type -> the actions bound to it. An action binding both
        //a key and a gamepad button appears in both buckets, which is why _actions rather
        //than this one owns them.
        std::unordered_map<ReyEngine::EventId, std::vector<InputAction*>> _byEvent;
        //addAction/removeAction calls that arrived mid-dispatch, replayed on unwind.
        std::vector<std::function<void()>> _deferred;
        size_t _dispatchDepth = 0;
        bool _needsCompaction = false;
        //Handles hold a weak_ptr to this. It expires when we are destroyed, which is what
        //lets a handle that outlives its InputMap destruct harmlessly instead of dangling.
        std::shared_ptr<InputMap*> _lifetime = std::make_shared<InputMap*>(this);
    };
}