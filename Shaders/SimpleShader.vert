#version 450

//*****************************************************
//This function will be called for every vertex we have
//This code is exclusively for the graphics card and is
//not compiled by Visual Studio, but is used by it.
//Rather, this code is compiled by the compile.bat file
//*****************************************************

// Here we've specified our first vertex attribute
// The 'in' keyword signifies that this variable
// takes its value from a vertex buffer. The layout
// location sets the storage of where this variable
// value will come from. This is how we connect the
// attribute description to the variable we need to
// reference in the shader. Each time this code is run
// the position will have a different x and y component
// These are connected to the attribute descriptions in Model.cpp

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragPosWorld;
layout (location = 2) out vec3 fragNormalWorld;

// set and binding must match what we used when setting up our descriptor set layout
layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionViewMatrix;
	vec4 ambientLightColor; // The 4th dimension is intensity
	vec3 lightPosition;
	vec4 lightColor;		// The 4th dimension is intensity
} ubo;

// This communicates with the push constants struct in RenderSystem.cpp
layout (push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

//We will get vertices from the assembler and output a position
void main() {

	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
	gl_Position = ubo.projectionViewMatrix * positionWorld;

	// To properly compute the lighting we require the normals to be tranformed 
	// from model space to world space. We do this by using the model matrix
	// The mat3 function converts a 4x4 matrix to a 3x3 matrix by removing 
	// a row and column. This means that normals represent directions, 
	// not positions and are therefor not affected by translations.
	
	fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
	fragPosWorld = positionWorld.xyz;
	fragColor = color;
}