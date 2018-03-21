#include "IBLTechnique.h"

CIBLTechnique::CIBLTechnique(void)
{
}

CIBLTechnique::~CIBLTechnique(void)
{
}

//***********************************************************
//FUNCTION:
bool CIBLTechnique::initTechniqueV()
{
	//IBL Partition :
	//1.Translate Equirectangular to a cubemap
	CShader* pEquirectangular2CubemapPass = new CShader;
	pEquirectangular2CubemapPass->addShader("Shaders/cubemap_vs.glsl", VERTEX_SHADER);
	pEquirectangular2CubemapPass->addShader("Shaders/equirectangular_to_cubemap_fs.glsl", FRAGMENT_SHADER);
	addShader("Equirectangular2CubemapPass", pEquirectangular2CubemapPass);

	//2.Irradiance cubemap calculate__Diffuse Lighting 
	CShader* pIrradianceCubemapPass = new CShader;
	pIrradianceCubemapPass->addShader("Shaders/cubemap_vs.glsl", VERTEX_SHADER);
	pIrradianceCubemapPass->addShader("Shaders/irradiance_convolution_fs.glsl", FRAGMENT_SHADER);
	addShader("IrradianceCubemapPass", pIrradianceCubemapPass);

	//3.Prefilter cubemap calculate__Specular Lighting
	CShader* pPrefilterCubemapPass = new CShader;
	pPrefilterCubemapPass->addShader("Shaders/cubemap_vs.glsl", VERTEX_SHADER);
	pPrefilterCubemapPass->addShader("Shaders/prefilter_fs.glsl", FRAGMENT_SHADER);
	addShader("PrefilterCubemapPass", pPrefilterCubemapPass);

	//4.BRDF texture calculate__Specular Lighting
	CShader* pBRDFIntegrationPass = new CShader;
	pBRDFIntegrationPass->addShader("Shaders/brdf_vs.glsl", VERTEX_SHADER);
	pBRDFIntegrationPass->addShader("Shaders/brdf_fs.glsl", FRAGMENT_SHADER);
	addShader("BRDFIntegrationPass", pBRDFIntegrationPass);

	//5.PBR integratedPass with texture by self
	CShader* pPBRIntegratedPass = new CShader;
	pPBRIntegratedPass->addShader("Shaders/pbr_integrated_vs.glsl", VERTEX_SHADER);
	pPBRIntegratedPass->addShader("Shaders/pbr_integrated_fs.glsl", FRAGMENT_SHADER);
	addShader("PBRIntegratedPass", pPBRIntegratedPass);
	//5.PBR IntegratedPass with Model
	CShader* pPBRModelIntegratedPass = new CShader;
	pPBRModelIntegratedPass->addShader("Shaders/pbr_model_integrated_vs.glsl", VERTEX_SHADER);
	pPBRModelIntegratedPass->addShader("Shaders/pbr_model_integrated_fs.glsl", FRAGMENT_SHADER);
	addShader("PBRModelIntegratedPass", pPBRModelIntegratedPass);

	//SSAO Partion
	//1.GBuffer Pass
	CShader* pSSAOGeometryPass = new CShader;
	pSSAOGeometryPass->addShader("Shaders/ssaoGeometry_vs.glsl", VERTEX_SHADER);
	pSSAOGeometryPass->addShader("Shaders/ssaoGeometry_fs.glsl", FRAGMENT_SHADER);
	addShader("SSAOGeometryPass", pSSAOGeometryPass);
	//2.SSAO Shadow texture generate
	CShader* pSSAOShadowPass = new CShader;
	pSSAOShadowPass->addShader("Shaders/ssao_vs.glsl", VERTEX_SHADER);
	pSSAOShadowPass->addShader("Shaders/ssao_fs.glsl", FRAGMENT_SHADER);
	addShader("SSAOShadowPass", pSSAOShadowPass);
	//3.Shadow texture blur
	CShader* pSSAOBlurPass = new CShader;
	pSSAOBlurPass->addShader("Shaders/ssao_vs.glsl", VERTEX_SHADER);
	pSSAOBlurPass->addShader("Shaders/ssaoBlur_fs.glsl", FRAGMENT_SHADER);
	addShader("SSAOBlurPass", pSSAOBlurPass);

	//Tool Kit Pass:
	//Draw Environment cube map
	CShader* pBackgroundCubemapPass = new CShader;
	pBackgroundCubemapPass->addShader("Shaders/background_vs.glsl", VERTEX_SHADER);
	pBackgroundCubemapPass->addShader("Shaders/background_fs.glsl", FRAGMENT_SHADER);
	addShader("BackgroundCubemapPass", pBackgroundCubemapPass);
	//Draw Picture 
	CShader* pDrawScreenQuadPass = new CShader;
	pDrawScreenQuadPass->addShader("Shaders/DrawScreenQuad_vs.glsl", VERTEX_SHADER);
	pDrawScreenQuadPass->addShader("Shaders/DrawScreenQuad_fs.glsl", FRAGMENT_SHADER);
	addShader("DrawScreenQuadPass", pDrawScreenQuadPass);

	return true;
}