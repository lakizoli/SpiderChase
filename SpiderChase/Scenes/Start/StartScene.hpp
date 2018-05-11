#pragma once

#include "scene.hpp"
#include "gamemesh.hpp"
#include "firstpersoncamera.hpp"

class StartScene : public Scene {
	DECLARE_SCENE (StartScene);

	std::shared_ptr<Assets> _assets;
	std::shared_ptr<GameMesh> _mesh;
	std::shared_ptr<FirstPersonCamera> _camera;

	double _lastTime;
	float _xRot;

public:
	StartScene ();
	virtual ~StartScene () = default;

public:
	void Init() override;
	void Release() override;

	SceneResults Update(double currentTimeInSec, std::map<uint8_t, bool> keys) override;
	void Render() override;
};

