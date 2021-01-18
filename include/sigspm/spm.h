/*=======================================================================
   Copyright (c) 2020 Renato Farias.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

#pragma once

#include <sig/gs_polygons.h>
#include <sig/gs_camera.h>
#include <sig/gs_model.h>
#include <sig/gs_mat.h>
#include <sig/gs_vec.h>
#include <sigogl/gl_context.h>
#include <vector>

#include "spm_shader_program.h"
#include "spm_vbo.h"

//---------------------------------------------------------------------------------------------
//	HELPFUL STRUCTS
//---------------------------------------------------------------------------------------------
struct SpmVertexPos {
	float XYZW[4];
};

struct SpmVertexColor {
	float RGBA[4];
};

struct SpmVertex {
	float XYZW[4];
	float RGBA[4];
};

struct SpmFullVertex {
	float XYZW[4];
	float RGBA[4];
	float N[4];
};

const size_t SpmVertexSize     = sizeof( SpmVertex );
const size_t SpmFullVertexSize = sizeof( SpmFullVertex );
const size_t SpmRgbaOffset     = sizeof( decltype( SpmVertex::XYZW ) );
const size_t SpmNormalOffset   = sizeof( decltype( SpmFullVertex::XYZW ) ) + sizeof( decltype( SpmFullVertex::RGBA ) );
const size_t SpmVertexPosSize  = sizeof( SpmVertexPos );

//---------------------------------------------------------------------------------------------

class ShortestPathMap
{
public:
	ShortestPathMap();
   ~ShortestPathMap();

public:
	//---------------------------------------------------------------------------------------------
	//	DATA
	//---------------------------------------------------------------------------------------------

	void SetName( const std::string& );
	const std::string& GetName() const;

	void SetBufferDimensions( int w, int h );
	void SetOrthoProjectionMatrix( const GsMat& mat );
	void SetShaderStorageBufferVariables( GLuint _ssbId, size_t n );
	void SetBufferIds( GLuint _fbId, GLuint _tId, GLenum _rbId );
	void Invalidate();
	bool IsReadyToQuery();

	//---------------------------------------------------------------------------------------------
	//	RESULT ARRAY
	//---------------------------------------------------------------------------------------------

	// Return a reference to the ResultArray
	const std::vector< SpmVertexPos >& GetResultArray() const;

	// Load the ResultArray from the shader storage buffer object
	void LoadResultArrayFromGPU();
	void LoadResultArrayFromGPU( GLuint _ssbId, size_t n, bool print_info = false );

	// Load a saved ResultArray from a file
	bool LoadResultArrayFromFile( const std::string& filepath );

	// Save the ResultArray to a file
	bool SaveResultArrayToFile( const std::string& filepath ) const;

	//---------------------------------------------------------------------------------------------
	//	MAP
	//---------------------------------------------------------------------------------------------

	// Return a reference to the SPM's map
	const std::vector< float >& GetMap() const;

	// Load SPM from the GPU
	void LoadMapFromGPU();
	void LoadMapFromGPU( GLuint _fbId, GLenum _rbId );

	// Load SPM from an image file
	bool LoadMapFromImageFile( const std::string& filepath );

	// Save SPM to a GsImage in memory by taking a snapshot of the buffer
	void SaveMapToGsImage( GsImage& img ) const;

	// Save SPM to an image file
	bool SaveMapToImageFile( const std::string& filepath ) const;

	//---------------------------------------------------------------------------------------------
	//	ACCESS
	//---------------------------------------------------------------------------------------------

	// Load necessary things for SPM to be usable
	void LoadSPM();

	// Return a pointer to the spm buffer
	const float* GetMapBuffer () const { return &(Map[0]); }

	// Return the map width
	int Width () const { return bufferWidth; }

	// Return the map height
	int Height () const { return bufferHeight; }

	// Find the closest (parent) point of the point whose 1d coordinate in the map is 'pos'
	int FindClosestPoint( int pos );

	// Get the shortest path back to the destination (SPM source), starting from coordinates (x,y) in world-space
	// If maxnp>0, the returned path may be truncated in order to contain only up to maxnp points
	bool GetShortestPath( float _x, float _y, std::vector<GsPnt2>& path, int maxnp=-1 );

	// Get the SPM vector field direction at coordinates (x,y) in world-space (Optimized version added by MK)
	bool GetDirection( float _x, float _y, GsVec2& dir, float normalize=true );

	// Get the direction on the shortest path back to the destination (SPM source), starting from coordinates (x,y) in world-space
	// A maximum of maxnp points in the shortest path are retrieved and parent points that are closer than threshold will be skipped
	// Considered points are returned in path
	bool GetFilteredDirection( float _x, float _y, GsVec2& dir, std::vector<GsVec2>& path, float threshold=0.01f, float normalize=true, int maxnp=3 );

private:
	std::vector< float > Map;
	std::vector< SpmVertexPos > ResultArray;

	std::string Name;
	int bufferWidth, bufferHeight;
	GsMat OrthoProjectionCompleteMatrix;
	GsMat OrthoProjectionCompleteMatrixInv;

	// variables to load array/map from GPU
	GLuint shaderStorageBufferId;
	size_t shaderStorageBufferSize;
	GLuint framebufferId;
	GLuint textureId;
	GLenum readbufferId;

	bool readyToQuery; // when false, SPM will load buffers from GPU when prompted to answer a path query
};
