#pragma once
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <GLM/glm.hpp>
#include <vector>
#include <boost/any.hpp>

enum EShaderType
{
	VERTEX_SHADER = 0,
	FRAGMENT_SHADER,
	GEOMETRY_SHADER,
	COMPUTE_SHADER
};

class CShader
{
public:
	CShader();
	~CShader();

	void addShader(const std::string& vShaderName, EShaderType vShaderType);
	void enableShader() const {glUseProgram(m_ProgramID);}
	void disableShader() const {glUseProgram(0);}
	unsigned int getProgramID() const { return m_ProgramID;}

	void updateStandShaderUniform(const std::string& vUniformName, boost::any vValue);
	void bindTextureShaderUniform(GLuint vTarget, GLuint vTex, const std::string& vUniformName, int32_t vUnit);

	void setVec3(const std::string &name, const glm::vec3 &value);
	void setTextureUniformValue(const char *name, unsigned int n);
	void setMat4UniformValue(const char *name, const glm::mat4 &mat);
	void setMat3UniformValue(const char *name, const glm::mat3 &mat);

	void setFloatUniformValue(const char *name, GLfloat v0);
	void setFloatUniformValue(const char *name, GLfloat v0, GLfloat v1);
	void setFloatUniformValue(const char *name, GLfloat v0, GLfloat v1, GLfloat v2);
	void setFloatUniformValue(const char *name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

	void setIntUniformValue(const char *name, GLint v0);
	void setIntUniformValue(const char *name, GLint v0, GLint v1);
	void setIntUniformValue(const char *name, GLint v0, GLint v1, GLint v2);
	void setIntUniformValue(const char *name, GLint v0, GLint v1, GLint v2, GLint v3);

private:
	const GLchar* const __readShaderFile(const std::string& vFileName);
	void __compileShader(GLuint& vShader);
	void __linkProgram(GLuint& vProgram);
	void __printUniformWarningInfo(const std::string& vUniform);

	GLuint m_ProgramID;
	std::vector<unsigned int> ShaderID;
};