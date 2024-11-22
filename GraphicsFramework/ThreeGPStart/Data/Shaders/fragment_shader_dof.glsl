#version 330
uniform sampler2D sampler_depth_tex;
uniform sampler2D sampler_colour_tex;
uniform vec2 screen_resolution;
uniform float aperture;
uniform float focalLength;
uniform float planeInFocus;

in vec2 uvCoord;

out vec4 fragment_colour;

float near = 0.01f;
float far = 100.0f;



//https://www.youtube.com/watch?app=desktop&v=3xGKu4T4SCU&t=270s
float LinearDepth(float depth)
{
	return (2.0f * near * far) / (far + near - (depth * 2.0f - 1.0f) * (far - near));
}

float CircleOfConfusion(float depth)
{
	float z = 1.0f / ((1.0f / near) - (((far - near) / (near * far)) * depth));
	return abs(aperture * (focalLength * (depth - planeInFocus)) / (depth * (planeInFocus - focalLength)));
	//return aperture * ((z - planeInFocus) / z);
}

void main(void)
{
	vec2 pixelScreenPos = (gl_FragCoord.xy / screen_resolution.xy);

	float rawDepth = texture2D(sampler_depth_tex, uvCoord).r;
	float finalDepth = LinearDepth(rawDepth);

	float CoC = CircleOfConfusion(finalDepth);

	//blur here

	vec4 pixelData = texture2D(sampler_colour_tex, uvCoord);
	vec2 texelSize = 1.0f / screen_resolution.xy;

	int count = 0;
	vec4 combinedColour = vec4(0.0f, 0.0f, 0.0f, 0.0f);

	for (float step = -CoC; step < CoC; step += texelSize.x)
	{
		combinedColour += texture2D(sampler_colour_tex, vec2(step * texelSize.x, gl_FragCoord.y));
		count++;
	}

	for (float step = -CoC; step < CoC; step += texelSize.y)
	{
		combinedColour += texture2D(sampler_colour_tex, vec2(gl_FragCoord.x, step * texelSize.y));
		count++;
	}

	//fragment_colour = vec4(finalDepth, finalDepth, finalDepth, 1.0f);
	//fragment_colour = combinedColour / count;
	fragment_colour = vec4(CoC, 0.0f, 0.0f, 1.0f);
	//fragment_colour = texture2D(sampler_depth_tex, uvCoord);
}