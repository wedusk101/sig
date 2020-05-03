/*=======================================================================
   Copyright (c) 2020 Renato Farias and M. Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# pragma once

# include <sigogl/glr_base.h>
# include <sigogl/gl_objects.h>

class GlrSpm : public GlrBase
{ protected:
	GlObjects _glo;
	gsuint _psize;

  public:
	GlrSpm ();
	virtual ~GlrSpm ();
	virtual void init ( SnShape* s ) override;
	virtual void render ( SnShape* s, GlContext* c ) override;
};

//================================ End of File =================================================
