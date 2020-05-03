/*=======================================================================
   Copyright (c) 2020 Renato Farias and M. Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include "sigspm/sn_spm.h"
# include "sigspm/glr_spm.h"

# include <sig/gs_array.h>
# include <sig/gs_quat.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= GlrLines ====================================

GlrSpm::GlrSpm ()
{
	GS_TRACE1 ( "Constructor" );
	_psize = 0;
}

GlrSpm::~GlrSpm ()
{
	GS_TRACE1 ( "Destructor" );
}

static const GlProgram* Prog=0; // These are static because they are the same for all GlrSpm instances

void GlrSpm::init ( SnShape* s )
{
	GS_TRACE2( "Generating program objects" );
	// Initialize program and buffers if needed:
	if( !Prog )
	{	// M. Kallmann: below we retrieve the predefined shaders, and use SIG's shader management to allow shaders to be shared by other scene nodes:
		const char* pdrawvert = ((SnSpm*)s)->manager()->GetPredefSpmDrawVertShader();
		const char* pdrawfrag = ((SnSpm*)s)->manager()->GetPredefSpmDrawFragShader();
		const GlShader* VtxShader = GlResources::declare_shader( GL_VERTEX_SHADER, "SpmDrawVert", "../../sig/src/sigspm/shaders/spm_draw.vert", pdrawvert );
		const GlShader* FragShader = GlResources::declare_shader( GL_FRAGMENT_SHADER, "SpmDrawFrag", "../../sig/src/sigspm/shaders/spm_draw.frag", pdrawfrag );
		const GlProgram* p = GlResources::declare_program( "MySpmProgram", 2, VtxShader, FragShader );
		GlResources::declare_uniform( p, 0, "vProj" );
		GlResources::declare_uniform( p, 1, "vView" );
		GlResources::declare_uniform( p, 2, "bufferDim" );
		GlResources::declare_uniform( p, 3, "drawTexId" );
		GlResources::declare_uniform( p, 4, "contourLines" );
		GlResources::declare_uniform( p, 5, "contourInterval" );
		GlResources::declare_uniform( p, 6, "contourThickness" );
		GlResources::declare_uniform( p, 7, "distanceField" );
		GlResources::compile_program( p );
		Prog = p; // Save in Prog a direct pointer to the program used by this node!
	}
	_glo.gen_vertex_arrays( 1 );
	_glo.gen_buffers( 2 );
}

void GlrSpm::render ( SnShape* s, GlContext* ctx )
{
	GS_TRACE2( "GL4 Render " << s->instance_name() );
	SnSpm& spm = *( (SnSpm*)s );

	// 1. Set buffer data if node has been changed:
	if( s->changed() & SnShape::Changed ) // flags are: Unchanged, RenderModeChanged, MaterialChanged, Changed
	{
		glBindVertexArray( _glo.va[0] );

		// xyz coordinates
		GsVec P[4] = { { spm.minx, spm.miny, 0.0f },
		               { spm.minx + spm.width, spm.miny, 0.0f },
		               { spm.minx + spm.width, spm.miny + spm.height, 0.0f },
		               { spm.minx, spm.miny + spm.height, 0.0f } };

		glEnableVertexAttribArray( 0 );
		glBindBuffer( GL_ARRAY_BUFFER, _glo.buf[0] );
		glBufferData( GL_ARRAY_BUFFER, 4*sizeof(GsVec), &P[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

		// texture coordinates
		GsVec T[4] = { { 0.0f, 0.0f, 0.0f },
		               { 1.0f, 0.0f, 0.0f },
		               { 1.0f, 1.0f, 0.0f },
		               { 0.0f, 1.0f, 0.0f } };

		glEnableVertexAttribArray( 1 );
		glBindBuffer( GL_ARRAY_BUFFER, _glo.buf[1] );
		glBufferData( GL_ARRAY_BUFFER, 4*sizeof(GsVec), &T[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );

		_psize = 4;
	}

	// 2. Enable/bind needed elements and draw:
	if( spm.manager() ) // protection
	{
		GS_TRACE2( "Rendering SPM with custom scene node..." );
		unsigned int drawId = spm.manager()->GetDrawTexId();

		glActiveTexture( GL_TEXTURE0 + drawId );
		glBindTexture( GL_TEXTURE_RECTANGLE, drawId );

		ctx->use_program( Prog->id ); // ctx tests if the program is being changed

		// projection matrices
		glUniformMatrix4fv( Prog->uniloc[0], 1, GLTRANSPMAT, ctx->projection()->e );
		glUniformMatrix4fv( Prog->uniloc[1], 1, GLTRANSPMAT, ctx->modelview()->e );

		// GPU buffer dimensions
		int dim[2] = { spm.manager()->GetBufferWidth(), spm.manager()->GetBufferHeight() };
		glUniform2iv( Prog->uniloc[2], 1, dim );

		// texture ID to read from
		glUniform1i( Prog->uniloc[3], (int)drawId );

		// visualization options
		glUniform1i( Prog->uniloc[4], spm.contourLines );
		glUniform1f( Prog->uniloc[5], spm.contourInterval );
		glUniform1f( Prog->uniloc[6], spm.contourThickness );
		glUniform1i( Prog->uniloc[7], spm.distanceField );

		glBindVertexArray( _glo.va[0] );
		glDrawArrays( GL_QUADS, 0, _psize );
	}

	glBindVertexArray( 0 ); // done
}

//================================ EOF =================================================
