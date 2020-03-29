#version 430

in  vec4 geom_Color;
out vec4 frag_Color;

void main(void)
{
	frag_Color = geom_Color;
}
