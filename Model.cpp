#include "Model.h"

struct MODEL_CONSTANT_BUFFER
{
	XMMATRIX WorldViewProjection ;	// 64 bytes ( 4 x 4 = 16 floats x 4 bytes)
}; // TOTAL SIZE = 64 bytes


Model::Model(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11SamplerState* sampler, ID3D11ShaderResourceView* texture)
{
	m_pD3DDevice = device;
	m_pImmediateContext = context;
	m_pTexture0 = texture;
	m_pSampler0 = sampler;

	m_x = 0.0f;
	m_y = 0.0f;
	m_z = 15.0f;
	m_xangle = 0.0f;
	m_yangle = 0.0f;
	m_zangle = 0.0f;
	m_scale = 1.0f;
	m_dx = 0.0f;
	m_dz = 0.0f;
}

void Model::setTexture(ID3D11ShaderResourceView* tex)
{
	m_pTexture0 = tex;
}

void Model::setSampler(ID3D11SamplerState* sam)
{
	m_pSampler0 = sam;
}

void Model::CalculateModelCentrePoint()
{
	float mid_x = 0;
	float mid_y = 0;
	float mid_z = 0;
	float max_x = 0;
	float min_x = 0;
	for(int i=0; i<m_pObject->numverts; i++)
	{
		if (m_pObject->vertices[i].Pos.x > max_x)
		{
			max_x = m_pObject->vertices[i].Pos.x;
		}
		if (m_pObject->vertices[i].Pos.x < min_x)
		{
			min_x = m_pObject->vertices[i].Pos.x;
		}
	}
	float max_y = 0;
	float min_y = 0;
	for(int i=0; i<m_pObject->numverts; i++)
	{
		if (m_pObject->vertices[i].Pos.y > max_y)
		{
			max_y = m_pObject->vertices[i].Pos.y;
		}
		if (m_pObject->vertices[i].Pos.y < min_y)
		{
			min_y = m_pObject->vertices[i].Pos.y;
		}
	}
	float max_z = 0;
	float min_z = 0;
	for(int i=0; i<m_pObject->numverts; i++)
	{
		if (m_pObject->vertices[i].Pos.z > max_z)
		{
			max_z = m_pObject->vertices[i].Pos.z;
		}
		if (m_pObject->vertices[i].Pos.z < min_z)
		{
			min_z = m_pObject->vertices[i].Pos.z;
		}
	}
	mid_x = (min_x + max_x) / 2;
	mid_y = (min_y + max_y) / 2;
	mid_z = (min_z + max_z) / 2;

	m_bounding_sphere_centre_x = mid_x;
	m_bounding_sphere_centre_y = mid_y;
	m_bounding_sphere_centre_z = mid_z;
}

void Model::CalculateBoundingSphereRadius()
{
	float distance_squared;

	for(int i=0; i<m_pObject->numverts; i++)
	{
		distance_squared = (pow(m_bounding_sphere_centre_x - m_pObject->vertices[i].Pos.x, 2)) + (pow(m_bounding_sphere_centre_y - m_pObject->vertices[i].Pos.y, 2)) + (pow(m_bounding_sphere_centre_z - m_pObject->vertices[i].Pos.z, 2));

		if (distance_squared > m_bounding_sphere_radius)
		{
			m_bounding_sphere_radius = distance_squared;
		}
	}
	m_bounding_sphere_radius = sqrt(m_bounding_sphere_radius);
}

int Model::LoadObjModel(char* filename)
{
	m_pObject = new ObjFileModel(filename, m_pD3DDevice, m_pImmediateContext);

	if(m_pObject->filename == "FILE NOT LOADED") return S_FALSE;

	CalculateModelCentrePoint();
	CalculateBoundingSphereRadius();

	HRESULT hr = S_OK;

	// Create constant buffer
	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));

	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;	// Can use UpdateSubresource() to update
	constant_buffer_desc.ByteWidth = 64;	// MUST be a multiple of 16, calculate from CB struct
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;// Use as a constant buffer

	hr = m_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &m_pConstantBuffer);

	//Loading and compilation
	ID3DBlob *ModelVS, *ModelPS, *error;

	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelVS", "vs_4_0", 0, 0, 0, &ModelVS, &error, 0);

	if(error != 0) // check for shader compilation error
    {
        OutputDebugStringA((char*)error->GetBufferPointer());
        error->Release();
        if(FAILED(hr)) // don't fail if error is just a warning
        {
            return hr;
        };
    }

	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelPS", "ps_4_0", 0, 0, 0, &ModelPS, &error, 0);

	if(error != 0) // check for shader compilation error
    {
        OutputDebugStringA((char*)error->GetBufferPointer());
        error->Release();
        if(FAILED(hr)) // don't fail if error is just a warning
        {
            return hr;
        };
    }

	// Creation of the shaders
	hr = m_pD3DDevice->CreateVertexShader(ModelVS->GetBufferPointer(), ModelVS->GetBufferSize(), NULL, &m_pVShader);

	if(FAILED(hr))
	{
		return hr;
	}
	
	hr = m_pD3DDevice->CreatePixelShader(ModelPS->GetBufferPointer(), ModelPS->GetBufferSize(), NULL, &m_pPShader);

	if(FAILED(hr))
	{
		return hr;
	}

	// Create and set the input layout object
    D3D11_INPUT_ELEMENT_DESC iedesc[] =
    {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	  {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	  {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

	hr = m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), ModelVS->GetBufferPointer(), ModelVS->GetBufferSize(), &m_pInputLayout);

    if(FAILED(hr))
    {
        return hr;
    }

   return S_OK;

}

XMVECTOR Model::GetBoundingSphereWorldSpacePosition()
{
	XMMATRIX world;

	world = XMMatrixRotationX(XMConvertToRadians(m_xangle));
	world *= XMMatrixRotationY(XMConvertToRadians(m_yangle));
	world *= XMMatrixRotationZ(XMConvertToRadians(m_zangle));
	world *=XMMatrixScaling(m_scale, m_scale, m_scale);
	world *= XMMatrixTranslation(m_x,m_y,m_z);

	XMVECTOR offset;

	offset = XMVectorSet(m_bounding_sphere_centre_x, m_bounding_sphere_centre_y, m_bounding_sphere_centre_z, 0.0);

	offset =  XMVector3Transform(offset, world);

	return offset;
}

float Model::GetBoundingSphereRadius()
{
	 return (m_bounding_sphere_radius * m_scale);
}

bool Model::CheckCollision(Model* m)
{
	// Disable collisions with yourself
	if (m == this)
	{
		return false;
	}

	// Get the current and model bounding sphere positions 
	XMVECTOR current_pos = this->GetBoundingSphereWorldSpacePosition();  
	XMVECTOR model_pos = m->GetBoundingSphereWorldSpacePosition();
	
	// Access the coords of the current model
	float c_x = XMVectorGetX(current_pos);
	float c_y = XMVectorGetY(current_pos);
	float c_z = XMVectorGetZ(current_pos);

	// Access the coords of the model being passed in
	float mod_x = XMVectorGetX(model_pos);
	float mod_y = XMVectorGetY(model_pos);
	float mod_z = XMVectorGetZ(model_pos);

	// Pythagorean theorem to calculate the distance between the two models
	float dist_squared = (pow(c_x - mod_x,2)) + (pow(c_y - mod_y,2)) + (pow(c_z - mod_z,2));
	float d = sqrt(dist_squared);

	// Check to see if the distance is less than the sum of the radii
	if (d < (this->GetBoundingSphereRadius() + m->GetBoundingSphereRadius()))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Model::Draw(XMMATRIX* world, XMMATRIX* view, XMMATRIX* projection)
{
	//XMMATRIX world;

	//world = XMMatrixRotationX(XMConvertToRadians(m_xangle));
	//world *= XMMatrixRotationY(XMConvertToRadians(m_yangle));
	//world *= XMMatrixRotationZ(XMConvertToRadians(m_zangle));
	//world *=XMMatrixScaling(m_scale, m_scale, m_scale);
	//world *= XMMatrixTranslation(m_x,m_y,m_z);


	MODEL_CONSTANT_BUFFER model_cb_values;
	model_cb_values.WorldViewProjection = (*world)*(*view)*(*projection);

	// upload the new values for the constant buffer
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &model_cb_values, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);


	// Set the shader objects as active
    m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
    m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);

	// Set the Input layer as active
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	m_pImmediateContext->PSSetSamplers(0, 1, &m_pSampler0);
	m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTexture0);

	m_pObject->Draw();
}

void Model::LookAt_XZ(float x, float z)
{
	//calculates the distance between the model and camera (or passed in coords)
	m_dx = (x - m_x);
	m_dz = (z - m_z);

	//updates the angle accordingly 
	m_yangle = atan2(m_dx,m_dz) * (180.0 / XM_PI);
}

void Model::MoveForward(float dist)
{
	// Moves the model in the direction it is facing
	m_x += sin(m_yangle * (XM_PI/180.0)) * dist;
	m_z += cos(m_yangle * (XM_PI/180.0)) * dist;
}

Model::~Model()
{
	delete m_pObject;
	if(m_pInputLayout) m_pInputLayout->Release();
	if(m_pVShader) m_pVShader->Release();
	if(m_pPShader) m_pPShader->Release();
	if(m_pConstantBuffer) m_pConstantBuffer->Release();
}

