#pragma once

#include "scene.hpp"

class StartScene : public Scene {
	DECLARE_SCENE (StartScene);

	//Vertex layout
	const uint32_t VertexArray = 0;

	//Data
	std::shared_ptr<Assets> _assets;
	uint32_t _vbo; // VBO handle

public:
	StartScene ();
	virtual ~StartScene () = default;

public:
	void Init() override;
	void Release() override;

	SceneResults Update(double frameTime) override;
	void Render() override;
};

