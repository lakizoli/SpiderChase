#pragma once

#include "scene.hpp"
#include "input.hpp"

class Game {
	InputHandler _inputHandler; ///< The input handler of the game.
	std::shared_ptr<Scene> _scene; ///< The current scene of the game.

//Construction
private:
	Game ();

public:
	static Game& Get ();
	~Game () = default;

//Interface
public:
	bool Step (double currentTimeInSec); //Have to return false, when exit needed
	void Exit ();

	InputHandler& GetInputHandler () {
		return _inputHandler;
	}
};

