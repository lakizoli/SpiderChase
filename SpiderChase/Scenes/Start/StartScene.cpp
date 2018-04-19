#include "stdafx.h"
#include "StartScene.hpp"
#include "EglContext.h"

IMPLEMENT_SCENE (StartScene, "start");

StartScene::StartScene () {
}

void StartScene::Init () {
	_assets = LoadPak ("start", [&] (uint32_t programID) {
		gl::BindAttribLocation (programID, 0, "pos");
	});

	// Actually use the created program
	uint32_t programID = _assets->programs["start.program"];
	gl::UseProgram (programID);

	// Sets the clear color
	gl::ClearColor (0.00f, 0.70f, 0.67f, 1.0f);

	// Create mesh from collada model
	std::shared_ptr<aiScene> scene = _assets->colladaScenes["spidergood.dae"];
	_mesh = std::make_shared<GameMesh> (scene->mMeshes[0]);

	// Enable culling
	gl::Enable (GL_CULL_FACE);
}

void StartScene::Release () {
	_mesh->Release ();
	ReleaseAssets (_assets);
}

Scene::SceneResults StartScene::Update (double frameTime) {
	//TODO: ...
	return SceneResults::Continue;
}

void StartScene::Render () {
	// Matrix used for view projection
	GLfloat afIdentity[] =
	{
		0.2f, 0, 0, 0,
		0, 0.2f, 0, 0,
		0, 0, 0.2f, 0,
		0, 0, 0, 1.f
	};

	//  Clears the color buffer. glClear() can also be used to clear the depth or stencil buffer
	//  (GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)
	gl::Clear (GL_COLOR_BUFFER_BIT);


	//  Bind the projection model view matrix (PMVMatrix) to
	//  the associated uniform variable in the shader
	// First gets the location of that variable in the shader using its name
	uint32_t programID = _assets->programs["start.program"];
	int32_t i32Location = gl::GetUniformLocation (programID, "mvpMatrix");

	// Then passes the matrix to that variable
	gl::UniformMatrix4fv (i32Location, 1, GL_FALSE, afIdentity);

	// Render our mesh
	_mesh->Render ();

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
