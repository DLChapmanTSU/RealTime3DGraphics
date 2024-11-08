#version 330
uniform sampler2D sampler_tex;
uniform vec2 screen_resolution;

in vec2 uvCoord;

out vec4 fragment_colour;

float targetContrast = 0.01f;

float weightsRowOne[5] = float[5](1.0f, 1.25f, 2.5f, 1.25f, 1.0f);
float weightsRowTwo[5] = float[5](1.0f, 2.5f, 3.0f, 2.5f, 1.0f);
float weightsRowThree[5] = float[5](2.5f, 3.0f, 3.5f, 3.0f, 2.5f);
float weightsRowFour[5] = float[5](1.0f, 2.5f, 3.0f, 2.5f, 1.0f);
float weightsRowFive[5] = float[5](1.0f, 1.25f, 2.5f, 1.25f, 1.0f);

struct LuminanceSample
{
	float m;
	float n;
	float s;
	float e;
	float w;
	float ne;
	float se;
	float sw;
	float nw;
	float contrast;
};

//https://stackoverflow.com/questions/596216/formula-to-determine-perceived-brightness-of-rgb-color
float calculateLuminance(vec4 color)
{
	return ((0.2126 * color.x) + (0.7152 * color.y) + (0.0772 * color.z));
}

vec4 calculateFXAA(vec2 pos, vec4 pixel, vec2 texSize)
{
	LuminanceSample luminanceSample;
	luminanceSample.m = calculateLuminance(pixel);
	vec4 rgbN = pixel;
	vec4 rgbS = pixel;
	vec4 rgbE = pixel;
	vec4 rgbW = pixel;
	vec4 rgbNE = pixel;
	vec4 rgbSE = pixel;
	vec4 rgbSW = pixel;
	vec4 rgbNW = pixel;

	//Need to use a mat4 instead. This is too messy
	//Need to define a mat4 to aply weights

	vec4 coloursRowOne[5];
	vec4 coloursRowTwo[5];
	vec4 coloursRowThree[5];
	vec4 coloursRowFour[5];
	vec4 coloursRowFive[5];
	coloursRowThree[2] = pixel * weightsRowThree[2];

	float currentContrast = 0.0f;
	float baseLuminance = calculateLuminance(pixel);

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
			float pixLum = calculateLuminance(pixColour);

			if (abs(pixLum - baseLuminance) > currentContrast)
			{
				currentContrast = abs(pixLum - baseLuminance);
			}

			switch (i)
			{
			case 0:
				coloursRowOne[j] = texture2D(sampler_tex, pixPos) * weightsRowOne[j];
				break;
			case 1:
				coloursRowTwo[j] = texture2D(sampler_tex, pixPos) * weightsRowTwo[j];
				break;
			case 2:
				coloursRowThree[j] = texture2D(sampler_tex, pixPos) * weightsRowThree[j];
				break;
			case 3:
				coloursRowFour[j] = texture2D(sampler_tex, pixPos) * weightsRowFour[j];
				break;
			case 4:
				coloursRowFive[j] = texture2D(sampler_tex, pixPos) * weightsRowFive[j];
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

	vec4 finalColour = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	float sumOfWeights = 0.0f;

	for (int i = 0; i < 5; i++)
	{
		finalColour += coloursRowOne[i] + coloursRowTwo[i] + coloursRowThree[i] + coloursRowFour[i] + coloursRowFive[i];
		sumOfWeights += weightsRowOne[i] + weightsRowTwo[i] + weightsRowThree[i] + weightsRowFour[i] + weightsRowFive[i];
	}

	return finalColour / sumOfWeights;

	if (pos.y + texSize.y > 1.0f)
	{
		luminanceSample.n = luminanceSample.m;
	}
	else
	{
		vec2 nPos = pos;
		nPos.y = nPos.y + texSize.y;
		rgbN = texture2D(sampler_tex, nPos);
		luminanceSample.n = calculateLuminance(rgbN);
	}

	luminanceSample.contrast = abs(luminanceSample.n - luminanceSample.m);

	if (pos.y - texSize.y < -1.0f)
	{
		luminanceSample.s = luminanceSample.m;
	}
	else
	{
		vec2 sPos = pos;
		sPos.y = sPos.y - texSize.y;
		rgbS = texture2D(sampler_tex, sPos);
		luminanceSample.s = calculateLuminance(rgbS);
	}

	if (abs(luminanceSample.s - luminanceSample.m) > luminanceSample.contrast)
	{
		luminanceSample.contrast = abs(luminanceSample.s - luminanceSample.m);
	}

	if (pos.x + texSize.x > 1.0f)
	{
		luminanceSample.e = luminanceSample.m;
	}
	else
	{
		vec2 ePos = pos;
		ePos.x = ePos.x + texSize.x;
		rgbE = texture2D(sampler_tex, ePos);
		luminanceSample.e = calculateLuminance(rgbE);
	}

	if (abs(luminanceSample.e - luminanceSample.m) > luminanceSample.contrast)
	{
		luminanceSample.contrast = abs(luminanceSample.e - luminanceSample.m);
	}

	if (pos.x - texSize.x < -1.0f)
	{
		luminanceSample.w = luminanceSample.m;
	}
	else
	{
		vec2 wPos = pos;
		wPos.x = wPos.x - texSize.x;
		rgbW = texture2D(sampler_tex, wPos);
		luminanceSample.w = calculateLuminance(rgbW);
	}

	if (abs(luminanceSample.w - luminanceSample.m) > luminanceSample.contrast)
	{
		luminanceSample.contrast = abs(luminanceSample.w - luminanceSample.m);
	}

	//Diagonals
	if (pos.x + texSize.x > 1.0f && pos.y + texSize.y > 1.0f)
	{
		luminanceSample.ne = luminanceSample.m;
	}
	else
	{
		vec2 nePos = pos;
		nePos.x = nePos.x + texSize.x;
		nePos.y = nePos.y + texSize.y;
		rgbNE = texture2D(sampler_tex, nePos);
		luminanceSample.ne = calculateLuminance(rgbNE);
	}

	if (abs(luminanceSample.ne - luminanceSample.m) > luminanceSample.contrast)
	{
		luminanceSample.contrast = abs(luminanceSample.ne - luminanceSample.m);
	}

	if (pos.x + texSize.x > 1.0f && pos.y - texSize.y < -1.0f)
	{
		luminanceSample.se = luminanceSample.m;
	}
	else
	{
		vec2 sePos = pos;
		sePos.x = sePos.x + texSize.x;
		sePos.y = sePos.y - texSize.y;
		rgbSE = texture2D(sampler_tex, sePos);
		luminanceSample.se = calculateLuminance(rgbSE);
	}

	if (abs(luminanceSample.se - luminanceSample.m) > luminanceSample.contrast)
	{
		luminanceSample.contrast = abs(luminanceSample.se - luminanceSample.m);
	}

	if (pos.x - texSize.x < -1.0f && pos.y - texSize.y < -1.0f)
	{
		luminanceSample.sw = luminanceSample.m;
	}
	else
	{
		vec2 swPos = pos;
		swPos.x = swPos.x - texSize.x;
		swPos.y = swPos.y - texSize.y;
		rgbSW = texture2D(sampler_tex, swPos);
		luminanceSample.se = calculateLuminance(rgbSW);
	}

	if (abs(luminanceSample.sw - luminanceSample.m) > luminanceSample.contrast)
	{
		luminanceSample.contrast = abs(luminanceSample.sw - luminanceSample.m);
	}

	if (pos.x - texSize.x < -1.0f && pos.y + texSize.y > 1.0f)
	{
		luminanceSample.nw = luminanceSample.m;
	}
	else
	{
		vec2 nwPos = pos;
		nwPos.x = nwPos.x - texSize.x;
		nwPos.y = nwPos.y + texSize.y;
		rgbSW = texture2D(sampler_tex, nwPos);
		luminanceSample.se = calculateLuminance(rgbNW);
	}

	if (abs(luminanceSample.nw - luminanceSample.m) > luminanceSample.contrast)
	{
		luminanceSample.contrast = abs(luminanceSample.nw - luminanceSample.m);
	}

	if (luminanceSample.contrast < targetContrast)
	{
		return pixel;
	}

	//TODO - Calculate edge direction and decide which pixel samples to blend with
	//DO horizontal and vertical first. Could do diagonals later

	return ((pixel * 4.0f) + (rgbN * 2.0f) + (rgbS * 2.0f) + (rgbE * 2.0f) + (rgbW * 2.0f) + rgbNE + rgbSE + rgbSW + rgbNW) / 16.0f;
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