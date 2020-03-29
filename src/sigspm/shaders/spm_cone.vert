#version 430

layout(location=0) in vec4 in_Position;
layout(location=1) in vec4 in_Color;
out vec4 vert_Color;

void main(void)
{
	gl_Position = in_Position;
	vert_Color  = in_Color;
}
