/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_plane.h>

//============================== GsPlane ======================================

// Important: Static initializations cannot use other statically-initialized
// variables (like GsVec::i etc.) because order of initialization is unknown

const GsPlane GsPlane::XY ( GsPnt(0,0,0), GsVec(0,0,1) );
const GsPlane GsPlane::XZ ( GsPnt(0,0,0), GsVec(0,1,0) );
const GsPlane GsPlane::YZ ( GsPnt(0,0,0), GsVec(1,0,0) );

void GsPlane::set ( const GsVec& center, const GsVec& normal )
{
	n = normalize ( normal );
	setn ( center, normal );
}

bool GsPlane::parallel ( const GsVec& p1, const GsVec& p2, float ds ) const
{
	float fact = dot ( n, p1-p2 );
	return GS_NEXTZ(fact,ds);
}

// Derivation: find t: dot ( n, p1+t*(p2-p1) ) + w = 0 
// => n.p1 + t( n.(p2-p1) ) + w = 0 => t = -w-n.p1 / n.(p2-p1) = (n.p1+w) / n.(p1-p2)
GsVec GsPlane::intersect ( const GsVec& p1, const GsVec& p2, float *t ) const
{
	float fact = dot ( n, p1-p2 );
	if ( fact==0.0 ) return GsVec::null;
	float k = (w+dot(n,p1)) / fact;
	if (t) *t=k;
	return mix ( p1, p2, k );
}
