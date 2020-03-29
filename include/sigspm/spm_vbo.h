/*=======================================================================
   Copyright (c) 2020 Renato Farias.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

#pragma once

#include <sigogl/glcorearb.h>
#include <sigogl/glcorearb_functions.h>
#include <set>

class SpmVertexBufferObject
{
public:
	SpmVertexBufferObject();
   ~SpmVertexBufferObject();

	// Prevent copying
	SpmVertexBufferObject( const SpmVertexBufferObject& ) = delete;
	SpmVertexBufferObject& operator=( const SpmVertexBufferObject& ) = delete;

public:
	void Draw() const;
	void DrawElements() const;
	void DrawRangeElements( GLuint start, GLsizei count ) const;
	void DrawRangeElements( GLuint start, GLuint end, GLsizei count ) const;
	void Bind() const;
	void Unbind() const;

	void BindDrawUnbind() const;
	void BindDrawElementsUnbind() const;
	void BindDrawRangeElementsUnbind( GLuint start, GLsizei count ) const;
	void BindDrawRangeElementsUnbind( GLuint start, GLuint end, GLsizei count ) const;

	void GenVertexArray();
	void GenVertexBuffer( GLsizeiptr size, const GLvoid* data, GLenum usage = GL_STATIC_DRAW );
	void GenIndexBuffer ( GLsizeiptr size, const GLvoid* data, GLenum usage = GL_STATIC_DRAW );

	void Delete();
	void DeleteVertexBuffer();
	void DeleteIndexBuffer();

	void SetAttribPointer( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer = 0 );
	void SetDrawMode( GLenum );
	void SetIndexType( GLenum );

private:
	GLuint VertexArrayID;
	GLuint VertexBufferID;
	GLuint IndexBufferID;

	std::set<GLuint> AttribIndices;

	GLsizei VerticesSize;
	GLsizei IndicesSize;

	GLenum DrawMode;
	GLenum IndexType;
};
