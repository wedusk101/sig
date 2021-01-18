/*=======================================================================
   Copyright (c) 2020 Renato Farias.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

#include <sigspm/spm.h>

#include <sigogl/glcorearb.h>
#include <sigogl/glcorearb_functions.h>
#include <sigogl/gl_shader.h>
#include <sigogl/gl_program.h>
#include <sigogl/gl_tools.h>
#include <sigogl/ws_window.h>

#include <sig/gs_geo2.h>
#include <sig/gs_image.h>

#include <algorithm>
#include <fstream>
#include <limits>

using namespace std;

/*=================================================================================================
	CONSTRUCTION / DESTRUCTION
=================================================================================================*/

ShortestPathMap::ShortestPathMap()
{
	Name = "SPM";

	bufferWidth  = 0;
	bufferHeight = 0;

	shaderStorageBufferId = 0;
	shaderStorageBufferSize = 0;
	framebufferId = 0;
	readbufferId  = 0;

	readyToQuery = false;
}

ShortestPathMap::~ShortestPathMap()
{
}

/*=================================================================================================
	DATA
=================================================================================================*/

void ShortestPathMap::SetName( const std::string& _name )
{
	Name = _name;
}

const std::string& ShortestPathMap::GetName( void ) const
{
	return Name;
}

void ShortestPathMap::SetBufferDimensions( int w, int h )
{
	bufferWidth  = w;
	bufferHeight = h;
}

void ShortestPathMap::SetOrthoProjectionMatrix( const GsMat& mat )
{
	OrthoProjectionCompleteMatrix = mat;
}

void ShortestPathMap::SetShaderStorageBufferVariables( GLuint _ssbId, size_t n )
{
	shaderStorageBufferId = _ssbId;
	shaderStorageBufferSize = n;
}

void ShortestPathMap::SetBufferIds( GLuint _fbId, GLuint _tId, GLenum _rbId )
{
	framebufferId = _fbId;
	textureId = _tId;
	readbufferId  = _rbId;
}

void ShortestPathMap::Invalidate( void )
{
	readyToQuery = false;
}

bool ShortestPathMap::IsReadyToQuery()
{
	return readyToQuery;
}

/*=================================================================================================
	RESULT ARRAY
=================================================================================================*/

const std::vector< SpmVertexPos >& ShortestPathMap::GetResultArray( void ) const
{
	return ResultArray;
}

void ShortestPathMap::LoadResultArrayFromGPU( void )
{
	LoadResultArrayFromGPU( shaderStorageBufferId, shaderStorageBufferSize );
}

void ShortestPathMap::LoadResultArrayFromGPU( GLuint _ssbId, size_t n, bool print_info )
{
	ResultArray.resize( n );

	//glBindFramebuffer( GL_FRAMEBUFFER, framebufferId );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, _ssbId );
	glGetBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, SpmVertexPosSize * ResultArray.size(), &ResultArray[ 0 ] );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );

	if( print_info )
	{
		gsout << "RESULT BUFFER\n";
		for( int i = 0; i < (int)ResultArray.size(); ++i )
		{
			const SpmVertexPos& v = ResultArray[ i ];
			gsout << i << ": (" << v.XYZW[ 0 ] << ", " << v.XYZW[ 1 ] << ", " << v.XYZW[ 2 ] << ", " << v.XYZW[ 3 ] << ")\n";
		}
		gsout << "\n";
	}
}

bool ShortestPathMap::LoadResultArrayFromFile( const std::string& filepath )
{
	ifstream file( filepath );
	if( file.is_open() == false )
		return false;

	ResultArray.clear();

	SpmVertexPos vp;
	while( file.good() )
	{
		file >> vp.XYZW[0];
		file >> vp.XYZW[1];
		file >> vp.XYZW[2];
		file >> vp.XYZW[3];
		ResultArray.push_back( vp );
	}

	return true;
}

bool ShortestPathMap::SaveResultArrayToFile( const std::string& filepath ) const
{
	ofstream file( filepath, std::ofstream::out | std::ofstream::trunc );
	if( file.is_open() == false )
		return false;

	for( int i = 0; i < (int)ResultArray.size(); ++i )
	{
		const SpmVertexPos& v = ResultArray[ i ];
		file << v.XYZW[ 0 ] << " " << v.XYZW[ 1 ] << " " << v.XYZW[ 2 ] << " " << v.XYZW[ 3 ];
		if( i < (int)ResultArray.size() - 1 )
			file << endl;
	}
	file.close();

	return true;
}

/*=================================================================================================
	MAP
=================================================================================================*/

static void my_gl_snapshot( GsImage& img )
{
	int vp[ 4 ];

	glGetIntegerv( GL_VIEWPORT, vp );

	int x = vp[ 0 ];
	int y = vp[ 1 ];
	int w = vp[ 2 ] - x;
	int h = vp[ 3 ] - y;
	if( x < 0 || y < 0 || w <= 0 || h <= 0 ) return; // ogl not initialized

	img.init( w, h );

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );

	glReadPixels( x, y, w, h,
		GL_RGBA, // GLenum format
		GL_UNSIGNED_BYTE, //GLenum type
		(void*)img.data() //GLvoid *pixels
	);

	img.vertical_mirror();
}

const std::vector< float >& ShortestPathMap::GetMap( void ) const
{
	return Map;
}

void ShortestPathMap::LoadMapFromGPU( void )
{
	LoadMapFromGPU( framebufferId, readbufferId );
}

void ShortestPathMap::LoadMapFromGPU( GLuint _fbId, GLenum _rbId )
{
	if( bufferWidth == 0 || bufferHeight == 0 )
		return;

	const int no_pixels = bufferWidth * bufferHeight;
	const int no_floats = no_pixels * 4;

	Map.resize( no_floats );

	glBindFramebuffer( GL_FRAMEBUFFER, _fbId );
	glReadBuffer( _rbId );
	glReadPixels( 0, 0, bufferWidth, bufferHeight, GL_RGBA, GL_FLOAT, &Map[0] );
	//glReadnPixels( 0, 0, bufferWidth, bufferHeight, GL_RGBA, GL_FLOAT, no_floats, &Map[0] );
}

bool ShortestPathMap::LoadMapFromImageFile( const std::string& filepath )
{
	GsImage img;
	if( img.load( filepath.c_str() ) == false )
		return false;

	img.vertical_mirror();

	const int no_pixels = img.w() * img.h();
	const int no_floats = no_pixels * 4;

	Map.clear();
	Map.resize( no_floats );

	GsColor* data = img.data();

	for( int i = 0; i < no_pixels; ++i )
	{
		Map[(i*4)  ] = data[i].r / 255.0f;
		Map[(i*4)+1] = data[i].g / 255.0f;
		Map[(i*4)+2] = data[i].b / 255.0f;
		Map[(i*4)+3] = data[i].a / 255.0f;
	}

	return true;
}

void ShortestPathMap::SaveMapToGsImage( GsImage& img ) const
{
	if( Map.empty() == true )
		return;

	img.init( bufferWidth, bufferHeight );

	GsColor* data = img.data();

	for( int i=0, s=img.h()*img.w(); i<s; ++i )
		data[i].set( Map[(i*4)], Map[(i*4)+1], Map[(i*4)+2], Map[(i*4)+3] );

	// contour lines
	//for( int i = 0; i < img.h() * img.w(); ++i )
	//	if( int( Map[(i*4)+2] * 255 ) % 6 == 0 )
	//		data[i].set( 1.0f, 1.0f, 1.0f, 1.0f );
}

bool ShortestPathMap::SaveMapToImageFile( const std::string& filepath ) const
{
	GsImage img;
	SaveMapToGsImage( img );
	img.vertical_mirror();
	return img.save( filepath.c_str() );
}

/*=================================================================================================
	ACCESS
=================================================================================================*/

void ShortestPathMap::LoadSPM( void )
{
	//LoadResultArrayFromGPU();
	//LoadMapFromGPU();

	if( Map.empty() == true || ResultArray.empty() == true )
		readyToQuery = false;
	else
		readyToQuery = true;
}

int ShortestPathMap::FindClosestPoint( int pos )
{
	if( pos < 0 || pos >= (int)Map.size() - 3 )
		return -1;

	float x = Map[ pos     ];
	float y = Map[ pos + 1 ];
	float z = Map[ pos + 2 ];
	float w = Map[ pos + 3 ];

	if( w < 1.0f )
		return -1;

	x = ( x * 2.0f ) - 1.0f;
	y = ( y * 2.0f ) - 1.0f;

	int minIdx = -1;
	double minDist = -1.0f;

	for( unsigned int i = 2; i < ResultArray.size() - 1; i += 2 )
	{
		const SpmVertexPos& originalPoint = ResultArray[ i ];
		GsVec p = OrthoProjectionCompleteMatrix * GsVec( originalPoint.XYZW[ 0 ], originalPoint.XYZW[ 1 ], 0.0f );

		double dist = gs_dist( p.x, p.y, x, y );

		if( minIdx == -1 || dist < minDist )
		{
			minIdx = i;
			minDist = dist;
		}
	}

	return minIdx;
}

bool ShortestPathMap::GetShortestPath( float _x, float _y, vector<GsPnt2>& path, int maxnp )
{
	// SpmTodo:
	// -optimize method
	// -GsVec -> GsVec2
   // - precomp OrthoProjectionCompleteMatrix.inverse()

	if( !readyToQuery )
	{	LoadSPM();
		if( !readyToQuery )	return false;
	}

	// first point (agent's coordinates)
	GsVec2 current(_x,_y), projectedCurrent(_x,_y);
	OrthoProjectionCompleteMatrix.mult2d(projectedCurrent);

	path.clear();
	path.push_back( current );

	// Path back to source
	float x = ( projectedCurrent.x + 1.0f ) / 2.0f;
	float y = ( projectedCurrent.y + 1.0f ) / 2.0f;
	int ix = (int)( x * bufferWidth );
	int iy = (int)( y * bufferHeight );
	if (ix<0) ix=0; if(ix>=bufferWidth) ix=bufferWidth-1;
	if (iy<0) iy=0; if(iy>=bufferHeight) iy=bufferHeight-1;
	int pos = 4 * ( iy * bufferWidth + ix );
	int curPos = (int)Map[ pos + 3 ];
	if( curPos < 2 ) return false;

	// if the pixel points directly to source, just use its parent coordinates
	// note: this depends on the pixel storing xy coordinates, and not color information
	if( curPos == (int)ResultArray[ curPos + 1 ].XYZW[ 3 ] )
	{
		current.x = Map[ pos     ];
		current.y = Map[ pos + 1 ];

		current.x = ( current.x * 2.0f ) - 1.0f;
		current.y = ( current.y * 2.0f ) - 1.0f;

		OrthoProjectionCompleteMatrix.inverse().mult2d ( current );

		path.push_back( current );
	}
	else
	{
		while( true )
		{
			const SpmVertexPos& originalPoint1 = ResultArray[ curPos     ];
			const SpmVertexPos& originalPoint2 = ResultArray[ curPos + 1 ];

			GsVec projectedPoint1 = GsVec( originalPoint1.XYZW[ 0 ], originalPoint1.XYZW[ 1 ], 0.0f );
			GsVec projectedPoint2 = GsVec( originalPoint2.XYZW[ 0 ], originalPoint2.XYZW[ 1 ], 0.0f );

			if( gs_dist( current.x, current.y, projectedPoint1.x, projectedPoint1.y ) < gs_dist( current.x, current.y, projectedPoint2.x, projectedPoint2.y ) )
			{
				current.x = projectedPoint1.x;
				current.y = projectedPoint1.y;
			}
			else
			{
				current.x = projectedPoint2.x;
				current.y = projectedPoint2.y;
			}

			path.push_back( current );
			if ( path.size()==maxnp ) break;

			if( curPos == (int)ResultArray[ curPos + 1 ].XYZW[ 3 ] ) break;

			curPos = (int)ResultArray[ curPos + 1 ].XYZW[ 3 ];
		}
	}

	return true;
}

bool ShortestPathMap::GetNextDirection( float _x, float _y, GsVec2& dir, float threshold, float normalize, int maxnp )
{
	dir.x = 0.0f;
	dir.y = 0.0f;
	vector<GsVec2> path; // SpmTodo: only the first point should be needed to retrieve a direction!
	if( !GetShortestPath( _x, _y, path, maxnp ) || path.size()<2 ) return false;
	GsVec origin( _x, _y, 0.0f );

	for( int i=1; i<(int)path.size(); ++i )
	{
		if( gs_dist( origin.x, origin.y, path[ i ].x, path[ i ].y ) >= threshold || i == path.size() - 1 )
		{
			dir = path[ i ] - origin;
			if ( normalize ) dir.normalize();
			break;
		}
	}
	return true;
}
