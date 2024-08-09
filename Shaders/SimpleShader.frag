#version 450

//*******************************************************************************************************
//This is the fragment shader. A fragment shader is capable of outputting to multiple different locations
//There is no built in output variable for this one so we have to build our own.
//*******************************************************************************************************

layout (location = 0) in vec3 fragColor;

// Layout qualifier takes a location value, the "out" qualifier indicates 
// that this variable is for output the rest is for the name and type.
layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Push {
	mat4 transform;
	vec3 color;
} push;

void main() {

	// These values are RGB and the Alpha, meaning the value of the color. This is just a compiling 
	// stage that tells the graphics card which pixels the geometry mostly contains during the restorization 
	// stage. It will use this to properly color the pixels later.

	outColor = vec4(fragColor, 1.0);
}