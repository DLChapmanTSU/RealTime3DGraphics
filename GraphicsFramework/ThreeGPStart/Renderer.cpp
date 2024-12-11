#include "Renderer.h"
#include "Camera.h"
#include "Model.h"
#include "Light.h"
#include "Terrain.h"
#include "Skybox.h"

using namespace Helpers;

Renderer::Renderer() 
{

}

// On exit must clean up any OpenGL resources e.g. the program, the buffers
Renderer::~Renderer()
{
	// TODO: clean up any memory used including OpenGL objects via glDelete* calls
	glDeleteProgram(m_program);
	glDeleteBuffers(1, &m_VAO);
}

// Use IMGUI for a simple on screen GUI
void Renderer::DefineGUI()
{
	// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	{
		ImGui::Begin("RTG");                    // Create a window called "RTG" and append into it.

		ImGui::Text("Visibility.");             // Display some text (you can use a format strings too)	

		ImGui::Checkbox("Wireframe", &m_wireframe);	// A checkbox linked to a member variable

		ImGui::Checkbox("FXAA", &m_antiAliasing);

		ImGui::SliderFloat("Aperture", &m_aperture, 0.1f, 100.0f);

		ImGui::SliderFloat("Focal Length", &m_focalLength, 0.1f, 1.0f);

		ImGui::SliderFloat("Plane In Focus", &m_planeInFocus, 0.1f, 50.0f);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		
		ImGui::End();
	}
}

// Load, compile and link the shaders and create a program object to host them
bool Renderer::CreateProgram(GLuint& p, std::string v, std::string f)
{
	// Create a new program (returns a unqiue id)
	p = glCreateProgram();

	// Load and create vertex and fragment shaders
	GLuint vertex_shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, v) };
	GLuint fragment_shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, f) };
	if (vertex_shader == 0 || fragment_shader == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(p, vertex_shader);

	// The attibute location 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself
	//glBindAttribLocation(m_program, 0, "vertex_position");

	// Attach the fragment shader (copies it)
	glAttachShader(p, fragment_shader);

	// Done with the originals of these as we have made copies
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(p))
		return false;

	return true;
}

// Load / create geometry into OpenGL buffers	
bool Renderer::InitialiseGeometry()
{
	// Load and compile shaders into m_program
	if (!CreateProgram(m_program, "Data/Shaders/vertex_shader.glsl", "Data/Shaders/fragment_shader.glsl"))
		return false;

	if (!CreateProgram(m_skyProgram, "Data/Shaders/vertex_shader_skybox.glsl", "Data/Shaders/fragment_shader_skybox.glsl"))
		return false;

	if (!CreateProgram(m_rectProgram, "Data/Shaders/vertex_shader_rect.glsl", "Data/Shaders/fragment_shader_rect.glsl"))
		return false;

	if (!CreateProgram(m_ambientProgram, "Data/Shaders/vertex_shader_ambient.glsl", "Data/Shaders/fragment_shader_ambient.glsl"))
		return false;

	if (!CreateProgram(m_lightProgram, "Data/Shaders/vertex_shader_light.glsl", "Data/Shaders/fragment_shader_light.glsl"))
		return false;

	if (!CreateProgram(m_fxaaProgram, "Data/Shaders/vertex_shader_fxaa.glsl", "Data/Shaders/fragment_shader_fxaa.glsl"))
		return false;

	if (!CreateProgram(m_depthProgram, "Data/Shaders/vertex_shader_depth.glsl", "Data/Shaders/fragment_shader_depth.glsl"))
		return false;

	if (!CreateProgram(m_dofProgram, "Data/Shaders/vertex_shader_dof.glsl", "Data/Shaders/fragment_shader_dof.glsl"))
		return false;

	// Helpers has an object for loading 3D geometry, supports most types
	
	// Load in the jeep
	/*Helpers::ModelLoader loader;
	if (!loader.LoadFromFile("Data\\Models\\Jeep\\jeep.obj"))
		return false;

	// Now we can loop through all the mesh in the loaded model:
	for (const Helpers::Mesh& mesh : loader.GetMeshVector())
	{
		// We can extract from the mesh via:
		//mesh.vertices  - a vector of glm::vec3 (3 floats) giving the position of each vertex
		//mesh.elements - a vector of unsigned ints defining which vertices make up each triangle

		// TODO: create VBO for the vertices and a EBO for the elements
		// TODO: create a VBA to wrap everything and specify locations in the shaders

		GLuint vertexVBO;

		glGenBuffers(1, &vertexVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint coloursVBO;

		glGenBuffers(1, &coloursVBO);
		glBindBuffer(GL_ARRAY_BUFFER, coloursVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint elementsVBO;

		glGenBuffers(1, &elementsVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.elements.size(), mesh.elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		tempElementCount = mesh.elements.size();

		//GLuint textureCoordsVBO;

		//glGenBuffers(1, &textureCoordsVBO);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textureCoordsVBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.uvCoords.size(), mesh.uvCoords.data(), GL_STATIC_DRAW);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, coloursVBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//glBindBuffer(GL_ARRAY_BUFFER, textureCoordsVBO);
		//glEnableVertexAttribArray(2);
		//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsVBO);

		glBindVertexArray(0);
	}*/

	//Initialize the diferent lights in the scene
	Light pointTest1;
	pointTest1.m_position = glm::vec3(25.0f, 10.0f, 0.0f);
	pointTest1.m_intensity = 0.5f;
	pointTest1.m_type = 1;
	pointTest1.m_direction = glm::vec3(1.0f, -1.0f, 1.0f);
	pointTest1.m_colour = glm::vec3(1.0f, 1.0f, 0.0f);

	Light pointTest2;
	pointTest2.m_position = glm::vec3(500.0f, 10.0f, 200.0f);
	pointTest2.m_intensity = 1.0f;
	pointTest2.m_type = 1;
	pointTest2.m_direction = glm::vec3(1.0f, -1.0f, 1.0f);
	pointTest2.m_colour = glm::vec3(1.0f, 0.0f, 1.0f);

	Light pointTest6;
	pointTest6.m_position = glm::vec3(150.0f, 50.0f, 300.0f);
	pointTest6.m_intensity = 1.0f;
	pointTest6.m_type = 1;
	pointTest6.m_direction = glm::vec3(1.0f, -1.0f, 1.0f);
	pointTest6.m_colour = glm::vec3(0.6f, 1.0f, 0.0f);

	Light pointTest7;
	pointTest7.m_position = glm::vec3(400.0f, 10.0f, 400.0f);
	pointTest7.m_intensity = 1.0f;
	pointTest7.m_type = 1;
	pointTest7.m_direction = glm::vec3(1.0f, -1.0f, 1.0f);
	pointTest7.m_colour = glm::vec3(1.0f, 0.0f, 0.0f);

	Light pointTest8;
	pointTest8.m_position = glm::vec3(410.0f, 10.0f, 410.0f);
	pointTest8.m_intensity = 1.0f;
	pointTest8.m_type = 1;
	pointTest8.m_direction = glm::vec3(1.0f, -1.0f, 1.0f);
	pointTest8.m_colour = glm::vec3(0.0f, 0.0f, 1.0f);

	Light pointTest9;
	pointTest9.m_position = glm::vec3(400.0f, 10.0f, 410.0f);
	pointTest9.m_intensity = 1.0f;
	pointTest9.m_type = 1;
	pointTest9.m_direction = glm::vec3(1.0f, -1.0f, 1.0f);
	pointTest9.m_colour = glm::vec3(0.0f, 1.0f, 0.0f);

	Light directionalLight;
	directionalLight.m_position = glm::vec3(200.0f, 10.0f, 200.0f);
	directionalLight.m_intensity = 0.2f;
	directionalLight.m_type = 0;
	directionalLight.m_direction = glm::vec3(1.0f, -1.0f, 1.0f);
	directionalLight.m_colour = glm::vec3(1.0f, 1.0f, 1.0f);

	Light spotLight1;
	spotLight1.m_position = glm::vec3(150.0f, 30.0f, 220.0f);
	spotLight1.m_intensity = 1.0f;
	spotLight1.m_type = 2;
	spotLight1.m_direction = glm::vec3(1.0f, 0.0f, 0.0f);
	spotLight1.m_colour = glm::vec3(1.0f, 1.0f, 1.0f);
	spotLight1.m_angle = 0.6f;

	Light spotLight2;
	spotLight2.m_position = glm::vec3(150.0f, 30.0f, 180.0f);
	spotLight2.m_intensity = 1.0f;
	spotLight2.m_type = 2;
	spotLight2.m_direction = glm::vec3(1.0f, 0.0f, 0.0f);
	spotLight2.m_colour = glm::vec3(1.0f, 1.0f, 1.0f);
	spotLight2.m_angle = 0.6f;

	m_lights.push_back(pointTest1);
	m_lights.push_back(pointTest2);
	m_lights.push_back(pointTest6);
	m_lights.push_back(pointTest7);
	m_lights.push_back(pointTest8);
	m_lights.push_back(pointTest9);
	m_lights.push_back(spotLight1);
	m_lights.push_back(spotLight2);
	m_lights.push_back(directionalLight);

	std::vector<std::string> objTemp;

	//Initialise the data for the two models in the scene. The aquapig and the jeep
	Helpers::ImageLoader imageLoader;

	if (!imageLoader.Load("Data\\Models\\AquaPig\\aqua_pig_2K.png")) {
		imageLoader.Load("Data\\Error.png");
	}

	//Creates a vector of model files and loads each one
	Helpers::ModelLoader modelLoader;

	objTemp.push_back("Data\\Models\\AquaPig\\hull.obj");
	objTemp.push_back("Data\\Models\\AquaPig\\wing_right.obj");
	objTemp.push_back("Data\\Models\\AquaPig\\wing_left.obj");
	objTemp.push_back("Data\\Models\\AquaPig\\propeller.obj");
	objTemp.push_back("Data\\Models\\AquaPig\\gun_base.obj");
	objTemp.push_back("Data\\Models\\AquaPig\\gun.obj");

	for (size_t i = 0; i < objTemp.size(); i++)
	{
		if (!modelLoader.LoadFromFile(objTemp[i])) {
			return false;
		}
	}

	m_models.push_back(std::make_shared<Model>(Model(glm::vec3(0), 500.0f)));
	m_models.push_back(std::make_shared<Model>(Model(glm::vec3(50.0f, 0.0f, 0.0f), 500.0f)));

	std::vector<glm::mat4> xFormsTemp;

	xFormsTemp.push_back(glm::mat4(1));
	glm::mat4 trans = glm::translate(glm::mat4(1), glm::vec3(-2.231, 0.272, -2.663));
	xFormsTemp.push_back(trans);

	trans = glm::translate(glm::mat4(1), glm::vec3(2.231, 0.272, -2.663));
	xFormsTemp.push_back(trans);

	trans = glm::translate(glm::mat4(1), glm::vec3(0.0, 1.395, -3.616));
	glm::mat4 rot = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1, 0, 0));
	xFormsTemp.push_back(trans * rot);

	trans = glm::translate(glm::mat4(1), glm::vec3(0.0, 0.569, -1.866));
	xFormsTemp.push_back(trans);

	trans = glm::translate(glm::mat4(1), glm::vec3(0.0, 1.506, -0.644));
	xFormsTemp.push_back(trans);

	for (size_t i = 0; i < m_models.size(); i++)
	{
		m_models[i]->Initialise(xFormsTemp, imageLoader, modelLoader);
	}

	std::string tex = "Data\\Textures\\grass_green-01_df_.dds";
	std::string hMap = "Data\\Heightmaps\\3gp_heightmap.bmp";

	std::shared_ptr<Terrain> terrain = std::make_shared<Terrain>(1024, 1024);
	terrain->Initialise(glm::mat4(1), tex, hMap);
	m_models.push_back(terrain);

	//Initialises skybox and its textures
	m_skybox = std::make_shared<Skybox>(
		"Data\\Models\\Sky\\Clouds\\SkyBox_Right.tga",
		"Data\\Models\\Sky\\Clouds\\SkyBox_Left.tga",
		"Data\\Models\\Sky\\Clouds\\SkyBox_Bottom.tga",
		"Data\\Models\\Sky\\Clouds\\SkyBox_Top.tga",
		"Data\\Models\\Sky\\Clouds\\SkyBox_Front.tga",
		"Data\\Models\\Sky\\Clouds\\SkyBox_Back.tga",
		"Data\\Models\\Sky\\Clouds\\skybox.x");

	m_skybox->Initialize();

	//From Learn OpenGL
	
	glGenFramebuffers(1, &m_rectFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_rectFBO);

	
	glGenTextures(1, &m_rectTexture);
	glBindTexture(GL_TEXTURE_2D, m_rectTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_rectTexture, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);

	
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_rectTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glDeleteFramebuffers(1, &m_rectFBO);

	glGenFramebuffers(1, &m_rectAAFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_rectAAFBO);


	glGenTextures(1, &m_rectAATexture);
	glBindTexture(GL_TEXTURE_2D, m_rectAATexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_rectAATexture, 0);

	unsigned int aarbo;
	glGenRenderbuffers(1, &aarbo);
	glBindRenderbuffer(GL_RENDERBUFFER, aarbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);


	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_rectTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, aarbo);

	if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &m_rectDOFFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_rectDOFFBO);


	glGenTextures(1, &m_rectDOFTexture);
	glBindTexture(GL_TEXTURE_2D, m_rectDOFTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_rectDOFTexture, 0);

	unsigned int dofrbo;
	glGenRenderbuffers(1, &dofrbo);
	glBindRenderbuffer(GL_RENDERBUFFER, dofrbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);


	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_rectTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dofrbo);

	if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Framebuffer setup for depth test texture
	glGenFramebuffers(1, &m_rectDepthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_rectDepthFBO);


	glGenTextures(1, &m_rectDepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_rectDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1280, 720, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_rectDepthTexture, 0);

	glDrawBuffer(GL_NONE);

	//unsigned int depthrbo;
	//glGenRenderbuffers(1, &depthrbo);
	//glBindRenderbuffer(GL_RENDERBUFFER, depthrbo);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1280, 720);

	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrbo);

	if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	for (int i = 0; i < 12; i++)
	{
		glGenFramebuffers(1, &m_accumulationFBO[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, m_accumulationFBO[i]);

		glActiveTexture(GL_TEXTURE0 + 20 + i);

		glGenTextures(1, &m_accumulationSamples[i]);
		glBindTexture(GL_TEXTURE_2D, m_accumulationSamples[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_accumulationSamples[i], 0);

		unsigned int sampleRBO;
		glGenRenderbuffers(1, &sampleRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, sampleRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sampleRBO);

		if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	m_cameraOffsets[0].first = 0.1f;
	m_cameraOffsets[0].second = 0.f;
	m_cameraOffsets[1].first = 0.1f;
	m_cameraOffsets[1].second = 0.1f;
	m_cameraOffsets[2].first = 0.0f;
	m_cameraOffsets[2].second = 0.1f;
	m_cameraOffsets[3].first = -0.1f;
	m_cameraOffsets[3].second = 0.1f;
	m_cameraOffsets[4].first = -0.1f;
	m_cameraOffsets[4].second = 0.0f;
	m_cameraOffsets[5].first = -0.1f;
	m_cameraOffsets[5].second = -0.1f;
	m_cameraOffsets[6].first = 0.0f;
	m_cameraOffsets[6].second = -0.1f;
	m_cameraOffsets[7].first = 0.1f;
	m_cameraOffsets[7].second = 0.0f;
	m_cameraOffsets[8].first = 0.2f;
	m_cameraOffsets[8].second = 0.0f;
	m_cameraOffsets[9].first = -0.2f;
	m_cameraOffsets[9].second = 0.0f;
	m_cameraOffsets[10].first = 0.0f;
	m_cameraOffsets[10].second = 0.2f;
	m_cameraOffsets[11].first = 0.0f;
	m_cameraOffsets[11].second = -0.2f;

	const float windowVerts[] = { -1.0f, 1.0f, 0.3f, -1.0f,-1.0f, 0.3f, 1.0f, -1.0f, 0.3f, 1.0f, -1.0f, 0.3f, 1.0f, 1.0f, 0.3f, -1.0f, 1.0f, 0.3f };
	const float windowQuadUVs[] = { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };

	GLuint vertexVBO;

	glGenBuffers(1, &vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(windowVerts), windowVerts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint uvVBO;

	glGenBuffers(1, &uvVBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(windowQuadUVs), windowQuadUVs, GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(0);

	// Good idea to check for an error now:	
	Helpers::CheckForGLError();

	
	return true;
}

// Render the scene. Passed the delta time since last called.
void Renderer::Render(Camera& camera, float deltaTime)
{
	glm::vec3 originalCameraPos = camera.GetPosition();
	glUseProgram(m_skyProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, m_rectFBO);
	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Wireframe mode controlled by ImGui
	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Clear buffers from previous frame
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: Compute viewport and projection matrix
	// Compute viewport and projection matrix
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspect_ratio = viewportSize[2] / (float)viewportSize[3];
	glm::mat4 projection_xform = glm::perspective(glm::radians(45.0f), aspect_ratio, 1.0f, 1000000.0f);

	// TODO: Compute camera view matrix and combine with projection matrix for passing to shader
	// Compute camera view matrix and combine with projection matrix for passing to shader
	glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
	glm::mat4 combined_xform = projection_xform * view_xform;
	glm::mat4 sky_view_xform = glm::mat4(glm::mat3(view_xform));
	glm::mat4 sky_combined_xform = projection_xform * sky_view_xform;

	// Send the combined matrix to the shader in a uniform
	GLuint sky_combined_xform_id = glGetUniformLocation(m_skyProgram, "combined_xform");
	glUniformMatrix4fv(sky_combined_xform_id, 1, GL_FALSE, glm::value_ptr(sky_combined_xform));

	//Disables then re-enables depth when drawing the skybox so that it appears behind every other mesh in the scene
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	m_skybox->Render(m_skyProgram, sky_combined_xform);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	//Renders terrain using the terrain shader
	glUseProgram(m_program);

	// TODO: Send the combined matrix to the shader in a uniform
	// Send the combined matrix to the shader in a uniform
	GLuint combined_xform_id = glGetUniformLocation(m_program, "combined_xform");
	glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

	// TODO: render each mesh. Send the correct model matrix to the shader in a uniform
	GLuint model_xform_id = glGetUniformLocation(m_program, "model_xform");
	glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(tempXForm));

	//glBindVertexArray(m_VAO);
	//glDrawElements(GL_TRIANGLES, tempElementCount, GL_UNSIGNED_INT, (void*)0);
	//glBindVertexArray(0);

	//for (std::shared_ptr<Model>& model : m_models) {
	//	model->Render(m_program, combined_xform, tempXForm, m_lights, camera);
	//}

	//depth test program calls
	glBindFramebuffer(GL_FRAMEBUFFER, m_rectDepthFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_depthProgram);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_BLEND);

	for (std::shared_ptr<Model>& model : m_models)
	{
		model->RenderDepthPass(m_depthProgram, combined_xform, tempXForm);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_rectFBO);

	glUseProgram(m_ambientProgram);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_BLEND);

	for (std::shared_ptr<Model>& model : m_models) 
	{
		model->RenderAmbientPass(m_ambientProgram, combined_xform, tempXForm);
	}
	
	glUseProgram(m_lightProgram);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	for (std::shared_ptr<Model>& model : m_models) 
	{
		model->Render(m_lightProgram, combined_xform, tempXForm, m_lights, camera);
	}

	for (size_t i = 0; i < 12; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_accumulationFBO[i]);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glUseProgram(m_skyProgram);
		//glDisable(GL_DEPTH_TEST);
		//glDepthMask(GL_FALSE);
		//m_skybox->Render(m_skyProgram, sky_combined_xform);
		//glEnable(GL_DEPTH_TEST);
		//glDepthMask(GL_TRUE);

		glUseProgram(m_ambientProgram);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glDisable(GL_BLEND);

		camera.SetPosition(camera.GetPosition() + (camera.GetRightVector() * m_cameraOffsets[i].first) + (camera.GetUpVector() * m_cameraOffsets[i].second));

		glm::mat4 sample_view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
		glm::mat4 sample_combined_xform = projection_xform * sample_view_xform;

		for (std::shared_ptr<Model>& model : m_models)
		{
			model->RenderAmbientPass(m_ambientProgram, sample_combined_xform, tempXForm);
		}

		glUseProgram(m_lightProgram);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		for (std::shared_ptr<Model>& model : m_models)
		{
			model->Render(m_lightProgram, sample_combined_xform, tempXForm, m_lights, camera);
		}

		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
	}

	//dof blur
	glBindFramebuffer(GL_FRAMEBUFFER, m_rectDOFFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_dofProgram);
	glBindVertexArray(m_VAO);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glBindTexture(GL_TEXTURE_2D, m_rectDepthTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_rectTexture);
	glActiveTexture(GL_TEXTURE0);
	//glActiveTexture(GL_TEXTURE0 + 2);
	//glBindTexture(GL_TEXTURE_2D, m_rectAATexture);
	glUniform1i(glGetUniformLocation(m_dofProgram, "sampler_depth_tex"), 0);
	glUniform1i(glGetUniformLocation(m_dofProgram, "sampler_colour_tex"), 1);
	glUniform1f(glGetUniformLocation(m_dofProgram, "aperture"), m_aperture);
	glUniform1f(glGetUniformLocation(m_dofProgram, "focalLength"), m_focalLength);
	glUniform1f(glGetUniformLocation(m_dofProgram, "planeInFocus"), m_planeInFocus);
	GLuint depthResolutionId = glGetUniformLocation(m_dofProgram, "screen_resolution");
	glUniform2fv(depthResolutionId, 1, glm::value_ptr(glm::vec2(1280, 720)));

	for (size_t i = 0; i < 12; i++)
	{
		glActiveTexture(GL_TEXTURE0 + 20 + i);
		glBindTexture(GL_TEXTURE_2D, m_accumulationSamples[i]);
		GLuint samplerId = glGetUniformLocation(m_dofProgram, std::string("sampler_accum_colours_tex[" + std::to_string(i) + "]").c_str());
		glUniform1i(samplerId, m_accumulationSamples[i]);
	}

	glActiveTexture(GL_TEXTURE0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (m_antiAliasing)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_rectAAFBO);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(m_fxaaProgram);
		glBindVertexArray(m_VAO);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		GLuint resolutionId = glGetUniformLocation(m_fxaaProgram, "screen_resolution");
		glUniform2fv(resolutionId, 1, glm::value_ptr(glm::vec2(1280, 720)));

		glBindTexture(GL_TEXTURE_2D, m_rectDOFTexture);
		//glActiveTexture(GL_TEXTURE0 + 2);
		//glBindTexture(GL_TEXTURE_2D, m_rectAATexture);
		glUniform1i(glGetUniformLocation(m_fxaaProgram, "sampler_tex"), 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//glBindFramebuffer(GL_READ_FRAMEBUFFER, m_rectFBO);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_rectProgram);
		glBindVertexArray(m_VAO);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		glDisable(GL_BLEND);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_rectDOFTexture);
		//glUniform1i(glGetUniformLocation(m_rectProgram, "screenSampler"), m_rectTexture);


		glDrawArrays(GL_TRIANGLES, 0, 6);
		//glNamedFramebufferDrawBuffer(m_rectFBO, GL_FRONT_AND_BACK);
		glBindVertexArray(0);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//glBindFramebuffer(GL_READ_FRAMEBUFFER, m_rectFBO);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_rectProgram);
		glBindVertexArray(m_VAO);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		glDisable(GL_BLEND);
		glBindTexture(GL_TEXTURE_2D, m_rectTexture);
		//glUniform1i(glGetUniformLocation(m_rectProgram, "screenSampler"), m_rectTexture);


		glDrawArrays(GL_TRIANGLES, 0, 6);
		//glNamedFramebufferDrawBuffer(m_rectFBO, GL_FRONT_AND_BACK);
		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, m_rectAAFBO);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	camera.SetPosition(originalCameraPos);
	/*glBindFramebuffer(GL_READ_FRAMEBUFFER, m_rectFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitNamedFramebuffer(m_rectFBO, 0, 0, 0, 800, 600, 0, 0, 800, 600, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_FRONT_AND_BACK);*/
	
	// Always a good idea, when debugging at least, to check for GL errors each frame
	Helpers::CheckForGLError();
}

