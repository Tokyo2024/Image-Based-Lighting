#include "FBO.h"
#include <iostream>

CFBO::CFBO(void)
{

}

CFBO::~CFBO(void)
{

}

//*********************************************************************************
//FUNCTION:
void CFBO::createFBO(int vWidth, int vHeight)
{
	glGenFramebuffers(1, &m_FBO);
	glGenRenderbuffers(1, &m_RBO);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, vWidth, vHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}