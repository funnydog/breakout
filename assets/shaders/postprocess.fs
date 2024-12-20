#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D scene;

uniform vec2 offsets[9];
uniform int edge_kernel[9];
uniform float blur_kernel[9];

uniform bool chaos;
uniform bool confuse;
uniform bool shake;

void main()
{
	vec3 mysample[9];
	if (chaos || shake) {
		for (int i = 0; i < 9; i++)
			mysample[i] = vec3(texture(scene, TexCoords.st + offsets[i]));
	}

	if (chaos) {
		color = vec4(0.0);
		for (int i = 0; i < 9; i++) {
			color += vec4(mysample[i] * edge_kernel[i], 0.0);
		}
		color.a = 1.0f;
	} else if (confuse) {
		color = vec4(1.0 - texture(scene, TexCoords).rgb, 1.0);
	} else if (shake) {
		color = vec4(0.0);
		for (int i = 0; i < 9; i++) {
			color += vec4(mysample[i] * blur_kernel[i], 0.0f);
		}
		color.a = 1.0f;
	} else {
		color = texture(scene, TexCoords);
	}
}
