/*=======================================================================
   Copyright (c) 2020 Renato Farias.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

#include <sigspm/spm_shader.h>
#include <iostream>
#include <fstream>

bool SpmShaderSrc::CanLoad () const
{
	if ( !folder || !folder[0] ) return false;
	std::string file;
	file = folder;
	file.append ( fname );
	std::ifstream srcFile ( file );
	return srcFile.is_open();
}

/*=================================================================================================
  CONSTRUCTORS
=================================================================================================*/

SpmShader::SpmShader()
{
	ID = 0;
	Type = GL_INVALID_ENUM;
	File = "";
	PreDefSrc = 0;
	PreDef = true;
}

/*=================================================================================================
  DESTRUCTOR
=================================================================================================*/

SpmShader::~SpmShader()
{
	Delete();
}

/*=================================================================================================
  CREATE
  3/20/2020 M. Kallmann integration notes: modified to manage pre-defined shaders.
=================================================================================================*/

void SpmShader::Create( const SpmShaderSrc& shaderSrc, GLenum shaderType )
{
	ID = glCreateShader( shaderType );
	//CheckGlErrors( __FILE__, __LINE__ );

	Type = shaderType;
	PreDefSrc = shaderSrc.predef;

	if ( !shaderSrc.folder || !shaderSrc.folder[0] )
	{	File = shaderSrc.fname;
		PreDef = true;
	}
	else
	{	File = shaderSrc.folder;
		File.append ( shaderSrc.fname );
		PreDef = false;
	}

	Load();
}

/*=================================================================================================
  DELETE
=================================================================================================*/

void SpmShader::Delete( void )
{
	glDeleteShader( ID );
	//CheckGlErrors( __FILE__, __LINE__ );

	ID = 0;
	Type = GL_INVALID_ENUM;
	File = "";
	PreDefSrc = 0;
}

/*=================================================================================================
  LOAD
  3/20/2020 M. Kallmann integration notes: modified to manage pre-defined shaders.
=================================================================================================*/

void SpmShader::Load( void )
{
	if( ID == 0 )
		return;

	std::string shaderSrc, line;
	const char* srcpt=0;
	// gs_show_console(); // need to call this to show messages in console (or convert error message to gsout)

	if ( !PreDef && File.length()>0 )
	{	std::ifstream srcFile( File );
		std::string line;
		line.reserve(160);
		shaderSrc.reserve(1024);
		// std::cerr << "trying: " << File << std::endl;
		if( srcFile.is_open() == true )
		{
			while( std::getline( srcFile, line ) )
			{
				shaderSrc += line;
				shaderSrc += '\n';
			}
			srcFile.close();

			srcpt = shaderSrc.c_str();
		}
		else
		{	PreDef = true;
		}
	}

	if ( srcpt )
	{	// std::cerr << "loaded: " << File << std::endl;
		PreDefSrc=0;
	}
	else
	{ 	// std::cerr << "using predefined version of: " << File << std::endl;
		srcpt=PreDefSrc;
	}

	glShaderSource( ID, 1, &srcpt, NULL );
	//CheckGlErrors( __FILE__, __LINE__ );

	glCompileShader( ID );
	//CheckGlErrors( __FILE__, __LINE__ );

	// If the shader didn't compile successfully, print log
	if( GetCompileStatus() == 0 )
	{	std::cerr << File << "("<<(PreDefSrc?"predef":"loaded")<<")"<< std::endl << GetInfoLog() << std::endl;
		exit (1);
	}
}

/*=================================================================================================
  GET STATUS
=================================================================================================*/

//-1: invalid/uninitialized shader
int SpmShader::GetStatus( GLenum en ) const
{
	if( ID == 0 )
		return -1;

	GLint status;
	glGetShaderiv( ID, en, &status );
	//CheckGlErrors( __FILE__, __LINE__ );

	return status == GL_TRUE ? 1 : 0;
}

// 0: shader is currently not flagged for deletion
//+1: shader is flagged for deletion
int SpmShader::GetDeleteStatus( void ) const {
	return GetStatus( GL_DELETE_STATUS );
}

// 0: shader did not compile successfully
//+1: shader compiled without errors
int SpmShader::GetCompileStatus( void ) const {
	return GetStatus( GL_COMPILE_STATUS );
}

/*=================================================================================================
  GET INFO LOG
=================================================================================================*/

std::string SpmShader::GetInfoLog( void ) const
{
	if( ID == 0 )
		return "";

	GLint logLength = 0;
	GLsizei charsWritten = 0;
	GLchar* infoLog;
	std::string stringLog = "";

	glGetShaderiv( ID, GL_INFO_LOG_LENGTH, &logLength );
	//CheckGlErrors( __FILE__, __LINE__ );

	if( logLength > 0 )
	{
		infoLog = (GLchar*)malloc( sizeof(GLchar) * logLength );
		glGetShaderInfoLog( ID, (GLsizei)logLength, &charsWritten, infoLog );
		//CheckGlErrors( __FILE__, __LINE__ );
		stringLog = infoLog;
		free( infoLog );
	}

	return stringLog;
}

/*=================================================================================================
  GET SOURCE (from GPU)
=================================================================================================*/

std::string SpmShader::GetSource( void ) const
{
	if( ID == 0 )
		return "";

	GLint bufSize = 0;
	GLsizei charsWritten = 0;
	GLchar* src;
	std::string stringSrc = "";

	glGetShaderiv( ID, GL_SHADER_SOURCE_LENGTH, &bufSize );
	//CheckGlErrors( __FILE__, __LINE__ );

	if( bufSize > 0 )
	{
		src = (GLchar*)malloc( sizeof(GLchar) * bufSize );
		glGetShaderSource( ID, (GLsizei)bufSize, &charsWritten, src );
		//CheckGlErrors( __FILE__, __LINE__ );
		stringSrc = src;
		free( src );
	}

	return stringSrc;
}
