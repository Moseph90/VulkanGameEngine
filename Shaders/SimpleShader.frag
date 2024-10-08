#version 450

//*******************************************************************************************************
//This is the fragment shader. A fragment shader is capable of outputting to multiple different locations
//There is no built in output variable for this one so we have to build our own.
//*******************************************************************************************************

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

// Layout qualifier takes a location value, the "out" qualifier indicates 
// that this variable is for output the rest is for the name and type.

layout (location = 0) out vec4 outColor;

struct PointLight {
	vec4 position;	// Ignore w
	vec4 color;	// w is intensity
};

// set and binding must match what we used when setting up our descriptor set layout
layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	mat4 inverseView;
	vec4 ambientLightColor; // The 4th dimension is intensity
	PointLight pointLights[10];
	int numLights;
} ubo;

// This communicates with the push constants struct in RenderSystem.cpp
layout (push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main() {
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLight = vec3(0.0); 	// Holds the total for each point light specular contribution
	vec3 surfaceNormal = normalize(fragNormalWorld);

	vec3 cameraPosWorld = ubo.inverseView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	// Loop through each point light available
	for (int i = 0; i < ubo.numLights; i++) {
		PointLight light = ubo.pointLights[i];
		
		// We only care about the first 3 dimensions in this calculation
		vec3 directionToLight = light.position.xyz - fragPosWorld;
	
		// Add attenuation. Using the dot product of a vector with itself is an easy
		// and efficient way to	calculate the length of the vector squared. We also 
		// this before we normalize the direction vector to get an accurate value
		float attenuation = 1.0 / dot(directionToLight, directionToLight);
		directionToLight = normalize(directionToLight);
		
		float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuseLight += intensity * cosAngleIncidence;

		// Specular lighting
		vec3 halfAngle = normalize(directionToLight + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);

		// Ignore cases when the viewer and light are on opossite sides of the surface
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, 512.0);	// Highter value = sharper highlight
		specularLight += intensity * blinnTerm;
	}

	// These values are RGB and the Alpha, meaning the value of the color. This is just a compiling 
	// stage that tells the graphics card which pixels the geometry mostly contains during the restorization 
	// stage. It will use this to properly color the pixels later.
	outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}