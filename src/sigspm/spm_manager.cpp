/*=======================================================================
   Copyright (c) 2020 Renato Farias.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

#include <sigspm/spm_manager.h>

#include <algorithm>
#include <sig/gs_geo2.h>

using namespace std;

/*=================================================================================================
	CONSTRUCTION / DESTRUCTION
=================================================================================================*/

ShortestPathMapManager::ShortestPathMapManager()
{
	initialized = false;

	envSinks = nullptr;
	envObstacles = nullptr;

	bufferWidth  = 1000;
	bufferHeight = 1000;
}

ShortestPathMapManager::~ShortestPathMapManager()
{
}

void ShortestPathMapManager::Initialize( void )
{
	CreateShaders();
	CreateBuffers();
	CreateQuadVbo();

	initialized = true;
}

void ShortestPathMapManager::SetShadersFolder ( const char* folder )
{
	shadersFolder = folder? folder:"";
}

bool ShortestPathMapManager::CanLoadShaders ()
{
	bool test;
	CreateShaders ( &test );
	return test;
}

/*=================================================================================================
	DATA
=================================================================================================*/

ShortestPathMap* ShortestPathMapManager::CreateSPM( void )
{
	ShortestPathMap* new_spm = new ShortestPathMap;
	new_spm->SetBufferDimensions( bufferWidth, bufferHeight );
	new_spm->SetOrthoProjectionMatrix( OrthoProjectionCompleteMatrix );
	ShortestPathMaps.push_back( new_spm );
	return new_spm;
}

bool ShortestPathMapManager::LoadSPM( const std::string& spmName, const std::string& spmPath, const std::string& raPath )
{
	ShortestPathMap* new_spm = CreateSPM();
	
	if( new_spm->LoadMapFromImageFile( spmPath ) == false || new_spm->LoadResultArrayFromFile( raPath ) == false )
	{
		delete new_spm;
		ShortestPathMaps.pop_back();
		return false;
	}

	new_spm->SetName( spmName );

	return true;
}

bool ShortestPathMapManager::SaveSPM( int i, const std::string& spmPath, const std::string& raPath ) const
{
	if( i < 0 || i >= (int)ShortestPathMaps.size() )
		return false;

	if( ShortestPathMaps[i]->SaveMapToImageFile( spmPath ) == false || ShortestPathMaps[i]->SaveResultArrayToFile( raPath ) == false )
		return false;

	return true;
}

//SpmTodo: fix problem when there are no obstacles
//SpmTodo: add method to change the WxH frame buffer resolution

ShortestPathMap* ShortestPathMapManager::Compute( GlContext* context, ShortestPathMap* spm )
{
	if( initialized == false )
		Initialize();

	if( context == nullptr )
		return nullptr;

	// Fill the shader storage array with the necessary data and send it to the GPU
	ComputeCriticalPointsForSourceLines();
	FillShaderStorageArray();
	if( ShaderStorageArray.size() <= 2 )
		return nullptr;

	// Save context states to restore them later ------------------------------------------------//
	// M. Kallmann integration notes: here we save the context state using GlContext methods; however,
	// ideally OpenGL states modified by the SPM library should be saved and later restored
	// directly using OpenGL functions (without using GlContext), in order to become independent of
	// GlContext and to include any states that are not tracked by GlContext. This will be important
	// when linking to external projects using OpenGL.
	int viewport_w = context->w();
	int viewport_h = context->h();
	GsColor clear_color = context->clear_color();
	float point_size = context->point_size();
	float line_size = context->line_width();
	bool line_smoothing = context->line_smoothing();
	bool transparency = context->transparency();
	bool cull_face = context->cull_face();
	bool depth_test = context->depth_test();
	GLuint curprogram = context->cur_program();

	//-------------------------------------------------------------------------------------------//

	UpdateDomainBoundaries();
	UpdateShaderUniforms();
	CreateSceneVbo();

	// set the context to what SPM needs --------------------------------------------------------//
	
	glViewport ( 0, 0, bufferWidth, bufferHeight );
	glClearColor ( 1.0f, 1.0f, 1.0f, 0.0f );
	glPointSize ( 1.0f );
	glLineWidth ( 1.0f );
	glDisable ( GL_LINE_SMOOTH );
	glDisable ( GL_BLEND );
	glDisable ( GL_CULL_FACE );
	glDisable ( GL_DEPTH_TEST );

	//-------------------------------------------------------------------------------------------//

	// Clear the draw buffers before we start
	glBindFramebuffer( GL_FRAMEBUFFER, framebufferId );

	glDrawBuffers( 1, &colorAttachments[ DrawTexIdA ] );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glDrawBuffers( 1, &colorAttachments[ DrawTexIdB ] );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glBindBuffer( GL_SHADER_STORAGE_BUFFER, shaderStorageBufferId );
	glBufferData( GL_SHADER_STORAGE_BUFFER, SpmVertexPosSize*ShaderStorageArray.size(), &ShaderStorageArray[0], GL_DYNAMIC_DRAW );

	bool drawA = true; // Draw in buffer A? If not, draw in B

	int limit1 = ( (int)ShaderStorageArray.size() - 3 ) / 2;
	//int limit2 = ( (int)ShaderStorageArray.size() - 3 - ( ShaderStorageArrayLeap * ShaderStorageArrayNumLeaps ) ) / 2;
	int limit2 = limit1;

	//int max = SpmMaxGenerators;
	int max = 0;
	if( max == 0 || max > limit1 )
		max = limit1;

	for( int i = 0; i < max; ++i )
	{
		if( i > 0 )
		{
			// Search for the next generator
			SPM2_SearchShader.Use();
			glDispatchCompute( 1, 1, 1 );
			//glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
		}

		// Draw the shadow volume
		glDrawBuffers( 1, &colorAttachments[ StencilTexId ] );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		if( envObstacles->size()>0 )
		{
			SPM2_ShadowShader.Use();
			SceneVbo.BindDrawElementsUnbind();
		}

		// Draw the cone
		SPM2_ConeShader.Use();

		if( drawA == true )
		{
			glDrawBuffers( 1, &colorAttachments[ DrawTexIdA ] );
			glActiveTexture( GL_TEXTURE0 + textureId[ DrawTexIdB ] );
			glBindTexture( GL_TEXTURE_RECTANGLE, textureId[ DrawTexIdB ] );
			SPM2_ConeShader.SetUniform( "drawId", (int)textureId[ DrawTexIdB ] );
		}
		else
		{
			glDrawBuffers( 1, &colorAttachments[ DrawTexIdB ] );
			glActiveTexture( GL_TEXTURE0 + textureId[ DrawTexIdA ] );
			glBindTexture( GL_TEXTURE_RECTANGLE, textureId[ DrawTexIdA ] );
			SPM2_ConeShader.SetUniform( "drawId", (int)textureId[ DrawTexIdA ] );
		}

		glActiveTexture( GL_TEXTURE0 + textureId[ StencilTexId ] );
		glBindTexture( GL_TEXTURE_RECTANGLE, textureId[ StencilTexId ] );

		SPM2_ConeShader.SetUniform( "texId", (int)textureId[ StencilTexId ] );

		QuadVbo.BindDrawElementsUnbind();

		// Update the distances of points visible from the current generator
		SPM2_DistanceShader.Use();

		glActiveTexture( GL_TEXTURE0 + textureId[ StencilTexId ] );
		glBindTexture( GL_TEXTURE_RECTANGLE, textureId[ StencilTexId ] );

		SPM2_DistanceShader.SetUniform( "texId", (int)textureId[ StencilTexId ] );

		glDispatchCompute( limit2, 1, 1 );
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

		drawA = !drawA;
	}

	if( drawA == false )
		DrawTexId = DrawTexIdA;
	else
		DrawTexId = DrawTexIdB;

	if( spm == nullptr )
		spm = CreateSPM();

	spm->LoadResultArrayFromGPU( ShaderStorageArray.size() );
	spm->LoadMapFromGPU( framebufferId, colorAttachments[ DrawTexId ] );

	glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 ); // bind the default framebuffer again for the rest of the application

	// restore the context to previous values ---------------------------------------------------//
	// M. Kallmann integration notes: reset methods will ensure context is set independently of GlContext's internal flags
	context->viewport( viewport_w, viewport_h );
	context->reset_clear_color( clear_color );
	context->reset_point_size( point_size );
	context->reset_line_width( line_size );
	context->reset_line_smoothing( line_smoothing );
	context->reset_transparency( transparency );
	context->reset_cull_face( cull_face );
	context->reset_depth_test( depth_test );
	context->reset_use_program( curprogram );
	//-------------------------------------------------------------------------------------------//

	return spm;
}

void ShortestPathMapManager::SetDomain( const GsPolygon& domain )
{
	envDomain = domain; // this should be a small rectangular polygon so just copy it
}

void ShortestPathMapManager::SetEnv( GsPolygons* obstacles, GsPolygons* sinks )
{
	if ( envObstacles ) envObstacles->unref();
	envObstacles = obstacles;
	envObstacles->ref();
	if ( envSinks ) envSinks->unref();
	envSinks = sinks;
	envSinks->ref();
}

void ShortestPathMapManager::CreateOrthogonalMatrices( void )
{
	double vnear = -1.0f;
	double vfar  =  1.0f;

	double tx = -( domainRight + domainLeft ) / ( domainRight - domainLeft );
	double ty = -( domainTop + domainBottom ) / ( domainTop - domainBottom );
	double tz = -( vfar + vnear ) / ( vfar - vnear );

	OrthoProjectionMatrix = GsMat( 2.0f / ( domainRight - domainLeft ), 0.0f, 0.0f, (float)tx,
		0.0f, 2.0f / ( domainTop - domainBottom ), 0.0f, (float)ty,
		0.0f,     0.0f, 2.0f / (float)( vfar - vnear ), (float)-tz,
		0.0f, 0.0f, 0.0f, 1.0f );
	OrthoViewMatrix.identity();
	OrthoModelMatrix.identity();

	OrthoProjectionCompleteMatrix.ortho( domainLeft, domainRight, domainBottom, domainTop, (float)vnear, (float)vfar );
}

void ShortestPathMapManager::UpdateDomainBoundaries( void )
{
	if( envDomain.empty() == true )
		return;

	float left   = ( std::numeric_limits<float>::max )();
	float right  = ( std::numeric_limits<float>::min )();
	float bottom = ( std::numeric_limits<float>::max )();
	float top    = ( std::numeric_limits<float>::min )();

	for( int i = 0; i < envDomain.size(); ++i )
	{
		float x = envDomain[i].x;
		float y = envDomain[i].y;

		left   = min(   left, x );
		right  = max(  right, x );
		bottom = min( bottom, y );
		top    = max(    top, y );
	}

	domainLeft   = (float)left;
	domainRight  = (float)right;
	domainTop    = (float)top;
	domainBottom = (float)bottom;

	CreateOrthogonalMatrices();
}

void ShortestPathMapManager::GetDomainBoundaries( float& left, float& right, float& top, float& bottom ) const
{
	left   = domainLeft;
	right  = domainRight;
	top    = domainTop;
	bottom = domainBottom;
}

std::vector< ShortestPathMap* >& ShortestPathMapManager::GetSPMs( void )
{
	return ShortestPathMaps;
}

void ShortestPathMapManager::FillShaderStorageArray( void )
{
	const float wExpandedVert  = 0.5f;
	const float wObstacleVert  = 1.0f;
	const float wSourceVert    = 1.5f;
	const float wSourceSegment = 2.0f;

	SpmVertexPos empty = { { 0.0f, 0.0f, 0.0f, 0.0f } };

	// these two first positions will be filled with the first source point later
	ShaderStorageArray.clear();
	ShaderStorageArray.push_back( empty );
	ShaderStorageArray.push_back( empty );

	// this keeps track of the current point id in the buffer
	unsigned int curId = 0;

	//---------------------------------------------------------------------------------------------
	// add source points & source segment endpoints
	//---------------------------------------------------------------------------------------------
	for( int j = 0; j < envSinks->size(); ++j )
	{
		const GsPolygon& poly = envSinks->get(j);
		for( int i = 0; i < poly.size(); ++i, ++curId )
		{
			ShaderStorageArray.push_back( { { poly[i].x, poly[i].y, 0.0f, wSourceVert  } } );
			ShaderStorageArray.push_back( { { poly[i].x, poly[i].y, 1.0f, (float)curId } } );
		}
	}

	//---------------------------------------------------------------------------------------------
	// add segment source critical points
	//---------------------------------------------------------------------------------------------
	for( int i = 0; i+1 < (int)CriticalPointsSS.size(); i += 2, ++curId )
	{
		ShaderStorageArray.push_back( { { CriticalPointsSS[i  ].x, CriticalPointsSS[i  ].y, 0.0f, wSourceSegment } } );
		ShaderStorageArray.push_back( { { CriticalPointsSS[i+1].x, CriticalPointsSS[i+1].y, 1.0f, (float)curId   } } );
	}

	// if there are no sources at this point, stop
	if( ShaderStorageArray.size() <= 2 )
		return;

	//---------------------------------------------------------------------------------------------
	// add obstacle vertices
	//---------------------------------------------------------------------------------------------
	float pertOffset = 0.0028f;

	for( int j = 0; j < envObstacles->size(); ++j )
	{
		const GsPolygon& poly = envObstacles->get(j);

		for( int i = 0; i < poly.size(); ++i, ++curId )
		{
			// calculate slightly offset coordinates (for visibility purposes only)
			const GsPnt2& pPrev = poly[ i > 0 ? i - 1 : poly.size() - 1 ];
			const GsPnt2& p = poly[ i ];
			const GsPnt2& pNext = poly[ ( i + 1 ) % poly.size() ];

			GsVec2 dir1, dir2, bis;

			if( poly.ccw() )
			{
				//dir1 = pPrev - p;
				//dir2 = pNext - p;
				dir1 = p - pNext;
				dir2 = pPrev - p;
			}
			else
			{
				//dir1 = p - pPrev;
				//dir2 = p - pNext;
				dir1 = pNext - p;
				dir2 = p - pPrev;
			}

			dir1.normalize();
			dir2.normalize();

			dir1 = GsVec2( dir1.y, -dir1.x );
			dir2 = GsVec2( dir2.y, -dir2.x );

			bis = ( dir1 + dir2 ) / 2.0f;

			GsPnt2 pPert = p + ( -bis * pertOffset );
			//GsPnt2 pPert = p;

			ShaderStorageArray.push_back( { { pPert.x, pPert.y, 0.0f, wObstacleVert } } );
			ShaderStorageArray.push_back( { { p.x, p.y, 1.0f, 0.0f } } );
		}
	}

	// set necessary shader uniforms
	int ShaderStorageArrayLeap = 0;
	int ShaderStorageArrayNumLeaps = 0;

	SPM2_DistanceShader.Use();
	SPM2_DistanceShader.SetUniform( "arrayLeap", ShaderStorageArrayLeap );
	SPM2_DistanceShader.SetUniform( "numLeaps", ShaderStorageArrayNumLeaps );

	// move the first source to generator position
	ShaderStorageArray[0] = ShaderStorageArray[2];
	ShaderStorageArray[1] = ShaderStorageArray[3];
	ShaderStorageArray[2].XYZW[3] = wExpandedVert;

	// lastly, add an empty position to denote end of the array
	ShaderStorageArray.push_back( empty );

	// transfer the array to GPU memory
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, shaderStorageBufferId );
	glBufferData( GL_SHADER_STORAGE_BUFFER, SpmVertexPosSize * ShaderStorageArray.size(), &ShaderStorageArray[ 0 ], GL_DYNAMIC_DRAW );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );
}

bool ShortestPathMapManager::SourceSegmentCompare( GsPnt2& v1, GsPnt2& v2 )
{
	GsPnt2& o = SourceSegments[ currentCriticalPointSortBaseId ];

	double d1 = gs_dist( v1.x, v1.y, o.x, o.y );
	double d2 = gs_dist( v2.x, v2.y, o.x, o.y );

	if ( d1 < d2 ) return true;
	return false;
}

bool ShortestPathMapManager::IntersectsAnyPolygonSegmentNoEndpoints( float x1, float y1, float x2, float y2, float eps )
{
	for( int p = 0; p < envObstacles->size(); ++p )
	{
		const GsPolygon& poly = envObstacles->get(p);

		int polygonTouchedCount = 0;

		for( int v = 0; v < poly.size(); ++v )
		{
			const GsVec2& p0 = poly.get( v );
			const GsVec2& p1 = poly.get( ( v + 1 ) % poly.size() );

			// avoid false positive from self-intersection
			if( ( p0.x == x1 && p0.y == y1 ) || ( p0.x == x2 && p0.y == y2 ) ||
				( p1.x == x1 && p1.y == y1 ) || ( p1.x == x2 && p1.y == y2 ) )
				continue;

			if( gs_segments_intersect( x1, y1, x2, y2, p0.x, p0.y, p1.x, p1.y ) == true )
			{
				// we only truly consider it an intersection if it's not at one of the endpoints
				if( gs_in_segment( p0.x, p0.y, p1.x, p1.y, x1, y1, eps ) == false &&
					gs_in_segment( p0.x, p0.y, p1.x, p1.y, x2, y2, eps ) == false )
					return true;

				++polygonTouchedCount;
			}
		}

		// if it intersected a polygon more than once, even just at endpoints, that means it crossed it
		if( polygonTouchedCount > 1 )
			return true;
	}

	return false;
}

void ShortestPathMapManager::ComputeCriticalPointsForSourceLines( void )
{
	SourceSegments.clear();
	for( int s = 0; s < envSinks->size(); ++s )
	{
		const GsPolygon& poly = envSinks->get(s);
		for( int i = 0; i < poly.size()-1; ++i )
		{
			SourceSegments.push_back( poly[i  ] );
			SourceSegments.push_back( poly[i+1] );
		}
	}

	CriticalPointsSS.clear();

	for( int l = 0; l+1 < (int)SourceSegments.size(); l += 2 )
	{
		vector< GsPnt2 > criticalPoints;

		// creates critical points by projecting all vertices with direct line-of-sight onto the line segment source
		for( int p = 0; p < envObstacles->size(); ++p )
		{
			const GsPolygon& poly = envObstacles->get(p);

			for( int v = 0; v < poly.size(); ++v )
			{
				GsPnt2 point = poly[v];

				// if this point's projection onto the line segment isn't within the boundary (closer to the middle than the endpoints), skip it
				double qx, qy;
				if( gs_segment_projection( SourceSegments[l].x, SourceSegments[l].y, SourceSegments[l+1].x, SourceSegments[l+1].y, point.x, point.y, qx, qy, 0.001 ) != 3 )
					continue;

				if( IntersectsAnyPolygonSegmentNoEndpoints( point.x, point.y, (float)qx, (float)qy ) == false )
					criticalPoints.push_back( GsPnt2( qx, qy ) );
			}
		}

		// order critical points
		currentCriticalPointSortBaseId = l;
		if( criticalPoints.size() > 0 )
			std::sort( criticalPoints.begin(), criticalPoints.end(), [this](GsPnt2& l, GsPnt2& r) {return SourceSegmentCompare(l, r);} );

		// add the endpoints to the beginning and end
		criticalPoints.insert( criticalPoints.begin(), GsPnt2( SourceSegments[l].x, SourceSegments[l].y ) );
		criticalPoints.push_back( GsPnt2( SourceSegments[l+1].x, SourceSegments[l+1].y ) );

		// get rid of duplicates
		auto it = std::unique( criticalPoints.begin(), criticalPoints.end() );
		criticalPoints.erase( it, criticalPoints.end() );

		// insert the critical points into the master list
		for( int i = 0; i+1 < (int)criticalPoints.size(); ++i )
		{
			CriticalPointsSS.push_back( criticalPoints[i  ] );
			CriticalPointsSS.push_back( criticalPoints[i+1] );
		}
	}
}

/*=================================================================================================
	BUFFERS
=================================================================================================*/

void ShortestPathMapManager::CreateBuffers( void )
{
	// Create a framebuffer object
	glGenFramebuffers( 1, &framebufferId );
	glBindFramebuffer( GL_FRAMEBUFFER, framebufferId );

	// Fill array with GL_COLOR_ATTACHMENT ids
	for( int i = 0; i < numTextures; ++i )
		colorAttachments[ i ] = GL_COLOR_ATTACHMENT0 + i;

	// Create textures
	glGenTextures( numTextures, &textureId[0] );
	for( int i = 0; i < numTextures; ++i )
	{
		glBindTexture( GL_TEXTURE_RECTANGLE, textureId[i] );
		glTexParameterf( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexImage2D( GL_TEXTURE_RECTANGLE, 0, GL_RGBA32F, bufferWidth, bufferHeight, 0, GL_RGBA, GL_FLOAT, 0 );
		//glTexImage2D( GL_TEXTURE_RECTANGLE, 0, GL_RGBA, bufferWidth, bufferHeight, 0, GL_RGBA, GL_FLOAT, 0 );
		//gsout<<textureId[i]<<gsnl;
	}

	// Attach texture objects to the framebuffer color attachment points
	for( int i = 0; i < numTextures; ++i )
		glFramebufferTexture2D( GL_FRAMEBUFFER, colorAttachments[i], GL_TEXTURE_RECTANGLE, textureId[i], 0 );

	// Create a renderbuffer object to store depth/stencil info
	glGenRenderbuffers( 1, &renderbufferId );
	glBindRenderbuffer( GL_RENDERBUFFER, renderbufferId );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, bufferWidth, bufferHeight );

	// Attach the render buffer to the framebuffer depth/stencil attachment point
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbufferId );

	// Switch back to window-system-provided framebuffer
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// Create shader storage buffer
	glGenBuffers( 1, &shaderStorageBufferId );
	GLuint block_index = 0, ssbo_binding_point_index = 0;
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, ssbo_binding_point_index, shaderStorageBufferId );
}

void ShortestPathMapManager::DeleteBuffers( void )
{
	glDeleteTextures( numTextures, &textureId[ 0 ] );
	glDeleteRenderbuffers( 1, &renderbufferId );
	glDeleteFramebuffers( 1, &framebufferId );
	glDeleteBuffers( 1, &shaderStorageBufferId );
}

/*=================================================================================================
	VBOs
=================================================================================================*/

void ShortestPathMapManager::CreateQuadVbo( void )
{
	vector< SpmVertex > Vertices;

	Vertices.push_back( { { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } } );
	Vertices.push_back( { {  1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } } );
	Vertices.push_back( { {  1.0f,  1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } } );
	Vertices.push_back( { { -1.0f,  1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } } );

	vector< unsigned int > Indices;
	for( int i = 0; i < (int)Vertices.size(); ++i )
		Indices.push_back( i );

	const size_t BufferSize = sizeof( Vertices[ 0 ] ) * Vertices.size();
	const size_t IndicesSize = sizeof( Indices[ 0 ] ) * Indices.size();

	QuadVbo.GenVertexBuffer( BufferSize, &Vertices[0] );
	QuadVbo.SetAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, SpmVertexSize, 0 );
	QuadVbo.SetAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, SpmVertexSize, (GLvoid*)SpmRgbaOffset );
	QuadVbo.GenIndexBuffer( IndicesSize, &Indices[0] );
	QuadVbo.SetDrawMode( GL_QUADS );
	QuadVbo.SetIndexType( GL_UNSIGNED_INT );
}

void ShortestPathMapManager::CreateSceneVbo( void )
{
	if( envObstacles->empty() )
		return;

	vector< SpmVertex > SceneVertices;
	vector< unsigned int > SceneIndices;

	for( int p = 0; p < envObstacles->size(); ++p )
	{
		vector< SpmVertex > PolyVertices;
		vector< unsigned int > PolyIndices;

		const GsPolygon& region = envObstacles->get(p);

		for( int v = 0; v < region.size(); ++v )
		{
			SpmVertex vert = { { region[v].x, region[v].y, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } };

			PolyVertices.push_back( vert );
			SceneVertices.push_back( vert );

			if( v > 0 )
			{
				PolyIndices.push_back( (unsigned int)SceneVertices.size() - 2 );
				PolyIndices.push_back( (unsigned int)SceneVertices.size() - 1 );
			}
		}

		PolyIndices.push_back( (unsigned int)SceneVertices.size() - 1 );
		PolyIndices.push_back( (unsigned int)SceneVertices.size() - region.size() );

		if( region.ccw() )
			SceneIndices.insert( SceneIndices.begin(), PolyIndices.begin(), PolyIndices.end() );
		else
			SceneIndices.insert( SceneIndices.begin(), PolyIndices.rbegin(), PolyIndices.rend() );
	}

	size_t BufferSize = SceneVertices.size() * sizeof( SceneVertices[0] );
	size_t IndicesSize = SceneIndices.size() * sizeof( SceneIndices[0] );

	SceneVbo.GenVertexBuffer( BufferSize, &SceneVertices[0] );
	SceneVbo.SetAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, SpmVertexSize, 0 );
	SceneVbo.SetAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, SpmVertexSize, (GLvoid*)SpmRgbaOffset );
	SceneVbo.GenIndexBuffer( IndicesSize, &SceneIndices[0] );
	SceneVbo.SetDrawMode( GL_LINES );
	SceneVbo.SetIndexType( GL_UNSIGNED_INT );
}

/*=================================================================================================
	SHADERS
=================================================================================================*/

void ShortestPathMapManager::SetShaderBufferDimensions( void )
{
	int dim[2] = { bufferWidth, bufferHeight };

	for( int i = 0; i < (int)ShaderDimensionList.size(); ++i )
		if( ShaderDimensionList[i] != nullptr )
		{
			ShaderDimensionList[i]->Use();
			ShaderDimensionList[i]->SetUniform( "window", &dim[0], 2, 1 );
		}
}

void ShortestPathMapManager::SetShaderOrthogonalMatrices( void )
{
	for( int i = 0; i < (int)ShaderOrthoList.size(); ++i )
		if( ShaderOrthoList[i] != nullptr )
		{
			ShaderOrthoList[i]->Use();
			ShaderOrthoList[i]->SetUniform( "projectionMatrix", &OrthoProjectionMatrix.e[0], 4, GL_FALSE, 1 );
			ShaderOrthoList[i]->SetUniform( "viewMatrix", &OrthoViewMatrix.e[0], 4, GL_FALSE, 1 );
			ShaderOrthoList[i]->SetUniform( "modelMatrix", &OrthoModelMatrix.e[0], 4, GL_FALSE, 1 );
		}
}

void ShortestPathMapManager::UpdateShaderUniforms( void )
{
	//tmp
	bool DistancePrecision = true;
	int ShaderStorageArrayLeap = 0;
	int ShaderStorageArrayNumLeaps = 0;

	SPM2_DistanceShader.Use();
	SPM2_DistanceShader.SetUniform( "prec", DistancePrecision );
	SPM2_DistanceShader.SetUniform( "arrayLeap", ShaderStorageArrayLeap );
	SPM2_DistanceShader.SetUniform( "numLeaps", ShaderStorageArrayNumLeaps );

	SetShaderBufferDimensions();
	SetShaderOrthogonalMatrices();
}

void ShortestPathMapManager::ReloadShaders( void )
{
	for( int i = 0; i < (int)ShaderList.size(); ++i )
		if( ShaderList[i] != nullptr )
			ShaderList[i]->Reload();

	UpdateShaderUniforms();
}

# include "spm_predef_shaders.inc"

void ShortestPathMapManager::CreateShaders( bool* loadtest )
{
	const char* folder = shadersFolder.c_str();

	// M. Kallmann integration notes: for maximum flexibility shaders can be loaded
	// from pre-defined strings, or from user-provided folder:
	SpmShaderSrc spm_cone_vert ( folder, "spm_cone.vert", pds_spm_cone_vert );
	SpmShaderSrc spm_cone_frag ( folder, "spm_cone.frag", pds_spm_cone_frag );
	SpmShaderSrc spm_distance_comp ( folder, "spm_distance.comp", pds_spm_distance_comp );
	SpmShaderSrc spm_search_comp ( folder, "spm_search.comp", pds_spm_search_comp );
	SpmShaderSrc spm_shadow_vert ( folder, "spm_shadow.vert", pds_spm_shadow_vert );
	SpmShaderSrc spm_shadow_geom ( folder, "spm_shadow.geom", pds_spm_shadow_geom );
	SpmShaderSrc spm_shadow_frag ( folder, "spm_shadow.frag", pds_spm_shadow_frag );

	if ( loadtest )
	{	*loadtest = spm_cone_vert.CanLoad(); if (!*loadtest) return;
		*loadtest = spm_cone_frag.CanLoad(); if (!*loadtest) return;
		*loadtest = spm_distance_comp.CanLoad(); if (!*loadtest) return;
		*loadtest = spm_search_comp.CanLoad(); if (!*loadtest) return;
		*loadtest = spm_shadow_vert.CanLoad(); if (!*loadtest) return;
		*loadtest = spm_shadow_geom.CanLoad(); if (!*loadtest) return;
		*loadtest = spm_shadow_frag.CanLoad();
		return;
	}

	SPM2_ConeShader.Create( spm_cone_vert, spm_cone_frag );
	SPM2_DistanceShader.Create( spm_distance_comp );
	SPM2_SearchShader.Create( spm_search_comp );
	SPM2_ShadowShader.Create( spm_shadow_vert, spm_shadow_geom, spm_shadow_frag );

	// Add shaders to relevant lists
	ShaderList.push_back( &SPM2_ConeShader );
	ShaderList.push_back( &SPM2_DistanceShader );
	ShaderList.push_back( &SPM2_SearchShader );
	ShaderList.push_back( &SPM2_ShadowShader );

	ShaderOrthoList.push_back( &SPM2_ConeShader );
	ShaderOrthoList.push_back( &SPM2_DistanceShader );
	ShaderOrthoList.push_back( &SPM2_ShadowShader );

	ShaderDimensionList.push_back( &SPM2_ConeShader );
	ShaderDimensionList.push_back( &SPM2_DistanceShader );
}

