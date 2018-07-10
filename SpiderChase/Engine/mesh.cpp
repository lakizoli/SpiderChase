#include "stdafx.h"
#include "mesh.hpp"
#include "material.hpp"
#include <assimp/mesh.h>



Mesh::Mesh (const std::string& name, const aiMesh* colladaMesh, const std::vector<std::shared_ptr<Material>>& materials) {
	//Management
	_name = name;

	if (colladaMesh->mMaterialIndex < materials.size ()) {
		_material = materials[colladaMesh->mMaterialIndex];
	}
	
	//Compose vertex array
	

	
	if (colladaMesh->HasTextureCoords (0))
	{
		_uvChannelCount = 1;
	}
	else
	{
		_uvChannelCount = 0;
	}
	
	_boneCount = colladaMesh->mNumBones;

	_vertexCount = colladaMesh->mNumVertices;
	for (uint32_t i = 0; i < _vertexCount; ++i) {
		
		VertexAttribute vertexAttribute;
		vertexAttribute.pos[0] = colladaMesh->mVertices[i].x;
		vertexAttribute.pos[1] = colladaMesh->mVertices[i].y;
		vertexAttribute.pos[2] = colladaMesh->mVertices[i].z;
		
		vertexAttribute.norm[0] = colladaMesh->mNormals[i].x;
		vertexAttribute.norm[1] = colladaMesh->mNormals[i].y;
		vertexAttribute.norm[2] = colladaMesh->mNormals[i].z;
		
		if (_uvChannelCount > 0)
		{
			vertexAttribute.diffuv[0] = colladaMesh->mTextureCoords[0][i].x;
			vertexAttribute.diffuv[1] = colladaMesh->mTextureCoords[0][i].y;
		}
		else
		{
			vertexAttribute.diffuv[0] = 0;
			vertexAttribute.diffuv[1] = 0;
		}
		
		for (uint32_t i = 0; i < maxBoneWeightPerVertex; i++)
		{
			vertexAttribute.blendIndeces[i] = 0;
			vertexAttribute.blendWeights[i] = 0;
		}
		
		_vertexArray.push_back (vertexAttribute);
	}
	
	if (_boneCount > 0)
	{
		for(uint32_t boneIndex = 0; boneIndex <_boneCount; boneIndex++)
		{
			_bones.push_back (ConvertAiMatrixToGlmMatrix (colladaMesh->mBones[boneIndex]->mOffsetMatrix));
		}
		
		std::vector<uint32_t> weightCountPerVertex;
		weightCountPerVertex.resize (_vertexCount);
		for (uint32_t i = 0; i < _vertexCount; i++)
		{
			weightCountPerVertex[i] = 0;
		}
		
		for(uint32_t boneIndex = 0; boneIndex <_boneCount; boneIndex++)
		{
			for(uint32_t weightIndex=0; weightIndex< colladaMesh->mBones[boneIndex]->mNumWeights; weightIndex++)
			{
				uint32_t vertexIndex = colladaMesh->mBones[boneIndex]->mWeights[weightIndex].mVertexId;
				float weight = colladaMesh->mBones[boneIndex]->mWeights[weightIndex].mWeight;
				
				//FloatOrUInt boneIndexAttrib; boneIndexAttrib._uint/*_float*/ = boneIndex;
				//FloatOrUInt weightAttrib; weightAttrib._float = weight;
				
				//_vertexArray [vertexIndex * _attribCount + blendIndexOffsetOnVertexstride + weightCountPerVertex[vertexIndex]] = boneIndexAttrib;
				
				_vertexArray [vertexIndex].blendIndeces[weightCountPerVertex[vertexIndex]] = boneIndex;
				_vertexArray [vertexIndex].blendWeights[weightCountPerVertex[vertexIndex]] = weight;

				weightCountPerVertex[vertexIndex]++;
			}
		}
	}
	
	//Compose index array
	_faceCount = colladaMesh->mNumFaces;
	for (uint32_t i = 0; i < colladaMesh->mNumFaces; ++i) {
		_indexArray.push_back (colladaMesh->mFaces[i].mIndices[0]);
		_indexArray.push_back (colladaMesh->mFaces[i].mIndices[1]);
		_indexArray.push_back (colladaMesh->mFaces[i].mIndices[2]);
	}

	//Move data to the video card
	gl::GenBuffers (1, &_vbo);
	gl::BindBuffer (GL_ARRAY_BUFFER, _vbo);
	gl::BufferData (GL_ARRAY_BUFFER, _vertexCount * sizeof (VertexAttribute), &_vertexArray[0], GL_STATIC_DRAW);

	gl::GenBuffers (1, &_ibo);
	gl::BindBuffer (GL_ELEMENT_ARRAY_BUFFER, _ibo);
	gl::BufferData (GL_ELEMENT_ARRAY_BUFFER, _faceCount * 3 * sizeof (GLuint), &_indexArray[0], GL_STATIC_DRAW);

	const size_t coord_offset = 0 * sizeof (GLfloat);
	const size_t normal_offset = 3 * sizeof (GLfloat);
	const size_t diffuv_offset = 6 * sizeof (GLfloat);
	const size_t blendIndex_offset = (6 + 1 * 2) * sizeof (GLfloat);
	const size_t blendweight_offset = (6 + 1 * 2) * sizeof (GLfloat) + maxBoneWeightPerVertex * sizeof (GLushort);
	const GLsizei stride = sizeof (VertexAttribute);

	gl::GenVertexArrays (1, &_vao);
	gl::BindVertexArray (_vao);
	gl::BindBuffer (GL_ARRAY_BUFFER, _vbo);
	gl::VertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, stride, (void*) coord_offset);
	gl::VertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, stride, (void*) normal_offset);

	gl::VertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, stride, (void*) diffuv_offset);

	gl::VertexAttribPointer (3, 4, GL_UNSIGNED_SHORT, GL_FALSE, stride, (void*) (blendIndex_offset));
	gl::VertexAttribPointer (4, 4, GL_FLOAT, GL_FALSE, stride, (void*) (blendweight_offset));

	gl::EnableVertexAttribArray (0);
	gl::EnableVertexAttribArray (1);
	gl::EnableVertexAttribArray (2);

	gl::EnableVertexAttribArray (3);
	gl::EnableVertexAttribArray (4);


	//Unbind buffer objects
	gl::BindBuffer (GL_ARRAY_BUFFER, 0);
	gl::BindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
	gl::BindVertexArray (0);
}

void Mesh::Update (double frameTime) {
	// Update material
	if (_material) {
		_material->Update (frameTime);
	}

	//...
}

void Mesh::Render (uint32_t programID) const {
	glm::mat4x4 boneTransforms[4];
	
	static const glm::mat4x4 rot = glm::transpose(glm::rotate (glm::mat4x4 (), 90.0f, { 0.0f, 0.0f, 1.0f }));
	static const glm::mat4x4 rot2 = glm::transpose(glm::rotate (glm::mat4x4 (), 45.0f, { 0.0f, 0.0f, 1.0f }));
	
	//static glm::mat4x4 asd = glm::rotate (glm::mat4x4 (), 30.0f, { 0.0f, 0.0f, 1.0f });
	//boneTransforms[2] = /*_bones[2] * */glm::transpose(glm::translate (glm::mat4x4 (), { 0.0f, 0.0f, 3.0f }))/* * glm::inverse (_bones[2])*/;
	boneTransforms[2] = _bones[2] * rot * glm::inverse (_bones[2]);
	boneTransforms[3] = _bones[3] * rot2 * glm::inverse (_bones[3]);
	//glm::rotate(glm::mat4x4 (), 0.3f, glm::vec3(0.1,0.1,0.1));
	
	if (_boneCount > 0)
	{
		gl::Uniform1i(gl::GetUniformLocation (programID, "hasAnimation"), 1);
		gl::UniformMatrix4fv (gl::GetUniformLocation (programID, "bones"), 4, GL_FALSE, &boneTransforms[0][0][0]);
	}
	else
	{
		gl::Uniform1i(gl::GetUniformLocation (programID, "hasAnimation"), 0);
	}
	
	// Render material
	if (_material) {
		_material->Render ();
	}

	// Render mesh
	gl::BindVertexArray (_vao);
	gl::BindBuffer (GL_ELEMENT_ARRAY_BUFFER, _ibo);
	
	gl::DrawElements (GL_TRIANGLES, (GLsizei) _indexArray.size(), GL_UNSIGNED_INT, (void*) 0);

	// Unbind buffer objects
	gl::BindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
	gl::BindVertexArray (0);
}

void Mesh::Release () {
	if (_material) {
		_material->Release ();
		_material = nullptr;
	}

	if (_vao > 0) {
		gl::DeleteVertexArrays (1, &_vao);
		_vao = 0;
	}

	if (_ibo > 0) {
		gl::DeleteBuffers (1, &_ibo);
		_ibo = 0;
	}

	if (_vbo > 0) {
		gl::DeleteBuffers (1, &_vbo);
		_vbo = 0;
	}

	_vertexArray.clear ();
	_indexArray.clear ();

	_vertexCount = 0;
	_faceCount = 0;
}

template<class T>
glm::mat4x4 Mesh::ConvertAiMatrixToGlmMatrix (T aiMatrix)
{
	glm::mat4x4 glmMatrtix;
	for (uint32_t i = 0; i < 4; i++)
	{
		for (uint32_t j = 0; j < 4; j++)
		{
			glmMatrtix[i][j] = aiMatrix[i][j];
		}
	}
	
	return glmMatrtix;
}
