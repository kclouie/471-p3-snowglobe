#version 330 core 
in vec3 fragNor;
uniform float colorMode;
out vec4 color;

void main()
{
	vec3 normal = normalize(fragNor);
	// Map normal in the range [-1, 1] to color in range [0, 1];
	vec3 Ncolor = 0.5*normal + 0.5;
	if (colorMode != 0){
		color = vec4(0,0,0,1);
	}
	else {
		color = vec4(Ncolor, 1.0);
	}
}
