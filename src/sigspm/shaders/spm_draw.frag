#version 430

in vec3 TexCoord;

uniform ivec2 bufferDim;
uniform sampler2DRect drawTexId;

uniform bool contourLines; // Draw contour lines?
uniform float contourInterval;
uniform float contourThickness;
uniform bool distanceField; // Draw distance field?

out vec4 frag_Color;

void main()
{
	ivec2 frag_TexCoord = ivec2( bufferDim.x * TexCoord.x, bufferDim.y * TexCoord.y );
	frag_Color = texelFetch( drawTexId, frag_TexCoord );

	if( frag_Color.w == 0.0 )
		discard;

	if( contourLines == true && mod( abs( frag_Color.z ), contourInterval ) <= contourThickness ) {
		frag_Color = vec4( 1.0 );
		return;
	}

	if( distanceField == true && frag_Color != vec4( 1.0 ) ) {
		double d = abs ( frag_Color.z );
		frag_Color = vec4( d, 1.0-d, 0.0, 1.0 );
		return;
	}

	// This is to eliminate the z of non-scene pixels. For them, z is used as depth instead of color
	if( frag_Color != vec4( 1.0 ) )
		frag_Color.z = 0.0;
}
