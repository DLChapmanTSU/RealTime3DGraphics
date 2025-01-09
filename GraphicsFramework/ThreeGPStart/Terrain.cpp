#include "Terrain.h"
#include "ImageLoader.h"
#include "Light.h"

Terrain::Terrain(int x, int z) : Model(glm::vec3(0), 0.0f)
{
	m_cellCountX = x;
	m_cellCountZ = z;
}

void Terrain::Initialise(glm::mat4 x, std::string& p, std::string& h)
{
	//Calculates the amount of vertices in the mesh
	int vertCountX{ m_cellCountX + 1 };
	int vertCountZ{ m_cellCountZ + 1 };
	int vertTotalCount = vertCountX * vertCountZ;

	Helpers::ImageLoader heightLoader;

	//Generates each vertex at a set interval
	//Uses height map to decide the y axis
	//If the heightmap fails to load, the y axis is randomized
	if (!heightLoader.Load(h)) {
		for (size_t i = 0; i < vertCountZ; i++)
		{
			for (size_t j = 0; j < vertCountX; j++)
			{
				m_vertices.push_back(glm::vec3(i * 10, 1, j * 10));
				m_uvs.push_back(glm::vec2(0));
				m_normals.push_back(glm::vec3(0, 0, 0));
			}
		}
	}
	else {
		float xToImage = ((float)heightLoader.Width() - 1) / vertCountX;
		float zToImage = ((float)heightLoader.Height() - 1) / vertCountZ;

		BYTE* imageData = heightLoader.GetData();

		for (size_t z = 0; z < vertCountZ; z++)
		{
			for (size_t x = 0; x < vertCountX; x++)
			{
				int imageX = (int)(xToImage * (vertCountX - x));
				int imageZ = (int)(zToImage * z);

				size_t offset = ((size_t)imageX + (size_t)imageZ * heightLoader.Width()) * 4;
				BYTE height = imageData[offset];

				m_vertices.push_back(glm::vec3(x * 10, (float)height, z * 10));
				m_uvs.push_back(glm::vec2(0));
				m_normals.push_back(glm::vec3(0, 0, 0));
			}
		}
	}

	bool alternateEdge{ true };

	//Sets the elements for the mesh
	//The boolean determines which direction to draw the hypotenuse of each triangle to allow for a diamond-shaped pattern in the mesh
	//If there is an odd number of cells, the pattern won't alternate on the final edge of each row
	if (m_cellCountX % 2 == 0) {
		for (int z = 0; z < m_cellCountZ; z++)
		{
			for (int x = 0; x < m_cellCountX; x++)
			{
				int startVertIndex = (z * vertCountX) + x;

				if (alternateEdge == true) {
					m_elements.push_back(startVertIndex + vertCountX);
					m_elements.push_back(startVertIndex + 1);
					m_elements.push_back(startVertIndex);

					m_elements.push_back(startVertIndex + vertCountX);
					m_elements.push_back(startVertIndex + vertCountX + 1);
					m_elements.push_back(startVertIndex + 1);
				}
				else {
					m_elements.push_back(startVertIndex + vertCountX + 1);
					m_elements.push_back(startVertIndex + 1);
					m_elements.push_back(startVertIndex);

					m_elements.push_back(startVertIndex + vertCountX);
					m_elements.push_back(startVertIndex + vertCountX + 1);
					m_elements.push_back(startVertIndex);
				}

				alternateEdge = !alternateEdge;
			}
			alternateEdge = !alternateEdge;
		}
	}
	else {
		for (int z = 0; z < m_cellCountZ; z++)
		{
			for (int x = 0; x < m_cellCountX; x++)
			{
				int startVertIndex = (z * vertCountX) + x;

				if (alternateEdge == true) {
					m_elements.push_back(startVertIndex + vertCountX);
					m_elements.push_back(startVertIndex + 1);
					m_elements.push_back(startVertIndex);

					m_elements.push_back(startVertIndex + vertCountX);
					m_elements.push_back(startVertIndex + vertCountX + 1);
					m_elements.push_back(startVertIndex + 1);
				}
				else {
					m_elements.push_back(startVertIndex + vertCountX + 1);
					m_elements.push_back(startVertIndex + 1);
					m_elements.push_back(startVertIndex);

					m_elements.push_back(startVertIndex + vertCountX);
					m_elements.push_back(startVertIndex + vertCountX + 1);
					m_elements.push_back(startVertIndex);
				}

				alternateEdge = !alternateEdge;
			}
		}
	}

	//Sets the uv co-ordinates
	for (int z = 0; z < vertCountZ; z++) {
		for (int x = 0; x < vertCountX; x++) {
			int startVertIndex = (z * vertCountX) + x;
			m_uvs[startVertIndex] = glm::vec2((float)x, (float)z);
		}
	}



	m_elementCount = (int)m_elements.size();

	//Calculates normals for each vertex
	for (size_t i = 0; i < m_elements.size(); i += 3)
	{
		glm::vec3 newNormal = glm::cross(m_vertices[m_elements[i + 1]] - m_vertices[m_elements[i]], m_vertices[m_elements[i + 2]] - m_vertices[m_elements[i]]);
		m_normals[m_elements[i]] += newNormal;
		m_normals[m_elements[i + 1]] += newNormal;
		m_normals[m_elements[i + 2]] += newNormal;
	}

	for (size_t i = 0; i < m_normals.size(); i++) {
		glm::normalize(m_normals[i]);
	}

	//Applies noise to each vertex
	for (size_t i = 0; i < m_vertices.size(); i++)
	{
		m_vertices[i].y += KenPerlin(m_vertices[i].x, m_vertices[i].z);
	}

	//Creates VBOs for each piece of data the shader needs and binds them to a VAO
	GLuint vertexVBO;

	glGenBuffers(1, &vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint elementsVBO;

	glGenBuffers(1, &elementsVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_elements.size(), m_elements.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLuint textureCoordsVBO;

	glGenBuffers(1, &textureCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, textureCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * m_uvs.size(), m_uvs.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint normalsVBO;

	glGenBuffers(1, &normalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * m_normals.size(), m_normals.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, textureCoordsVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsVBO);

	glBindVertexArray(0);

	Helpers::ImageLoader image;

	//Loads a given texture for rendering if it exists
	if (image.Load(p)) {
		glGenTextures(1, &m_tex);
		glBindTexture(GL_TEXTURE_2D, m_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.Width(), image.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetData());
		glGenerateMipmap(GL_TEXTURE_2D);

	}
	else {
		image.Load("Data\\Error.png");
		glGenTextures(1, &m_tex);
		glBindTexture(GL_TEXTURE_2D, m_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.Width(), image.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	m_xForm = x;
}

void Terrain::Render(GLuint& p, glm::mat4 c, glm::mat4 m, std::vector<Light>& l, const Helpers::Camera& cam, GLuint& shadow, glm::mat4 ls)
{
	// Send the combined matrix to the shader in a uniform
	GLuint combined_xform_id = glGetUniformLocation(p, "combined_xform");
	glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(c));

	// Send the model matrix to the shader in a uniform
	GLuint model_xform_id = glGetUniformLocation(p, "model_xform");
	glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(m_xForm));

	GLuint camera_direction_id = glGetUniformLocation(p, "camera_direction");
	glUniform3fv(camera_direction_id, 1, glm::value_ptr(cam.GetLookVector()));

	GLuint shininess_id = glGetUniformLocation(p, "shininess");
	glUniform1f(shininess_id, 1.0f);

	//Decides which lights to use if the number of lights in the scene exceeds the total that can be passed into the shader
	//Decides based on the distance from the floor of the terrain
	//Directional lights are prioritised
	std::vector<Light> lightsToUse;
	if (l.size() <= 10) {
		lightsToUse = l;
	}
	else {
		std::vector<Light> lightsToConsider;

		for (Light& light : l) {
			if (lightsToConsider.size() == 0) {
				lightsToConsider.push_back(light);
				continue;
			}

			if (light.m_type == 0) {
				lightsToConsider.insert(lightsToConsider.begin(), light);
				continue;
			}

			float dist = glm::distance(glm::vec3(light.m_position.x, 0, light.m_position.z), light.m_position);

			for (size_t i = 0; i < lightsToConsider.size(); i++)
			{
				if (dist < glm::distance(glm::vec3(lightsToConsider[i].m_position.x, 0, lightsToConsider[i].m_position.z), lightsToConsider[i].m_position) && lightsToConsider[i].m_type != 0) {
					lightsToConsider.insert(lightsToConsider.begin() + 1, light);
					break;
				}

				if (i == lightsToConsider.size() - 1 && lightsToConsider.size() < 10) {
					lightsToConsider.push_back(light);
				}
			}

			//Removes the furthest light if the container becomes too full
			if (lightsToConsider.size() > 10) {
				lightsToConsider.pop_back();
			}
		}

		lightsToUse = lightsToConsider;
	}

	//Passes through the values of each light to the uniforms in the shader
	//Sends through lights with no colour or intensity if there are less lights than the maximum
	for (size_t i = 0; i < 10; i++)
	{
		if (i < lightsToUse.size()) {
			GLuint intensity_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_intensity").c_str());
			glUniform1f(intensity_id, lightsToUse[i].m_intensity);

			GLuint position_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_position").c_str());
			glUniform3fv(position_id, 1, glm::value_ptr(lightsToUse[i].m_position));

			GLuint type_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_type").c_str());
			glUniform1i(type_id, lightsToUse[i].m_type);

			GLuint direction_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_direction").c_str());
			glUniform3fv(direction_id, 1, glm::value_ptr(lightsToUse[i].m_direction));

			GLuint colour_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_colour").c_str());
			glUniform3fv(colour_id, 1, glm::value_ptr(lightsToUse[i].m_colour));

			GLuint angle_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_angle").c_str());
			glUniform1f(angle_id, lightsToUse[i].m_angle);
		}
		else {
			GLuint intensity_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_intensity").c_str());
			glUniform1f(intensity_id, 0.0f);

			GLuint position_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_position").c_str());
			glUniform3fv(position_id, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));

			GLuint type_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_type").c_str());
			glUniform1i(type_id, 0);

			GLuint direction_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_direction").c_str());
			glUniform1f(intensity_id, 0.0f);

			GLuint colour_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_colour").c_str());
			glUniform3fv(colour_id, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));

			GLuint angle_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_angle").c_str());
			glUniform1f(angle_id, 0.0f);
		}
	}

	GLuint ls_id = glGetUniformLocation(p, "lightSpaceMatrix");
	glUniformMatrix4fv(ls_id, 1, GL_FALSE, glm::value_ptr(ls));

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadow);
	glUniform1i(glGetUniformLocation(p, "shadow_tex"), 1);

	//Binds the texture and VAO, then draws the terrain
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glUniform1i(glGetUniformLocation(p, "sampler_tex"), 0);


	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_elementCount, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}

void Terrain::RenderAmbientPass(GLuint& p, glm::mat4 c, glm::mat4 m)
{
	// Send the combined matrix to the shader in a uniform
	GLuint combined_xform_id = glGetUniformLocation(p, "combined_xform");
	glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(c));

	// Send the model matrix to the shader in a uniform
	GLuint model_xform_id = glGetUniformLocation(p, "model_xform");
	glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(m_xForm));

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_elementCount, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}

void Terrain::RenderDepthPass(GLuint& p, glm::mat4 c, glm::mat4 m)
{
	// Send the combined matrix to the shader in a uniform
	GLuint combined_xform_id = glGetUniformLocation(p, "combined_xform");
	glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(c));

	// Send the model matrix to the shader in a uniform
	GLuint model_xform_id = glGetUniformLocation(p, "model_xform");
	glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(m_xForm));

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_elementCount, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}

float Terrain::PerlinNoise(int x, int y)
{
	int n = x + y * 57;
	n = (n >> 13) ^ n;
	int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;

	return 1.0f - ((float)nn / 1073741824.0f);
}

float Terrain::CosineLerp(float a, float b, float x)
{
	float ft = x * 3.1415927f;
	float f = (1.0f - std::cos(ft)) * 0.5f;
	return a * (1.0f - f) + b * f;
}

float Terrain::KenPerlin(float x, float z)
{
	float s = PerlinNoise((int)x, (int)z);
	float t = PerlinNoise((int)x + 1, (int)z);
	float u = PerlinNoise((int)x, (int)z + 1);
	float v = PerlinNoise((int)x + 1, (int)z + 1);
	float p1 = CosineLerp(s, t, x);
	float p2 = CosineLerp(u, v, x);
	return std::clamp(CosineLerp(p1, p2, z), -1.0f, 1.0f);
}