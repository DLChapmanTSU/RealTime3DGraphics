#pragma once
#include "ExternalLibraryHeaders.h"

//Holds the data for the lights
//This includes the type of light, the intensity, the position, the rotation, the colour and the angle
//Not all lights will use all of these since some lights don't need a position, direction or rotation
struct Light {
public:
	int m_type{ 0 };
	float m_intensity{ 0.0f };
	glm::vec3 m_position{ glm::vec3(0) };
	glm::vec3 m_direction{ glm::vec3(0.0f, -1.0f, 0.0f) };
	glm::vec3 m_colour{ glm::vec3(1) };
	float m_angle{ 0.0f };
	GLuint m_shadowMap;
	glm::mat4 m_lightSpaceMatrix;
};