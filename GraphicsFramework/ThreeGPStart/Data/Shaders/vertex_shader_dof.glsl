#version 330

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_uv;

out vec2 uvCoord;

void main(void)
{
	gl_Position = vec4(vertex_position, 1.0);
	uvCoord = vertex_uv;	
}