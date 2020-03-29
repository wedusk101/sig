/*=======================================================================
   Copyright (c) 2020 Renato Farias.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

#pragma once

#include <sigogl/glcorearb.h>
#include <sigogl/glcorearb_functions.h>
#include <string>

struct SpmShaderSrc // M. Kallmann integration notes: new class to manage pre-defined shaders
{	const char* folder;
	const char* fname;
	const char* predef;
	SpmShaderSrc ( const char* f, const char* fn, const char* pd ) { folder=f; fname=fn; predef=pd; }
	bool CanLoad () const;
};

class SpmShader
{
public:
	SpmShader();
   ~SpmShader();

public:
	void Create( const SpmShaderSrc& shaderSrc, GLenum shaderType );
	void Delete();
	void Load();

public:
	int GetStatus( GLenum ) const;
	int GetDeleteStatus() const;
	int GetCompileStatus() const;

	std::string GetInfoLog() const;
	std::string GetSource() const;

	GLuint      GetID()   const { return ID;   }
	GLenum      GetType() const { return Type; }
	std::string GetFile() const { return File; }

private:
	GLuint ID;
	GLenum Type;
	std::string File;
	const char* PreDefSrc;
	bool PreDef;
};
