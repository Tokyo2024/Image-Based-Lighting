#pragma once

#include "Shader.h"
#include <map>

class CTechnique
{
public:
	CTechnique(void);
	virtual ~CTechnique(void);

	virtual bool initTechniqueV() { return true; }

	void addShader(const std::string& vShaderSig, CShader* vShader);

	void updateStandShaderUniform(const std::string& vUniformName, boost::any vValue);
	void updateTextureShaderUniform(const std::string& vUniformName, int vValue);

	void enableShader(const std::string& vShaderSig);
	void disableShader();
	unsigned int getProgramID(const std::string& vShaderSig);

protected:
	std::map<std::string, CShader*> m_ShaderSet;

	CShader* m_CurrentShader;
};