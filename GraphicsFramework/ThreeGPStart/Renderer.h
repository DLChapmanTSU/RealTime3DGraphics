#pragma once

#include "ExternalLibraryHeaders.h"

#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"

class Model;
struct Light;
class Skybox;

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
	GLuint m_rectFBO{ 0 };
	GLuint m_rectTexture{ 0 };
	GLuint m_rectAAFBO{ 0 };
	GLuint m_rectDOFFBO{ 0 };
	GLuint m_rectAATexture{ 0 };
	GLuint m_rectDepthFBO{ 0 };
	GLuint m_rectDepthTexture{ 0 };
	GLuint m_rectDOFTexture{ 0 };

	// Vertex Array Object to wrap all render settings
	GLuint m_VAO{ 0 };

	// Number of elments to use when rendering
	GLuint m_numElements{ 0 };

	bool m_wireframe{ false };
	bool m_antiAliasing{ false };

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
	void Render(const Helpers::Camera& camera, float deltaTime);
};

