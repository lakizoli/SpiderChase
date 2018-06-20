#pragma once

#ifdef PLATFORM_WINDOWS
#	include <PVRShell/Shell.h>
#elif defined (PLATFORM_MACOS) || defined (PLATFORM_IOS)

namespace pvr {
	
enum class Keys : uint8_t
{
	//!\cond NO_DOXYGEN
	//Whenever possible, keys get ASCII values of their default (non-shifted) values of a default US keyboard.
	Backspace = 0x08,
	Tab = 0x09,
	Return = 0x0D,
	
	Shift = 0x10, Control = 0x11, Alt = 0x12,
	
	Pause = 0x13,
	PrintScreen = 0x2C,
	CapsLock = 0x14,
	Escape = 0x1B,
	Space = 0x20,
	
	PageUp = 0x21, PageDown = 0x22, End = 0x23, Home = 0x24,
	
	Left = 0x25, Up = 0x26, Right = 0x27, Down = 0x28,
	
	Insert = 0x2D, Delete = 0x2E,
	
	//ASCII-Based
	Key0 = 0x30, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,
	
	A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M,
	N = 0x4E, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	
	
	Num0 = 0x60, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, NumPeriod,
	
	F1 = 0x70, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	
	SystemKey1 = 0x5B, SystemKey2 = 0x5D,
	WindowsKey = 0x5B, MenuKey = 0x5D, //ALIASES
	
	NumMul = 0x6A, NumAdd = 0x6B, NumSub = 0x6D, NumDiv = 0x6E,
	NumLock = 0x90, ScrollLock = 0x91,
	
	Semicolon = 0xBA, Equals = 0xBB, Minus = 0xBD,
	
	Slash = 0xBF,
	
	Comma = 0xBC, Period = 0xBE,
	
	Backquote = 0xC0,
	
	SquareBracketLeft = 0xDB, SquareBracketRight = 0xDD, Quote = 0xDE, Backslash = 0xDC,
	
	MaxNumKeyCodes,
	Unknown = 0xFF
	//!\endcond
};
	
}//namespace pvr

#else //PLATFORM
#	error "Os not implemented!"
#endif //PLATFORM

class InputState {
	std::set<pvr::Keys> _lastDownKeys; ///< The list of the keys was down in the last state.
	std::set<pvr::Keys> _downKeys; ///< The list of the keys currently in down state.

//	bool _dragging;
//	bool _lastDragging;
//
//	pvr::Shell::PointerNormalisedLocation _pointerLocation;
//	pvr::Shell::PointerNormalisedLocation _lastPointerLocation;

//Management
public:
	InputState () {}

	void StepKeys (const std::set<pvr::Keys> keys);
//	void StepPointer (bool dragging, pvr::Shell::PointerNormalisedLocation pointerLocation);

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

//	glm::vec2 GetPointerDelta() const {
//
//		if (_dragging && _lastDragging) {
//			return glm::vec2 (_pointerLocation.x - _lastPointerLocation.x, _pointerLocation.y - _lastPointerLocation.y);
//
//		} else {
//			return glm::vec2(0, 0);
//
//		}
//	}

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

	//std::function <pvr::Shell::PointerNormalisedLocation(void)> _pointerLocationGetter;

public:
	InputHandler ();

	void Update (double currentTimeInSec);

	void OnKeyDown (pvr::Keys key);
	void OnKeyUp (pvr::Keys key);

//	void DragStart ();
//	void DragFinished ();
//	void SetPointerLocationGetter (std::function <pvr::Shell::PointerNormalisedLocation(void)> getter);

public:
	const InputState& GetState () const {
		return _state;
	}
};

