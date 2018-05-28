
precision mediump float;

uniform sampler2D ambientMap;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D lightMap;
uniform int hasAmbientMap;
uniform int hasDiffuseMap;

varying vec3 fNorm;

void main (void) {
	vec3 lightDir = normalize (vec3 (0.0, 1.0, -1.0));

	float lightDotNorm = dot (lightDir, fNorm);

	vec4 diffColor;
	if (hasDiffuseMap) {
		diffColor = texture2D (diffuseMap, vec2 (0, 0));
	} else {
		diffColor = vec4 (1.0, 1.0, 0.66  ,1.0);
	}
	gl_FragColor = diffColor * lightDotNorm;
}