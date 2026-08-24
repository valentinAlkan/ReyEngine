#include "InputActions.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////
InputAction::InputActionHandle::InputActionHandle(InputActionHandle&& other) noexcept
: _uv(other._uv)
, _owner(std::move(other._owner))
{
   //leave the source empty so only one handle ever deregisters
   other._owner.reset();
}

/////////////////////////////////////////////////////////////////////////////////////
InputAction::InputActionHandle& InputAction::InputActionHandle::operator=(InputActionHandle&& other) noexcept {
   if (this == &other) return *this;
   //drop whatever we were already holding before taking over the new registration
   reset();
   _uv = other._uv;
   _owner = std::move(other._owner);
   other._owner.reset();
   return *this;
}

/////////////////////////////////////////////////////////////////////////////////////
InputAction::InputActionHandle::~InputActionHandle(){
   reset();
}

/////////////////////////////////////////////////////////////////////////////////////
void InputAction::InputActionHandle::reset(){
   //lock() fails if the InputMap died first, in which case the registration is already gone
   if (auto owner = _owner.lock()) {
      (*owner)->deregisterCallback(_uv);
   }
   _owner.reset();
}

/////////////////////////////////////////////////////////////////////////////////////
InputMap::InputMap() = default;

/////////////////////////////////////////////////////////////////////////////////////
InputMap::~InputMap() = default; //_lifetime expires here, neutering any outstanding handles

/////////////////////////////////////////////////////////////////////////////////////
InputAction::InputActionHandle InputMap::registerCallback(const std::string& actionName, InputAction::InputActionCallbackFx fx){
   auto it = _actions.find(actionName);
   //no such action - hand back an empty handle rather than registering into nowhere
   if (it == _actions.end()) return {};
   auto uv = Internal::UniqueGenerator::instance().makeNew();
   it->second.callbacks.push_back({std::move(fx), uv, false});
   return {uv, _lifetime};
}

/////////////////////////////////////////////////////////////////////////////////////
void InputMap::deregisterCallback(const UniqueValue& uv){
   for (auto& action : _actions | std::views::values) {
      for (auto& callback : action.callbacks) {
         if (callback._uv != uv || callback.dead) continue;
         callback.dead = true;
         //release anything the callable captured right away - the entry itself has to
         //stick around until dispatch unwinds, but the captures do not.
         callback.fx = nullptr;
         _needsCompaction = true;
         if (!_dispatchDepth) compact();
         return;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////
void InputMap::compact(){
   if (_needsCompaction) {
      for (auto& action : _actions | std::views::values) {
         std::erase_if(action.callbacks, [](const InputAction::InputActionCallback& cb){return cb.dead;});
      }
      _needsCompaction = false;
   }
   //Structural changes that arrived mid-dispatch. Swap first: applying one of these calls
   //back into addAction/removeAction, which would otherwise re-enter the queue we are
   //iterating. _dispatchDepth is already zero here, so they take effect immediately.
   if (!_deferred.empty()) {
      auto deferred = std::move(_deferred);
      _deferred.clear();
      for (auto& fx : deferred) fx();
   }
}

/////////////////////////////////////////////////////////////////////////////////////
InputMap::DispatchScope::~DispatchScope(){
   //only the outermost scope may mutate - an inner one unwinding still leaves an active loop
   if (--_map._dispatchDepth == 0) _map.compact();
}

/////////////////////////////////////////////////////////////////////////////////////
void InputMap::_index(InputAction& action){
   for (const auto& identifier : action.identifiers) {
      auto& bucket = _byEvent[identifier.first];
      //an action can bind the same event type more than once (two different keys); it still
      //belongs in the bucket exactly once
      if (std::ranges::find(bucket, &action) == bucket.end()) bucket.push_back(&action);
   }
}

/////////////////////////////////////////////////////////////////////////////////////
void InputMap::_unindex(const InputAction* action){
   for (auto it = _byEvent.begin(); it != _byEvent.end();) {
      std::erase(it->second, action);
      //drop empty buckets so actionsFor keeps returning nullptr for unbound event types
      it = it->second.empty() ? _byEvent.erase(it) : std::next(it);
   }
}

/////////////////////////////////////////////////////////////////////////////////////
void InputMap::addAction(InputAction&& action){
   if (_dispatchDepth) {
      //a callback rebound something mid-dispatch; overwriting in place now would destroy an
      //action the dispatcher is still walking
      _deferred.push_back([this, pending = std::move(action)]() mutable {addAction(std::move(pending));});
      return;
   }
   auto name = action.actionName;
   //keyed by name, so this overwrites any action already using the name. Callbacks
   //registered against the replaced action go with it; their handles will find nothing
   //to deregister and quietly do nothing.
   if (auto existing = _actions.find(name); existing != _actions.end()) _unindex(&existing->second);
   auto [it, inserted] = _actions.insert_or_assign(std::move(name), std::move(action));
   _index(it->second);
}

/////////////////////////////////////////////////////////////////////////////////////
void InputMap::removeAction(const std::string& actionName){
   if (_dispatchDepth) {
      _deferred.push_back([this, actionName]{removeAction(actionName);});
      return;
   }
   auto it = _actions.find(actionName);
   if (it == _actions.end()) return;
   _unindex(&it->second);
   _actions.erase(it);
}

/////////////////////////////////////////////////////////////////////////////////////
void InputMap::removeAction(const InputAction* action){
   if (!action) return;
   auto it = _actions.find(action->actionName);
   //make sure we are removing this exact action and not a namesake that replaced it
   if (it == _actions.end() || &it->second != action) return;
   removeAction(action->actionName);
}
