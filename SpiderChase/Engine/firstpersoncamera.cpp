
#include "stdafx.h"

#include "firstpersoncamera.hpp"

FirstPersonCamera::FirstPersonCamera () {
	_position = glm::vec3 (0.0f, 0.0f, -1.0f);
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

void FirstPersonCamera::Animate (float deltaTimeInSec, FPSCameraAnimDirs dirs, float throttleFactor) {

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
	
	/*
	_yaw += mouseDelta.x * 0.02f;
	_pitch += mouseDelta.y * 0.02f;
	_pitch = float1(pitch).clamp(-3.14f / 2.0f, +3.14f / 2.0f);

	mouseDelta = float2::zero;

	ahead = float3(sin(yaw)*cos(pitch), -sin(pitch), cos(yaw)*cos(pitch));
	*/
	UpdateView ();
}

void FirstPersonCamera::UpdateView () {
	glm::vec3 zaxis = glm::normalize (_ahead);
	glm::vec3 xaxis = glm::normalize (glm::cross (glm::vec3 (0.0, 1.0f, 0.0f), (zaxis)));
	glm::vec3 yaxis = glm::cross (zaxis, xaxis);

	_view = glm::mat4 (	xaxis.x,						yaxis.x,						zaxis.x,						0.0f,
						xaxis.y,						yaxis.y,						zaxis.y,						0.0f,
						xaxis.z,						yaxis.z,						zaxis.z,						0.0f,
						glm::dot (-xaxis, _position),	glm::dot (-yaxis, _position),	glm::dot (-zaxis, _position),	1.0f);


	_right = glm::normalize (glm::cross (glm::vec3 (0.0f, 1.0f, 0.0f), _ahead));
	_yaw = atan2f(_ahead.x, _ahead.z);
	_pitch = -atan2f(_ahead.y, glm::length (glm::vec2 (_ahead.x, _ahead.z)));
}

void FirstPersonCamera::UpdateProjection () {
	float yScale = 1.0f / ::tanf(_fov * 0.5f);
	float xScale = yScale / _aspect;
	_projection = glm::mat4 (	xScale,		0.0f,		0.0f,													0.0f,
								0.0f,		yScale,		0.0f,													0.0f,
								0.0f,		0.0f,		_farPlane / (_farPlane - _nearPlane),					1.0f,
								0.0f,		0.0f,		-_nearPlane * _farPlane / (_farPlane - _nearPlane),		0.0f);
}
/*


void Cam::FirstPerson::processMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN)
	{
		if (wParam == 'W')
			wPressed = true;
		else if (wParam == 'A')
			aPressed = true;
		else if (wParam == 'S')
			sPressed = true;
		else if (wParam == 'D')
			dPressed = true;
		else if (wParam == 'Q')
			qPressed = true;
		else if (wParam == 'E')
			ePressed = true;
		else if (wParam == VK_SHIFT)
			shiftPressed = true;
	}
	else if (uMsg == WM_KEYUP)
	{
		if (wParam == 'W')
			wPressed = false;
		else if (wParam == 'A')
			aPressed = false;
		else if (wParam == 'S')
			sPressed = false;
		else if (wParam == 'D')
			dPressed = false;
		else if (wParam == 'Q')
			qPressed = false;
		else if (wParam == 'E')
			ePressed = false;
		else if (wParam == VK_SHIFT)
			shiftPressed = false;
		else if (wParam == VK_ADD)
			speed *= 2;
		else if (wParam == VK_SUBTRACT)
			speed *= 0.5;
	}
	else if (uMsg == WM_KILLFOCUS)
	{
		wPressed = false;
		aPressed = false;
		sPressed = false;
		dPressed = false;
		qPressed = false;
		ePressed = false;
		shiftPressed = false;
	}
	else if (uMsg == WM_LBUTTONDOWN)
	{
		lastMousePos = int2(LOWORD(lParam), HIWORD(lParam));
	}
	else if (uMsg == WM_LBUTTONUP)
	{
		mouseDelta = float2::zero;
	}
	else if (uMsg == WM_MOUSEMOVE && (wParam & MK_LBUTTON))
	{
		int2 mousePos(LOWORD(lParam), HIWORD(lParam));
		mouseDelta = mousePos - lastMousePos;

		lastMousePos = mousePos;
	}
}





static  view(const float3& eye, const float3& ahead, const float3& up)
{
	float3 zaxis = ahead.normalize();
	float3 xaxis = up.cross(zaxis).normalize();
	float3 yaxis = zaxis.cross(xaxis);

	return float4x4(
		xaxis.x, yaxis.x, zaxis.x, 0,
		xaxis.y, yaxis.y, zaxis.y, 0,
		xaxis.z, yaxis.z, zaxis.z, 0,
		-xaxis.dot(eye), -yaxis.dot(eye), -zaxis.dot(eye), 1);
}

static float4x4 proj(float fovy, float aspect, float zn, float zf)
{
	float yScale = 1.0f / ::tanf(fovy * 0.5f);
	float xScale = yScale / aspect;
	return float4x4(
		xScale, 0.0f, 0.0f, 0.0f,
		0.0f, yScale, 0.0f, 0.0f,
		0.0f, 0.0f, zf / (zf - zn), 1,
		0.0f, 0.0f, -zn * zf / (zf - zn), 0);
}

//rayDirMatrix = (float4x4::view(float3::zero, ahead, float3::yUnit) * projMatrix).invert();
*/