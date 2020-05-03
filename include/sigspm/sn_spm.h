/*=======================================================================
   Copyright (c) 2020 Renato Farias and M. Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# pragma once

# include <sig/sn_shape.h>
# include <sigspm/spm_manager.h>

class SnSpm : public SnShape
{ public:
	GsVec center;
	GsVec normal;

	float minx;
	float miny;
	float width;
	float height;

	bool contourLines;
	float contourInterval, contourThickness;
	bool distanceField;

  protected:
	ShortestPathMapManager* _manager; // assumed to always be valid

  public: // required declarations
	static const char* class_name;
	SN_SHAPE_RENDERER_DECLARATIONS;

  public:

	/* Default constructor */
	SnSpm ( ShortestPathMapManager* m );

	/* Destructor */
	~SnSpm ();

	/*! Method touch has to be called to force sending new data to shaders
		when geometry-related parameters are changed. */
	void touch () { SnShape::touch(); }

	/*! Access to the always-valid SPM manager */
	ShortestPathMapManager* manager() { return _manager; }

	/*! Returns the bounding box, can be empty. */
	virtual void get_bounding_box ( GsBox &b ) const override;
};

/*	The function below is automatically called the first time SnSpm is used,
	with a call from SnSpm's constructor. SnSpm will need to be modified if
	connection to a different renderer is needed. */
void SnSpmRegisterRenderer ();
