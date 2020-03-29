/*=======================================================================
   Copyright (c) 2020 Renato Farias.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

#include <sigspm/spm_vbo.h>

/*=================================================================================================
  CONSTRUCTORS
=================================================================================================*/

SpmVertexBufferObject::SpmVertexBufferObject()
{
	VertexArrayID  = 0;
	VertexBufferID = 0;
	IndexBufferID  = 0;

	VerticesSize = 0;
	IndicesSize  = 0;

	DrawMode  = GL_POINTS;
	IndexType = GL_UNSIGNED_INT;
}

/*=================================================================================================
  DESTRUCTOR
=================================================================================================*/

SpmVertexBufferObject::~SpmVertexBufferObject()
{
	Delete();
}

/*=================================================================================================
  USAGE
=================================================================================================*/

void SpmVertexBufferObject::Draw( void ) const
{
	glDrawArrays( DrawMode, 0, IndicesSize );
}

void SpmVertexBufferObject::DrawElements( void ) const
{
	glDrawElements( DrawMode, IndicesSize, IndexType, NULL );
}

void SpmVertexBufferObject::DrawRangeElements( GLuint start, GLsizei count ) const
{
	glDrawRangeElements( DrawMode, start, IndicesSize, count, IndexType, NULL );
}

void SpmVertexBufferObject::DrawRangeElements( GLuint start, GLuint end, GLsizei count ) const
{
	glDrawRangeElements( DrawMode, start, end, count, IndexType, NULL );
}

void SpmVertexBufferObject::Bind( void ) const
{
	glBindVertexArray( VertexArrayID );
	glBindBuffer( GL_ARRAY_BUFFER, VertexBufferID );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IndexBufferID );
}

void SpmVertexBufferObject::Unbind( void ) const
{
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

void SpmVertexBufferObject::BindDrawUnbind() const
{
	Bind();
	Draw();
	Unbind();
}

void SpmVertexBufferObject::BindDrawElementsUnbind() const
{
	Bind();
	DrawElements();
	Unbind();
}

void SpmVertexBufferObject::BindDrawRangeElementsUnbind( GLuint start, GLsizei count ) const
{
	Bind();
	DrawRangeElements( start, count );
	Unbind();
}

void SpmVertexBufferObject::BindDrawRangeElementsUnbind( GLuint start, GLuint end, GLsizei count ) const
{
	Bind();
	DrawRangeElements( start, end, count );
	Unbind();
}

/*=================================================================================================
  CREATION
=================================================================================================*/

void SpmVertexBufferObject::GenVertexArray( void )
{
	if( VertexArrayID != 0 )
		return;

	glGenVertexArrays( 1, &VertexArrayID );
}

void SpmVertexBufferObject::GenVertexBuffer( GLsizeiptr size, const GLvoid* data, GLenum usage )
{
	if( VertexArrayID == 0 )
		GenVertexArray();

	glBindVertexArray( VertexArrayID );

	if( VertexBufferID == 0 )
		glGenBuffers( 1, &VertexBufferID );

	glBindBuffer( GL_ARRAY_BUFFER, VertexBufferID );
	glBufferData( GL_ARRAY_BUFFER, size, data, usage );

	VerticesSize = size;
}

void SpmVertexBufferObject::GenIndexBuffer( GLsizeiptr size, const GLvoid* data, GLenum usage )
{
	if( VertexArrayID == 0 )
		GenVertexArray();

	glBindVertexArray( VertexArrayID );

	if( IndexBufferID == 0 )
		glGenBuffers( 1, &IndexBufferID );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IndexBufferID );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, size, data, usage );

	IndicesSize = size;
}

/*=================================================================================================
  DELETION
=================================================================================================*/

void SpmVertexBufferObject::Delete( void )
{
	DeleteVertexBuffer();
	DeleteIndexBuffer();

	glBindVertexArray( 0 );
	glDeleteVertexArrays( 1, &VertexArrayID );

	VertexArrayID = 0;
}

void SpmVertexBufferObject::DeleteVertexBuffer( void )
{
	for( auto i : AttribIndices )
		glDisableVertexAttribArray( i );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glDeleteBuffers( 1, &VertexBufferID );

	VertexBufferID = 0;
	VerticesSize = 0;
}

void SpmVertexBufferObject::DeleteIndexBuffer( void )
{
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glDeleteBuffers( 1, &IndexBufferID );

	IndexBufferID = 0;
	IndicesSize = 0;
}

/*=================================================================================================
  SETTERS
=================================================================================================*/

void SpmVertexBufferObject::SetAttribPointer( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer )
{
	glVertexAttribPointer( index, size, type, normalized, stride, pointer );
	glEnableVertexAttribArray( index );
	AttribIndices.insert( index );
}

void SpmVertexBufferObject::SetDrawMode( GLenum mode )
{
	DrawMode = mode;
}

void SpmVertexBufferObject::SetIndexType( GLenum type )
{
	IndexType = type;
}
