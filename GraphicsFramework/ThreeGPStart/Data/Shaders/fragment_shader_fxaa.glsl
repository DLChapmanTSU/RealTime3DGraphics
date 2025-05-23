#version 330
uniform sampler2D sampler_tex;
uniform vec2 screen_resolution;
uniform float screen_scaling;

in vec2 uvCoord;

out vec4 fragment_colour;

float targetContrast = 0.0f;
int stepLimit = 24;

float weightsRowOne[5] = float[5](1.0f, 1.25f, 2.0f, 1.25f, 1.0f);
float weightsRowTwo[5] = float[5](1.0f, 1.5f, 2.0f, 1.5f, 1.0f);
float weightsRowThree[5] = float[5](2.0f, 2.0f, 3.0f, 2.0f, 2.0f);
float weightsRowFour[5] = float[5](1.0f, 1.5f, 2.0f, 1.5f, 1.0f);
float weightsRowFive[5] = float[5](1.0f, 1.25f, 2.0f, 1.25f, 1.0f);

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
	/*LuminanceSample luminanceSample;
	luminanceSample.m = calculateLuminance(pixel);
	vec4 rgbN = pixel;
	vec4 rgbS = pixel;
	vec4 rgbE = pixel;
	vec4 rgbW = pixel;
	vec4 rgbNE = pixel;
	vec4 rgbSE = pixel;
	vec4 rgbSW = pixel;
	vec4 rgbNW = pixel;*/

	//Need to use a mat4 instead. This is too messy
	//Need to define a mat4 to aply weights

	vec4 coloursRowOne[5];
	vec4 coloursRowTwo[5];
	vec4 coloursRowThree[5];
	vec4 coloursRowFour[5];
	vec4 coloursRowFive[5];
	coloursRowThree[2] = pixel * weightsRowThree[2];

	float luminanceRowOne[5];
	float luminanceRowTwo[5];
	float luminanceRowThree[5];
	float luminanceRowFour[5];
	float luminanceRowFive[5];

	float currentContrast = 0.0f;
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

	//TODO - Edge Detection
	//https://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html
	float horizontalEdge = abs(-2.0f * luminanceRowThree[1] + (luminanceRowTwo[1] + luminanceRowFour[1])) + abs(-2.0f * luminanceRowThree[2] + (luminanceRowTwo[2] + luminanceRowFour[2])) + abs(-2.0f * luminanceRowThree[3] + (luminanceRowTwo[3] + luminanceRowFour[3]));
	float verticalEdge = abs(-2.0f * luminanceRowTwo[2] + (luminanceRowTwo[1] + luminanceRowTwo[3])) + abs(-2.0f * luminanceRowThree[2] + (luminanceRowThree[1] + luminanceRowThree[3])) + abs(-2.0f * luminanceRowFour[2] + (luminanceRowFour[1] + luminanceRowFour[3]));

	bool isHorizontal = (horizontalEdge > verticalEdge);

	float neighbourOneLuma = isHorizontal ? luminanceRowFour[2] : luminanceRowThree[1];
	float neighbourTwoLuma = isHorizontal ? luminanceRowTwo[2] : luminanceRowThree[3];

	float neighbourOneDiff = neighbourOneLuma - luminanceRowThree[2];
	float neighbourTwoDiff = neighbourTwoLuma - luminanceRowThree[2];

	bool isOneLarger = abs(neighbourOneDiff) >= abs(neighbourTwoDiff);

	float scaled = 0.25f * max(abs(neighbourOneDiff), abs(neighbourTwoDiff));
	float stepLength = isHorizontal ? texSize.y : texSize.x;

	float averageLumaInDir = 0.0f;

	if (isOneLarger)
	{
		stepLength = -stepLength;
		averageLumaInDir = (neighbourOneLuma + luminanceRowThree[2]) / 2.0f;
	}
	else
	{
		//stepLength = -stepLength;
		averageLumaInDir = (neighbourTwoLuma + luminanceRowThree[2]) / 2.0f;
	}

	vec2 currentPos = pos;

	if (isHorizontal)
	{
		currentPos.y += stepLength * 0.5f;
	}
	else
	{
		currentPos.x += stepLength * 0.5f;
	}

	vec2 offset = isHorizontal ? vec2(texSize.x, 0.0f) : vec2(0.0f, texSize.y);

	
	bool endOneReached = false;
	bool endTwoReached = false;
	bool bothReached;
	int stepCount = 1;
	float lumaEndOne = 0.0f;
	float lumaEndTwo = 0.0f;
	vec2 dirEndOne = currentPos;
	vec2 dirEndTwo = currentPos;

	while (stepCount <= stepLimit && !bothReached)
	{
		if (!endOneReached)
		{
			lumaEndOne = calculateLuminance(texture2D(sampler_tex, dirEndOne));
			lumaEndOne -= averageLumaInDir;
		}

		if (!endTwoReached)
		{
			lumaEndTwo = calculateLuminance(texture2D(sampler_tex, dirEndTwo));
			lumaEndTwo -= averageLumaInDir;
			dirEndTwo += offset;
		}
			

		endOneReached = abs(lumaEndOne) >= scaled;
		endTwoReached = abs(lumaEndTwo) >= scaled;
		bothReached = endOneReached && endTwoReached;
		stepCount++;

		if (!endOneReached)
			dirEndOne -= offset;

		if (!endTwoReached)
			dirEndTwo += offset;

		if (dirEndOne.x <= 0.1f || dirEndOne.x >= 0.9f || dirEndOne.y <= 0.1f || dirEndOne.y >= 0.9f || texture2D(sampler_tex, dirEndOne).a != 1.0f)
			endOneReached = true;

		if (dirEndTwo.x <= 0.1f || dirEndTwo.x >= 0.9f || dirEndTwo.y <= 0.1f || dirEndTwo.y >= 0.9f || texture2D(sampler_tex, dirEndTwo).a != 1.0f)
			endTwoReached = true;

		bothReached = endOneReached && endTwoReached;
	}

	float distToOne = isHorizontal ? (currentPos.x - dirEndOne.x) : (currentPos.y - dirEndOne.y);
	float distToTwo = isHorizontal ? (dirEndTwo.x - currentPos.x) : (dirEndTwo.y - currentPos.y);
	bool isDirOne = distToOne < distToTwo;
	float finalDist = min(distToOne, distToTwo);
	float edgeLength = distToOne + distToTwo;
	float pixOffset = -finalDist / edgeLength + 0.5f;
	bool isMidSmaller = luminanceRowThree[2] < averageLumaInDir;
	bool isVarCorrect = ((isDirOne ? lumaEndOne : lumaEndTwo) < 0.0f) != isMidSmaller;
	float offsetFinal = isVarCorrect ? pixOffset : 0.0f;

	float surroundingLuma = (1.0f / 12/0f) * (2.0f * (luminanceRowTwo[2] + luminanceRowFour[2] + luminanceRowThree[1] + luminanceRowThree[3]) + luminanceRowTwo[1] + luminanceRowFour[1] + luminanceRowTwo[3] + luminanceRowFour[3]);

	float subPixelOffset1 = clamp(abs(surroundingLuma - luminanceRowThree[2]) / currentContrast, 0.0, 1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * 0.75f;
	offsetFinal = max(offsetFinal, subPixelOffsetFinal);

	vec2 blurredUv = currentPos;
	if (isHorizontal)
		blurredUv.y += offsetFinal * stepLength;
	else
		blurredUv.x += offsetFinal * stepLength;

	return texture2D(sampler_tex, blurredUv);
}

void main(void)
{
	vec2 pixelScreenPos = (gl_FragCoord.xy / (screen_resolution.xy));
	vec4 pixelData = texture2D(sampler_tex, pixelScreenPos);
	vec2 texelSize = 1.0f / (screen_resolution.xy);
	vec4 finalColour = calculateFXAA(pixelScreenPos, pixelData, texelSize);
	finalColour = vec4(finalColour.xyz, 1.0f);
	fragment_colour = finalColour;
	//fragment_colour = vec4(texture2D(sampler_tex, uvCoord).rgb, 1.0f);
	//fragment_colour = vec4(pixelScreenPos, 0.0f, 1.0f);
}