#include "stdafx.h"
#include "input.hpp"

/******************************************************************************************
 * class InputState
 ******************************************************************************************/

InputState::InputState () {
}

#ifdef INPUT_KEYBOARD_AVAILABLE
void InputState::StepKeys (const std::set<KeyCode>& keys) {
	_lastDownKeys = _downKeys;
	_downKeys = keys;
}
#endif //INPUT_KEYBOARD_AVAILABLE

#ifdef INPUT_TOUCH_AVAILABLE
void InputState::StepTouches (const std::vector<TouchEvent>& touches) {
	_touches = touches;
}

void InputState::EnumerateTouches (TouchEventHandler touchBegin, TouchEventHandler touchMove, TouchEventHandler touchEnd) const {
	for (const TouchEvent& event : _touches) {
		switch (event.event) {
			case TouchEvents::Begin:
				touchBegin (event.id, event.pos);
				break;
			case TouchEvents::Move:
				touchMove (event.id, event.pos);
				break;
			case TouchEvents::End:
				touchEnd (event.id, event.pos);
				break;
			default:
				break;
		}
	}
}
#endif //INPUT_TOUCH_AVAILABLE

/******************************************************************************************
 * class InputHandler
 ******************************************************************************************/

InputHandler::InputHandler () {
#ifdef INPUT_KEYBOARD_AVAILABLE
	_usedKeys = std::set<KeyCode> {
		KeyCode::Shift,
		KeyCode::Control,
		KeyCode::Alt,
		
		KeyCode::Space,
		
		KeyCode::Left,
		KeyCode::Up,
		KeyCode::Right,
		KeyCode::Down,
		
		KeyCode::Key0, KeyCode::Key1, KeyCode::Key2, KeyCode::Key3, KeyCode::Key4,
		KeyCode::Key5, KeyCode::Key6, KeyCode::Key7, KeyCode::Key8, KeyCode::Key9,
		
		KeyCode::A, KeyCode::B, KeyCode::C, KeyCode::D, KeyCode::E, KeyCode::F, KeyCode::G, KeyCode::H, KeyCode::I, KeyCode::J,
		KeyCode::K, KeyCode::L, KeyCode::M, KeyCode::N, KeyCode::O, KeyCode::P, KeyCode::Q, KeyCode::R, KeyCode::S, KeyCode::T,
		KeyCode::U, KeyCode::V, KeyCode::W, KeyCode::X, KeyCode::Y, KeyCode::Z,
		
		
		KeyCode::Num0, KeyCode::Num1, KeyCode::Num2, KeyCode::Num3, KeyCode::Num4,
		KeyCode::Num5, KeyCode::Num6, KeyCode::Num7, KeyCode::Num8, KeyCode::Num9,
		
		KeyCode::F1, KeyCode::F2, KeyCode::F3, KeyCode::F4, KeyCode::F5, KeyCode::F6,
		KeyCode::F7, KeyCode::F8, KeyCode::F9, KeyCode::F10, KeyCode::F11, KeyCode::F12,
	};
#endif //INPUT_KEYBOARD_AVAILABLE
}

void InputHandler::Update (double currentTimeInSec) {
#ifdef INPUT_KEYBOARD_AVAILABLE
	_state.StepKeys (_downKeys);
#endif //INPUT_KEYBOARD_AVAILABLE
	
#ifdef INPUT_TOUCH_AVAILABLE
	_state.StepTouches (_touches);
	_touches.clear ();
#endif //INPUT_TOUCH_AVAILABLE
}

#ifdef INPUT_KEYBOARD_AVAILABLE
void InputHandler::OnKeyDown (KeyCode key) {
	if (_usedKeys.find (key) == _usedKeys.end ()) {
		return;
	}

	_downKeys.insert (key);
}

void InputHandler::OnKeyUp (KeyCode key) {
	if (_usedKeys.find (key) == _usedKeys.end ()) {
		return;
	}

	_downKeys.erase (key);
}
#endif //INPUT_KEYBOARD_AVAILABLE

#ifdef INPUT_TOUCH_AVAILABLE
void InputHandler::OnTouchBegan (uint64_t touchID, const glm::vec2& pos) {
	_touches.push_back (TouchEvent { touchID, TouchEvents::Begin, pos });
	//printf ("OnTouchBegan id: %llu, pos.x: %.2f, pos.y: %.2f\n", touchID, pos.x, pos.y);
}

void InputHandler::OnTouchMoved (uint64_t touchID, const glm::vec2& pos) {
	_touches.push_back (TouchEvent { touchID, TouchEvents::Move, pos });
	//printf ("OnTouchMoved id: %llu, pos.x: %.2f, pos.y: %.2f\n", touchID, pos.x, pos.y);
}

void InputHandler::OnTouchEnded (uint64_t touchID, const glm::vec2& pos) {
	_touches.push_back (TouchEvent { touchID, TouchEvents::End, pos });
	//printf ("OnTouchEnded id: %llu, pos.x: %.2f, pos.y: %.2f\n", touchID, pos.x, pos.y);
}
#endif //INPUT_TOUCH_AVAILABLE
