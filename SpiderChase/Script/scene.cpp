#include "stdafx.h"
#include "scene.hpp"

std::map<std::string, Scene::SceneCreator>& Scene::GetCreatorMap () {
	static std::map<std::string, SceneCreator> creators;
	return creators;
}

Scene::Scene () :
	_isInited (false)
{
}

uint32_t Scene::RegisterSceneCreator (const std::string& sceneID, SceneCreator sceneCreator) {
	std::map<std::string, SceneCreator>& creators = GetCreatorMap ();
	creators.emplace (sceneID, sceneCreator);
	return 42;
}

std::shared_ptr<Scene> Scene::Create (const std::string& sceneID) {
	const std::map<std::string, SceneCreator>& creators = GetCreatorMap ();
	auto it = creators.find (sceneID);
	if (it == creators.end ()) {
		return nullptr;
	}

	return it->second ();
}

Scene::SceneResults Scene::Step () {
	if (!_isInited) {
		Init ();

		//TODO: measure time
	}

	SceneResults updateRes = Update (0);
	if (updateRes != SceneResults::Continue) { //Have to exit scene...
		Release ();

		_isInited = false;

		return updateRes;
	}

	Render ();

	return SceneResults::Continue;
}
