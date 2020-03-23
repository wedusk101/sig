/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_TEXTURE_H
# define GL_TEXTURE_H

# include <sig/gs_image.h>
# include <sigogl/gl_types.h>

class GlResources;
class GlTextureDecl;

//====================== GlTexture =====================

class GlTexture
{  public :
	enum Settings { Nearest, Linear, NearestMipMapped, LinearMipMapped, NearestMipMapLinear, LinearMipMapLinear };
	GLuint id;
	gsuint16 width, height;
   private : // resource management information
	GlTextureDecl* _decl;
	friend GlResources;
   public :
	GlTexture (); // OGL id starts as 0
   ~GlTexture (); // delete OGL id if it is >0
	void init (); // initialize and delete OGL id if it is >0
	bool valid () const { return id>0; }
	void data ( const GsImage* img, Settings s=LinearMipMapped );
	void data ( const GsBytemap* bmp, Settings s=LinearMipMapped );
	void data ( const float* img, int w, int h, Settings s=LinearMipMapped ); // img has to have rgba data
};

//================================= End of File ===============================

# endif // GL_TEXTURE_H
