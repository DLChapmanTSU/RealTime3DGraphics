#version 330
uniform sampler2D sampler_tex;
uniform vec2 screen_resolution;

in vec2 uvCoord;

out vec4 fragment_colour;

float targetContrast = 0.0f;
int stepLimit = 24;

//https://stackoverflow.com/questions/596216/formula-to-determine-perceived-brightness-of-rgb-color
float calculateLuminance(vec4 color)
{
	return ((0.2126 * color.x) + (0.7152 * color.y) + (0.0772 * color.z));
}

vec4 calculateFXAA(vec2 pos, vec4 pixel, vec2 texSize)
{
	//Need to use a mat4 instead. This is too messy
	//Need to define a mat4 to aply weights

	vec4 coloursRowOne[5];
	vec4 coloursRowTwo[5];
	vec4 coloursRowThree[5];
	vec4 coloursRowFour[5];
	vec4 coloursRowFive[5];
	coloursRowThree[2] = pixel;

	float luminanceRowOne[5];
	float luminanceRowTwo[5];
	float luminanceRowThree[5];
	float luminanceRowFour[5];
	float luminanceRowFive[5];

	float currentContrast = 0.001f;
	float baseLuminance = calculateLuminance(pixel);

	luminanceRowThree[2] = baseLuminance;

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (i == 2 && j == 2)
			{
				continue;
			}
			vec2 offset = vec2((i - 2) * texSize.x, (j - 2) * texSize.y);
			vec2 pixPos = pos + offset;
			vec4 pixColour = texture2D(sampler_tex, pixPos);

			if (pixPos.x <= 0.1f || pixPos.x >= 0.9f || pixPos.y <= 0.1f || pixPos.y >= 0.9f || pixColour.a != 1.0f)
			{
				pixColour = pixel;
				pixPos = pos;
			}

			float pixLum = calculateLuminance(pixColour);

			if (abs(pixLum - baseLuminance) > currentContrast)
			{
				currentContrast = abs(pixLum - baseLuminance);
			}

			switch (i)
			{
			case 0:
				coloursRowOne[j] = texture2D(sampler_tex, pixPos);
				luminanceRowOne[j] = pixLum;
				break;
			case 1:
				coloursRowTwo[j] = texture2D(sampler_tex, pixPos);
				luminanceRowTwo[j] = pixLum;
				break;
			case 2:
				coloursRowThree[j] = texture2D(sampler_tex, pixPos);
				luminanceRowThree[j] = pixLum;
				break;
			case 3:
				coloursRowFour[j] = texture2D(sampler_tex, pixPos);
				luminanceRowFour[j] = pixLum;
				break;
			case 4:
				coloursRowFive[j] = texture2D(sampler_tex, pixPos);
				luminanceRowFive[j] = pixLum;
				break;
			default:
				break;
			}
		}
	}

	if (currentContrast < targetContrast)
	{
		return pixel;
	}
	
	vec4 newColour = vec4(0);
	newColour += coloursRowTwo[1] * 0.0947416;
	newColour += coloursRowTwo[3] * 0.0947416;
	newColour += coloursRowFour[1] * 0.0947416;
	newColour += coloursRowFour[3] * 0.0947416;
	newColour += coloursRowTwo[2] * 0.118318;
	newColour += coloursRowThree[1] * 0.118318;
	newColour += coloursRowThree[3] * 0.118318;
	newColour += coloursRowFour[2] * 0.118318;
	newColour += coloursRowThree[2] * 0.147761;

	return newColour;
}

void main(void)
{
	vec2 pixelScreenPos = (gl_FragCoord.xy / screen_resolution.xy);
	vec4 pixelData = texture2D(sampler_tex, pixelScreenPos);
	vec2 texelSize = 1.0f / screen_resolution.xy;
	vec4 finalColour = calculateFXAA(pixelScreenPos, pixelData, texelSize);
	finalColour = vec4(finalColour.xyz, 1.0f);
	fragment_colour = finalColour;
	//fragment_colour = vec4(texture2D(sampler_tex, uvCoord).rgb, 1.0f);
	//fragment_colour = vec4(pixelScreenPos, 0.0f, 1.0f);
}