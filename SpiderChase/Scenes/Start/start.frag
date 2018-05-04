
precision mediump float; // Shit happens :D

varying vec3 fNorm;

void main (void) {
	vec3 lightDir = normalize (vec3 (0.0, 1.0, -1.0));

	float lightDotNorm = dot (lightDir, fNorm);

	vec4 diffColor = vec4 (1.0, 1.0, 0.66  ,1.0);
	gl_FragColor = diffColor * lightDotNorm;
}