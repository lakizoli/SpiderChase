#include "stdafx.h"
#include "StartScene.hpp"
#include "EglContext.h"

IMPLEMENT_SCENE (StartScene, "start");

StartScene::StartScene () :
	_vbo (0)
{
}

void StartScene::Init () {
	_assets = LoadPak ("start", [&] (uint32_t programID) {
		gl::BindAttribLocation (programID, VertexArray, "myVertex");
	});

	// Actually use the created program
	uint32_t programID = _assets->programs["start.program"];
	gl::UseProgram (programID);

	// Sets the clear color
	gl::ClearColor (0.00f, 0.70f, 0.67f, 1.0f);

	// Create VBO for the triangle from our data

	// Vertex data
	GLfloat afVertices[] = { -0.4f, -0.4f, 0.0f,  0.4f, -0.4f, 0.0f,   0.0f, 0.4f, 0.0f };

	// Gen VBO
	gl::GenBuffers (1, &_vbo);

	// Bind the VBO
	gl::BindBuffer (GL_ARRAY_BUFFER, _vbo);

	// Set the buffer's data
	gl::BufferData (GL_ARRAY_BUFFER, 3 * (3 * sizeof (GLfloat)) /* 3 Vertices of 3 floats in size */, afVertices, GL_STATIC_DRAW);

	// Unbind the VBO
	gl::BindBuffer (GL_ARRAY_BUFFER, 0);

	// Enable culling
	gl::Enable (GL_CULL_FACE);
}

void StartScene::Release () {
	// Release Vertex buffer object.
	gl::DeleteBuffers (1, &_vbo);

	ReleaseAssets (_assets);
}

Scene::SceneResults StartScene::Update (double frameTime) {
	//TODO: ...
	return SceneResults::Continue;
}

void StartScene::Render () {
	// Matrix used for projection model view
	float afIdentity[] =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	//  Clears the color buffer. glClear() can also be used to clear the depth or stencil buffer
	//  (GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)
	gl::Clear (GL_COLOR_BUFFER_BIT);


	//  Bind the projection model view matrix (PMVMatrix) to
	//  the associated uniform variable in the shader
	// First gets the location of that variable in the shader using its name
	uint32_t programID = _assets->programs["start.program"];
	int32_t i32Location = gl::GetUniformLocation (programID, "myPMVMatrix");

	// Then passes the matrix to that variable
	gl::UniformMatrix4fv (i32Location, 1, GL_FALSE, afIdentity);

	// Bind the VBO
	gl::BindBuffer (GL_ARRAY_BUFFER, _vbo);

	// Enable the custom vertex attribute at index VERTEX_ARRAY.
	// We previously binded that index to the variable in our shader "vec4 MyVertex;"
	gl::EnableVertexAttribArray (VertexArray);

	// Points to the data for this vertex attribute
	gl::VertexAttribPointer (VertexArray, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Draws a non-indexed triangle array from the pointers previously given.
	// This function allows the use of other primitive types : triangle strips, lines, ...
	// For indexed geometry, use the function glDrawElements() with an index list.
	gl::DrawArrays (GL_TRIANGLES, 0, 3);

	// Unbind the VBO
	gl::BindBuffer (GL_ARRAY_BUFFER, 0);

	if (gl::InvalidateFramebuffer != nullptr) {
		GLenum invalidateAttachments[2];
		invalidateAttachments[0] = GL_DEPTH;
		invalidateAttachments[1] = GL_STENCIL;

		gl::InvalidateFramebuffer (GL_FRAMEBUFFER, 2, &invalidateAttachments[0]);
	} else if (gl::isGlExtensionSupported ("GL_EXT_discard_framebuffer")) {
		GLenum invalidateAttachments[2];
		invalidateAttachments[0] = GL_DEPTH_EXT;
		invalidateAttachments[1] = GL_STENCIL_EXT;

		gl::ext::DiscardFramebufferEXT (GL_FRAMEBUFFER, 2, &invalidateAttachments[0]);
	}
}
