
#include "stdafx.h"

#include "firstpersoncamera.hpp"

FirstPersonCamera::FirstPersonCamera () {
	_position = glm::vec3 (0.0f, 0.0f, -5.0f);
	_ahead = glm::vec3 (0.0f, 0.0f, 1.0f);
	_right = glm::vec3 (1.0f, 0.0f, 0.0f);
	_yaw = 0.0;
	_pitch = 0.0;

	_fov = 1.57f;
	_nearPlane = 0.1f;
	_farPlane = 1000.0f;
	_aspect = 1.33f;

	_speed = 0.5f;

	UpdateView ();
	UpdateProjection ();
}

const glm::vec3& FirstPersonCamera::GetEyePosition () {
	return _position;
}

const glm::vec3& FirstPersonCamera::GetAhead () {
	return _ahead;
}

const glm::mat4& FirstPersonCamera::GetViewMatrix () {
	return _view;
}


const glm::mat4& FirstPersonCamera::GetProjectionMatrix () {
	return _projection;
}

const glm::mat4& FirstPersonCamera::GetRayDirMatrix () {
	return _raydir;
}

void FirstPersonCamera::SetView (glm::vec3 position, glm::vec3 ahead) {
	_position = position;
	_ahead = ahead;
	UpdateView ();
}

void FirstPersonCamera::SetProjection (float fov, float aspect, float nearPlane, float farPlane) {
	_fov = fov;
	_aspect = aspect;
	_nearPlane = nearPlane;
	_farPlane = farPlane;
	UpdateProjection ();
}

void FirstPersonCamera::SetSpeed (float speed) {
	_speed = speed;
}

void FirstPersonCamera::SetAspect (float aspect) {
	this->_aspect = aspect;
	UpdateProjection();
}

void FirstPersonCamera::Animate (float deltaTimeInSec, FPSCameraAnimDirs dirs, glm::vec2 pointerDelta, float throttleFactor) {

	if ((dirs & FPSCameraAnimDirs::Ahead) == FPSCameraAnimDirs::Ahead) {
		_position += _ahead * throttleFactor * deltaTimeInSec;
	}

	if ((dirs & FPSCameraAnimDirs::Backward) == FPSCameraAnimDirs::Backward) {
		_position -= _ahead * throttleFactor * deltaTimeInSec;
	}

	if ((dirs & FPSCameraAnimDirs::Left) == FPSCameraAnimDirs::Left) {
		_position -= _right * throttleFactor * deltaTimeInSec;
	}

	if ((dirs & FPSCameraAnimDirs::Right) == FPSCameraAnimDirs::Right) {
		_position += _right * throttleFactor * deltaTimeInSec;
	}

	if ((dirs & FPSCameraAnimDirs::Up) == FPSCameraAnimDirs::Up) {
		_position -= glm::vec3 (0.0f, 1.0f, 0.0f) * throttleFactor * deltaTimeInSec;
	}

	if ((dirs & FPSCameraAnimDirs::Down) == FPSCameraAnimDirs::Down) {
		_position += glm::vec3 (0.0f, 1.0f, 0.0f) * throttleFactor * deltaTimeInSec;
	}
	
	
	_yaw += pointerDelta.x * 3.0f;
	_pitch += pointerDelta.y * 3.0f;

	// clamp [-3.14f / 2.0f, +3.14f / 2.0f]
	if (_pitch < -3.14f / 2.0f) {
		_pitch = -3.14f / 2.0f;
	}
	if (_pitch > +3.14f / 2.0f) {
		_pitch = +3.14f / 2.0f;
	}

	_ahead = glm::vec3 (sin(_yaw)*cos(_pitch), -sin(_pitch), cos(_yaw)*cos(_pitch));
	
	UpdateView ();
}

void FirstPersonCamera::UpdateView () {
	glm::vec3 zaxis = glm::normalize (_ahead);
	glm::vec3 xaxis = glm::normalize (glm::cross (glm::vec3 (0.0, 1.0f, 0.0f), (zaxis)));
	glm::vec3 yaxis = glm::cross (zaxis, xaxis);

	_view = glm::mat4 (	xaxis.x,	xaxis.y,	xaxis.z,	glm::dot(-xaxis, _position),
						yaxis.x,	yaxis.y,	yaxis.z,	glm::dot(-yaxis, _position),
						zaxis.x,	zaxis.y,	zaxis.z,	glm::dot(-zaxis, _position),
						0.0f,		0.0f,		0.0f,		1.0f);


	_right = glm::normalize (glm::cross (glm::vec3 (0.0f, 1.0f, 0.0f), _ahead));
	_yaw = atan2f(_ahead.x, _ahead.z);
	_pitch = -atan2f(_ahead.y, glm::length (glm::vec2 (_ahead.x, _ahead.z)));
}

void FirstPersonCamera::UpdateProjection () {
	float yScale = 1.0f / ::tanf(_fov * 0.5f);
	float xScale = yScale / _aspect;
	_projection = glm::mat4(xScale,	0.0f,	0.0f,									0.0f,
							0.0f,	yScale,	0.0f,									0.0f,
							0.0f,	0.0f,	_farPlane / (_farPlane - _nearPlane),	-_nearPlane * _farPlane / (_farPlane - _nearPlane) ,
							0.0f,	0.0f,	1.0f,									0.0f);
}
