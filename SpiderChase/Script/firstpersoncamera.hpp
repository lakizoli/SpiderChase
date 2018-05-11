#pragma once

#include "stdafx.h"

#include <glm/glm.hpp>

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

	void Animate (float dt, std::map<uint8_t, bool> keys);

private:
	void UpdateView ();
	void UpdateProjection ();
};