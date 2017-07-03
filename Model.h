#pragma once

#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT

#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <xnamath.h>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

#include "objfilemodel.h"

class Model
{
private:
	ID3D11Device*           	m_pD3DDevice;
	ID3D11DeviceContext*    	m_pImmediateContext;

	ObjFileModel*		m_pObject;
	ID3D11VertexShader*	m_pVShader;   
	ID3D11PixelShader*	m_pPShader;     
	ID3D11InputLayout*	m_pInputLayout; 
	ID3D11Buffer*		m_pConstantBuffer;   

	ID3D11ShaderResourceView*	m_pTexture0;    
	ID3D11SamplerState*	m_pSampler0;

	float m_x, m_y, m_z;
	float m_xangle, m_zangle, m_yangle;
	float m_scale;
	float m_dx, m_dz;
	float m_bounding_sphere_centre_x, m_bounding_sphere_centre_y, m_bounding_sphere_centre_z, m_bounding_sphere_radius; 
	
	void CalculateModelCentrePoint();
	void CalculateBoundingSphereRadius();

public:
	Model(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11SamplerState* sampler, ID3D11ShaderResourceView* texture);
	~Model();

	int LoadObjModel(char* filename);
	void Draw(XMMATRIX* world, XMMATRIX* view, XMMATRIX* projection);

	void LookAt_XZ(float x, float z);
	void MoveForward(float dist);

	XMVECTOR GetBoundingSphereWorldSpacePosition();
	float GetBoundingSphereRadius();
	bool CheckCollision(Model*);

	void setTexture(ID3D11ShaderResourceView* tex);
	void setSampler(ID3D11SamplerState* sam);
	
	// Set methods - *no longer used*
	void setXPos(float num) {m_x = num;}
	void setYPos(float num) {m_y = num;} 
	void setZPos(float num) {m_z = num;}
	void setXAngle(float num) {m_xangle = num;}
	void setYAngle(float num) {m_yangle = num;}
	void setZAngle(float num) {m_zangle = num;}
	void setScale(float num) {m_scale = num;}

	//Get methods
	float getXPos() {return m_x;}
	float getYPos() {return m_y;}
	float getZPos() {return m_z;}
	float getXAngle() {return m_xangle;}
	float getYAngle() {return m_yangle;}
	float getZAngle() {return m_zangle;}
	float getScale() {return m_scale;}
	float getBoundingSphere_x() { return m_bounding_sphere_centre_x;}
	float getBoundingSphere_y() { return m_bounding_sphere_centre_y;}
	float getBoundingSphere_z() { return m_bounding_sphere_centre_z;}
	
	//Adjustment methods - *no longer used*
	void AdjXPos(float num) {m_x =+ num;}
	void AdjYPos(float num) {m_y =+ num;}
	void AdjZPos(float num) {m_z =+ num;}
	void AdjXAngle(float num) {m_xangle =+ num;}
	void AdjYAngle(float num) {m_yangle =+ num;}
	void AdjZAngle(float num) {m_zangle =+ num;}
	void AdjScale(float num) {m_scale =+ num;}
};