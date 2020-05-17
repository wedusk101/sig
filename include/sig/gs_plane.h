/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_PLANE_H
# define GS_PLANE_H

# include <sig/gs_vec.h>

class GsPlane
{  public :
	GsVec n; //<! normalized normal
	float w; //<! w such that dot(n,p)+w=0 for all p in plane
	static const GsPlane XY, //<! The XY plane
						 XZ, //<! The XZ plane
						 YZ; //<! The YZ plane
   public:

	/*! The default constructor initializes as the plane passing at (0,0,0)
		with normal (0,0,1). */
	GsPlane () : n ( GsVec::k ), w(0) {}

	/*! Copy constructor. */
	GsPlane ( const GsPlane& pl ) : n(pl.n), w(pl.w) {}

	/*! Constructor from center and normal. Normal does no need to be normalized. */
	GsPlane ( const GsVec& center, const GsVec& normal ) { set ( center, normal ); }

	/*! Constructor from three points in the plane. */
	GsPlane ( const GsVec& p1, const GsVec& p2, const GsVec& p3 ) { set ( p1, p2, p3 ); }

	/*! Returns the (normalized) normal vector of the plane. */
	const GsVec& normal () const { return n; }

	/*! Set as the plane passing at center with given normal.
		Parameter normal does not need to be normalized. */
	void set ( const GsVec& center, const GsVec& normal );

	/*! Set as the plane passing at center with given normal, which must be normalized.
		Derivation of w: dot(n,p-c)=0 => dot(n,p)+dot(n,-c)=0 => w=-dot(n,c). */
	void setn ( const GsVec& center, const GsVec& normal ) { n=normal; w=-dot(n,center); }

	/*! Set as the plane passing trough three points.
		Parameter normal does not need to be normalized.
		Returns true if the normal is valid and the plane is set, and false otherwise */
	void set ( const GsVec& p1, const GsVec& p2, const GsVec& p3 ) { set(p1,cross(p2-p1,p3-p1)); }

	/*! determines if the plane is parallel to the line [p1,p2], according to the (optional)
		given precision ds. This method is fast as it only perform one subraction and one
		dot product. True is returned if the dot product is smaller than ds, and false otherwise. */
	bool parallel ( const GsVec& p1, const GsVec& p2, float ds=0 ) const;

	/*! Returns p, that is the intersection between GsPlane and the infinite
		line {p1,p2}. (0,0,0) is returned if they are parallel. Use parallel()
		to test this before. If t is non null, t will contain the interpolation
		factor such that p=p1(1-t)+p2(t). */
	GsVec intersect ( const GsVec& p1, const GsVec& p2, float *t=0 ) const;
};

# endif // GS_PLANE_H
