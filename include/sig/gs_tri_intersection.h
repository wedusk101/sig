/*=======================================================================
   Copyright (c) 2020 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_TRI_INTERSECTION_H
# define GS_TRI_INTERSECTION_H

/** \file gs_tri_intersection.h 
 * triangle-triangle intersection computations
 */

# include <sig/gs_vec.h>

//===== wrapper functions based on GsVec/GsPnt types =====

bool triangles_intersect ( const GsPnt& a, const GsPnt& b, const GsPnt& c,
						   const GsPnt& d, const GsPnt& e, const GsPnt& f );

bool triangles_intersect ( const GsPnt& a, const GsPnt& b, const GsPnt& c,
						   const GsPnt& d, const GsPnt& e, const GsPnt& f,
						   GsPnt& x1, GsPnt& x2, bool* coplanar );

bool triangles_intersect ( const GsPnt2& a, const GsPnt2& b, const GsPnt2& c,
						   const GsPnt2& d, const GsPnt2& e, const GsPnt2& f );

bool coplanar_triangles_intersect ( const GsPnt& a, const GsPnt& b, const GsPnt& c,
									const GsPnt& d, const GsPnt& e, const GsPnt& f,
									const GsVec& n1, const GsVec& n2 );

//===== original intersection tests from JGT repository (under MIT License) =====

/*! Three-dimensional Triangle-Triangle Overlap Test.
	Returns 1 if the triangles (including their boundary) intersect, otherwise 0. */
int gs_tri_tri_overlap_test_3d ( double p1[3], double q1[3], double r1[3],
								 double p2[3], double q2[3], double r2[3] );

/*! Three-dimensional Triangle-Triangle Overlap Test.
	Additionaly computes the segment of intersection of the two triangles if it exists. 
	Parameter coplanar returns whether the triangles are coplanar, and
	source and target are the endpoints of the line segment of intersection.
	Returns 1 if the triangles (including their boundary) intersect, otherwise 0. */
int gs_tri_tri_intersection_test_3d ( double p1[3], double q1[3], double r1[3], 
									  double p2[3], double q2[3], double r2[3],
									  int* coplanar, double source[3], double target[3] );

/*! Three-dimensional Triangle-Triangle Overlap Test for co-planar triangles.
	N1 and N2 are the normal vectors of the triangles.
	Returns 1 if the triangles (including their boundary) intersect, otherwise 0. */
int gs_coplanar_tri_tri3d ( double  p1[3], double q1[3], double r1[3],
							double  p2[3], double q2[3], double r2[3],
							double  N1[3], double N2[3] );

// Two dimensional Triangle-Triangle Overlap Test
int gs_tri_tri_overlap_test_2d ( double p1[2], double q1[2], double r1[2], 
								 double p2[2], double q2[2], double r2[2] );

/* Triangle-Triangle Overlap Test Routines
*  July, 2002
*  Updated December 2003
*
*  This file contains C implementation of algorithms for
*  performing two and three-dimensional triangle-triangle intersection test
*  The algorithms and underlying theory are described in
*
* "Fast and Robust Triangle-Triangle Overlap Test
*  Using Orientation Predicates"  P. Guigue - O. Devillers
*
*  Journal of Graphics Tools, 8(1), 2003
*
*  Several geometric predicates are defined.  Their parameters are all
*  points.  Each point is an array of two or three double precision
*  floating point numbers. The geometric predicates implemented in
*  this file are:
*
*    int tri_tri_overlap_test_3d(p1,q1,r1,p2,q2,r2)
*    int tri_tri_overlap_test_2d(p1,q1,r1,p2,q2,r2)
*
*    int tri_tri_intersection_test_3d(p1,q1,r1,p2,q2,r2,coplanar,source,target)
*
*       is a version that computes the segment of intersection when
*       the triangles overlap (and are not coplanar)
*
*    each function returns 1 if the triangles (including their
*    boundary) intersect, otherwise 0
*
*  Other information are available from the Web page
*  http://www.acm.org/jgt/papers/GuigueDevillers03/
*/

//================================ End of File =================================================

# endif // GS_TRI_INTERSECTION_H
