#version 330

uniform mat4 combined_xform;
uniform mat4 model_xform;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_texcoord;
layout(location = 2) in vec3 vertex_normal;


out vec2 varying_coord;
out vec3 varying_normal;
out vec4 varying_position;

void main(void)
{
	varying_coord = vertex_texcoord;
	varying_normal = vertex_normal;
	varying_position = model_xform * vec4(vertex_position, 1.0);

	gl_Position = combined_xform * model_xform * vec4(vertex_position, 1.0);
}