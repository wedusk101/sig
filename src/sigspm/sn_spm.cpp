
# include "sigspm/sn_spm.h"

const char* SnSpm::class_name = "SnSpm"; // static
SN_SHAPE_RENDERER_DEFINITIONS(SnSpm);

//===== SnSpm =====

SnSpm::SnSpm ( ShortestPathMapManager* _m ) : SnShape ( class_name )
{
	normal = GsVec::k;
	minx = miny = -1.0f;
	width = height = 2.0f;

	contourLines = true;
	contourInterval = 0.02f;
	contourThickness = 0.002f;
	distanceField = false;

	manager = _m;

	if ( !SnSpm::renderer_instantiator ) SnSpmRegisterRenderer ();
}

SnSpm::~SnSpm ()
{
}

void SnSpm::get_bounding_box ( GsBox& b ) const
{
	b.a = GsPnt( minx, miny, 0.0f );
	b.b = GsPnt( minx + width, miny + height, 0.0f );
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
