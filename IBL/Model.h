#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"
#include "Mesh.h"


class CDrawModel 
{
public:
	CDrawModel(const std::string& vPath);
	~CDrawModel();
	void renderModel(unsigned int vProgramID, GLuint vTextureUnit);
	void fetchModelVertexPosition(std::vector<float>& voVertexPosition) const;
	void fetchModelVertexIndex(std::vector<unsigned int>& voVertexIndex) const;

private:
	std::vector<CMesh> m_MesheSet;
	std::string m_Directory;
	std::vector<STexture> m_TextureLoadedSet;	
	SMaterial m_Material;

	void __loadModel(const std::string& vPath);
	void __processNode(aiNode* vNode, const aiScene* vScene);
	CMesh __processMesh(aiMesh* vMesh, const aiScene* vScene);
	void __LoadMaterial(aiMaterial* vMaterial, aiTextureType vTextureType, SMaterial &voMaterial);
	std::vector<STexture> __loadMaterialTextures(aiMaterial* vMaterial, aiTextureType vTextureType, const std::string& vTypeName);
	GLint __generateTextureFromFile(const std::string& vPath, const std::string& vDictionary);
};





