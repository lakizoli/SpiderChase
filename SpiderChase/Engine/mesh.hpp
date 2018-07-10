#pragma once

#include <DynamicGles.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct aiMesh;
class Material;

const uint32_t maxSupportedTextureCount = 4;
const uint32_t maxBoneWeightPerVertex = 4;

//struct aiMatrix4x4;

class Mesh {
	
	
	
	std::string _name;

	GLuint _vbo; //Vertex buffer object id
	GLuint _ibo; //Index buffer object id
	GLuint _vao; //Vertex attrib object id

	//uint32_t _attribCount;
	uint32_t _vertexCount;
	uint32_t _faceCount;
	uint32_t _uvChannelCount;
	uint32_t _boneCount;
	
	struct VertexAttribute
	{
		GLfloat		pos[3];
		GLfloat		norm[3];
		GLfloat		diffuv[2];
		GLushort	blendIndeces[maxBoneWeightPerVertex];
		GLfloat		blendWeights[maxBoneWeightPerVertex];
	};

	std::vector<VertexAttribute> _vertexArray;
	std::vector<GLuint> _indexArray;
	std::vector<glm::mat4x4> _bones;

	std::shared_ptr<Material> _material;

public:
	Mesh (const std::string& name, const aiMesh* colladaMesh, const std::vector<std::shared_ptr<Material>>& materials);

	void Update (double frameTime);
	void Render (uint32_t programID) const;

	void Release ();

public:
	const std::string& GetName () const {
		return _name;
	}
	
	template<class T>
	static glm::mat4x4 ConvertAiMatrixToGlmMatrix (T aiMatrix);
};
