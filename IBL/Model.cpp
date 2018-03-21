#include "Model.h"

CDrawModel::CDrawModel(const std::string& vPath)
{
	_ASSERT(vPath != "");

	__loadModel(vPath);
}

CDrawModel::~CDrawModel()
{

}

//*************************************************************
//Function:
void CDrawModel::renderModel(unsigned int vProgramID, GLuint vTextureUnit)
{
	for (auto& Mesh : m_MesheSet)
	{
		Mesh.drawMesh(vProgramID, vTextureUnit);
	}
}

//*************************************************************
//Function:
void CDrawModel::fetchModelVertexPosition(std::vector<float>& voVertexPosition) const
{
	std::vector<float> VertexPosition;
	for (auto& Mesh : m_MesheSet)
	{
		Mesh.fetchVertexPosition(voVertexPosition);
		voVertexPosition.insert(voVertexPosition.end(), VertexPosition.begin(), VertexPosition.end());
	}
}

//*************************************************************
//Function:
void CDrawModel::fetchModelVertexIndex(std::vector<unsigned int>& voVertexIndex) const
{
	std::vector<unsigned int> VertexIndex;
	for (auto& Mesh : m_MesheSet)
	{
		Mesh.fetchVertexIndex(VertexIndex);
		voVertexIndex.insert(voVertexIndex.end(), VertexIndex.begin(), VertexIndex.end());
	}
}

//*************************************************************
//Function:
void CDrawModel::__loadModel(const std::string& vPath)
{
	_ASSERT(vPath != "");

	Assimp::Importer Import;
	//aiProcess_Triangulate ensure the model are loaded by triangle
	const aiScene* Scene = Import.ReadFile(vPath, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!Scene || Scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP:: " << Import.GetErrorString() << std::endl;
		return;
	}
	m_Directory = vPath.substr(0, vPath.find_last_of('/'));
	__processNode(Scene->mRootNode, Scene);
}

//*************************************************************
//Function:
void CDrawModel::__processNode(aiNode* vNode, const aiScene* vScene)
{
	_ASSERT(vNode && vScene);

	for (GLuint i = 0; i < vNode->mNumMeshes; i++)
	{
		aiMesh* pMesh = vScene->mMeshes[vNode->mMeshes[i]];
		_ASSERT(pMesh);
		m_MesheSet.push_back(__processMesh(pMesh, vScene));
	}

	for (GLuint i = 0; i < vNode->mNumChildren; i++)
	{
		__processNode(vNode->mChildren[i], vScene);
	}
}

//*************************************************************
//Function:
CMesh CDrawModel::__processMesh(aiMesh* vMesh, const aiScene* vScene)
{
	_ASSERT(vMesh && vScene);

	std::vector<SVertex>  VerticesSet;
	std::vector<GLuint>   IndicesSet;
	std::vector<STexture> TexturesSet;

	for (GLuint i = 0; i < vMesh->mNumVertices; i++)
	{
		SVertex Vertex;
		Vertex.Position = glm::vec3(vMesh->mVertices[i].x, vMesh->mVertices[i].y, vMesh->mVertices[i].z);
		Vertex.Normal   = glm::vec3(vMesh->mNormals[i].x, vMesh->mNormals[i].y, vMesh->mNormals[i].z);
		if (vMesh->mTextureCoords[0])
		{
			Vertex.TexCoords = glm::vec2(vMesh->mTextureCoords[0][i].x, vMesh->mTextureCoords[0][i].y);
		}
		else
			Vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		VerticesSet.push_back(Vertex);
	}

	for (GLuint i = 0; i < vMesh->mNumFaces; i++)
	{
		aiFace Face = vMesh->mFaces[i];
		for (GLuint j = 0; j < Face.mNumIndices; j++)
			IndicesSet.push_back(Face.mIndices[j]);
	}

	SMaterial Material;
	if (vMesh->mMaterialIndex >= 0)
	{
		aiMaterial* pMaterial = vScene->mMaterials[vMesh->mMaterialIndex];
		_ASSERT(pMaterial);
		std::vector<STexture> DiffuseTextureSet = __loadMaterialTextures(pMaterial, aiTextureType_DIFFUSE, "texture_diffuse");
		TexturesSet.insert(TexturesSet.end(), DiffuseTextureSet.begin(), DiffuseTextureSet.end());

		std::vector<STexture> SpecularTextureSet = __loadMaterialTextures(pMaterial, aiTextureType_SPECULAR, "texture_specular");
		TexturesSet.insert(TexturesSet.end(), SpecularTextureSet.begin(), SpecularTextureSet.end());

		__LoadMaterial(pMaterial, aiTextureType_DIFFUSE, Material);
		__LoadMaterial(pMaterial, aiTextureType_AMBIENT, Material);
		__LoadMaterial(pMaterial, aiTextureType_SPECULAR, Material);
	}

	return CMesh(VerticesSet, IndicesSet, TexturesSet, Material);
}

//*********************************************************************************
//FUNCTION:
void CDrawModel::__LoadMaterial(aiMaterial* vMaterial, aiTextureType vTextureType, SMaterial &voMaterial)
{
	aiColor4D color(0.f, 0.f, 0.f, 0.f);
	switch (vTextureType)
	{
	case aiTextureType_DIFFUSE:
		aiGetMaterialColor(vMaterial, AI_MATKEY_COLOR_DIFFUSE, &color);
		if (color.r < 0 || color.g < 0 || color.b < 0)
			color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);
		voMaterial.Diffuse = glm::vec3(color.r, color.g, color.b);
		break;
	case aiTextureType_SPECULAR:
		aiGetMaterialColor(vMaterial, AI_MATKEY_COLOR_SPECULAR, &color);
		if (color.r < 0 || color.g < 0 || color.b < 0)
			color = aiColor4D(0.0f, 0.0f, 0.0f, 1.0f);
		voMaterial.Specular = glm::vec3(color.r, color.g, color.b);
		break;
	case aiTextureType_AMBIENT:
		aiGetMaterialColor(vMaterial, AI_MATKEY_COLOR_AMBIENT, &color);
		if (color.r < 0 || color.g < 0 || color.b < 0 )
			color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);
		voMaterial.Ambient = glm::vec3(color.r, color.g, color.b);
		break;
	default:
		_ASSERT(0);
		break;
	}
}

//*************************************************************
//Function:
std::vector<STexture> CDrawModel::__loadMaterialTextures(aiMaterial* vMaterial, aiTextureType vTextureType, const std::string& vTypeName)
{
	_ASSERT(vMaterial && (vTypeName != ""));

	std::vector<STexture> TextureSet;
	for (GLuint i = 0; i < vMaterial->GetTextureCount(vTextureType); i++)
	{
		aiString TexturePath;
		vMaterial->GetTexture(vTextureType, i, &TexturePath);
		GLboolean SkipFlag = false;
		for (GLuint j = 0; j < m_TextureLoadedSet.size(); j++)
		{
			if (std::strcmp(m_TextureLoadedSet[j].TexturePath.C_Str(), TexturePath.C_Str()) == 0)
			{
				TextureSet.push_back(m_TextureLoadedSet[j]);
				SkipFlag = true;
				break;
			}
		}
		if (!SkipFlag)
		{
			STexture Texture;
			Texture.TextureID = __generateTextureFromFile(TexturePath.C_Str(), m_Directory);
			Texture.TextureType = vTypeName;
			Texture.TexturePath = TexturePath;
			TextureSet.push_back(Texture);
			m_TextureLoadedSet.push_back(Texture);
		}
	}
	return TextureSet;
}

//*************************************************************
//Function:
GLint CDrawModel::__generateTextureFromFile(const std::string& vPath, const std::string& vDictionary)
{
	_ASSERT((vPath != "") && (vDictionary != ""));

	std::string FileName = vPath;
	FileName = vDictionary + '/' + FileName;
	GLuint TextureID;
	glGenTextures(1, &TextureID);
	int Width, Height, Channels;
	unsigned char* pImage = stbi_load(FileName.c_str(), &Width, &Height, &Channels, 0);
	_ASSERT(pImage);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImage);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(pImage);
	return TextureID;
}
