#pragma once

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include "Model.h"
#include "Shader.h"
#include "Camera.h"
#include "IBLTechnique.h"

#define SCR_WIDTH 1280
#define SCR_HEIGHT 720

class CIBLRendering 
{
public:
	CIBLRendering();
    ~CIBLRendering();

	void init();
	void draw();

private:
	void __initGLFW(const std::string& vWindowTitle);
	void __debuggingTest();
	void __initShaders();
	void __initTexture();
	void __initModel();
	void __initCubemapMatrix();
	void __initBuffers();

	void __initGBuffer();
	void __initNoiseTextureAndSSAOKernel();

	void __equirectangular2CubemapPass();
	void __createIrradianceCubemapPass();
	void __createBRDFIntegrationTexPass();
	void __createPrefilterCubeMapPass();

	void __pbrIntegratedPass();
	void __drawScreenQuadPass();
	void __skyboxBackgroundPass();
	void __GeometryPass();
	void __SSAOPass();
	void __SSAOBlurPass();
	void __destory();

	CCamera*       m_pCamera;
	GLFWwindow*    m_pWindow;
	CDrawModel*    m_pDrawModel;
	CDrawModel*    m_pDrawModelplane;
	CIBLTechnique* m_pEffect;
	
	std::vector<glm::vec3> m_SSAOKernel;
	GLuint m_gBuffer;
	GLuint m_SSAOFBO, m_SSAOBlurFBO;
	GLuint m_gPositionDepth, m_gNormal, m_gAlbedo;
	GLuint m_SSAOColorBuffer, m_SSAOColorBufferBlur;
	GLuint m_NoiseTexture;

	GLuint m_CaptureFBO;
	GLuint m_CaptureRBO;
	GLuint m_EnvironmentTex;
	GLuint m_TestTex;
	GLuint m_BRDFIntegrationTex;
	GLuint m_EnvCubemap;
	GLuint m_IrradianceMap;
	GLuint m_PrefilterMap;
	GLint  m_EnvCubemap1;
	glm::mat4 m_CaptureProjection;
	glm::mat4 m_CaptureViews[6];
	glm::mat4 m_ProjectionMatrix;

	glm::mat4 m_ModelMatrix;
	glm::mat4 m_ModelPlaneMatrix;

	// load PBR material textures
	// --------------------------
	// rusted iron
	unsigned int ironAlbedoMap;
	unsigned int ironNormalMap;
	unsigned int ironMetallicMap;
	unsigned int ironRoughnessMap;
	unsigned int ironAOMap;

	// gold
	unsigned int goldAlbedoMap;
	unsigned int goldNormalMap;
	unsigned int goldMetallicMap;
	unsigned int goldRoughnessMap;
	unsigned int goldAOMap;

	// plastic
	unsigned int plasticAlbedoMap;
	unsigned int plasticNormalMap;
	unsigned int plasticMetallicMap;
	unsigned int plasticRoughnessMap;
	unsigned int plasticAOMap;

	// wall
	unsigned int wallAlbedoMap;
	unsigned int wallNormalMap;
	unsigned int wallMetallicMap;
	unsigned int wallRoughnessMap;
	unsigned int wallAOMap;
	glm::vec3 m_RGB;
};