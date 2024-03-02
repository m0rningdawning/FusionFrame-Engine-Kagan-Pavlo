#include "Mesh.h"

FUSIONCORE::TextureObj::TextureObj()
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

FUSIONCORE::TextureObj::~TextureObj()
{
	this->ObjectBuffer.clean();
}

void FUSIONCORE::TextureObj::Draw(Camera2D& camera, GLuint shader, Texture2D& texture)
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

void FUSIONCORE::TextureObj::Draw(Camera3D& camera, GLuint shader, Texture2D& texture, std::function<void()> ShaderPreperations)
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

FUSIONCORE::Mesh::Mesh(std::vector<std::shared_ptr<Vertex>>& vertices_i, std::vector<unsigned int>& indices_i, std::vector<Texture2D>& textures_i)
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

	std::vector<Vertex> rawVertices;
	rawVertices.reserve(vertices.size());

	for (const auto& vertexPtr : vertices) {
		rawVertices.push_back(*vertexPtr);
	}

	ObjectBuffer.Bind();

	ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, rawVertices.size() * sizeof(FUSIONCORE::Vertex), &rawVertices[0], GL_STATIC_DRAW);
	ObjectBuffer.BindEBO();
	ObjectBuffer.BufferDataFill(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	ObjectBuffer.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)0);
	ObjectBuffer.AttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, Normal));
	ObjectBuffer.AttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, TexCoords));
	ObjectBuffer.AttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, Tangent));
	ObjectBuffer.AttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, Bitangent));
	ObjectBuffer.AttribIPointer(5, 4, GL_INT, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, m_BoneIDs));
	ObjectBuffer.AttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, m_Weights));

	ObjectBuffer.Unbind();
}

FUSIONCORE::Mesh::Mesh(std::vector<std::shared_ptr<Vertex>>& vertices_i, std::vector<unsigned int>& indices_i, std::vector<std::shared_ptr<Face>>& Faces, std::vector<Texture2D>& textures_i)
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

	std::vector<Vertex> rawVertices;
	rawVertices.reserve(vertices.size());

	for (const auto& vertexPtr : vertices) {
		rawVertices.push_back(*vertexPtr);
	}

	this->ConstructHalfEdges();

	ObjectBuffer.Bind();

	ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, rawVertices.size() * sizeof(FUSIONCORE::Vertex), &rawVertices[0], GL_STATIC_DRAW);
	ObjectBuffer.BindEBO();
	ObjectBuffer.BufferDataFill(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	ObjectBuffer.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)0);
	ObjectBuffer.AttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, Normal));
	ObjectBuffer.AttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, TexCoords));
	ObjectBuffer.AttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, Tangent));
	ObjectBuffer.AttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, Bitangent));
	ObjectBuffer.AttribIPointer(5, 4, GL_INT, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, m_BoneIDs));
	ObjectBuffer.AttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, m_Weights));

	ObjectBuffer.Unbind();
}

void FUSIONCORE::Mesh::Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations)
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

void FUSIONCORE::Mesh::Draw(Camera3D& camera, Shader& shader, Material material, std::function<void()>& ShaderPreperations)
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

void FUSIONCORE::Mesh::Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap, Material material, float EnvironmentAmbientAmount)
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
	glBindTexture(GL_TEXTURE_2D, FUSIONCORE::brdfLUT);
	shader.setInt("LUT", 3);

	shader.setBool("EnableIBL", true);
	shader.setFloat("ao", EnvironmentAmbientAmount);
	
	material.SetMaterialShader(shader);
	
	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	ObjectBuffer.UnbindVAO();
	glActiveTexture(GL_TEXTURE0);
}

void FUSIONCORE::Mesh::DrawDeferred(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap, Material material, float EnvironmentAmbientAmount)
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

	material.SetMaterialShader(shader);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	ObjectBuffer.UnbindVAO();
	glActiveTexture(GL_TEXTURE0);
}

void FUSIONCORE::Mesh::ConstructHalfEdges()
{
	for (auto& face : this->Faces)
	{
		auto& indices = face->Indices;

		std::pair<glm::vec3, glm::vec3> pair1 = std::make_pair(vertices[indices[0]]->Position, vertices[indices[1]]->Position);
		std::pair<glm::vec3, glm::vec3> pair2 = std::make_pair(vertices[indices[1]]->Position, vertices[indices[2]]->Position);
		std::pair<glm::vec3, glm::vec3> pair3 = std::make_pair(vertices[indices[2]]->Position, vertices[indices[0]]->Position);

		this->HalfEdges.emplace_back(std::make_shared<HalfEdge>());
		HalfEdgeMap[pair1] = HalfEdges.size() - 1;
		this->HalfEdges.emplace_back(std::make_shared<HalfEdge>());
		HalfEdgeMap[pair2] = HalfEdges.size() - 1;
		this->HalfEdges.emplace_back(std::make_shared<HalfEdge>());
		HalfEdgeMap[pair3] = HalfEdges.size() - 1;

		auto &edge1Ptr = HalfEdges[HalfEdgeMap[pair1]];
		auto &edge2Ptr = HalfEdges[HalfEdgeMap[pair2]];
		auto &edge3Ptr = HalfEdges[HalfEdgeMap[pair3]];

		edge1Ptr->StartingVertex = vertices[indices[0]].get();
		edge1Ptr->EndingVertex = vertices[indices[1]].get();

		edge2Ptr->StartingVertex = vertices[indices[1]].get();
		edge2Ptr->EndingVertex = vertices[indices[2]].get();

		edge3Ptr->StartingVertex = vertices[indices[2]].get();
		edge3Ptr->EndingVertex = vertices[indices[0]].get();

		edge1Ptr->NextHalfEdge = edge2Ptr.get();
		edge2Ptr->NextHalfEdge = edge3Ptr.get();
		edge3Ptr->NextHalfEdge = edge1Ptr.get();

		edge1Ptr->PrevHalfEdge = edge3Ptr.get();
		edge2Ptr->PrevHalfEdge = edge1Ptr.get();
		edge3Ptr->PrevHalfEdge = edge2Ptr.get();

		edge1Ptr->Face = face.get();
		edge2Ptr->Face = face.get();
		edge3Ptr->Face = face.get();

		face->halfEdge = edge1Ptr.get();
	}

	std::unordered_map<std::pair<glm::vec3, glm::vec3>, int, PairVec3Hash>::iterator itr;
	for (itr = HalfEdgeMap.begin(); itr != HalfEdgeMap.end(); itr++)
	{
		auto twinPair = std::make_pair(itr->first.second, itr->first.first);
		if (HalfEdgeMap.find(twinPair) == HalfEdgeMap.end())
		{
			HalfEdges[itr->second]->BoundryEdge = true;
		}
		else
		{
			HalfEdges[itr->second]->TwinHalfEdge = HalfEdges[HalfEdgeMap[twinPair]].get();
			HalfEdges[itr->second]->BoundryEdge = false;
		}
	}
}
void FUSIONCORE::Mesh::DrawImportedMaterial(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap, float EnvironmentAmbientAmount)
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
	glBindTexture(GL_TEXTURE_2D, FUSIONCORE::brdfLUT);
	shader.setInt("LUT", 3);

	shader.setBool("EnableIBL", true);
	shader.setFloat("ao", EnvironmentAmbientAmount);

	this->ImportedMaterial.SetMaterialShader(shader);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	ObjectBuffer.UnbindVAO();
	glActiveTexture(GL_TEXTURE0);

}

void FUSIONCORE::Mesh::ConstructMesh()
{
	std::vector<Vertex> rawVertices;
	rawVertices.reserve(vertices.size());

	for (const auto& vertexPtr : vertices) {
		rawVertices.push_back(*vertexPtr);
	}

	ObjectBuffer.Bind();

	ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, rawVertices.size() * sizeof(FUSIONCORE::Vertex), &rawVertices[0], GL_STATIC_DRAW);
	ObjectBuffer.BindEBO();
	ObjectBuffer.BufferDataFill(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	ObjectBuffer.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)0);
	ObjectBuffer.AttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, Normal));
	ObjectBuffer.AttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, TexCoords));
	ObjectBuffer.AttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, Tangent));
	ObjectBuffer.AttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONCORE::Vertex), (void*)offsetof(Vertex, Bitangent));

	ObjectBuffer.Unbind();
}




