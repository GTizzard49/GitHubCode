#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT

#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <xnamath.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "Model.h"

class scene_node
{
private:
	Model* m_p_model;

	std::vector<scene_node*> m_children;

	float m_x, m_y, m_z;
	float m_xangle, m_zangle, m_yangle;
	float m_scale;
	float m_dx, m_dz;
	float m_world_scale;
	float m_world_centre_x, m_world_centre_y, m_world_centre_z; 


public:
	scene_node();

	void setModel(Model* mod);
	void addChildNode(scene_node *n);
	bool detatchNode(scene_node *n);
	void execute(XMMATRIX *world, XMMATRIX* view, XMMATRIX* projection);
	void update_collision_tree(XMMATRIX* world, float scale);
	bool check_collision(scene_node* compre_tree);
	bool check_collision(scene_node* compare_tree, scene_node* object_tree_root);

	// Set methods
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
	XMVECTOR get_world_centre_position();

	//Adjustment methods
	bool AdjXPos(float num, scene_node* rn); 
	bool AdjYPos(float num, scene_node* rn);
	bool AdjZPos(float num, scene_node* rn);
	bool AdjXAngle(float num, scene_node* rn); 
	bool AdjYAngle(float num, scene_node* rn);
	bool AdjZAngle(float num, scene_node* rn);
	bool AdjScale(float num, scene_node* rn);

	//Additional methods
	bool LookAt_XZ(float x, float z, scene_node*);
	bool MoveForward(float dist, scene_node*);
};
