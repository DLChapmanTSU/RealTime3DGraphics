#version 330
uniform sampler2D screenSampler;

in vec2 texCoords;

out vec4 fragment_colour;

void main(void)
{
	fragment_colour = texture(screenSampler, texCoords);
}