#version 330 core 
in vec3 fragNor;
out vec4 color;

void main()
{
	vec3 wNorm = normalize(vec3(0, 0, 1));
	vec3 normal = normalize(fragNor);
	// Map normal in the range [-1, 1] to color in range [0, 1];
	vec3 Ncolor = 0.5*normal + 0.5;
	if (dot(wNorm, normal) > 0 && dot(wNorm, normal) < .2) {
		color = vec4(.78, 0.83, 1, 1);
	}
	else if (dot(wNorm, normal) > 0) {
		discard;
	}
	else {
		color = vec4(1, 1, 1, 1.0)*(1 - distance(normal.z, 0));
	}
}
