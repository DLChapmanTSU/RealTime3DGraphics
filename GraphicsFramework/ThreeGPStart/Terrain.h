#pragma once
#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"
#include "Model.h"

struct Light;

class Terrain : public Model {
private:
	int m_cellCountX{ 0 };
	int m_cellCountZ{ 0 };
	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec2> m_uvs;
	std::vector<glm::vec3> m_normals;
	std::vector<GLuint> m_elements;
	std::vector<GLfloat> m_colours;
	GLuint m_VAO{ 0 };
	int m_elementCount{ 0 };
	glm::mat4 m_xForm{ glm::mat4(1) };
	//GLuint m_tex{ 0 };
	std::string m_heightPath;
public:
	Terrain(int x = 0, int z = 0);
	void Initialise(glm::mat4 x, std::string& p, std::string& h);
	void Render(GLuint& p, glm::mat4 c, glm::mat4 m, std::vector<Light>& l, const Helpers::Camera& cam) override;
private:
	float PerlinNoise(int x, int y);
	float CosineLerp(float a, float b, float x);
	float KenPerlin(float x, float z);
};