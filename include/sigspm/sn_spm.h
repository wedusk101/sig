# pragma once

# include <sig/gs_vec.h>
# include <sig/sn_shape.h>
# include <sigspm/spm_manager.h>

class GsPolygon; // forward declaration
class GsFontStyle; // forward declaration
class UiLabel; // forward declaration

class SnSpm : public SnShape
{  public :
	GsVec center;
	GsVec normal;

	float minx;
	float miny;
	float width;
	float height;

	bool contourLines;
	float contourInterval, contourThickness;
	bool distanceField;

	ShortestPathMapManager* manager; // assumed to always be valid

public :
	static const char* class_name; //<! Contains string SnLines2
	SN_SHAPE_RENDERER_DECLARATIONS;

public :

	/* Default constructor. */
	SnSpm ( ShortestPathMapManager* _m );

	/* Destructor. */
	~SnSpm ();

	/*! Returns the bounding box, can be empty. */
	virtual void get_bounding_box ( GsBox &b ) const override;
};

/*	The method below has to be called before drawing SnCircle in order to connect SnCircle
to its renderer. In this example it is automatically called the first time SnCircle is
used, with a call from SnCircle's constructor. However, if a SnNode is to be used
independently from its renderer, the connection should be called from another initilization
function, so that the node does not need to include or be linked with one particular renderer,
also allowing connections to diferent renderers when/if needed.
In sig there is a single initializer for all included renderers in the sigogl module. */
void SnSpmRegisterRenderer ();
