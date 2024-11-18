#version 330
uniform sampler2D sampler_depth_tex;
uniform vec2 screen_resolution;

in vec2 uvCoord;

out vec4 fragment_colour;



void main(void)
{
	fragment_colour = vec4(texture2D(sampler_depth_tex, uvCoord).a, texture2D(sampler_depth_tex, uvCoord).a, texture2D(sampler_depth_tex, uvCoord).a, 1.0f);
}