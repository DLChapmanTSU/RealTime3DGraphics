#pragma once

#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"
#include "ImageLoader.h"
#include "Camera.h"

struct Light;

//Class for a model.
//Mesh contains the data for the individual models that make up the larger one
class Model {
protected:
	struct Mesh {
	public:
		size_t m_elementCount{ 0 };
		GLuint m_VAO{ 0 };
		glm::mat4 m_modelXForm{ glm::mat4(1) };
	};
protected:
	std::vector<Mesh> m_meshes;
	GLuint m_tex;
	glm::vec3 m_modelWorldPosition;
	float m_shininess;
public:
	Model(glm::vec3 p, float s);
	void Initialise(std::vector<glm::mat4>& x, Helpers::ImageLoader& image, Helpers::ModelLoader& model);
	virtual void Render(GLuint& p, glm::mat4 c, glm::mat4 m, std::vector<Light>& l, const Helpers::Camera& cam);
	void Translate(glm::vec3& t);
};