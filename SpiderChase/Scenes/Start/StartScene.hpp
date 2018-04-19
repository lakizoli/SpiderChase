#pragma once

#include "scene.hpp"
#include "gamemesh.hpp"

class StartScene : public Scene {
	DECLARE_SCENE (StartScene);

	std::shared_ptr<Assets> _assets;
	std::shared_ptr<GameMesh> _mesh;

	////Vertex layout
	//const uint32_t VertexArray = 0;

	////Data
	//uint32_t _vbo; // VBO handle
	//uint32_t _ibo; // IBO handle

	//std::vector<GLuint> _indices;

public:
	StartScene ();
	virtual ~StartScene () = default;

public:
	void Init() override;
	void Release() override;

	SceneResults Update(double frameTime) override;
	void Render() override;
};

