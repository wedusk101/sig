#version 430

in  vec4 vert_Color;
out vec4 frag_Color;

layout( std430, binding=0 ) buffer VertexData
{
	vec4 Vert[];
};

uniform sampler2DRect texId, drawId;
uniform ivec2 window;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main(void)
{
	vec4 tex_value  = texelFetch( texId,  ivec2( gl_FragCoord.xy ) ); // for shadow test
	vec4 draw_value = texelFetch( drawId, ivec2( gl_FragCoord.xy ) ); // what's currently stored in this pixel

	// if nothing else happens, just pass on the current value
	frag_Color = draw_value;

	// stop if this fragment is in shadow
	if( tex_value.w != 0.0 )
		return;

	// 0..1 position of pixel
	vec2 spos = vec2( float( gl_FragCoord.x ) / float( window.x ), float( gl_FragCoord.y ) / float( window.y ) );
	vec2 parent;

	// current generator in screen space coordinates
	vec4 gen0 = projectionMatrix * viewMatrix * modelMatrix * vec4( Vert[0].xy, 0.0, 1.0 );
	vec4 gen1 = projectionMatrix * viewMatrix * modelMatrix * vec4( Vert[1].xy, 0.0, 1.0 );

	vec2 sgen0 = ( gen0.xy + 1.0 ) / 2.0;
	vec2 sgen1 = ( gen1.xy + 1.0 ) / 2.0;

	// distance for line segment source
	if( Vert[0].w == 2.0 )
	{
		float len = length( sgen1 - sgen0 ) * length( sgen1 - sgen0 );

		if( len == 0.0 )
			parent = sgen0;
		else
		{
			float d = dot( spos - sgen0, sgen1 - sgen0 ) / len;
			if( d < 0.0 )
				parent = sgen0;
			else if( d > 1.0 )
				parent = sgen1;
			else
				parent = sgen0 + d * ( sgen1 - sgen0 );
		}
	}
	// distance for source point
	else
		parent = sgen1;

	// calculate new total distance
	float dist = distance( spos, parent );
	//float depth = Vert[0].z + dist;
	float depth = ( dist * Vert[1].z ) + Vert[0].z;

	// update distance if the pixel has none or if new distance is smaller
	if( draw_value.w == 0.0 || depth < draw_value.z )
		frag_Color = vec4( parent.xy, depth, Vert[1].w );
		//frag_Color = vec4( parent.xy, depth, 1.0 );
}
