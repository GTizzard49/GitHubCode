#include "scene_node.h"

scene_node::scene_node()
{
	m_p_model = NULL;

	m_x = 0.0f;
	m_y = 0.0f;
	m_z = 0.0f;
	m_xangle = 0.0f;
	m_yangle = 0.0f;
	m_zangle = 0.0f;
	m_scale = 1.0f;

	m_dx = 0.0f;
	m_dz = 0.0f;

}

XMVECTOR  scene_node::get_world_centre_position()
{ 
	return XMVectorSet(m_world_centre_x, 
			  m_world_centre_y, 
			  m_world_centre_z, 0.0);
}

void scene_node::update_collision_tree(XMMATRIX* world, float scale)
{
		// the local_world matrix will be used to calculate the local transformations for this node
	XMMATRIX local_world = XMMatrixIdentity() ;
	
	local_world = XMMatrixRotationX(XMConvertToRadians(m_xangle));	
	local_world *= XMMatrixRotationY(XMConvertToRadians(m_yangle));
	local_world *= XMMatrixRotationZ(XMConvertToRadians(m_zangle));

	local_world *= XMMatrixScaling(m_scale, m_scale, m_scale);

	local_world *= XMMatrixTranslation(m_x,m_y,m_z);

	// the local matrix is multiplied by the passed in world matrix that contains the concatenated
	// transformations of all parent nodes so that this nodes transformations are relative to those
	local_world *= *world;

	// calc the world space scale of this object, is needed to calculate the  
	// correct bounding sphere radius of an object in a scaled hierarchy
	m_world_scale = scale * m_scale;

	XMVECTOR v;
	if(m_p_model)
	{
		v = XMVectorSet(m_p_model->getBoundingSphere_x(), 
				m_p_model->getBoundingSphere_y(), 
				m_p_model->getBoundingSphere_z(), 0.0);
	}
	else v = XMVectorSet(0,0,0,0); // no model, default to 0

	// find and store world space bounding sphere centre
	v =  XMVector3Transform(v, local_world);
	m_world_centre_x = XMVectorGetX(v);
	m_world_centre_y = XMVectorGetY(v);
	m_world_centre_z = XMVectorGetZ(v);

	// traverse all child nodes, passing in the concatenated world matrix and scale
	for(int i = 0; i< m_children.size();i++)
	{
		m_children[i]->update_collision_tree(&local_world, m_world_scale);
	}

}

bool scene_node::check_collision(scene_node* compare_tree)
{
	return check_collision(compare_tree, this);
}


bool scene_node::check_collision(scene_node* compare_tree, scene_node* object_tree_root)
{
	// check to see if root of tree being compared is same as root node of object tree being checked
	// i.e. stop object node and children being checked against each other
	if(object_tree_root == compare_tree) return false;

	// only check for collisions if both nodes contain a model
	if(m_p_model && compare_tree->m_p_model) 
	{
		XMVECTOR v1 = get_world_centre_position();
		XMVECTOR v2 = compare_tree->get_world_centre_position();
		XMVECTOR vdiff = v1-v2;

		//XMVECTOR a = XMVector3Length(vdiff);
		float x1 = XMVectorGetX(v1);
		float x2 = XMVectorGetX(v2);
		float y1 = XMVectorGetY(v1);
		float y2 = XMVectorGetY(v2);
		float z1 = XMVectorGetZ(v1);
		float z2 = XMVectorGetZ(v2);

		float dx = x1 - x2;
		float dy = y1 - y2;
		float dz = z1 - z2;

		// check bounding sphere collision
		if(sqrt(dx*dx+dy*dy+dz*dz) < 
		  (compare_tree->m_p_model->GetBoundingSphereRadius() * compare_tree->m_world_scale)+ 
		   (this->m_p_model->GetBoundingSphereRadius() * m_world_scale))
		{
				 return true;
		}
	}
	
	// iterate through compared tree child nodes
	for(int i = 0; i< compare_tree->m_children.size();i++) 
	{
	  // check for collsion against all compared tree child nodes 
	  if(check_collision(compare_tree->m_children[i], object_tree_root) == true) return true; 
	}

	// iterate through composite object child nodes
	for(int i = 0; i< m_children.size();i++) 
	{
	  // check all the child nodes of the composite object against compared tree
	  if(m_children[i]->check_collision(compare_tree, object_tree_root) == true) return true; 
	}

	return false;
}


void scene_node::setModel(Model* mod)
{
	m_p_model = mod;
}

void scene_node::addChildNode(scene_node *n)
{
	m_children.push_back(n);
}

bool scene_node::detatchNode(scene_node *n)
{
	// traverse tree to find node to detatch
	 for(int i=0; i < m_children.size(); i++)
	 {
		 if (n == m_children[i])
		 {
			 m_children.erase(m_children.begin() + i);
			 return true;
		 }
		 if(m_children[i]->detatchNode(n) == true) return true;
	 }	
	 return false; // node not in this tree
}
void scene_node::execute(XMMATRIX *world, XMMATRIX* view, XMMATRIX* projection)
{
	// the local_world matrix will be used to calc the local transformations for this node
	XMMATRIX local_world = XMMatrixIdentity() ;
	
	local_world = XMMatrixRotationX(XMConvertToRadians(m_xangle));	
	local_world *= XMMatrixRotationY(XMConvertToRadians(m_yangle));
	local_world *= XMMatrixRotationZ(XMConvertToRadians(m_zangle));

	local_world *=XMMatrixScaling(m_scale, m_scale, m_scale);

	local_world *= XMMatrixTranslation(m_x,m_y,m_z);

	// the local matrix is multiplied by the passed in world matrix that contains the concatenated
	// transformations of all parent nodes so that this nodes transformations are relative to those
	local_world *= *world;

	// only draw if there is a model attached
	if(m_p_model) m_p_model->Draw(&local_world, view, projection); 

	// traverse all child nodes, passing in the concatenated world matrix
	for(int i = 0; i< m_children.size();i++)
	{
		m_children[i]->execute(&local_world, view, projection);
	}
}

bool scene_node::AdjXPos(float num, scene_node* root_node)		
{
	float old_x = m_x;	// save current state 
	m_x += num;		// update state

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);
		
	// check for collision of this node (and children) against all other nodes
	if(check_collision(root_node) == true) 
	{
		// if collision restore state
		m_x = old_x; 
		
		return true;
	}		
	
	return false;		
}

bool scene_node::AdjYPos(float num, scene_node* root_node)		
{
	float old_y = m_y;	// save current state 
	m_y += num;		// update state

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);
		
	// check for collision of this node (and children) against all other nodes
	if(check_collision(root_node) == true) 
	{
		// if collision restore state
		m_y = old_y; 
		
		return true;
	}		
	
	return false;		
}

bool scene_node::AdjZPos(float num, scene_node* root_node)		
{
	float old_z = m_z;	// save current state 
	m_z += num;		// update state

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);
		
	// check for collision of this node (and children) against all other nodes
	if(check_collision(root_node) == true) 
	{
		// if collision restore state
		m_z = old_z; 
		
		return true;
	}		
	
	return false;		
}

bool scene_node::AdjXAngle(float num, scene_node* root_node)		
{
	float old_xangle = m_xangle;	// save current state 
	m_xangle += num;		// update state

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);
		
	// check for collision of this node (and children) against all other nodes
	if(check_collision(root_node) == true) 
	{
		// if collision restore state
		m_xangle = old_xangle; 
		
		return true;
	}		
	
	return false;		
}
bool scene_node::AdjYAngle(float num, scene_node* root_node)		
{
	float old_yangle = m_yangle;	// save current state 
	m_yangle += num;		// update state

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);
		
	// check for collision of this node (and children) against all other nodes
	if(check_collision(root_node) == true) 
	{
		// if collision restore state
		m_yangle = old_yangle; 
		
		return true;
	}		
	
	return false;		
}

bool scene_node::AdjZAngle(float num, scene_node* root_node)		
{
	float old_zangle = m_zangle;	// save current state 
	m_zangle += num;		// update state

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);
		
	// check for collision of this node (and children) against all other nodes
	if(check_collision(root_node) == true) 
	{
		// if collision restore state
		m_zangle = old_zangle; 
		
		return true;
	}		
	
	return false;		
}

bool scene_node::AdjScale(float num, scene_node* root_node)		
{
	float old_scale = m_scale;	// save current state 
	m_scale += num;		// update state

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);
		
	// check for collision of this node (and children) against all other nodes
	if(check_collision(root_node) == true) 
	{
		// if collision restore state
		m_scale = old_scale; 
		
		return true;
	}		
	
	return false;		
}

bool scene_node::LookAt_XZ(float x, float z, scene_node* root_node)		
{
	
	float old_dx = m_dx;
	float old_dz = m_dz;
	float old_yangle = m_yangle;

	//calculates the distance between the model and camera (or passed in coords)
	m_dx -=x;
	m_dz -=z;
	//updates the angle accordingly 
	m_yangle = atan2(m_dx,m_dz) * (180.0 / XM_PI);

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);
		
	// check for collision of this node (and children) against all other nodes
	if(check_collision(root_node) == true) 
	{
		// if collision restore state
		m_dx = old_dx;
		m_dz = old_dz;
		m_yangle = old_yangle;
		
		return true;
	}		
	
	return false;		
}

bool scene_node::MoveForward(float dist, scene_node* root_node)
{
		
	float old_x = m_x;
	float old_z = m_z;
	float old_yangle = m_yangle;

	// Moves the model in the direction it is facing
	m_x += sin(m_yangle * (XM_PI/180.0)) * dist;
	m_z += cos(m_yangle * (XM_PI/180.0)) * dist;

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);
		
	// check for collision of this node (and children) against all other nodes
	if(check_collision(root_node) == true) 
	{
		// if collision restore state
		m_x = old_x;
		m_z = old_z;
		m_yangle = old_yangle;
		
		return true;
	}		
	
	return false;

}