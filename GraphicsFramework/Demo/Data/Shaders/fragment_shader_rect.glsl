#version 330

uniform sampler2D sampler_tex;

in vec2 uv;

out vec4 fragment_colour;

void main(void)
{
	fragment_colour = vec4(texture2D(sampler_tex, uv).rgb,1.0f);
}