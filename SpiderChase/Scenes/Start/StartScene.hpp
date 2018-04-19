#pragma once

#include "scene.hpp"
#include "gamemesh.hpp"

class StartScene : public Scene {
	DECLARE_SCENE (StartScene);

	std::shared_ptr<Assets> _assets;
	std::shared_ptr<GameMesh> _mesh;

	double _lastTime;
	float _xRot;

public:
	StartScene ();
	virtual ~StartScene () = default;

public:
	void Init() override;
	void Release() override;

	SceneResults Update(double currentTimeInSec) override;
	void Render() override;
};

