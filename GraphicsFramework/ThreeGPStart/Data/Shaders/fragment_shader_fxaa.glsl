#version 330

uniform sampler2DMS screenTexture;

void main(void)
{
	vec2 pixelScreenPos = gl_GlobalInvocationID.xy;
	vec4 pixelData = texelFetch(screenTexture, pixelScreenPos, 3);
}