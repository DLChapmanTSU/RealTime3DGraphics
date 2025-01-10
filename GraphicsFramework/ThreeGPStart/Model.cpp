#include "Model.h"
#include "Helper.h"
#include "ImageLoader.h"
#include "Light.h"

#include <list>

Model::Model(glm::vec3 p, float s)
{
	//m_objLocations = l;
	//m_texLocation = t;
	m_tex = 0;
	m_modelWorldPosition = p;
	m_shininess = s;
}

void Model::Initialise(std::vector<glm::mat4>& x, Helpers::ImageLoader& image, Helpers::ModelLoader& model)
{
	/*if (x.size() != m_objLocations.size()) {
		return;
	}*/

	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.Width(), image.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	size_t xFormToUse{ 0 };

	//Loop loads creates a mesh for each model loaded
	//Each mesh has its own VAO and VBOs
	//Each mesh utilises the same texture
	for (Helpers::Mesh& mesh : model.GetMeshVector()) {
		Mesh temp;
		temp.m_elementCount = mesh.elements.size();

		//Creates VBOs out of loaded data
		GLuint vertexVBO;

		glGenBuffers(1, &vertexVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint elementsVBO;

		glGenBuffers(1, &elementsVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.elements.size(), mesh.elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		GLuint textureCoordsVBO;

		glGenBuffers(1, &textureCoordsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, textureCoordsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.uvCoords.size(), mesh.uvCoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint normalsVBO;

		glGenBuffers(1, &normalsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//Binds all VBOs to the VAO
		glGenVertexArrays(1, &temp.m_VAO);
		glBindVertexArray(temp.m_VAO);

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


		//Sets local xForms for the mesh
		temp.m_modelXForm = x[xFormToUse];
		xFormToUse++;

		m_meshes.push_back(temp);
	}


}

void Model::Render(GLuint& p, glm::mat4 c, glm::mat4 m, std::vector<Light>& l, const Helpers::Camera& cam)
{
	//Decides which lights to use if the number of lights in the scene exceeds the total that can be passed into the shader
	//Decides based on the distance from the root of the model
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

			float dist = glm::distance(m_modelWorldPosition, light.m_position);

			for (size_t i = 0; i < lightsToConsider.size(); i++)
			{
				if (dist < glm::distance(m_modelWorldPosition, lightsToConsider[i].m_position) && lightsToConsider[i].m_type != 0) {
					lightsToConsider.insert(lightsToConsider.begin() + 1, light);
					break;
				}

				if (i == lightsToConsider.size() - 1 && lightsToConsider.size() < 10) {
					lightsToConsider.push_back(light);
				}
			}

			if (lightsToConsider.size() > 10) {
				lightsToConsider.pop_back();
			}
		}

		lightsToUse = lightsToConsider;
	}

	GLuint camera_direction_id = glGetUniformLocation(p, "camera_direction");
	glUniform3fv(camera_direction_id, 1, glm::value_ptr(cam.GetLookVector()));

	GLuint shininess_id = glGetUniformLocation(p, "shininess");
	glUniform1f(shininess_id, m_shininess);

	//For each mesh, a draw call is made
	//The uniforms for the lights are consistent between each mesh
	//Sends through lights with no colour or intensity if there are less lights than the maximum
	for (size_t m = 0; m < m_meshes.size(); m++) {
		GLuint combined_xform_id = glGetUniformLocation(p, "combined_xform");
		glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(c));

		// Send the model matrix to the shader in a uniform
		GLuint model_xform_id = glGetUniformLocation(p, "model_xform");
		glm::mat4 translation = glm::translate(glm::mat4(1), m_modelWorldPosition);
		glm::mat4 newWorldPosition = translation * m_meshes[m].m_modelXForm;
		glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(newWorldPosition));

		//glm::vec3 camDir = cam.GetLookVector();




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

				GLuint matrix_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_lightSpaceMatrix").c_str());
				glUniformMatrix4fv(matrix_id, 1, GL_FALSE, glm::value_ptr(lightsToUse[i].m_lightSpaceMatrix));

				glActiveTexture(GL_TEXTURE1 + i);
				glBindTexture(GL_TEXTURE_2D, lightsToUse[i].m_shadowMap);
				glUniform1i(glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_shadowMap").c_str()), 1 + i);
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

				GLuint matrix_id = glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_lightSpaceMatrix").c_str());
				glUniformMatrix4fv(matrix_id, 1, GL_FALSE, glm::value_ptr(glm::mat4(0)));

				glActiveTexture(GL_TEXTURE1 + i);
				glBindTexture(GL_TEXTURE_2D, 0);
				glUniform1i(glGetUniformLocation(p, std::string("lights[" + std::to_string(i) + "].m_shadowMap").c_str()), 1 + i);
			}
		}

		/*GLuint ls_id = glGetUniformLocation(p, "lightSpaceMatrix");
		glUniformMatrix4fv(ls_id, 1, GL_FALSE, glm::value_ptr(ls));

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadow);
		glUniform1i(glGetUniformLocation(p, "shadow_tex"), 1);*/

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_tex);
		glUniform1i(glGetUniformLocation(p, "sampler_tex"), 0);


		glBindVertexArray(m_meshes[m].m_VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)m_meshes[m].m_elementCount, GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
	}
}

void Model::RenderAmbientPass(GLuint& p, glm::mat4 c, glm::mat4 m)
{
	for (size_t m = 0; m < m_meshes.size(); m++) {
		GLuint combined_xform_id = glGetUniformLocation(p, "combined_xform");
		glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(c));

		// Send the model matrix to the shader in a uniform
		GLuint model_xform_id = glGetUniformLocation(p, "model_xform");
		glm::mat4 translation = glm::translate(glm::mat4(1), m_modelWorldPosition);
		glm::mat4 newWorldPosition = translation * m_meshes[m].m_modelXForm;
		glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(newWorldPosition));


		glBindVertexArray(m_meshes[m].m_VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)m_meshes[m].m_elementCount, GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
	}
}

void Model::RenderDepthPass(GLuint& p, glm::mat4 c, glm::mat4 m)
{
	for (size_t m = 0; m < m_meshes.size(); m++) {
		GLuint combined_xform_id = glGetUniformLocation(p, "combined_xform");
		glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(c));

		// Send the model matrix to the shader in a uniform
		GLuint model_xform_id = glGetUniformLocation(p, "model_xform");
		glm::mat4 translation = glm::translate(glm::mat4(1), m_modelWorldPosition);
		glm::mat4 newWorldPosition = translation * m_meshes[m].m_modelXForm;
		glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(newWorldPosition));


		glBindVertexArray(m_meshes[m].m_VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)m_meshes[m].m_elementCount, GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
	}
}

void Model::Translate(glm::vec3& t)
{
	m_modelWorldPosition += t;
}