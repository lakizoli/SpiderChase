#pragma once

#include "scene.hpp"

class StartScene : public Scene {
	DECLARE_SCENE (StartScene);

public:
	StartScene ();
	virtual ~StartScene () = default;

public:
	void Init() override;
	void Release() override;

	SceneResults Update(double frameTime) override;
	void Render() override;
};

