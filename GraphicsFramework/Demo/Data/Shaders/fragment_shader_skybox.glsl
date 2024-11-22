#version 330

//uniform sampler2D sampler_tex;

//uniform vec4 diffuse_colour;
uniform samplerCube skySampler;

in vec3 texCoords;

out vec4 fragment_colour;

void main(void)
{
	fragment_colour = texture(skySampler, texCoords);
}