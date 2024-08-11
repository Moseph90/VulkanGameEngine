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

// This is connected to the push constants struct in RenderSystem.cpp
layout (push_constant) uniform Push {
	mat4 transform;			// Projection * view * model
	mat4 normalMatrix;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.02;

//We will get vertices from the assembler and output a position
void main() {

	// This value will act as our output. It is a GLSL specific variable. The vertex index is a 
	// built in GLSL variable that keeps track of which index we are on. This is a vector 4, note that 
	// the first component counts as two because it is a vector2. The second one is a Z layer or depth. 
	// The last parameter is what the entire vector is divided by in order to turn it into a normalized 
	// coordinate. For now it's one
	
	gl_Position = push.transform * vec4(position, 1.0);

	// To properly compute the lighting we require the normals to be tranformed 
	// from model space to world space. We do this by using the model matrix
	// The mat3 function converts a 4x4 matrix to a 3x3 matrix by removing 
	// a row and column. This means that normals represent directions, 
	// not positions and are therefor not affected by translations.
	
	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	float lightIntesity = AMBIENT + max(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0);

	fragColor = lightIntesity * color;
}