#version 330

// TODO
uniform mat4 combined_xform;
uniform mat4 model_xform;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_colour;
//layout(location = 2) in vec2 vertex_texcoord;

out vec3 varying_colour;

void main(void)
{
	varying_colour = normalize(vertex_colour);
	gl_Position = combined_xform * model_xform * vec4(vertex_position, 1.0);
}