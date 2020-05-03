#version 430

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vTex;

uniform mat4 vProj;
uniform mat4 vView;

out vec3 TexCoord;

void main()
{
	gl_Position = vec4( vPos.x, vPos.y, vPos.z, 1.0 ) * vView * vProj;
	TexCoord = vTex;
}
