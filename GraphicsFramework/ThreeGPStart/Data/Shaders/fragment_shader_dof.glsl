#version 330
uniform sampler2D sampler_depth_tex;
uniform sampler2D sampler_colour_tex;
uniform sampler2D sampler_accum_colours_tex[12];
uniform vec2 screen_resolution;
uniform float aperture;
uniform float focalLength;
uniform float planeInFocus;

in vec2 uvCoord;

out vec4 fragment_colour;

float near = 1.0f;
float far = 100.0f;
float circleOfConfusionCap = 10.0f;



//https://www.youtube.com/watch?app=desktop&v=3xGKu4T4SCU&t=270s
float LinearDepth(float depth)
{
	return (2.0f * near * far) / (far + near - (depth * 2.0f - 1.0f) * (far - near));
}

float CircleOfConfusion(float depth)
{
	float fLength = (1 / depth) + (1 / focalLength);
	fLength = fLength * fLength;
	float CoCScale = (aperture * fLength * planeInFocus * (far - near)) /
		((planeInFocus - fLength) * near * far);
	float CoCBias = (aperture * fLength * (near - planeInFocus)) /
		((planeInFocus * fLength) * near);

	
	float z = 1.0f / ((1.0f / near) - (((far - near) / (near * far)) * depth));
	return abs(z * CoCScale + CoCBias);
	
	//return abs(aperture * (fLength * (planeInFocus - depth)) / (depth * (planeInFocus - fLength)));
	//return aperture * ((z - planeInFocus) / z);
}

void main(void)
{
	vec2 pixelScreenPos = (gl_FragCoord.xy / screen_resolution.xy);

	float rawDepth = texture2D(sampler_depth_tex, uvCoord).r;
	float finalDepth = LinearDepth(rawDepth);
	vec4 pixelData = texture2D(sampler_colour_tex, uvCoord);
	vec2 texelSize = 1.0f / screen_resolution.xy;

	if (finalDepth <= 0.0f)
	{
		fragment_colour = pixelData;
		return;
	}

	float CoC = CircleOfConfusion(finalDepth);

	//blur here

	

	int count = 0;
	vec4 combinedColour = vec4(0.0f, 0.0f, 0.0f, 0.0f);

	if (CoC <= 0.0f)
	{
		fragment_colour = pixelData;
	}
	else
	{
		if (CoC > circleOfConfusionCap)
			CoC = circleOfConfusionCap;

		vec4 accumulation = pixelData;

		for (int i = 0; i < 12; i++)
		{
			accumulation += texture2D(sampler_accum_colours_tex[i], pixelScreenPos);
		}

		//accumulation *= CoC;
	
		vec4 averageColour = (accumulation) / 13.0f;
		//averageColour = (averageColour / circleOfConfusionCap);
		//averageColour += pixelData;
		//averageColour /= 2.0f;

		//if (length(averageColour) != length(pixelData))
		//{
		//	averageColour = normalize(averageColour);
		//	averageColour *= length(pixelData);
		//	averageColour.w = pixelData.w;
		//}

		//fragment_colour = (pixelData + (averageColour * CoC)) / (2.0f + CoC);
		fragment_colour = averageColour;
	}
}