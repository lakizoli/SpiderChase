#pragma once

#include <PVRShell/Shell.h>

class InputState {
	std::set<pvr::Keys> _lastDownKeys; ///< The list of the keys was down in the last state.
	std::set<pvr::Keys> _downKeys; ///< The list of the keys currently in down state.

//Management
public:
	InputState () {}

	void StepKeys (const std::set<pvr::Keys> keys);

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
	InputState _state; ///< The per frame input state of the game.

public:
	InputHandler ();

	void Update (double currentTimeInSec);

	void OnKeyDown (pvr::Keys key);
	void OnKeyUp (pvr::Keys key);

	//TODO: implement mouse envent handlers...
	//TODO: implement touch envent handlers...

public:
	const InputState& GetState () const {
		return _state;
	}
};

