#version 430

layout( lines ) in;
layout( triangle_strip, max_vertices=18 ) out;

in vec4 vert_Color[2];
out vec4 geom_Color;

layout( std430, binding=0 ) buffer VertexData
{
	vec4 Vert[];
};

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main(void)
{
	vec4 pos0 = projectionMatrix * viewMatrix * modelMatrix * vec4( gl_in[0].gl_Position.xy, 0.0, 1.0 );
	vec4 pos1 = projectionMatrix * viewMatrix * modelMatrix * vec4( gl_in[1].gl_Position.xy, 0.0, 1.0 );
	vec4 posm = ( pos0 + pos1 ) / 2.0;

	float dx = pos1.x - pos0.x;
	float dy = pos1.y - pos0.y;

	{
		vec4 poss = projectionMatrix * viewMatrix * modelMatrix * vec4( Vert[0].xy, 0.0, 1.0 );

		//vec4 n0  = normalize( vec4( -dy, dx, 0.0, 0.0 ) );
		vec4 n1  = normalize( vec4( dy, -dx, 0.0, 0.0 ) );
		vec4 n0s = normalize( pos0 - poss );
		vec4 n1s = normalize( pos1 - poss );
		vec4 n   = normalize( posm - poss );
		vec4 nm  = normalize( n0s + n1s );

		vec4 pos0s = pos0 + ( n0s * 4.0 );
		vec4 pos1s = pos1 + ( n1s * 4.0 );
		vec4 posms = posm + ( nm  * 4.0 );

		float d = dot( n, n1 );

		//if( d < 0.1 ) // front-facing check; disable to not worry about segment orientation, but might affect execution time
		{
			geom_Color = vec4( 0.0, 0.0, 0.0, 1.0 );

			gl_Position = pos0;
			EmitVertex();
			gl_Position = posms;
			EmitVertex();
			gl_Position = pos0s;
			EmitVertex();
			EndPrimitive();

			gl_Position = pos1;
			EmitVertex();
			gl_Position = pos1s;
			EmitVertex();
			gl_Position = posms;
			EmitVertex();
			EndPrimitive();

			gl_Position = pos0;
			EmitVertex();
			gl_Position = pos1;
			EmitVertex();
			gl_Position = posms;
			EmitVertex();
			EndPrimitive();
		}
	}

	if( Vert[0].w == 2.0 )
	{
		vec4 poss = projectionMatrix * viewMatrix * modelMatrix * vec4( Vert[1].xy, 0.0, 1.0 );

		//vec4 n0  = normalize( vec4( -dy, dx, 0.0, 0.0 ) );
		vec4 n1  = normalize( vec4( dy, -dx, 0.0, 0.0 ) );
		vec4 n0s = normalize( pos0 - poss );
		vec4 n1s = normalize( pos1 - poss );
		vec4 n   = normalize( posm - poss );
		vec4 nm  = normalize( n0s + n1s );

		vec4 pos0s = pos0 + ( n0s * 4.0 );
		vec4 pos1s = pos1 + ( n1s * 4.0 );
		vec4 posms = posm + ( nm  * 4.0 );

		float d = dot( n, n1 );

		//if( d < 0.1 ) // front-facing check; disable to not worry about segment orientation, but might affect execution time
		{
			geom_Color = vec4( 0.0, 0.0, 0.0, 1.0 );

			gl_Position = pos0;
			EmitVertex();
			gl_Position = posms;
			EmitVertex();
			gl_Position = pos0s;
			EmitVertex();
			EndPrimitive();

			gl_Position = pos1;
			EmitVertex();
			gl_Position = pos1s;
			EmitVertex();
			gl_Position = posms;
			EmitVertex();
			EndPrimitive();

			gl_Position = pos0;
			EmitVertex();
			gl_Position = pos1;
			EmitVertex();
			gl_Position = posms;
			EmitVertex();
			EndPrimitive();
		}
	}
}
