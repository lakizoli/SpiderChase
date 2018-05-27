#pragma once

#include <DynamicGles.h>

struct aiMesh;
class Material;

class Mesh {
	std::string _name;

	GLuint _vbo; //Vertex buffer object id
	GLuint _ibo; //Index buffer object id
	GLuint _vao; //Vertex attrib object id

	uint32_t _attribCount;
	uint32_t _vertexCount;
	uint32_t _faceCount;
	uint32_t _uvChannelCount;

	std::vector<GLfloat> _vertexArray;
	std::vector<GLuint> _indexArray;

	std::shared_ptr<Material> _material;

public:
	Mesh (const std::string& name, const aiMesh* colladaMesh, const std::vector<std::shared_ptr<Material>>& materials);

	void Update (double frameTime);
	void Render () const;

	void Release ();

public:
	const std::string& GetName () const {
		return _name;
	}
};
