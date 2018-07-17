#pragma once

/**
 * We can use keyboard, touch and mouse events in the game, if supported by the platform.
 *
 * The support must be checked with the underlying macros:
 *    INPUT_KEYBOARD_AVAILABLE: macro for check keyboard support.
 *    INPUT_TOUCH_AVAILABLE: macro for check touch support.
 *    INPUT_MOUSE_AVAILABLE: macro for check mouse support.
 */

#include <glm/glm.hpp>

//Windows input handling definitions
#ifdef PLATFORM_WINDOWS

#	include <PVRShell/Shell.h>

#	define INPUT_KEYBOARD_AVAILABLE
typedef pvr::Keys KeyCode;

//macOS and iOS input handling definitions
#elif defined (PLATFORM_MACOS) || defined (PLATFORM_IOS)

#	define INPUT_TOUCH_AVAILABLE

#else //PLATFORM
#	error "Os not implemented!"
#endif //PLATFORM

enum class TouchEvents {
	Begin,
	Move,
	End
};

struct TouchEvent {
	uint64_t id;
	TouchEvents event;
	glm::vec2 pos;
};

class InputState {
#ifdef INPUT_KEYBOARD_AVAILABLE
	std::set<KeyCode> _lastDownKeys; ///< The list of the keys was down in the last state.
	std::set<KeyCode> _downKeys; ///< The list of the keys currently in down state.
#endif //INPUT_KEYBOARD_AVAILABLE
	
#ifdef INPUT_TOUCH_AVAILABLE
	std::vector<TouchEvent> _touches; ///< All of the touch events occured during the frame in order of occuring.
#endif //INPUT_TOUCH_AVAILABLE

//Management
public:
	InputState ();

//Keyboard management and events
#ifdef INPUT_KEYBOARD_AVAILABLE
public:
	void StepKeys (const std::set<KeyCode>& keys);

	/**
	 * Detect key down state in the frame.
	 */
	bool IsKeyDown (KeyCode key) const {
		return _downKeys.find (key) != _downKeys.end ();
	}

	/**
	 * Detect key up state in the frame.
	 */
	bool IsKeyUp (KeyCode key) const {
		return _downKeys.find (key) == _downKeys.end ();
	}

	/**
	* Press button event detector. Returns true, when the key is just pressed in this frame.
	*/
	bool IsKeyPressed (KeyCode key) const {
		return _lastDownKeys.find (key) == _lastDownKeys.end () && _downKeys.find (key) != _downKeys.end ();
	}

	/**
	* Release button event detector. Returns true, when the key is just released in this frame.
	*/
	bool IsKeyReleased (KeyCode key) const {
		return _lastDownKeys.find (key) != _lastDownKeys.end () && _downKeys.find (key) == _downKeys.end ();
	}
	
#endif //INPUT_KEYBOARD_AVAILABLE

//Mouse management and events
public:
	//TODO: implement mouse envent detectors...

//Touch management and events
#ifdef INPUT_TOUCH_AVAILABLE
public:
	void StepTouches (const std::vector<TouchEvent>& touches);
	
	typedef std::function<void (uint64_t touchID, const glm::vec2& pos)> TouchEventHandler;
	void EnumerateTouches (TouchEventHandler touchBegin, TouchEventHandler touchMove, TouchEventHandler touchEnd) const;
	
	//TODO: implement touch envent detectors...
	
#endif //INPUT_TOUCH_AVAILABLE
};

class InputHandler {
#ifdef INPUT_KEYBOARD_AVAILABLE
	std::set<KeyCode> _usedKeys; ///< The list of the used keys in the whole game.
	std::set<KeyCode> _downKeys; ///< The list of the keys in down state.
#endif //INPUT_KEYBOARD_AVAILABLE
	
#ifdef INPUT_TOUCH_AVAILABLE
	std::vector<TouchEvent> _touches; ///< All of the touch events occured during the frame in order of occuring.
#endif //INPUT_TOUCH_AVAILABLE
	
	InputState _state; ///< The per frame input state of the game.

public:
	InputHandler ();

	void Update (double currentTimeInSec);

#ifdef INPUT_KEYBOARD_AVAILABLE
	void OnKeyDown (KeyCode key);
	void OnKeyUp (KeyCode key);
#endif //INPUT_KEYBOARD_AVAILABLE
	
#ifdef INPUT_TOUCH_AVAILABLE
	void OnTouchBegan (uint64_t touchID, const glm::vec2& pos);
	void OnTouchMoved (uint64_t touchID, const glm::vec2& pos);
	void OnTouchEnded (uint64_t touchID, const glm::vec2& pos);
#endif //INPUT_TOUCH_AVAILABLE

public:
	const InputState& GetState () const {
		return _state;
	}
};

