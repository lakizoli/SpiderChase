#pragma once

#include "stdafx.h"
#include <glm/glm.hpp>
#include "flagsenum.hpp"

enum class FPSCameraAnimDirs : uint8_t {
	None = 0x0000,

	Ahead = 0x0001,
	Backward = 0x0002,
	Left = 0x0004,
	Right = 0x0008,
	Up = 0x0010,
	Down = 0x0020
};
FLAGS_ENUM (FPSCameraAnimDirs);

class FirstPersonCamera {
private:
	// view data
	glm::vec3 _position;
	glm::vec3 _ahead;
	glm::vec3 _right;
	float _yaw;
	float _pitch;

	// projection data
	float _fov;
	float _aspect;
	float _nearPlane;
	float _farPlane;

	// derived values
	glm::mat4 _view;
	glm::mat4 _projection;
	glm::mat4 _raydir;

	// animation
	float _speed;

public:
	FirstPersonCamera ();

	const glm::vec3& GetEyePosition ();
	const glm::vec3& GetAhead ();
	
	const glm::mat4& GetViewMatrix ();
	const glm::mat4& GetProjectionMatrix ();
	const glm::mat4& GetRayDirMatrix ();

	void SetView (glm::vec3 position, glm::vec3 ahead);
	void SetProjection (float fov, float aspect, float nearPlane, float farPlane);
	void SetSpeed (float speed);
	void SetAspect (float aspect);

public:
	void Animate (float deltaTimeInSec, FPSCameraAnimDirs dirs, glm::vec2 pointerDelta, float throttleFactor = 1.0f);

private:
	void UpdateView ();
	void UpdateProjection ();
};