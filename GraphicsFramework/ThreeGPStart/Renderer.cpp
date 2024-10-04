#include "Renderer.h"
#include "Camera.h"
#include "Model.h"
#include "Light.h"
#include "Terrain.h"

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

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		
		ImGui::End();
	}
}

// Load, compile and link the shaders and create a program object to host them
bool Renderer::CreateProgram()
{
	// Create a new program (returns a unqiue id)
	m_program = glCreateProgram();

	// Load and create vertex and fragment shaders
	GLuint vertex_shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/vertex_shader.glsl") };
	GLuint fragment_shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/fragment_shader.glsl") };
	if (vertex_shader == 0 || fragment_shader == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(m_program, vertex_shader);

	// The attibute 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself
	//glBindAttribLocation(m_program, 0, "vertex_position");

	// Attach the fragment shader (copies it)
	glAttachShader(m_program, fragment_shader);

	// Done with the originals of these as we have made copies
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(m_program))
		return false;

	return !Helpers::CheckForGLError();
}

// Load / create geometry into OpenGL buffers	
bool Renderer::InitialiseGeometry()
{
	// Load and compile shaders into m_program
	if (!CreateProgram())
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

	// Good idea to check for an error now:	
	Helpers::CheckForGLError();

	
	return true;
}

// Render the scene. Passed the delta time since last called.
void Renderer::Render(const Helpers::Camera& camera, float deltaTime)
{			
	glUseProgram(m_program);
	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	// Wireframe mode controlled by ImGui
	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Clear buffers from previous frame
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: Compute viewport and projection matrix
	// Compute viewport and projection matrix
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspect_ratio = viewportSize[2] / (float)viewportSize[3];
	glm::mat4 projection_xform = glm::perspective(glm::radians(45.0f), aspect_ratio, 1.0f, 100000.0f);

	// TODO: Compute camera view matrix and combine with projection matrix for passing to shader
	// Compute camera view matrix and combine with projection matrix for passing to shader
	glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
	glm::mat4 combined_xform = projection_xform * view_xform;

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

	for (std::shared_ptr<Model>& model : m_models) {
		model->Render(m_program, combined_xform, tempXForm, m_lights, camera);
	}

	// Always a good idea, when debugging at least, to check for GL errors each frame
	Helpers::CheckForGLError();
}

