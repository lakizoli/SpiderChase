
attribute vec3 pos;
attribute vec3 vNorm;
attribute vec2 vUV;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

uniform	mat4 model;
uniform	mat4 view;
uniform	mat4 proj;

uniform int hasAnimation;
uniform	mat4 bones[4];

varying vec3 fNorm;
varying vec2 fUV;

void main (void) {
	if (hasAnimation == 1)
	{
		mat4 composed = blendWeights.x * bones[int(blendIndices.x)] +
						blendWeights.y * bones[int(blendIndices.y)] +
						blendWeights.z * bones[int(blendIndices.z)] +
						blendWeights.w * bones[int(blendIndices.w)];
		
		gl_Position = vec4 (pos, 1.0) * composed * model * view * proj;
	}
	else
	{
		gl_Position = vec4 (pos, 1.0) * model * view * proj;
	}
	
	fNorm = (vec4 (vNorm, 1.0) * model).xyz;
	fUV = vUV;
}
