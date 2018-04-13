#pragma once

#include "scene.hpp"

class Game {
	std::shared_ptr<Scene> _scene; ///< The current scene of the game.

//Construction
private:
	Game ();

public:
	static Game& Get ();
	~Game () = default;

//Interface
public:
	bool Step (); //Have to return false, when exit needed
	void Exit ();
};

