#pragma once

#include <PVRShell/Shell.h>

class InputState {
	std::set<pvr::Keys> _lastDownKeys; ///< The list of the keys was down in the last state.
	std::set<pvr::Keys> _downKeys; ///< The list of the keys currently in down state.

	bool _dragging;
	bool _lastDragging;

	pvr::Shell::PointerNormalisedLocation _pointerLocation;
	pvr::Shell::PointerNormalisedLocation _lastPointerLocation;

//Management
public:
	InputState () {}

	void StepKeys (const std::set<pvr::Keys> keys);
	void StepPointer (bool dragging, pvr::Shell::PointerNormalisedLocation pointerLocation);

//Keyboard events
public:
	/**
	 * Detect key down state in the frame.
	 */
	bool IsKeyDown (pvr::Keys key) const {
		return _downKeys.find (key) != _downKeys.end ();
	}

	/**
	 * Detect key up state in the frame.
	 */
	bool IsKeyUp (pvr::Keys key) const {
		return _downKeys.find (key) == _downKeys.end ();
	}

	/**
	* Press button event detector. Returns true, when the key is just pressed in this frame.
	*/
	bool IsKeyPressed (pvr::Keys key) const {
		return _lastDownKeys.find (key) == _lastDownKeys.end () && _downKeys.find (key) != _downKeys.end ();
	}

	/**
	* Release button event detector. Returns true, when the key is just released in this frame.
	*/
	bool IsKeyReleased (pvr::Keys key) const {
		return _lastDownKeys.find (key) != _lastDownKeys.end () && _downKeys.find (key) == _downKeys.end ();
	}

	glm::vec2 GetPointerDelta() const {

		if (_dragging && _lastDragging) {
			return glm::vec2 (_pointerLocation.x - _lastPointerLocation.x, _pointerLocation.y - _lastPointerLocation.y);

		} else {
			return glm::vec2(0, 0);

		}
	}

//Mouse events
public:
	//TODO: implement mouse envent detectors...

//Touch events
public:
	//TODO: implement touch envent detectors...
};

class InputHandler {
	std::set<pvr::Keys> _usedKeys; ///< The list of the used keys in the whole game.
	std::set<pvr::Keys> _downKeys; ///< The list of the keys in down state.
	bool _dragging;
	InputState _state; ///< The per frame input state of the game.

	std::function <pvr::Shell::PointerNormalisedLocation(void)> _pointerLocationGetter;

public:
	InputHandler ();

	void Update (double currentTimeInSec);

	void OnKeyDown (pvr::Keys key);
	void OnKeyUp (pvr::Keys key);

	void DragStart ();
	void DragFinished ();
	void SetPointerLocationGetter (std::function <pvr::Shell::PointerNormalisedLocation(void)> getter);

public:
	const InputState& GetState () const {
		return _state;
	}
};

