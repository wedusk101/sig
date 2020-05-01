/*=======================================================================
   Copyright (c) 2020 Renato Farias.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

#pragma once

#include <sigogl/gl_context.h>
#include <vector>

#include "spm.h"
#include "spm_shader_program.h"
#include "spm_vbo.h"

class ShortestPathMapManager
{
public:
	ShortestPathMapManager();
   ~ShortestPathMapManager();

public:
	// This has to be the 1st function called. It will set the folder to look for the needed shaders. Default folder is "./" and if shader files are not found the built-in pre-defined shaders are used
	void SetShadersFolder ( const char* folder );

	// Test if shaders can be loaded from given source folder
	bool CanLoadShaders ();

	// Creates a new empty SPM with initial buffer dimensions and projection matrix. Use this instead of pushing into the vector directly
	ShortestPathMap* CreateSPM();

	// Creates a new SPM and loads its map and result array from a file. Returns false if files were not found
	bool LoadSPM( const std::string& spmName, const std::string& spmPath, const std::string& raPath );

	// Saves an SPM's map and result array to specified files
	bool SaveSPM( int i, const std::string& spmPath, const std::string& raPath ) const;

	// Computes an SPM. Overwrites 'spm' if not null, otherwise creates new SPM and inserts it into the list. Returns a pointer to the computed SPM
	ShortestPathMap* Compute( GlContext* context, ShortestPathMap* spm = nullptr, bool loadFromGPU = false );

	// Defines the domain of the environment:
	// Domain has to be a closed rectangular-like polygon
	void SetDomain( const GsPolygon& domain );

	// Defines the environment:
	// Parameters obstacles and sinks are shared pointers. Obstacles are closed polygons and sinks are open polygons.
	void SetEnv( GsPolygons* obstacles, GsPolygons* sinks );

	// Updates domain boundaries based on the environment's current domain
	void UpdateDomainBoundaries();

	// Returns current domain boundaries, which should be equal to the current environment's
	void GetDomainBoundaries( float& left, float& right, float& top, float& bottom ) const;

	// Returns a reference to the list of SPMs currently stored in the manager
	std::vector< ShortestPathMap* >& GetSPMs();

	// Returns the Id of the framebuffer the SPM manager is using
	GLuint GetFramebufferId();

	// Returns the texture Id where the last draw call was executed (where the SPM is stored)
	GLuint GetDrawTexId();

	// Sets the dimensions of the GPU buffers and updates them
	void SetBufferDimensions( int width, int height );

	// Returns the current dimensions of the GPU buffers
	int GetBufferWidth();
	int GetBufferHeight();

private:
	void Initialize(); // called once the first time an SPM is computed

	// DATA
	void CreateOrthogonalMatrices();
	void FillShaderStorageArray();
	bool SourceSegmentCompare( GsPnt2& v1, GsPnt2& v2 );
	bool IntersectsAnyPolygonSegmentNoEndpoints( float x1, float y1, float x2, float y2, float eps = 0.001f );
	void ComputeCriticalPointsForSourceLines();

	// BUFFERS
	void CreateBuffers();
	void DeleteBuffers();

	// VBOs
	void CreateQuadVbo();
	void CreateSceneVbo();

	// SHADERS
	void SetShaderBufferDimensions();
	void SetShaderOrthogonalMatrices();
	void UpdateShaderUniforms();
	void ReloadShaders();
	void CreateShaders( bool* onlyloadtest=0 );

private:
	bool initialized;
	std::string shadersFolder;

	//---------------------------------------------------------------------------------------------
	//	SPMs
	//---------------------------------------------------------------------------------------------

	std::vector< ShortestPathMap* > ShortestPathMaps;

	//---------------------------------------------------------------------------------------------
	//	DATA
	//---------------------------------------------------------------------------------------------

	int bufferWidth, bufferHeight;
	GsPolygon envDomain;
	GsPolygons* envObstacles, *envSinks; // shared pointers

	std::vector< SpmVertexPos > ShaderStorageArray;

	unsigned int currentCriticalPointSortBaseId; // what point to use as a base when sorting the critical point list
	std::vector< GsPnt2 > SourceSegments;
	std::vector< GsPnt2 > CriticalPointsSS; // ordered list of critical points for segment sources

	// Orthogonal projection
	GsMat OrthoProjectionCompleteMatrix;
	GsMat OrthoProjectionMatrix, OrthoViewMatrix, OrthoModelMatrix;

	// Domain size
	float domainLeft = -1.0f, domainRight = 1.0f, domainTop = 1.0f, domainBottom = -1.0f;

	//---------------------------------------------------------------------------------------------
	//	BUFFERS
	//---------------------------------------------------------------------------------------------

	int numTextures = 4;

	// Explicitly assigning roles to each of the color attachments
	GLuint DrawTexId    = 0; // This variable will hold the id of the texture buffer where the last draw ocurred
	GLuint DrawTexIdA   = 0;
	GLuint DrawTexIdB   = 1;
	GLuint StencilTexId = 2;

	// Buffer IDs
	GLuint framebufferId, renderbufferId, stencilbufferId, shaderStorageBufferId;
	GLuint textureId[4];
	GLenum colorAttachments[4];

	//---------------------------------------------------------------------------------------------
	//	VBOs
	//---------------------------------------------------------------------------------------------

	SpmVertexBufferObject QuadVbo;  // Simple quad covering the viewport ([-1,-1] to [1,1])
	SpmVertexBufferObject SceneVbo; // Contains the obstacles of the scene

	//---------------------------------------------------------------------------------------------
	//	SHADERS
	//---------------------------------------------------------------------------------------------

	SpmShaderProgram SPM2_ConeShader;
	SpmShaderProgram SPM2_DistanceShader;
	SpmShaderProgram SPM2_SearchShader;
	SpmShaderProgram SPM2_ShadowShader;

	std::vector< SpmShaderProgram* > ShaderList;          // shaders placed here will receive universal updates
	std::vector< SpmShaderProgram* > ShaderOrthoList;     // shaders placed here will receive orthographic projection matrix updates
	std::vector< SpmShaderProgram* > ShaderDimensionList; // shaders placed here will receive buffer dimension updates
};
