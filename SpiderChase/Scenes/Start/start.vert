
attribute vec3 pos;
attribute vec3 vNorm;

uniform	mat4 mvpMatrix;

varying vec3 fNorm;

void main (void) {
	
	gl_Position = mvpMatrix * vec4 (pos, 1.0);
	fNorm = (mvpMatrix * vec4 (vNorm, 1.0)).xyz;
}