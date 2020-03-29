/*=======================================================================
   Copyright (c) 2020 Renato Farias.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

#include <sigspm/spm_shader_program.h>
#include <iostream>

/*=================================================================================================
  CONSTRUCTORS
=================================================================================================*/

SpmShaderProgram::SpmShaderProgram()
{
	ID = 0;
}

/*=================================================================================================
  DESTRUCTOR
=================================================================================================*/

SpmShaderProgram::~SpmShaderProgram()
{
	Delete();
}

/*=================================================================================================
  CREATE
=================================================================================================*/

void SpmShaderProgram::Create( const SpmShaderSrc& csh )
{
	ID = glCreateProgram();
	//CheckGlErrors( __FILE__, __LINE__ );

	if( ID != 0 )
	{
		computeShader.Create( csh, GL_COMPUTE_SHADER );
		glAttachShader( ID, computeShader.GetID() );
		//CheckGlErrors( __FILE__, __LINE__ );

		Link();
	}
}

void SpmShaderProgram::Create( const SpmShaderSrc& vsh, const SpmShaderSrc& fsh )
{
	ID = glCreateProgram();
	//CheckGlErrors( __FILE__, __LINE__ );

	if( ID != 0 )
	{
		vertexShader.Create( vsh, GL_VERTEX_SHADER );
		glAttachShader( ID, vertexShader.GetID() );
		//CheckGlErrors( __FILE__, __LINE__ );

		fragmentShader.Create( fsh, GL_FRAGMENT_SHADER );
		glAttachShader( ID, fragmentShader.GetID() );
		//CheckGlErrors( __FILE__, __LINE__ );

		Link();
	}
}

void SpmShaderProgram::Create( const SpmShaderSrc& vsh, const SpmShaderSrc& gsh, const SpmShaderSrc& fsh )
{
	ID = glCreateProgram();
	//CheckGlErrors( __FILE__, __LINE__ );

	if( ID != 0 )
	{
		vertexShader.Create( vsh, GL_VERTEX_SHADER );
		glAttachShader( ID, vertexShader.GetID() );
		//CheckGlErrors( __FILE__, __LINE__ );

		geometryShader.Create( gsh, GL_GEOMETRY_SHADER );
		glAttachShader( ID, geometryShader.GetID() );
		//CheckGlErrors( __FILE__, __LINE__ );

		fragmentShader.Create( fsh, GL_FRAGMENT_SHADER );
		glAttachShader( ID, fragmentShader.GetID() );
		//CheckGlErrors( __FILE__, __LINE__ );

		Link();
	}
}

/*=================================================================================================
  DELETE
=================================================================================================*/

void SpmShaderProgram::Delete( void )
{
	if( ID != 0 )
	{
		glDetachShader( ID, vertexShader.GetID() );
		//CheckGlErrors( __FILE__, __LINE__ );

		glDetachShader( ID, geometryShader.GetID() );
		//CheckGlErrors( __FILE__, __LINE__ );

		glDetachShader( ID, fragmentShader.GetID() );
		//CheckGlErrors( __FILE__, __LINE__ );

		glDetachShader( ID, computeShader.GetID() );
		//CheckGlErrors( __FILE__, __LINE__ );

		glDeleteProgram( ID );
		//CheckGlErrors( __FILE__, __LINE__ );

		ID = 0;
	}
}

/*=================================================================================================
  LINK
=================================================================================================*/

void SpmShaderProgram::Link( void )
{
	glLinkProgram( ID );
	//CheckGlErrors( __FILE__, __LINE__ );

	// If the program didn't link successfully, print log
	if( GetLinkStatus() == 0 )
		std::cerr << "shader program " << ID << " link log" << std::endl << GetInfoLog() << std::endl;
}

/*=================================================================================================
  VALIDATE
=================================================================================================*/

void SpmShaderProgram::Validate( void )
{
	glValidateProgram( ID );
	//CheckGlErrors( __FILE__, __LINE__ );

	// If the program didn't validate successfully, print log
	if( GetValidateStatus() == 0 )
		std::cerr << "shader program " << ID << " validate log" << std::endl << GetInfoLog() << std::endl;
}

/*=================================================================================================
  RELOAD
=================================================================================================*/

void SpmShaderProgram::Reload( void )
{
	vertexShader.Load();
	geometryShader.Load();
	fragmentShader.Load();
	computeShader.Load();
	Link();
}

/*=================================================================================================
  USE
=================================================================================================*/

void SpmShaderProgram::Use( void )
{
	glUseProgram( ID );
}

/*=================================================================================================
  GET STATUS
=================================================================================================*/

//-1: invalid/uninitialized program
int SpmShaderProgram::GetStatus( GLenum en ) const
{
	if( ID == 0 )
		return -1;

	GLint status;
	glGetProgramiv( ID, en, &status );
	//CheckGlErrors( __FILE__, __LINE__ );

	return status == GL_TRUE ? 1 : 0;
}

// 0: program is currently not flagged for deletion
// 1: program is flagged for deletion
int SpmShaderProgram::GetDeleteStatus( void ) const {
	return GetStatus( GL_DELETE_STATUS );
}

// 0: program did not link successfully
// 1: program linked without errors
int SpmShaderProgram::GetLinkStatus( void ) const {
	return GetStatus( GL_LINK_STATUS );
}

// 0: program did not validate
// 1: program validated
int SpmShaderProgram::GetValidateStatus( void ) const {
	return GetStatus( GL_VALIDATE_STATUS );
}

/*=================================================================================================
  GET NUMBER
=================================================================================================*/

//-1: invalid/uninitialized program
int SpmShaderProgram::GetNumber( GLenum en ) const
{
	if( ID == 0 )
		return -1;

	GLint num;
	glGetProgramiv( ID, en, &num );
	//CheckGlErrors( __FILE__, __LINE__ );

	return num;
}

int SpmShaderProgram::GetNumAttachedShaders( void ) const {
	return GetNumber( GL_ATTACHED_SHADERS );
}

int SpmShaderProgram::GetNumActiveAttributes( void ) const {
	return GetNumber( GL_ACTIVE_ATTRIBUTES );
}

int SpmShaderProgram::GetNumActiveUniforms( void ) const {
	return GetNumber( GL_ACTIVE_UNIFORMS );
}

int SpmShaderProgram::GetActiveAttributeMaxLength( void ) const {
	return GetNumber( GL_ACTIVE_ATTRIBUTE_MAX_LENGTH );
}

int SpmShaderProgram::GetActiveUniformMaxLength( void ) const {
	return GetNumber( GL_ACTIVE_UNIFORM_MAX_LENGTH );
}

/*=================================================================================================
  GET INFO LOG
=================================================================================================*/

std::string SpmShaderProgram::GetInfoLog( void ) const
{
	if( ID == 0 )
		return "";

	GLint logLength = 0;
	GLsizei charsWritten = 0;
	GLchar* infoLog;
	std::string stringLog = "";

	glGetProgramiv( ID, GL_INFO_LOG_LENGTH, &logLength );
	//CheckGlErrors( __FILE__, __LINE__ );

	if( logLength > 0 )
	{
		infoLog = (GLchar*)malloc( sizeof(GLchar) * logLength );
		glGetProgramInfoLog( ID, (GLsizei)logLength, &charsWritten, infoLog );
		//CheckGlErrors( __FILE__, __LINE__ );
		stringLog = infoLog;
		free( infoLog );
	}

	return stringLog;
}

/*=================================================================================================
  UNIFORM SETTERS
=================================================================================================*/

//Setting uniforms by value:
//Unsigned Integer & Location:
void SpmShaderProgram::SetUniform( GLint location, GLuint a, GLuint b, GLuint c, GLuint d ) {
	glUniform4ui( location, a, b, c, d );
}
void SpmShaderProgram::SetUniform( GLint location, GLuint a, GLuint b, GLuint c ) {
	glUniform3ui( location, a, b, c );
}
void SpmShaderProgram::SetUniform( GLint location, GLuint a, GLuint b ) {
	glUniform2ui( location, a, b );
}
void SpmShaderProgram::SetUniform( GLint location, GLuint a ) {
	glUniform1ui( location, a );
}
//Unsigned Integer & Name:
void SpmShaderProgram::SetUniform( const GLchar* name, GLuint a, GLuint b, GLuint c, GLuint d ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b, c, d );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLuint a, GLuint b, GLuint c ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b, c );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLuint a, GLuint b ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLuint a ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a );
}

//Integer & Location:
void SpmShaderProgram::SetUniform( GLint location, GLint a, GLint b, GLint c, GLint d ) {
	glUniform4i( location, a, b, c, d );
}
void SpmShaderProgram::SetUniform( GLint location, GLint a, GLint b, GLint c ) {
	glUniform3i( location, a, b, c );
}
void SpmShaderProgram::SetUniform( GLint location, GLint a, GLint b ) {
	glUniform2i( location, a, b );
}
void SpmShaderProgram::SetUniform( GLint location, GLint a ) {
	glUniform1i( location, a );
}
//Integer & Name:
void SpmShaderProgram::SetUniform( const GLchar* name, GLint a, GLint b, GLint c, GLint d ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b, c, d );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLint a, GLint b, GLint c ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b, c );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLint a, GLint b ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLint a ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a );
}

//Float & Location:
void SpmShaderProgram::SetUniform( GLint location, GLfloat a, GLfloat b, GLfloat c, GLfloat d ) {
	glUniform4f( location, a, b, c, d );
}
void SpmShaderProgram::SetUniform( GLint location, GLfloat a, GLfloat b, GLfloat c ) {
	glUniform3f( location, a, b, c );
}
void SpmShaderProgram::SetUniform( GLint location, GLfloat a, GLfloat b ) {
	glUniform2f( location, a, b );
}
void SpmShaderProgram::SetUniform( GLint location, GLfloat a ) {
	glUniform1f( location, a );
}
//Float & Name:
void SpmShaderProgram::SetUniform( const GLchar* name, GLfloat a, GLfloat b, GLfloat c, GLfloat d ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b, c, d );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLfloat a, GLfloat b, GLfloat c ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b, c );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLfloat a, GLfloat b ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLfloat a ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a );
}

//Double & Location:
void SpmShaderProgram::SetUniform( GLint location, GLdouble a, GLdouble b, GLdouble c, GLdouble d ) {
	glUniform4f( location, (GLfloat)a, (GLfloat)b, (GLfloat)c, (GLfloat)d );
}
void SpmShaderProgram::SetUniform( GLint location, GLdouble a, GLdouble b, GLdouble c ) {
	glUniform3f( location, (GLfloat)a, (GLfloat)b, (GLfloat)c );
}
void SpmShaderProgram::SetUniform( GLint location, GLdouble a, GLdouble b ) {
	glUniform2f( location, (GLfloat)a, (GLfloat)b );
}
void SpmShaderProgram::SetUniform( GLint location, GLdouble a ) {
	glUniform1f( location, (GLfloat)a );
}
//Double & name:
void SpmShaderProgram::SetUniform( const GLchar* name, GLdouble a, GLdouble b, GLdouble c, GLdouble d ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b, c, d );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLdouble a, GLdouble b, GLdouble c ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b, c );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLdouble a, GLdouble b ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a, b );
}
void SpmShaderProgram::SetUniform( const GLchar* name, GLdouble a ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, a );
}

//Setting Uniforms by vector:
//Location & Integer:
void SpmShaderProgram::SetUniform( GLint location, const GLint* v, GLuint nvalues, GLsizei count ) {
	switch( nvalues ) {
		case 1: glUniform1iv( location, count, v ); break;
		case 2: glUniform2iv( location, count, v ); break;
		case 3: glUniform3iv( location, count, v ); break;
		case 4: glUniform4iv( location, count, v ); break;
	}
}
//Location & Float:
void SpmShaderProgram::SetUniform( GLint location, const GLfloat* v, GLuint nvalues, GLsizei count ) {
	switch( nvalues ) {
		case 1: glUniform1fv( location, count, v ); break;
		case 2: glUniform2fv( location, count, v ); break;
		case 3: glUniform3fv( location, count, v ); break;
		case 4: glUniform4fv( location, count, v ); break;
	}
}

//Name & Integer:
void SpmShaderProgram::SetUniform( const GLchar* name, const GLint* v, GLuint nvalues, GLsizei count ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, v, nvalues, count );
}
//Name & Float:
void SpmShaderProgram::SetUniform( const GLchar* name, const GLfloat* v, GLuint nvalues, GLsizei count ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, v, nvalues, count );
}

//Setting uniform matrix value:
//Location:
void SpmShaderProgram::SetUniform( GLint location, const GLfloat* m, GLuint dim, GLboolean transpose, GLsizei count ) {
	switch( dim ) {
		case 2: glUniformMatrix2fv( location, count, transpose, m ); break;
		case 3: glUniformMatrix3fv( location, count, transpose, m ); break;
		case 4: glUniformMatrix4fv( location, count, transpose, m ); break;
	}
}
//Name:
void SpmShaderProgram::SetUniform( const GLchar* name, const GLfloat* m, GLuint dim, GLboolean transpose, GLsizei count ) {
	GLint location = getUniformLocation( name );
	SetUniform( location, m, dim, transpose, count );
}
