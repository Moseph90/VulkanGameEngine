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

layout (location = 0) in vec2 position;

layout (location = 1) in vec3 color;

layout (push_constant) uniform Push {
	mat2 transform;
	vec2 offset;
	vec3 color;
} push;

//We will get vertices from the assembler and output a position
void main() {

	// This value will act as our output. It is a GLSL specific variable. The vertex index is a 
	// built in GLSL variable that keeps track of which index we are on. This is a vector 4, note that 
	// the first component counts as two because it is a vector2. The second one is a Z layer or depth. 
	// The last parameter is what the entire vector is divided by in order to turn it into a normalized 
	// coordinate. For now it's one

	gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
}