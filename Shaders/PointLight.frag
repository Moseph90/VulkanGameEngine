#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

// set and binding must match what we used when setting up our descriptor set layout
layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	vec4 ambientLightColor; // The 4th dimension is intensity
	vec3 lightPosition;
	vec4 lightColor;		// The 4th dimension is intensity
} ubo;

void main() {
	float dist = sqrt(dot(fragOffset, fragOffset));
	if (dist >= 1.0) {
		discard;
	}
	outColor = vec4(ubo.lightColor.xyz, 1.0);
}