#pragma once
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>

class CFBO
{
public:
	CFBO(void);
    ~CFBO(void);

	void createFBO(int vWidth, int vHeight);

private:
	GLuint m_FBO;
	GLuint m_RBO;
	GLuint m_Width;
	GLuint m_Height;
	std::vector<GLuint> m_TexBuffers;
};