#include "camera.h"

Camera::Camera(float x, float y, float z, float r)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_camera_rotation = r;
	lives = 50;
	keys= 0;
	m_dx = sin(m_camera_rotation * (XM_PI/180.0));
	m_dz = cos(m_camera_rotation * (XM_PI/180.0));
}

void Camera::Rotate(float deg)
{
	m_camera_rotation = deg;

	m_dx = sin(m_camera_rotation * (XM_PI/180.0));
	m_dz = cos(m_camera_rotation * (XM_PI/180.0));
}

void Camera::Forward(float dist)
{
	//m_x = (m_dx * dist);
	//m_z = (m_dz * dist);

	// Moves the model in the direction it is facing
	m_x += sin(m_camera_rotation * (XM_PI/180.0)) * dist;
	m_z += cos(m_camera_rotation * (XM_PI/180.0)) * dist;
}

XMMATRIX Camera::GetViewMatrix()
{
	m_position = XMVectorSet(m_x, m_y, m_z, 0.0);
	m_lookat = XMVectorSet(m_x + m_dx, m_y, m_z + m_dz, 0.0);
	m_up = XMVectorSet(0.0, 1.0, 0.0, 0.0);

	XMMATRIX view = XMMatrixLookAtLH(m_position, m_lookat, m_up);
	return view;
}

void Camera::loselife(float damage)
{
	lives = (lives - damage);
}

void Camera::addKey()
{
	keys = (keys + 1);
}