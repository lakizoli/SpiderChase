#pragma once

#include <DynamicGles.h>
#include <glm/detail/type_mat4x4.hpp>

struct aiMesh;

class GameMesh {
	GLuint _vbo; //Vertex buffer object id
	GLuint _ibo; //Index buffer object id
	GLuint _vao; //Vertex attrib object id

	uint32_t _attribCount;
	uint32_t _vertexCount;
	uint32_t _faceCount;

	std::vector<GLfloat> _vertexArray;
	std::vector<GLuint> _indexArray;

public:
	GameMesh (const aiMesh* colladaMesh);

	void Update (double frameTime);
	void Render () const;

	void Release ();
};
