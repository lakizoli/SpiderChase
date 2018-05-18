#include "stdafx.h"
#include "Input.hpp"

void InputState::StepKeys (const std::set<pvr::Keys> keys) {
	_lastDownKeys = _downKeys;
	_downKeys = keys;
}

void InputState::StepPointer (bool dragging, pvr::Shell::PointerNormalisedLocation pointerLocation) {
	_lastDragging = _dragging;
	_dragging = dragging;

	_lastPointerLocation = _pointerLocation;
	_pointerLocation = pointerLocation;
}

InputHandler::InputHandler () :
	_usedKeys ({
		pvr::Keys::Shift,
		pvr::Keys::Control,
		pvr::Keys::Alt,

		pvr::Keys::Space,

		pvr::Keys::Left,
		pvr::Keys::Up,
		pvr::Keys::Right,
		pvr::Keys::Down,

		pvr::Keys::Key0, pvr::Keys::Key1, pvr::Keys::Key2, pvr::Keys::Key3, pvr::Keys::Key4,
		pvr::Keys::Key5, pvr::Keys::Key6, pvr::Keys::Key7, pvr::Keys::Key8, pvr::Keys::Key9,

		pvr::Keys::A, pvr::Keys::B, pvr::Keys::C, pvr::Keys::D, pvr::Keys::E, pvr::Keys::F, pvr::Keys::G, pvr::Keys::H, pvr::Keys::I, pvr::Keys::J,
		pvr::Keys::K, pvr::Keys::L, pvr::Keys::M, pvr::Keys::N, pvr::Keys::O, pvr::Keys::P, pvr::Keys::Q, pvr::Keys::R, pvr::Keys::S, pvr::Keys::T,
		pvr::Keys::U, pvr::Keys::V, pvr::Keys::W, pvr::Keys::X, pvr::Keys::Y, pvr::Keys::Z,


		pvr::Keys::Num0, pvr::Keys::Num1, pvr::Keys::Num2, pvr::Keys::Num3, pvr::Keys::Num4,
		pvr::Keys::Num5, pvr::Keys::Num6, pvr::Keys::Num7, pvr::Keys::Num8, pvr::Keys::Num9,

		pvr::Keys::F1, pvr::Keys::F2, pvr::Keys::F3, pvr::Keys::F4, pvr::Keys::F5, pvr::Keys::F6,
		pvr::Keys::F7, pvr::Keys::F8, pvr::Keys::F9, pvr::Keys::F10, pvr::Keys::F11, pvr::Keys::F12,
	}),

	_dragging (false)

{
}

void InputHandler::Update (double currentTimeInSec) {
	_state.StepKeys (_downKeys);
	_state.StepPointer (_dragging, _pointerLocationGetter());
}

void InputHandler::OnKeyDown (pvr::Keys key) {
	if (_usedKeys.find (key) == _usedKeys.end ()) {
		return;
	}

	_downKeys.insert (key);
}

void InputHandler::OnKeyUp (pvr::Keys key) {
	if (_usedKeys.find (key) == _usedKeys.end ()) {
		return;
	}

	_downKeys.erase (key);
}

void InputHandler::DragStart() {
	_dragging = true;
}

void InputHandler::DragFinished () {
	_dragging = false;
}

void InputHandler::SetPointerLocationGetter(std::function <pvr::Shell::PointerNormalisedLocation(void)> getter) {
	_pointerLocationGetter = getter;
}
