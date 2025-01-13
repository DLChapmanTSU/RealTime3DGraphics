#version 330
uniform sampler2D sampler_depth_tex;
uniform sampler2D sampler_colour_tex;
//uniform sampler2D sampler_accum_colours_tex[12];
uniform vec2 screen_resolution;
uniform float screen_scaling;
uniform float aperture;
uniform float focalLength;
uniform float planeInFocus;
uniform int horizontal;

in vec2 uvCoord;

out vec4 fragment_colour;

float near = 1.0f;
float far = 100.0f;
float circleOfConfusionCap = 10.0f;

float weights[9] = float[9](0.05f, 0.09f, 0.12f, 0.15f, 0.16f, 0.15f, 0.12f, 0.09f, 0.05f);



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
	vec2 pixelScreenPos = (gl_FragCoord.xy / (screen_resolution.xy));

	float rawDepth = texture2D(sampler_depth_tex, uvCoord).r;
	float finalDepth = LinearDepth(rawDepth);
	vec4 pixelData = texture2D(sampler_colour_tex, uvCoord);
	vec2 texelSize = 1.0f / (screen_resolution.xy);

	//if (finalDepth <= 0.0f)
	//{
	//	fragment_colour = pixelData;
	//	return;
	//}

	float CoC = CircleOfConfusion(finalDepth);

	//if (CoC <= 1.0f)
	//{
	//	fragment_colour = pixelData;
	//	return;
	//}

	if (CoC > circleOfConfusionCap)
	{
		CoC = circleOfConfusionCap;
	}

	//blur here

	float x,y,xx,yy,rr=CoC*CoC,d,w,w0;
    w0=0.5135/pow(CoC,0.96);
    vec2 p=uvCoord;
    vec4 col=vec4(0.0,0.0,0.0,0.0);

    if (horizontal==1)
	{
		for (int i = -4; i <= 4; i++)
		{
			if ((uvCoord.x + (texelSize.x * CoC) * i) > (0.95f * screen_scaling) || texture2D(sampler_colour_tex, vec2(clamp(uvCoord.x + ((texelSize.x * CoC) * i), 0.001f, 0.95f * screen_scaling), uvCoord.y)).a != 1.0f)
				col += texture2D(sampler_colour_tex, uvCoord) * weights[i + 4];
			else
				col += texture2D(sampler_colour_tex, vec2(clamp(uvCoord.x + ((texelSize.x * CoC) * i), 0.001f, 0.95f * screen_scaling), uvCoord.y)) * weights[i + 4];
		}
		/*col+=texture2D(sampler_colour_tex, vec2(uvCoord.x - ((texelSize.x * CoC) * 4.0f), uvCoord.y)) * 0.05f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x - ((texelSize.x * CoC) * 3.0f), uvCoord.y)) * 0.09f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x - ((texelSize.x * CoC) * 2.0f), uvCoord.y)) * 0.12f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x - ((texelSize.x * CoC) * 1.0f), uvCoord.y)) * 0.15f;
		col+=texture2D(sampler_colour_tex, uvCoord) * 0.16f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x + ((texelSize.x * CoC) * 1.0f), uvCoord.y)) * 0.15f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x + ((texelSize.x * CoC) * 2.0f), uvCoord.y)) * 0.12f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x + ((texelSize.x * CoC) * 3.0f), uvCoord.y)) * 0.09f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x + ((texelSize.x * CoC) * 4.0f), uvCoord.y)) * 0.05f;*/
		//for (d=texelSize.x,x=-CoC,p.x+=x*d;x<=CoC;x++,p.x+=d)
		//{ 
		//	w=w0*exp((-x*x)/(2.0*rr));
		//	col+=texture2D(sampler_colour_tex,p)*w; 
		//}
	}
    else
	{ 
		for (int i = -4; i <= 4; i++)
		{
			if ((uvCoord.y + (texelSize.y * CoC) * i) > (0.95f * screen_scaling) || texture2D(sampler_colour_tex, vec2(uvCoord.x, clamp(uvCoord.y + ((texelSize.y * CoC) * i), 0.001f, 0.95f * screen_scaling))).a != 1.0f)
				col += texture2D(sampler_colour_tex, uvCoord) * weights[i + 4];
			else
				col += texture2D(sampler_colour_tex, vec2(uvCoord.x, clamp(uvCoord.y + ((texelSize.y * CoC) * i), 0.001f, 0.95f * screen_scaling))) * weights[i + 4];
		}
		/*col+=texture2D(sampler_colour_tex, vec2(uvCoord.x, uvCoord.y - ((texelSize.y * CoC) * 4.0f))) * 0.05f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x, uvCoord.y - ((texelSize.y * CoC) * 3.0f))) * 0.09f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x, uvCoord.y - ((texelSize.y * CoC) * 2.0f))) * 0.12f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x, uvCoord.y - ((texelSize.y * CoC) * 1.0f))) * 0.15f;
		col+=texture2D(sampler_colour_tex, uvCoord) * 0.16f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x, uvCoord.y + ((texelSize.y * CoC) * 1.0f))) * 0.15f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x, uvCoord.y + ((texelSize.y * CoC) * 2.0f))) * 0.12f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x, uvCoord.y + ((texelSize.y * CoC) * 3.0f))) * 0.09f;
		col+=texture2D(sampler_colour_tex, vec2(uvCoord.x, uvCoord.y + ((texelSize.y * CoC) * 4.0f))) * 0.05f;*/
		//for (d=texelSize.y,y=-CoC,p.y+=y*d;y<=CoC;y++,p.y+=d)
		//{ 
		//	w=w0*exp((-y*y)/(2.0*rr));
		//	col+=texture2D(sampler_colour_tex,p)*w; 
		//}
	}
	fragment_colour = col;
}