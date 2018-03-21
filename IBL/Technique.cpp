#include "Technique.h"
#include <boost/any.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

CTechnique::CTechnique(void) : m_CurrentShader(nullptr)
{
	
}

CTechnique::~CTechnique(void)
{
	for (auto iter : m_ShaderSet)
		delete iter.second;

	m_ShaderSet.clear();
}

//*********************************************************************************
//FUNCTION:
void CTechnique::addShader(const std::string& vShaderSig, CShader* vShader)
{
	_ASSERT(!vShaderSig.empty());
	_ASSERT(!(vShader == nullptr));

	auto ShaderSetIter = m_ShaderSet.find(vShaderSig);
	if (ShaderSetIter == m_ShaderSet.end())
	{
		m_ShaderSet.insert(std::make_pair(vShaderSig, vShader));
	}
	else
	{
		std::cout << "\nError: Shader Signature repetition!" << std::endl;
		_ASSERT(false);
	}
}

//*********************************************************************************
//FUNCTION:
void CTechnique::updateStandShaderUniform(const std::string& vUniformName, boost::any vValue)
{
	_ASSERT(!vUniformName.empty());
	if (m_CurrentShader)
	{
		_ASSERT(m_CurrentShader);
		const std::type_info& typeInfo = vValue.type();
		if (typeInfo == typeid(glm::mat4))
		{
			glm::mat4 mat = boost::any_cast<glm::mat4>(vValue);
			m_CurrentShader->setMat4UniformValue(vUniformName.c_str(), mat);
		}
		else if (typeInfo == typeid(glm::mat3))
		{
			glm::mat3 mat = boost::any_cast<glm::mat3>(vValue);
			m_CurrentShader->setMat3UniformValue(vUniformName.c_str(), mat);
		}
		else if (typeInfo == typeid(glm::vec4))
		{
			glm::vec4 v = boost::any_cast<glm::vec4>(vValue);
			m_CurrentShader->setFloatUniformValue(vUniformName.c_str(), v[0], v[1], v[2], v[3]);
		}
		else if (typeInfo == typeid(glm::vec3))
		{
			glm::vec3 v = boost::any_cast<glm::vec3>(vValue);
			m_CurrentShader->setFloatUniformValue(vUniformName.c_str(), v[0], v[1], v[2]);
		}
		else if (typeInfo == typeid(glm::vec2))
		{
			glm::vec2 v = boost::any_cast<glm::vec2>(vValue);
			m_CurrentShader->setFloatUniformValue(vUniformName.c_str(), v[0], v[1]);
		}
		else if (typeInfo == typeid(float))
		{
			float v = boost::any_cast<float>(vValue);
			m_CurrentShader->setFloatUniformValue(vUniformName.c_str(), v);
		}
		else if (typeInfo == typeid(double))
		{
			double v = boost::any_cast<double>(vValue);
			m_CurrentShader->setFloatUniformValue(vUniformName.c_str(), (GLfloat)v);
		}
		else if (typeInfo == typeid(bool))
		{
			bool v = boost::any_cast<bool>(vValue);
			m_CurrentShader->setIntUniformValue(vUniformName.c_str(), v);
		}
		else if (typeInfo == typeid(int))
		{
			int v = boost::any_cast<int>(vValue);
			m_CurrentShader->setIntUniformValue(vUniformName.c_str(), v);
		}
		else if (typeInfo == typeid(unsigned int))
		{
			unsigned int v = boost::any_cast<unsigned int>(vValue);
			m_CurrentShader->setIntUniformValue(vUniformName.c_str(), v);
		}
/*		else if (typeInfo == typeid(GLuint64))
		{
			GLuint64 v = boost::any_cast<GLuint64>(vValue);
			m_CurrentShader->__setHandleui64UniformValue(vUniformName.c_str(), v);
		} */ //ÔÝÊ±²»¼Ó
		else
		{
			std::cout << vUniformName << " type is not defined" << std::endl;
		}
	}
}

//*********************************************************************************
//FUNCTION:
void CTechnique::updateTextureShaderUniform(const std::string& vUniformName, int vValue)
{
	_ASSERT(!vUniformName.empty());
	if (m_CurrentShader)
	{
		m_CurrentShader->setTextureUniformValue(vUniformName.c_str(), vValue);
	}
}

//*********************************************************************************
//FUNCTION:
void CTechnique::enableShader(const std::string& vShaderSig)
{
	if (m_ShaderSet.find(vShaderSig) == m_ShaderSet.end()) return;

	m_CurrentShader = m_ShaderSet[vShaderSig];
	m_CurrentShader->enableShader();
}

//*********************************************************************************
//FUNCTION:
void CTechnique::disableShader()
{
	m_CurrentShader = nullptr;
	glUseProgram(0);
}

//*********************************************************************************
//FUNCTION:
unsigned int CTechnique::getProgramID(const std::string& vShaderSig)
{
	if (m_ShaderSet.find(vShaderSig) == m_ShaderSet.end()) return 0;
	m_CurrentShader = m_ShaderSet[vShaderSig];
	return m_CurrentShader->getProgramID();
}