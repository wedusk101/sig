/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_CONTEXT_H
# define GL_CONTEXT_H

/** \file gl_context.h 
 *	Class used to access the OpenGL context
 */

# include <sig/gs_mat.h>
# include <sig/gs_light.h>
# include <sig/gs_shareable.h>

# include <sigogl/gl_types.h>
# include <sigogl/gl_program.h>

//====================== GlContext =====================

/*! GlContext keeps track of the current OpenGL settings during rendering operations. */
class GlContext : public GsShareable
{  private :
	// Viewport:
	int _vpw, _vph;
	// Transformations:
	const GsMat* _projection;
	const GsMat* _viewing;
	const GsMat* _modelview;
	const GsMat* _cameraview;
	const GsMat* _localframe;
	// Tracked state:
	GsColor _clearcolor;
	float _pointsize;
	float _linewidth;
	bool _transparency;
	bool _linesmoothing;
	bool _cullface;
	bool _depthtest;
	GLuint _curprogram;
	GLenum _polygonmode;
	// Lights:
	int _numl;

   public :
	static const int MaxLights=4; // max number of lights supported
	GsLight light[MaxLights]; // LightDev: multiple lights stored in context

   public :
	GlContext ();

	/*! Sets initial setup with transparency, line smoothing, point size 2, 
		and clear color light gray. Each respective call to OpenGL will only 
		be made if the GlContext flags indicate the call to be neeed. 
		Projection and model-view matrices are pointed to identity.
		Lights are not changed. */
	void init ();

	/*! Sets the OpenGL viewport, even if this is a redundant call. */
	void viewport ( int w, int h );
	int w () const { return _vpw; }
	int h () const { return _vph; }

	/*! Clears color and depth buffers. */
	void clear ();

	/*! Sets number of lights to use. The given value will be corrected to be in range
		1 <= n <= GlContext::MaxLights before being used. */
	void num_lights ( int n );
	int num_lights () const { return _numl; }

	/*! Set pointers for the transformations to be accessed by GlContext. 
		It is the user resposibility to provide pointers to valid matrices.
		Pointers have to remain valid while GlContext is used. No calls to OpenGL are made. */
	void projection ( const GsMat* m ) { _projection=m; }
	void modelview ( const GsMat* m ) { _localframe=m; }
	const GsMat* projection () { return _projection; }
	const GsMat* modelview () { return _localframe; }

	/*! Methods starting with reset_ are methods that will set the corresponding
		OpenGL state even if the member variables of GlContext already indicate
		the state to be set. */
	void reset_clear_color ( const GsColor& c );
	void clear_color ( const GsColor& c ) { if(_clearcolor!=c) reset_clear_color(c); }
	GsColor clear_color () const { return _clearcolor; }

	void reset_point_size ( float s );
	void point_size ( float s ) { if (_pointsize!=s) reset_point_size(s); }
	float point_size () const { return _pointsize; }

	void reset_line_width ( float w );
	void line_width ( float w ) { if (_linewidth!=w) reset_line_width(w); }
	float line_width () const { return _linewidth; }

	void reset_line_smoothing ( bool b );
	void line_smoothing ( bool b ) { if (_linesmoothing!=b) reset_line_smoothing(b); }
	bool line_smoothing () const { return _linesmoothing; }

	void reset_transparency ( bool b );
	void transparency ( bool b ) { if (_transparency!=b) reset_transparency(b); }
	bool transparency () const { return _transparency; }

	void reset_cull_face ( bool b );
	void cull_face ( bool b ) { if (_cullface!=b) reset_cull_face(b); }
	bool cull_face () const { return _cullface; }

	void reset_depth_test ( bool b );
	void depth_test ( bool b ) { if (_depthtest!=b ) reset_depth_test(b); }
	bool depth_test () { return _depthtest; }

	/*! Methods that set polygon mode for both back and front faces */
	void polygon_mode_fill ();
	void polygon_mode_line ();
	void polygon_mode_point ();
	void reset_polygon_mode ( GLenum pm );
	void polygon_mode ( GLenum pm ) { if(_polygonmode!=pm) reset_polygon_mode(pm); }
	GLenum polygon_mode () const { return _polygonmode; }

	void reset_use_program ( GLuint pid );
	void use_program ( GLuint pid ) { if(_curprogram!=pid) reset_use_program(pid); }
	void use_program ( const GlProgram* p ) { if(_curprogram!=p->id) reset_use_program(p->id); }
	GLuint cur_program () const { return _curprogram; }
};

//================================= End of File ===============================

# endif // GL_CONTEXT_H
