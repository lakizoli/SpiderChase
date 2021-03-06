#include "stdafx.h"
#include "game.hpp"
#include <DynamicEgl.h>
#include <DynamicGles.h>

Game::Game () {
}

Game& Game::Get () {
	static Game inst;

	//Init game
	if (inst._scene == nullptr) {
		inst._scene = Scene::Create ("start");
	}

	//Return the singleton
	return inst;
}

bool Game::Step (double currentTimeInSec) {
	//Input handling
	_inputHandler.Update (currentTimeInSec);

	//Scene handling
	if (_scene) {
		//Step the scene
		if (_scene->Step (currentTimeInSec, _inputHandler.GetState ()) == Scene::SceneResults::Continue) { //If scene continued
			return true; //continue the scene
		}

		//Exit the scene
		const std::string& nextSceneID = _scene->GetNextSceneID ();
		if (!nextSceneID.empty ()) { //We have a scene to continue with
			_scene = Scene::Create (nextSceneID);
			return true; //continue with the new scene
		}
	}

	return false; //Exit game
}

void Game::Exit () {
	_scene = nullptr;
}
