attribute vec3	pos;
uniform mat4	mvpMatrix;

void main (void) {
	gl_Position = mvpMatrix * vec4 (pos, 1.0);
}