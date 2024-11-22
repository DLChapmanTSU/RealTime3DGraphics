#version 330

uniform mat4 combined_xform;

layout (location=0) in vec3 vertex_position;
layout (location=1) in vec3 vertex_texcoord;

out vec3 texCoords;

void main(void)
{	
	texCoords = vertex_texcoord;

	gl_Position = combined_xform * vec4(vertex_position, 1.0);
}