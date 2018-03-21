#include <iostream>
#include <string>
#include <random>
#include "IBLRendering.h"
#include "Common.h"
#include "Texture.h"
#include "stb_image.h"
#include "Tools.h"

CIBLRendering::CIBLRendering() : m_pWindow(nullptr), m_pDrawModel(nullptr), m_pEffect(nullptr),  m_pDrawModelplane(nullptr)
{
	glm::dvec3 CameraPosition = glm::dvec3(0.0, 0.0, 2.7);
	glm::dvec3 CameraUp       = glm::dvec3(0.0, 1.0, 0.0);
	glm::dvec3 CameraFront    = glm::dvec3(0.0, 1.0, 1.0);
	GLfloat Yaw   = -90.0f;
	GLfloat Pitch = 0.0f;

	m_pCamera          = new CCamera(glm::dvec2(SCR_WIDTH / 2, SCR_HEIGHT / 2), CameraPosition, CameraUp, CameraFront, Yaw, Pitch);
	m_ProjectionMatrix = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	m_ModelMatrix      = glm::translate(m_ModelMatrix, glm::vec3(-1.0, -8.5, 0));
	m_ModelMatrix      = glm::scale(m_ModelMatrix, glm::vec3(0.22f, 0.22f, 0.22f));
	m_ModelPlaneMatrix = glm::translate(m_ModelPlaneMatrix, glm::vec3(0, -5.0, 0));
	m_ModelPlaneMatrix = glm::scale(m_ModelPlaneMatrix, glm::vec3(110.05f, 110.05f, 110.05f));
}

CIBLRendering::~CIBLRendering()
{
	__destory();
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::init()
{
	__initGLFW("IBL");
	__debuggingTest();
	__initShaders();
	__initCubemapMatrix();

	__initGBuffer();
	__initNoiseTextureAndSSAOKernel();

	__initBuffers();
	__initTexture();
	__initModel();
	__equirectangular2CubemapPass();
	__createIrradianceCubemapPass();
	__createBRDFIntegrationTexPass();
	__createPrefilterCubeMapPass();
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::draw()
{
	GLfloat CurrentTime;
	GLfloat LastTime = 0.0f;
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(m_pWindow))
	{
		CurrentTime = (GLfloat)glfwGetTime();
		m_pCamera->setFrameTime(CurrentTime - LastTime);
		LastTime = CurrentTime;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();

		m_RGB.r = (float)(glm::sin(CurrentTime * 0.5) + 1.0f);
		m_RGB.g = (float)(glm::cos(CurrentTime * 0.5) + 1.0f);
		m_RGB.b = (float)(glm::abs(glm::sin(CurrentTime* 0.5) + glm::cos(CurrentTime* 0.5)) * glm::sqrt(2));

		__GeometryPass(); 
		__SSAOPass();
		__SSAOBlurPass();
//		__drawScreenQuadPass();
		__pbrIntegratedPass();
		__skyboxBackgroundPass();

		_ASSERTE(glGetError() == GL_NO_ERROR);
		glfwSwapBuffers(m_pWindow);
		glfwPollEvents();
	}
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__initGLFW(const std::string& vWindowTitle)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // comment this line in a release build! 

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
	m_pWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, vWindowTitle.c_str(), nullptr, nullptr);
	_ASSERTE(m_pWindow);
	glfwMakeContextCurrent(m_pWindow);

	_ASSERTE(m_pCamera);
	m_pCamera->activeCamera(m_pWindow);
	glewInit();
	glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__debuggingTest()
{
	GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__initShaders()
{
	m_pEffect = new CIBLTechnique;
	m_pEffect->initTechniqueV();
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__initTexture()
{
	m_EnvCubemap1     = loadCubemap(faces1);
	m_EnvCubemap      = setupCubemap(512, 512, false);
	m_IrradianceMap   = setupCubemap(32, 32, false);
	m_PrefilterMap    = setupCubemap(128, 128, true);
	//Model/zhentou/zhentou4.obj 枕头 对应  1.hdr沙发
	m_EnvironmentTex  = loadTexture("images/1.hdr"); //3是桌子，11是过道，12花房，适合花瓶 
	m_BRDFIntegrationTex = setupTexture(512, 512);

	//// load PBR material textures
	//// --------------------------
	//// rusted iron
	//ironAlbedoMap    = loadTexture("images/pbr/rusted_iron/albedo.png");
	//ironNormalMap    = loadTexture("images/pbr/rusted_iron/normal.png");
	//ironMetallicMap  = loadTexture("images/pbr/rusted_iron/metallic.png");
	//ironRoughnessMap = loadTexture("images/pbr/rusted_iron/roughness.png");
	//ironAOMap        = loadTexture("images/pbr/rusted_iron/ao.png");

	//// gold
	//goldAlbedoMap    = loadTexture("images/pbr/gold/albedo.png");
	//goldNormalMap    = loadTexture("images/pbr/gold/normal.png");
	//goldMetallicMap  = loadTexture("images/pbr/gold/metallic.png");
	//goldRoughnessMap = loadTexture("images/pbr/gold/roughness.png");
	//goldAOMap        = loadTexture("images/pbr/gold/ao.png");

	//// plastic
	//plasticAlbedoMap    = loadTexture("images/pbr/plastic/albedo.png");
	//plasticNormalMap    = loadTexture("images/pbr/plastic/normal.png");
	//plasticMetallicMap  = loadTexture("images/pbr/plastic/metallic.png");
	//plasticRoughnessMap = loadTexture("images/pbr/plastic/roughness.png");
	//plasticAOMap        = loadTexture("images/pbr/plastic/ao.png");

	//// wall
	//wallAlbedoMap    = loadTexture("images/pbr/wall/albedo.png");
	//wallNormalMap    = loadTexture("images/pbr/wall/normal.png");
	//wallMetallicMap  = loadTexture("images/pbr/wall/metallic.png");
	//wallRoughnessMap = loadTexture("images/pbr/wall/roughness.png");
	//wallAOMap        = loadTexture("images/pbr/wall/ao.png");
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__initModel()
{
	m_pDrawModel      = new CDrawModel("Model/zhentou/zhentou4.obj"); //chairs/chairs   zhentou/zhentou1  nanosuit/nanosuit AOModle/aomodle
	m_pDrawModelplane = new CDrawModel("Model/plane/plane.obj");
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__initCubemapMatrix()
{
	m_CaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	m_CaptureViews[0] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	m_CaptureViews[1] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	m_CaptureViews[2] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	m_CaptureViews[3] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	m_CaptureViews[4] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	m_CaptureViews[5] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__initBuffers()
{
	glGenFramebuffers(1, &m_CaptureFBO);
	glGenRenderbuffers(1, &m_CaptureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_CaptureRBO);
	
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &m_SSAOFBO);  
	glGenFramebuffers(1, &m_SSAOBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);
	glGenTextures(1, &m_SSAOColorBuffer);
	glBindTexture(GL_TEXTURE_2D, m_SSAOColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;
	// - and blur stage
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurFBO);
	glGenTextures(1, &m_SSAOColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, m_SSAOColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__initGBuffer()
{
	glGenFramebuffers(1, &m_gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
	 
	glGenTextures(1, &m_gPositionDepth);
	glBindTexture(GL_TEXTURE_2D, m_gPositionDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPositionDepth, 0);
	 
	glGenTextures(1, &m_gNormal);
	glBindTexture(GL_TEXTURE_2D, m_gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormal, 0);
	 
	glGenTextures(1, &m_gAlbedo);
	glBindTexture(GL_TEXTURE_2D, m_gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gAlbedo, 0);
	// - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
	// - Create and attach depth buffer (renderbuffer)
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// - Finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "GBuffer Framebuffer not complete!" << std::endl;
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__initNoiseTextureAndSSAOKernel()
{
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;
	
	for (GLuint i = 0; i < 64; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		GLfloat scale = GLfloat(i / 64.0);

		// Scale samples s.t. they're more aligned to center of kernel
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		m_SSAOKernel.push_back(sample);
	}

	std::vector<glm::vec3> ssaoNoise;
	for (GLuint i = 0; i < 16; i++)
	{
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}
	glGenTextures(1, &m_NoiseTexture);
	glBindTexture(GL_TEXTURE_2D, m_NoiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__equirectangular2CubemapPass()
{
	m_pEffect->enableShader("Equirectangular2CubemapPass");
	m_pEffect->updateTextureShaderUniform("equirectangularMap", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_EnvironmentTex);
	glCheckError();

	m_pEffect->updateStandShaderUniform("projection", m_CaptureProjection);
	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		m_pEffect->updateStandShaderUniform("view", m_CaptureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_EnvCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_pEffect->disableShader();
	glBindTexture(GL_TEXTURE_2D, 0);
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__createIrradianceCubemapPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	m_pEffect->enableShader("IrradianceCubemapPass");
	m_pEffect->updateTextureShaderUniform("environmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);

	m_pEffect->updateStandShaderUniform("projection", m_CaptureProjection);
	glViewport(0, 0, 32, 32); 

	for (unsigned int i = 0; i < 6; ++i)
	{
		m_pEffect->updateStandShaderUniform("view", m_CaptureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_IrradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	m_pEffect->disableShader();
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__createBRDFIntegrationTexPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BRDFIntegrationTex, 0);

	glViewport(0, 0, 512, 512);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	m_pEffect->enableShader("BRDFIntegrationPass");
	renderCreenQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	m_pEffect->disableShader();
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__createPrefilterCubeMapPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 128, 128);

	m_pEffect->enableShader("PrefilterCubemapPass");
	m_pEffect->updateTextureShaderUniform("environmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
	m_pEffect->updateStandShaderUniform("projection", m_CaptureProjection);
	glViewport(0, 0, 128, 128);

	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth  = (unsigned int)(128 * std::pow(0.5, mip));
		unsigned int mipHeight = (unsigned int)(128 * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		m_pEffect->updateStandShaderUniform("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			m_pEffect->updateStandShaderUniform("view", m_CaptureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_PrefilterMap, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	m_pEffect->disableShader();
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__pbrIntegratedPass()
{
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_BRDFIntegrationTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_SSAOColorBufferBlur);

	m_pEffect->enableShader("PBRModelIntegratedPass");
	m_pEffect->updateTextureShaderUniform("irradianceMap", 0);
	m_pEffect->updateTextureShaderUniform("prefilterMap", 1);
	m_pEffect->updateTextureShaderUniform("brdfLUT", 2);
	m_pEffect->updateTextureShaderUniform("uSSAOTexture", 3);
	m_pEffect->updateStandShaderUniform("model", m_ModelMatrix);
	m_pEffect->updateStandShaderUniform("view", m_pCamera->getViewMatrix());
	m_pEffect->updateStandShaderUniform("projection", m_ProjectionMatrix);
	m_pEffect->updateStandShaderUniform("camPos", glm::vec3(m_pCamera->getCameraPosition()));
	m_pEffect->updateStandShaderUniform("ChangeRGB", m_RGB);
	m_pDrawModel->renderModel(m_pEffect->getProgramID("PBRModelIntegratedPass"), 4);

	/*m_pEffect->updateStandShaderUniform("model", m_ModelPlaneMatrix);
	m_pDrawModelplane->renderModel(m_pEffect->getProgramID("SSAOGeometryPass"), 10);*/

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	m_pEffect->disableShader();
	

	//m_pEffect->enableShader("PBRIntegratedPass");
	//m_pEffect->updateTextureShaderUniform("irradianceMap", 0);
	//m_pEffect->updateTextureShaderUniform("prefilterMap", 1);
	//m_pEffect->updateTextureShaderUniform("brdfLUT", 2);
	//m_pEffect->updateTextureShaderUniform("uSSAOTexture", 3);
	////1.rusted iron
	//glActiveTexture(GL_TEXTURE4);
	//glBindTexture(GL_TEXTURE_2D, ironAlbedoMap);
	//glActiveTexture(GL_TEXTURE5);
	//glBindTexture(GL_TEXTURE_2D, ironNormalMap);
	//glActiveTexture(GL_TEXTURE6);
	//glBindTexture(GL_TEXTURE_2D, ironMetallicMap);
	//glActiveTexture(GL_TEXTURE7);
	//glBindTexture(GL_TEXTURE_2D, ironRoughnessMap);
	//glActiveTexture(GL_TEXTURE8);
	//glBindTexture(GL_TEXTURE_2D, ironAOMap);
	//m_pEffect->updateTextureShaderUniform("albedoMap", 4);
	//m_pEffect->updateTextureShaderUniform("normalMap", 5);
	//m_pEffect->updateTextureShaderUniform("metallicMap", 6);
	//m_pEffect->updateTextureShaderUniform("roughnessMap", 7);
	//m_pEffect->updateTextureShaderUniform("aoMap", 8);
	//m_ModelMatrix = glm::mat4();
	//m_ModelMatrix = glm::scale(m_ModelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	//m_ModelMatrix = glm::translate(m_ModelMatrix, glm::vec3(-3.0, 0.0f, 3.0));
	//m_pEffect->updateStandShaderUniform("model", m_ModelMatrix);
	//m_pEffect->updateStandShaderUniform("view", m_pCamera->getViewMatrix());
	//m_pEffect->updateStandShaderUniform("projection", m_ProjectionMatrix);
	//m_pEffect->updateStandShaderUniform("camPos", glm::vec3(m_pCamera->getCameraPosition()));
	//renderSphere();

	////2.gold
	//glActiveTexture(GL_TEXTURE4);
	//glBindTexture(GL_TEXTURE_2D, goldAlbedoMap);
	//glActiveTexture(GL_TEXTURE5);
	//glBindTexture(GL_TEXTURE_2D, goldNormalMap);
	//glActiveTexture(GL_TEXTURE6);
	//glBindTexture(GL_TEXTURE_2D, goldMetallicMap);
	//glActiveTexture(GL_TEXTURE7);
	//glBindTexture(GL_TEXTURE_2D, goldRoughnessMap);
	//glActiveTexture(GL_TEXTURE8);
	//glBindTexture(GL_TEXTURE_2D, goldAOMap);
	//m_pEffect->updateTextureShaderUniform("albedoMap", 4);
	//m_pEffect->updateTextureShaderUniform("normalMap", 5);
	//m_pEffect->updateTextureShaderUniform("metallicMap", 6);
	//m_pEffect->updateTextureShaderUniform("roughnessMap", 7);
	//m_pEffect->updateTextureShaderUniform("aoMap", 8);
	//m_ModelMatrix = glm::mat4();
	//m_ModelMatrix = glm::scale(m_ModelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	//m_ModelMatrix = glm::translate(m_ModelMatrix, glm::vec3(-3.0, 0.0f, -3.0));
	//m_pEffect->updateStandShaderUniform("model", m_ModelMatrix);
	//m_pEffect->updateStandShaderUniform("view", m_pCamera->getViewMatrix());
	//m_pEffect->updateStandShaderUniform("projection", m_ProjectionMatrix);
	//m_pEffect->updateStandShaderUniform("camPos", glm::vec3(m_pCamera->getCameraPosition()));
	//renderSphere();

	////4.plastic
	//glActiveTexture(GL_TEXTURE4);
	//glBindTexture(GL_TEXTURE_2D, plasticAlbedoMap);
	//glActiveTexture(GL_TEXTURE5);
	//glBindTexture(GL_TEXTURE_2D, plasticNormalMap);
	//glActiveTexture(GL_TEXTURE6);
	//glBindTexture(GL_TEXTURE_2D, plasticMetallicMap);
	//glActiveTexture(GL_TEXTURE7);
	//glBindTexture(GL_TEXTURE_2D, plasticRoughnessMap);
	//glActiveTexture(GL_TEXTURE8);
	//glBindTexture(GL_TEXTURE_2D, plasticAOMap);
	//m_pEffect->updateTextureShaderUniform("albedoMap", 4);
	//m_pEffect->updateTextureShaderUniform("normalMap", 5);
	//m_pEffect->updateTextureShaderUniform("metallicMap", 6);
	//m_pEffect->updateTextureShaderUniform("roughnessMap", 7);
	//m_pEffect->updateTextureShaderUniform("aoMap", 8);
	//m_ModelMatrix = glm::mat4();
	//m_ModelMatrix = glm::scale(m_ModelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	//m_ModelMatrix = glm::translate(m_ModelMatrix, glm::vec3(3.0, 0.0f, -3.0));
	//m_pEffect->updateStandShaderUniform("model", m_ModelMatrix);
	//m_pEffect->updateStandShaderUniform("view", m_pCamera->getViewMatrix());
	//m_pEffect->updateStandShaderUniform("projection", m_ProjectionMatrix);
	//m_pEffect->updateStandShaderUniform("camPos", glm::vec3(m_pCamera->getCameraPosition()));
	//renderSphere();

	////5.wall
	//glActiveTexture(GL_TEXTURE4);
	//glBindTexture(GL_TEXTURE_2D, wallAlbedoMap);
	//glActiveTexture(GL_TEXTURE5);
	//glBindTexture(GL_TEXTURE_2D, wallNormalMap);
	//glActiveTexture(GL_TEXTURE6);
	//glBindTexture(GL_TEXTURE_2D, wallMetallicMap);
	//glActiveTexture(GL_TEXTURE7);
	//glBindTexture(GL_TEXTURE_2D, wallRoughnessMap);
	//glActiveTexture(GL_TEXTURE8);
	//glBindTexture(GL_TEXTURE_2D, wallAOMap);
	//m_pEffect->updateTextureShaderUniform("albedoMap", 4);
	//m_pEffect->updateTextureShaderUniform("normalMap", 5);
	//m_pEffect->updateTextureShaderUniform("metallicMap", 6);
	//m_pEffect->updateTextureShaderUniform("roughnessMap", 7);
	//m_pEffect->updateTextureShaderUniform("aoMap", 8);
	//m_ModelMatrix = glm::mat4();
	//m_ModelMatrix = glm::scale(m_ModelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	//m_ModelMatrix = glm::translate(m_ModelMatrix, glm::vec3(3.0, 0.0f, 3.0));
	//m_pEffect->updateStandShaderUniform("model", m_ModelMatrix);
	//m_pEffect->updateStandShaderUniform("view", m_pCamera->getViewMatrix());
	//m_pEffect->updateStandShaderUniform("projection", m_ProjectionMatrix);
	//m_pEffect->updateStandShaderUniform("camPos", glm::vec3(m_pCamera->getCameraPosition()));
	//renderSphere();
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__drawScreenQuadPass()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
//	glDepthFunc(GL_LEQUAL);
	m_pEffect->enableShader("DrawScreenQuadPass");
	m_pEffect->updateTextureShaderUniform("uColorTexture", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_gPositionDepth); //m_BRDFIntegrationTex m_SSAOColorBufferBlur m_gPositionDepth m_gNormal
	RenderQuad();
	m_pEffect->disableShader();
	glBindTexture(GL_TEXTURE_2D, 0);
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__skyboxBackgroundPass()
{
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glDepthFunc(GL_LEQUAL);


	m_pEffect->enableShader("BackgroundCubemapPass");
	m_pEffect->updateStandShaderUniform("view", m_pCamera->getViewMatrix());
	m_pEffect->updateStandShaderUniform("projection", m_ProjectionMatrix);
	m_pEffect->updateStandShaderUniform("ChangeRGB", m_RGB);
	m_pEffect->updateTextureShaderUniform("environmentMap", 0);
	m_pEffect->updateTextureShaderUniform("SSAOMap", 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
//	glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap); // display irradiance map
	//glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_SSAOColorBufferBlur);
	renderCube();
	m_pEffect->disableShader();
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__GeometryPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	 
	m_pEffect->enableShader("SSAOGeometryPass");
	m_pEffect->updateStandShaderUniform("view", m_pCamera->getViewMatrix());
	m_pEffect->updateStandShaderUniform("projection", m_ProjectionMatrix);

	m_pEffect->updateStandShaderUniform("model", m_ModelMatrix);
	m_pDrawModel->renderModel(m_pEffect->getProgramID("SSAOGeometryPass"), 10);

	/*m_ModelMatrix = glm::mat4();
	m_ModelMatrix = glm::scale(m_ModelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	m_ModelMatrix = glm::translate(m_ModelMatrix, glm::vec3(-3.0, 0.0f, 3.0));
	m_pEffect->updateStandShaderUniform("model", m_ModelMatrix);
	renderSphere();
	m_ModelMatrix = glm::mat4();
	m_ModelMatrix = glm::scale(m_ModelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	m_ModelMatrix = glm::translate(m_ModelMatrix, glm::vec3(-3.0f, 0.0, -3.0));
	m_pEffect->updateStandShaderUniform("model", m_ModelMatrix);
	renderSphere();
	m_ModelMatrix = glm::mat4();
	m_ModelMatrix = glm::scale(m_ModelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	m_ModelMatrix = glm::translate(m_ModelMatrix, glm::vec3(3.0f, 0.0, -3.0));
	m_pEffect->updateStandShaderUniform("model", m_ModelMatrix);
	renderSphere();
	m_ModelMatrix = glm::mat4();
	m_ModelMatrix = glm::scale(m_ModelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	m_ModelMatrix = glm::translate(m_ModelMatrix, glm::vec3(3.0f, 0.0, 3.0));
	m_pEffect->updateStandShaderUniform("model", m_ModelMatrix);
	renderSphere();*/

	m_pEffect->updateStandShaderUniform("model", m_ModelPlaneMatrix);
	m_pDrawModelplane->renderModel(m_pEffect->getProgramID("SSAOGeometryPass"), 10);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_pEffect->disableShader();
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__SSAOPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_gPositionDepth);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_NoiseTexture);

	m_pEffect->enableShader("SSAOShadowPass");
	m_pEffect->updateStandShaderUniform("projection", m_ProjectionMatrix);
	m_pEffect->updateTextureShaderUniform("gPositionDepth", 0);
	m_pEffect->updateTextureShaderUniform("gNormal", 1);
	m_pEffect->updateTextureShaderUniform("texNoise", 2);
	for (GLuint i = 0; i < 64; ++i)
		glUniform3fv(glGetUniformLocation(m_pEffect->getProgramID("SSAOShadowPass"), ("samples[" + std::to_string(i) + "]").c_str()), 1, &m_SSAOKernel[i][0]);
	RenderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_pEffect->disableShader();
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__SSAOBlurPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurFBO);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_SSAOColorBuffer);
	m_pEffect->enableShader("SSAOBlurPass");
	m_pEffect->updateTextureShaderUniform("ssaoInput", 0);
	RenderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_pEffect->disableShader();
}

//*********************************************************************************
//FUNCTION:
void CIBLRendering::__destory()
{
	if (m_pCamera)    delete m_pCamera;
	if (m_pDrawModel) delete m_pDrawModel;
	if (m_pEffect)    delete m_pEffect;
}