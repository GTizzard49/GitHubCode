#ifndef CAMERA_H
#define CAMERA_H
#include <d3d11.h>
#include <math.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <xnamath.h>

class Camera 
{
private:
	float m_x;
	float m_y;
	float m_z;
	float m_dx;
	float m_dz;
	float m_camera_rotation;
	float lives;
	int keys;
	XMVECTOR m_position;
	XMVECTOR m_lookat;
	XMVECTOR m_up;

public:
	//contstructor
	Camera(float x, float y, float z, float r);
	void Rotate(float deg);
	void Forward(float dist);
	XMMATRIX GetViewMatrix();
	void loselife(float damage);
	void addKey();

	//Get methods
	float getX() {return m_x;}
	float getY() {return m_y;}
	float getZ() {return m_z;}
	float getLives() {return lives;}
	int getKeys() {return keys;}
};
#endif