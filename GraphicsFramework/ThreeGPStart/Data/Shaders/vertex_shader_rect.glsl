#version 330

layout (location=0) in vec2 vertex_position;
layout (location=1) in vec2 vertex_texcoord;

out vec2 texCoords;

void main(void)
{	
	texCoords = vertex_texcoord;

	gl_Position = vec4(vertex_position, 0.0, 1.0);
}