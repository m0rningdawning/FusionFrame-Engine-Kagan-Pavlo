#include "Mesh.h"

FUSIONOPENGL::TextureObj::TextureObj()
{
	ObjectBuffer.Bind();

	float quadVertices[] = {

		-1.0f,  1.0f,  0.0f,    1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
		 1.0f, -1.0f,  0.0f,    0.0f, 0.0f, 1.0f,   1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f,    1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
		 1.0f, -1.0f,  0.0f,    0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
		 1.0f,  1.0f,  0.0f,    1.0f, 1.0f, 0.0f,   1.0f, 1.0f
	};

	ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	ObjectBuffer.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	ObjectBuffer.AttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	ObjectBuffer.AttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	ObjectBuffer.Unbind();
}

FUSIONOPENGL::TextureObj::~TextureObj()
{
	this->ObjectBuffer.clean();
}

void FUSIONOPENGL::TextureObj::Draw(Camera2D& camera, GLuint shader, Texture2D& texture)
{
	glUseProgram(shader);
	ObjectBuffer.BindVAO();

	transformation.SetModelMatrixUniformLocation(shader, "model");

	camera.SetProjMatrixUniformLocation(shader, "proj");
	camera.SetRatioMatrixUniformLocation(shader, "ratioMat");

	texture.Bind(0, shader, "texture0");

	glDrawArrays(GL_TRIANGLES, 0, 6);

	ObjectBuffer.UnbindVAO();
	texture.Unbind();
	glUseProgram(0);
}

void FUSIONOPENGL::TextureObj::Draw(Camera3D& camera, GLuint shader, Texture2D& texture, std::function<void()> ShaderPreperations)
{
	glUseProgram(shader);
	ObjectBuffer.BindVAO();

	transformation.SetModelMatrixUniformLocation(shader, "model");

	camera.SetProjMatrixUniformLocation(shader, "proj");
	camera.SetRatioMatrixUniformLocation(shader, "ratioMat");
	camera.SetViewMatrixUniformLocation(shader, "view");

	ShaderPreperations();

	texture.Bind(0, shader, "texture0");

	glDrawArrays(GL_TRIANGLES, 0, 6);

	ObjectBuffer.UnbindVAO();
	texture.Unbind();
	glUseProgram(0);
}

FUSIONOPENGL::Mesh3D::Mesh3D(std::vector<FUSIONOPENGL::Vertex>& vertices_i, std::vector<unsigned int>& indices_i, std::vector<Texture2D>& textures_i)
{
	this->vertices.assign(vertices_i.begin(), vertices_i.end());
	this->indices.assign(indices_i.begin(), indices_i.end());
	this->textures.assign(textures_i.begin(), textures_i.end());

	for (size_t i = 0; i < textures.size(); i++)
	{
		if (textures[i].PbrMapType == "texture_diffuse")
		{
			ImportedMaterial.PushTextureMap(TEXTURE_DIFFUSE0, textures[i]);
		}
		else if (textures[i].PbrMapType == "texture_specular")
		{
			ImportedMaterial.PushTextureMap(TEXTURE_SPECULAR0, textures[i]);
		}
		else if (textures[i].PbrMapType == "texture_normal")
		{
			ImportedMaterial.PushTextureMap(TEXTURE_NORMAL0, textures[i]);
		}
		else if (textures[i].PbrMapType == "texture_metalic")
		{
			ImportedMaterial.PushTextureMap(TEXTURE_METALIC0, textures[i]);
		}
	}

	ObjectBuffer.Bind();

	ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, vertices.size() * sizeof(FUSIONOPENGL::Vertex), &vertices[0], GL_STATIC_DRAW);
	ObjectBuffer.BindEBO();
	ObjectBuffer.BufferDataFill(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	ObjectBuffer.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)0);
	ObjectBuffer.AttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, Normal));
	ObjectBuffer.AttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, TexCoords));
	ObjectBuffer.AttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, Tangent));
	ObjectBuffer.AttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, Bitangent));
	ObjectBuffer.AttribIPointer(5, 4, GL_INT, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, m_BoneIDs));
	ObjectBuffer.AttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, m_Weights));

	ObjectBuffer.Unbind();
}

FUSIONOPENGL::Mesh3D::Mesh3D(std::vector<FUSIONOPENGL::Vertex>& vertices_i, std::vector<unsigned int>& indices_i, std::vector<Face>& Faces, std::vector<Texture2D>& textures_i)
{
	this->vertices.assign(vertices_i.begin(), vertices_i.end());
	this->indices.assign(indices_i.begin(), indices_i.end());
	this->Faces.assign(Faces.begin(), Faces.end());
	this->textures.assign(textures_i.begin(), textures_i.end());

	for (size_t i = 0; i < textures.size(); i++)
	{
		if (textures[i].PbrMapType == "texture_diffuse")
		{
			ImportedMaterial.PushTextureMap(TEXTURE_DIFFUSE0, textures[i]);
		}
		else if (textures[i].PbrMapType == "texture_specular")
		{
			ImportedMaterial.PushTextureMap(TEXTURE_SPECULAR0, textures[i]);
		}
		else if (textures[i].PbrMapType == "texture_normal")
		{
			ImportedMaterial.PushTextureMap(TEXTURE_NORMAL0, textures[i]);
		}
		else if (textures[i].PbrMapType == "texture_metalic")
		{
			ImportedMaterial.PushTextureMap(TEXTURE_METALIC0, textures[i]);
		}
	}

	this->ConstructHalfEdges();

	ObjectBuffer.Bind();

	ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, vertices.size() * sizeof(FUSIONOPENGL::Vertex), &vertices[0], GL_STATIC_DRAW);
	ObjectBuffer.BindEBO();
	ObjectBuffer.BufferDataFill(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	ObjectBuffer.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)0);
	ObjectBuffer.AttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, Normal));
	ObjectBuffer.AttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, TexCoords));
	ObjectBuffer.AttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, Tangent));
	ObjectBuffer.AttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, Bitangent));
	ObjectBuffer.AttribIPointer(5, 4, GL_INT, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, m_BoneIDs));
	ObjectBuffer.AttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, m_Weights));

	ObjectBuffer.Unbind();
}

void FUSIONOPENGL::Mesh3D::Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations)
{
	shader.use();
	ObjectBuffer.BindVAO();
	ShaderPreperations();

	camera.SetProjMatrixUniformLocation(shader.GetID(), "proj");
	camera.SetRatioMatrixUniformLocation(shader.GetID(), "ratioMat");
	camera.SetViewMatrixUniformLocation(shader.GetID(), "view");
	shader.setVec3("CameraPos", camera.Position);
	shader.setFloat("FarPlane", camera.FarPlane);
	shader.setFloat("NearPlane", camera.NearPlane);
	shader.setInt("EnableIBL", (int)false);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	ObjectBuffer.UnbindVAO();
}

void FUSIONOPENGL::Mesh3D::Draw(Camera3D& camera, Shader& shader, Material material, std::function<void()>& ShaderPreperations)
{
	shader.use();
	ObjectBuffer.BindVAO();
	ShaderPreperations();

	camera.SetProjMatrixUniformLocation(shader.GetID(), "proj");
	camera.SetRatioMatrixUniformLocation(shader.GetID(), "ratioMat");
	camera.SetViewMatrixUniformLocation(shader.GetID(), "view");
	shader.setVec3("CameraPos", camera.Position);
	shader.setFloat("FarPlane", camera.FarPlane);
	shader.setFloat("NearPlane", camera.NearPlane);
	shader.setInt("EnableIBL", (int)false);
	material.SetMaterialShader(shader);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	ObjectBuffer.UnbindVAO();
}

void FUSIONOPENGL::Mesh3D::Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap, Material material, float EnvironmentAmbientAmount)
{
	shader.use();
	ObjectBuffer.BindVAO();
	ShaderPreperations();

	//LOG("Mesh: " << this->MeshName << " Texture Count: " << textures.size());

	camera.SetProjMatrixUniformLocation(shader.GetID(), "proj");
	camera.SetRatioMatrixUniformLocation(shader.GetID(), "ratioMat");
	camera.SetViewMatrixUniformLocation(shader.GetID(), "view");
	shader.setVec3("CameraPos", camera.Position);
	shader.setFloat("FarPlane", camera.FarPlane);
	shader.setFloat("NearPlane", camera.NearPlane);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.GetConvDiffCubeMap());
	shader.setInt("ConvDiffCubeMap", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.GetPreFilteredEnvMap());
	shader.setInt("prefilteredMap", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, FUSIONOPENGL::brdfLUT);
	shader.setInt("LUT", 3);

	shader.setBool("EnableIBL", true);
	shader.setFloat("ao", EnvironmentAmbientAmount);
	
	material.SetMaterialShader(shader);
	
	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	ObjectBuffer.UnbindVAO();
	glActiveTexture(GL_TEXTURE0);
}

void FUSIONOPENGL::Mesh3D::DrawDeferred(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap, Material material, float EnvironmentAmbientAmount)
{
	shader.use();
	ObjectBuffer.BindVAO();
	ShaderPreperations();

	//LOG("Mesh: " << this->MeshName << " Texture Count: " << textures.size());

	camera.SetProjMatrixUniformLocation(shader.GetID(), "proj");
	camera.SetRatioMatrixUniformLocation(shader.GetID(), "ratioMat");
	camera.SetViewMatrixUniformLocation(shader.GetID(), "view");
	shader.setVec3("CameraPos", camera.Position);
	shader.setFloat("FarPlane", camera.FarPlane);
	shader.setFloat("NearPlane", camera.NearPlane);

	/*glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.GetConvDiffCubeMap());
	shader.setInt("ConvDiffCubeMap", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.GetPreFilteredEnvMap());
	shader.setInt("prefilteredMap", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, FUSIONOPENGL::brdfLUT);
	shader.setInt("LUT", 3);

	shader.setBool("EnableIBL", true);
	shader.setFloat("ao", EnvironmentAmbientAmount);*/

	material.SetMaterialShader(shader);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	ObjectBuffer.UnbindVAO();
	glActiveTexture(GL_TEXTURE0);
}

void FUSIONOPENGL::Mesh3D::ConstructHalfEdges()
{  
	for (auto& face : this->Faces)
	{
		auto& indices = face.Indices;

		HalfEdge edge1;
		HalfEdge edge2;
		HalfEdge edge3;

		std::pair<int, int> pair1 = std::make_pair(indices[0], indices[1]);
		std::pair<int, int> pair2 = std::make_pair(indices[1], indices[2]);
		std::pair<int, int> pair3 = std::make_pair(indices[2], indices[0]);

		this->HalfEdges.push_back(edge1);
		EdgeMap[pair1] = HalfEdges.size() - 1;
		this->HalfEdges.push_back(edge2);
		EdgeMap[pair2] = HalfEdges.size() - 1;
		this->HalfEdges.push_back(edge3);
		EdgeMap[pair3] = HalfEdges.size() - 1;

		HalfEdge* edge1Ptr = &HalfEdges[EdgeMap[pair1]];
		HalfEdge* edge2Ptr = &HalfEdges[EdgeMap[pair2]];
		HalfEdge* edge3Ptr = &HalfEdges[EdgeMap[pair3]];

		edge1Ptr->StartingVertex = &vertices[indices[0]];
		edge1Ptr->EndingVertex = &vertices[indices[1]];

		edge2Ptr->StartingVertex = &vertices[indices[1]];
		edge2Ptr->EndingVertex = &vertices[indices[2]];

		edge3Ptr->StartingVertex = &vertices[indices[2]];
		edge3Ptr->EndingVertex = &vertices[indices[0]];

		edge1Ptr->NextHalfEdge = edge2Ptr;
		edge2Ptr->NextHalfEdge = edge3Ptr;
		edge3Ptr->NextHalfEdge = edge1Ptr;

		edge1Ptr->PrevHalfEdge = edge3Ptr;
		edge2Ptr->PrevHalfEdge = edge1Ptr;
		edge3Ptr->PrevHalfEdge = edge2Ptr;

		edge1Ptr->Face = &face;
		edge2Ptr->Face = &face;
		edge3Ptr->Face = &face;

		face.halfEdge = edge1Ptr;
	}

	int TwinCount = 1;
	std::unordered_map<std::pair<int, int>, int, PairHash>::iterator itr;
	for (itr = EdgeMap.begin(); itr != EdgeMap.end(); itr++)
	{
		auto twinPair = std::make_pair(itr->first.second, itr->first.first);
		if (EdgeMap.find(twinPair) == EdgeMap.end())
		{
			HalfEdges[itr->second].BoundryEdge = true;
		}
		else
		{
			LOG("FOUND TWIN: " << TwinCount);
			HalfEdges[itr->second].TwinHalfEdge = &HalfEdges[EdgeMap[twinPair]];
			HalfEdges[itr->second].BoundryEdge = false;
			TwinCount++;
		}
	}
}

void FUSIONOPENGL::Mesh3D::DrawImportedMaterial(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap, float EnvironmentAmbientAmount)
{
	shader.use();
	ObjectBuffer.BindVAO();
	ShaderPreperations();

	camera.SetProjMatrixUniformLocation(shader.GetID(), "proj");
	camera.SetRatioMatrixUniformLocation(shader.GetID(), "ratioMat");
	camera.SetViewMatrixUniformLocation(shader.GetID(), "view");
	shader.setVec3("CameraPos", camera.Position);
	shader.setFloat("FarPlane", camera.FarPlane);
	shader.setFloat("NearPlane", camera.NearPlane);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.GetConvDiffCubeMap());
	shader.setInt("ConvDiffCubeMap", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.GetPreFilteredEnvMap());
	shader.setInt("prefilteredMap", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, FUSIONOPENGL::brdfLUT);
	shader.setInt("LUT", 3);

	shader.setBool("EnableIBL", true);
	shader.setFloat("ao", EnvironmentAmbientAmount);

	this->ImportedMaterial.SetMaterialShader(shader);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	ObjectBuffer.UnbindVAO();
	glActiveTexture(GL_TEXTURE0);

}

void FUSIONOPENGL::Mesh3D::ConstructMesh()
{
	ObjectBuffer.Bind();

	ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, vertices.size() * sizeof(FUSIONOPENGL::Vertex), &vertices[0], GL_STATIC_DRAW);
	ObjectBuffer.BindEBO();
	ObjectBuffer.BufferDataFill(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	ObjectBuffer.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)0);
	ObjectBuffer.AttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, Normal));
	ObjectBuffer.AttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, TexCoords));
	ObjectBuffer.AttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, Tangent));
	ObjectBuffer.AttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, Bitangent));

	ObjectBuffer.Unbind();
}





