#version 330

struct Light {
	int m_type;
	float m_intensity;
	vec3 m_position;
	vec3 m_direction;
	vec3 m_colour;
	float m_angle;
};

uniform vec4 diffuse_colour;
uniform sampler2D sampler_tex;
uniform Light lights[10];
//TODO - Move samplers to an array and matrix to the struct
uniform sampler2D shadow_tex;
uniform mat4 lightSpaceMatrix;
// End Of TODO
uniform vec3 camera_direction;
uniform float shininess;

in vec2 varying_coord;
in vec3 varying_normal;
in vec4 varying_position;

out vec4 fragment_colour;

float shadowBias = 0.005f;

bool IsInShadow(Light light)
{
	vec4 fragInLightSpace = lightSpaceMatrix * vec4(varying_position.xyz, 1.0f);
	vec3 projectionCoords = fragInLightSpace.xyz / fragInLightSpace.w;
	projectionCoords = (projectionCoords + 1.0) / 2.0;
	float currentDepth = projectionCoords.z;
	if (currentDepth > 1.0)
		return false;
	float closestDepth = texture2D(shadow_tex, projectionCoords.xy).r;
	bool isInShadow = currentDepth - shadowBias > closestDepth;
	return isInShadow;
}

void main(void)
{
	//Sets base values including the texture colour, ambient colour and the normals and position of the fragment
	vec3 tex_colour = texture(sampler_tex, varying_coord).rgb;
	vec3 normals = normalize(varying_normal);
	vec3 VP = vec3(varying_position.x, varying_position.y, varying_position.z);
	float totalIntensity = 0.1f;
	vec3 ambientColour = vec3(0.1f, 0.1f, 0.1f);
	vec3 I = ambientColour * tex_colour;
	vec3 sumOfColours = vec3(0.0f, 0.0f, 0.0f);
	vec3 rV;

	//Loops through each light and applies colour based on their type
	//0 = Directional, 1 = Point, 2 = Spot
	//Uses phong reflection, an expansion of lambert that includes specular lighting
	for (int i = 0; i < 10; i++) {
		int currentType = lights[i].m_type;

		if (currentType == 0) {
			//Directional light
			//Inverts the direction of the light to get the vector to the light
			//Uses it to calculate intensity based on the angle between it and the normals
			//Same thing for the specular reflection, except it is reflected and compared to the angle of the camera
			if (IsInShadow(lights[i]) == false)
			{
				rV = reflect(-lights[i].m_direction, varying_normal);
				rV = normalize(rV);
				float LR = max(0.0f, dot(normalize(camera_direction), rV));
				vec3 specular = tex_colour * pow(LR, shininess);
				vec3 zeroL = normalize(-lights[i].m_direction);
				float diffuse_intensity = max(0, dot(zeroL, normals));
				sumOfColours += ((diffuse_intensity * tex_colour) + specular) * lights[i].m_colour;
			}
		}
		/*else if (currentType == 1) {
			//Point light
			//Finds light direction by finding the vector between the vertex and the light
			//Inverts the direction of the light to get the vector to the light
			//Uses it to calculate intensity based on the angle between it and the normals
			//Same thing for the specular reflection, except it is reflected and compared to the angle of the camera
			vec3 L = lights[i].m_position - VP;
			float LMag = distance(lights[i].m_position, VP);
			float attenuation = 1.0 - smoothstep(0, 100, LMag);

			L = normalize(L);

			rV = reflect(L, varying_normal);
			rV = normalize(rV);
			float LR = max(0.0f, dot(normalize(camera_direction), rV));
			vec3 specular = tex_colour * pow(LR, shininess);
			specular *= attenuation;

			float intensity = max(0, dot(L, normals));
			intensity *= attenuation;

			sumOfColours += ((intensity * tex_colour) + specular) * lights[i].m_colour;
		}
		else if (currentType == 2) {
			//Spotlight
			//Same as a point light, except the fragment is only effected if it falls within the angle of the spotlight
			vec3 lightToFrag = normalize(VP - lights[i].m_position);
			vec3 dir = normalize(lights[i].m_direction);
			float dp = dot(lightToFrag, dir);
			float angle = acos(dp);

			if (angle <= lights[i].m_angle && angle >= -lights[i].m_angle) {
				vec3 L = lights[i].m_position - VP;
				float LMag = distance(lights[i].m_position, VP);
				float attenuation = 1.0 - smoothstep(0, 500, LMag);

				L = normalize(L);

				rV = reflect(L, varying_normal);
				rV = normalize(rV);
				float LR = max(0.0f, dot(normalize(camera_direction), rV));
				vec3 specular = tex_colour * pow(LR, shininess);
				specular *= attenuation;

				float intensity = max(0, dot(L, normals));
				intensity *= attenuation;

				sumOfColours += ((intensity * tex_colour) + specular) * lights[i].m_colour;
			}
		}*/


	}

	//Adds the sum of all of the lights to the base ambient colour
	I += sumOfColours;

	fragment_colour = vec4(I, 1.0);
}