#include "stdafx.h"
#include "Mesh.hpp"
#include <assimp/mesh.h>

Mesh::Mesh (const aiMesh* colladaMesh) {
	_attribCount = 6;
	//if (colladaMesh->HasTextureCoords (0)) {
	//	attribCount += 2;
	//}

	_vertexCount = colladaMesh->mNumVertices;
	for (uint32_t i = 0; i < _vertexCount; ++i) {
		_vertexArray.push_back (colladaMesh->mVertices[i].x);
		_vertexArray.push_back (colladaMesh->mVertices[i].y);
		_vertexArray.push_back (colladaMesh->mVertices[i].z);

		_vertexArray.push_back (colladaMesh->mNormals[i].x);
		_vertexArray.push_back (colladaMesh->mNormals[i].y);
		_vertexArray.push_back (colladaMesh->mNormals[i].z);

		//if (colladaMesh->HasTextureCoords (0)) {
		//	_vertexArray.push_back (colladaMesh->mTextureCoords[i][0].x);
		//	_vertexArray.push_back (colladaMesh->mTextureCoords[i][0].y);
		//}
	}

	_faceCount = colladaMesh->mNumFaces;
	for (uint32_t i = 0; i < colladaMesh->mNumFaces; ++i) {
		_indexArray.push_back (colladaMesh->mFaces[i].mIndices[0]);
		_indexArray.push_back (colladaMesh->mFaces[i].mIndices[1]);
		_indexArray.push_back (colladaMesh->mFaces[i].mIndices[2]);
	}

	//Move data to the video card
	gl::GenBuffers (1, &_vbo);
	gl::BindBuffer (GL_ARRAY_BUFFER, _vbo);
	gl::BufferData (GL_ARRAY_BUFFER, _vertexCount * _attribCount * sizeof (GLfloat), &_vertexArray[0], GL_STATIC_DRAW);

	gl::GenBuffers (1, &_ibo);
	gl::BindBuffer (GL_ELEMENT_ARRAY_BUFFER, _ibo);
	gl::BufferData (GL_ELEMENT_ARRAY_BUFFER, _faceCount * 3 * sizeof (GLuint), &_indexArray[0], GL_STATIC_DRAW);

	const size_t coord_offset = 0 * sizeof (GLfloat);
	const size_t normal_offset = 3 * sizeof (GLfloat);
	//const size_t uv_offset = 6 * sizeof (GLfloat);
	const GLsizei stride = _attribCount * sizeof (GLfloat);

	gl::GenVertexArrays (1, &_vao);
	gl::BindVertexArray (_vao);
	//gl::BindBuffer (GL_ELEMENT_ARRAY_BUFFER, _ibo);
	gl::BindBuffer (GL_ARRAY_BUFFER, _vbo);
	gl::VertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, stride, (void*) coord_offset);
	gl::VertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, stride, (void*) normal_offset);
	////gl::VertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, stride, (void*) uv_offset);
	gl::EnableVertexAttribArray (0);
	gl::EnableVertexAttribArray (1);
	////gl::EnableVertexAttribArray (2);

	// Unbind buffer objects
	gl::BindBuffer (GL_ARRAY_BUFFER, 0);
	gl::BindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
	gl::BindVertexArray (0);
}

void Mesh::Update (double frameTime) {
	//TODO: ...
}

void Mesh::Render () const {
	// Render mesh
	gl::BindVertexArray (_vao);
	gl::BindBuffer (GL_ELEMENT_ARRAY_BUFFER, _ibo);
	gl::DrawElements (GL_TRIANGLES, (GLsizei) _indexArray.size(), GL_UNSIGNED_INT, (void*) 0);

	// Unbind buffer objects
	gl::BindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
	gl::BindVertexArray (0);
}

void Mesh::Release () {
	gl::DeleteVertexArrays (1, &_vao);
	_vao = 0;

	gl::DeleteBuffers (1, &_ibo);
	_ibo = 0;

	gl::DeleteBuffers (1, &_vbo);
	_vbo = 0;

	_vertexArray.clear ();
	_indexArray.clear ();

	_vertexCount = 0;
	_faceCount = 0;
}
