#pragma once

#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"

#include <vector>
#include <string>

class Skybox {
private:
	std::string m_texLocations[6];
	GLuint m_VAO{ 0 };
	GLuint m_tex;
	std::string m_objLocation;
	Helpers::ModelLoader m_loader;
	int m_elementCount{ 0 };
public:
	Skybox(std::string r, std::string l, std::string t, std::string b, std::string k, std::string f, std::string o);
	void Initialize();
	void Render(GLuint& p, glm::mat4 c);
};
