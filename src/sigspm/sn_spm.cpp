/*=======================================================================
   Copyright (c) 2020 Renato Farias and M. Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include "sigspm/sn_spm.h"

const char* SnSpm::class_name = "SnSpm"; // static
SN_SHAPE_RENDERER_DEFINITIONS(SnSpm);

//===== SnSpm =====

SnSpm::SnSpm ( ShortestPathMapManager* m ) : SnShape ( class_name )
{
	normal = GsVec::k;
	minx = miny = -1.0f;
	width = height = 2.0f;

	contourLines = true;
	contourInterval = 0.04f;
	contourThickness = 0.004f;
	distanceField = false;

	_manager = m;

	if ( !SnSpm::renderer_instantiator ) SnSpmRegisterRenderer ();
}

SnSpm::~SnSpm ()
{
}

void SnSpm::get_bounding_box ( GsBox& b ) const
{
	b.a.set ( minx, miny, 0.0f );
	b.b.set ( minx + width, miny + height, 0.0f );
}

//===== Renderer Instantiator =====

# include "sigspm/glr_spm.h"

static SnShapeRenderer* GlrSpmInstantiator ()
{
	return new GlrSpm;
}

void SnSpmRegisterRenderer ()
{
	SnSpm::renderer_instantiator = &GlrSpmInstantiator;
}
