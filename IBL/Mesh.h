#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <GL/glew.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "Tools.h"

struct SVertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct STexture {
	GLuint      TextureID;
	aiString    TexturePath;
	std::string TextureType;

};

struct SMaterial
{
	glm::vec3 Ambient;
	glm::vec3 Diffuse;
	glm::vec3 Specular;
};

class CMesh 
{
public:
	CMesh(const std::vector<SVertex>& vVerticeSet, const std::vector<GLuint>& vIndiceSet, const std::vector<STexture>& vTextureSet, SMaterial vMaterial)
	{
		m_VerticSet  = vVerticeSet;
		m_IndiceSet  = vIndiceSet;
		m_TextureSet = vTextureSet;
		m_Material   = vMaterial;
		__setupMesh();
	}

	void drawMesh(unsigned int vProgramID, GLuint vTextureUnit)
	{
 		GLuint DiffuseNum  = 1;
		GLuint SpecularNum = 1;
		for(GLuint i = vTextureUnit; i < m_TextureSet.size() + vTextureUnit; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, m_TextureSet[i - vTextureUnit].TextureID);
			std::stringstream Number;
			std::string TextureName = m_TextureSet[i - vTextureUnit].TextureType;
			if(TextureName == "texture_diffuse")
				Number << DiffuseNum++;
			else if(TextureName == "texture_specular")
				Number << SpecularNum++;
			glUniform1i(glGetUniformLocation(vProgramID, (TextureName + Number.str()).c_str()), i);
		}
		
		glUniform3fv(glGetUniformLocation(vProgramID, "ambientColor"), 1, &m_Material.Ambient[0]);
		glUniform3fv(glGetUniformLocation(vProgramID, "specularColor"), 1, &m_Material.Specular[0]);
		glUniform3fv(glGetUniformLocation(vProgramID, "diffuseColor"), 1, &m_Material.Diffuse[0]);

		glUniform1f(glGetUniformLocation(vProgramID, "material.shininess"), 16.0f);
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, m_IndiceSet.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		
		for (GLuint i = vTextureUnit; i < m_TextureSet.size() + vTextureUnit; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		} 
	}

	void fetchVertexPosition(std::vector<float>& voVertexSet) const
	{
		for (auto &Vertex : m_VerticSet)
		{
			voVertexSet.push_back(Vertex.Position.x);
			voVertexSet.push_back(Vertex.Position.y);
			voVertexSet.push_back(Vertex.Position.z);
		}
	}

	void fetchVertexIndex(std::vector<unsigned int>& voIndexSet) const
	{
		voIndexSet = m_IndiceSet;
	}

private:
	GLuint m_VAO, m_VBO, m_EBO;
	std::vector<SVertex>  m_VerticSet;
	std::vector<GLuint>   m_IndiceSet;
	std::vector<STexture> m_TextureSet;
	SMaterial             m_Material;

	void __setupMesh()
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glCheckError();
		glBufferData(GL_ARRAY_BUFFER, m_VerticSet.size() * sizeof(SVertex), &m_VerticSet[0], GL_STATIC_DRAW);  
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_IndiceSet.size() * sizeof(GLuint), &m_IndiceSet[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);	
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), (GLvoid*)0);
		glEnableVertexAttribArray(1);	
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), (GLvoid*)offsetof(SVertex, Normal));
		glEnableVertexAttribArray(2);	
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SVertex), (GLvoid*)offsetof(SVertex, TexCoords));
		glBindVertexArray(0);
	}
};