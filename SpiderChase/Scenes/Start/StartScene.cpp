#include "stdafx.h"
#include "StartScene.hpp"
#include "EglContext.h"

IMPLEMENT_SCENE (StartScene, "start");

StartScene::StartScene () :
	_lastTime (-1),
	_xRot (0) {
}

void StartScene::Init () {
	_assets = LoadPak ("start", [&](uint32_t programID) {
		gl::BindAttribLocation (programID, 0, "pos");
		gl::BindAttribLocation (programID, 1, "vNorm");
	});

	// Actually use the created program
	uint32_t programID = _assets->programs["start.program"];
	gl::UseProgram (programID);

	// Sets the clear color
	gl::ClearColor (0.00f, 0.70f, 0.67f, 1.0f);

	// Create mesh from collada model
	std::shared_ptr<aiScene> scene = _assets->colladaScenes["spiderbad.dae"];
	_mesh = std::make_shared<Mesh> (scene->mMeshes[0]);

	_camera = std::make_shared<FirstPersonCamera> ();

	// Enable culling
	gl::Enable (GL_CULL_FACE);
	gl::CullFace (GL_BACK);
	gl::FrontFace (GL_CW);

	gl::DepthRangef (0.0, 1.0);
	gl::Enable (GL_DEPTH_TEST);
	gl::DepthFunc (GL_LESS);
}

void StartScene::Release () {
	_mesh->Release ();
	ReleaseAssets (_assets);
}

Scene::SceneResults StartScene::Update (double currentTimeInSec, const InputState& _inputState) {
	float deltaTime = 0;
	if (_lastTime < 0) {
		_lastTime = currentTimeInSec;
		_xRot = 0;
	} else {
		deltaTime = (float)(currentTimeInSec - _lastTime);
		_lastTime = currentTimeInSec;
	}

	if (deltaTime > 0) {
		//Animate camera
		FPSCameraAnimDirs cameraAnimDirs = FPSCameraAnimDirs::None;
		if (_inputState.IsKeyDown (pvr::Keys::Left) || _inputState.IsKeyDown (pvr::Keys::A)) {
			cameraAnimDirs |= FPSCameraAnimDirs::Left;
		}

		if (_inputState.IsKeyDown (pvr::Keys::Right) || _inputState.IsKeyDown (pvr::Keys::D)) {
			cameraAnimDirs |= FPSCameraAnimDirs::Right;
		}

		if (_inputState.IsKeyDown (pvr::Keys::Up) || _inputState.IsKeyDown (pvr::Keys::W)) {
			cameraAnimDirs |= FPSCameraAnimDirs::Ahead;
		}

		if (_inputState.IsKeyDown (pvr::Keys::Down) || _inputState.IsKeyDown (pvr::Keys::S)) {
			cameraAnimDirs |= FPSCameraAnimDirs::Backward;
		}

		if (_inputState.IsKeyDown (pvr::Keys::R)) {
			cameraAnimDirs |= FPSCameraAnimDirs::Up;
		}

		if (_inputState.IsKeyDown (pvr::Keys::F)) {
			cameraAnimDirs |= FPSCameraAnimDirs::Down;
		}

		_camera->Animate (deltaTime, cameraAnimDirs, _inputState.IsKeyDown (pvr::Keys::Shift) ? 5.0f : 1.0f);

		//Rotate test mesh
		float velocity = 2.0f * glm::pi<float> () / 5.0f;
		_xRot += velocity * deltaTime;
	}

	//TODO: ...

	return SceneResults::Continue;
}

void StartScene::Render () {
	// Matrix used for view projection
	glm::mat4x4 viewProjection = glm::scale (glm::mat4 (1.0f), { .2f, .2f, .2f });
	viewProjection = glm::rotate (viewProjection, _xRot, { 1.0f, 0.0f, 0.0f });

	//  Clears the color buffer. glClear() can also be used to clear the depth or stencil buffer
	//  (GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)
	gl::Clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	{
		//  Bind the projection model view matrix (PMVMatrix) to
		//  the associated uniform variable in the shader
		// First gets the location of that variable in the shader using its name
		uint32_t programID = _assets->programs["start.program"];
		int32_t i32Location = gl::GetUniformLocation (programID, "model");

		// Then passes the matrix to that variable
		gl::UniformMatrix4fv (i32Location, 1, GL_FALSE, &viewProjection[0][0]);
	}

	{
		uint32_t programID = _assets->programs["start.program"];
		int32_t i32Location = gl::GetUniformLocation (programID, "view");
		gl::UniformMatrix4fv (i32Location, 1, GL_FALSE, &(glm::transpose (_camera->GetViewMatrix ()))[0][0]);
	}

	{
		uint32_t programID = _assets->programs["start.program"];
		int32_t i32Location = gl::GetUniformLocation (programID, "proj");
		gl::UniformMatrix4fv (i32Location, 1, GL_FALSE, &(glm::transpose (_camera->GetProjectionMatrix ()))[0][0]);
	}

	glm::vec4 test = glm::vec4 (1.0, 1.0, 1.0, 1.0) * glm::transpose (_camera->GetViewMatrix ()) * glm::transpose ((_camera->GetProjectionMatrix ()));



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
