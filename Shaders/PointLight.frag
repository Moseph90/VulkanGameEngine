#version 450

//********************************************************************************************************
//This is the fragment shader. A fragment shader is capable of outputting to multiple different locations
//There is no built in output variable for this one so we have to build our own. This particular file is
//for the point lights, as point lights are rendered differently than regular game objects, this also has
//transparency and alpha blending functionality. Similarly, there is a .vert file for point lights as well
//********************************************************************************************************

layout (location = 0) in vec2 fragOffset;
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

layout(push_constant) uniform Push {
	vec4 position;
	vec4 color;
	float radius;
} push;

const float M_PI = 3.1415926538;

void main() {
	float dist = sqrt(dot(fragOffset, fragOffset));
	if (dist >= 1.0) {
		discard;
	}
	
	// Make the center of the light white and gradually change to the proper color near the edges
	float cosDist =  0.5 * (cos(dist * M_PI) + 1.0); // Ranges from 1 -> 0
	outColor = vec4(push.color.xyz + cosDist, cosDist);
}