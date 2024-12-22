#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 uniformColor;

void main()
{
	color = vec4(uniformColor, 1.0) * texture(image, TexCoords);
}
