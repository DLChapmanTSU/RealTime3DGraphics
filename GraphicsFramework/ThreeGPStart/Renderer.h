#pragma once

#include "ExternalLibraryHeaders.h"

#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"

class Model;
struct Light;
class Skybox;

using namespace Helpers;

class Renderer
{
private:
	// Program object - to host shaders
	GLuint m_program{ 0 };
	GLuint m_skyProgram{ 0 };
	GLuint m_rectProgram{ 0 };
	GLuint m_ambientProgram{ 0 };
	GLuint m_lightProgram{ 0 };
	GLuint m_fxaaProgram{ 0 };
	GLuint m_depthProgram{ 0 };
	GLuint m_dofProgram{ 0 };
	GLuint m_shadowDepthProgram{ 0 };
	GLuint m_rectFBO{ 0 };
	GLuint m_rectTexture{ 0 };
	GLuint m_rectAAFBO{ 0 };
	GLuint m_rectDOFFBO{ 0 };
	GLuint m_rectAATexture{ 0 };
	GLuint m_rectDepthFBO{ 0 };
	GLuint m_rectDepthTexture{ 0 };
	GLuint m_rectDOFTexture{ 0 };
	GLuint m_rectDOFPassTwoFBO{ 0 };
	GLuint m_rectDOFPassTwoTexture{ 0 };

	//Shadow Buffers
	GLuint m_rectShadowFBO{ 0 };
	GLuint m_rectShadowTexture{ 0 };
	std::vector<GLuint> m_lightFBOs;
	//Extra textures for extra light passes
	//GLuint m_accumulationSamples[12]{ 0 };
	//GLuint m_accumulationFBO[12]{ 0 };
	//Camera offsets for extra light passes
	//std::pair<float, float> m_cameraOffsets[12];

	// Vertex Array Object to wrap all render settings
	GLuint m_VAO{ 0 };
	GLuint m_vertexVBO{ 0 };
	GLuint m_uvVBO{ 0 };

	// Number of elments to use when rendering
	GLuint m_numElements{ 0 };

	bool m_wireframe{ false };
	bool m_isAntiAliasing{ false };
	bool m_isDepthOfField{ false };
	float m_aperture = 5.0f;
	float m_focalLength = 2.0f;
	float m_planeInFocus = 5.0f;

	float m_drsFactor = 1.0f;

	glm::mat4 tempXForm = glm::mat4(1);
	int tempElementCount = 0;

	std::vector<Light> m_lights;
	std::vector<std::shared_ptr<Model>> m_models;
	std::shared_ptr<Skybox> m_skybox;

	bool CreateProgram(GLuint& p, std::string v, std::string f);
public:
	Renderer();
	~Renderer();
	void DefineGUI();

	// Create and / or load geometry, this is like 'level load'
	bool InitialiseGeometry();

	// Render the scene
	void Render(Camera& camera, float deltaTime);
};

