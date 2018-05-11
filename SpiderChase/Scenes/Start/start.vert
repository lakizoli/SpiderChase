
attribute vec3 pos;
attribute vec3 vNorm;

uniform	mat4 model;
uniform	mat4 view;
uniform	mat4 proj;

varying vec3 fNorm;

void main (void) {	
	gl_Position = vec4 (pos, 1.0) * model * view * proj;
	fNorm = (vec4 (vNorm, 1.0) * model).xyz;
}