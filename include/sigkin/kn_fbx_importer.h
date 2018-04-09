/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_FBX_IMPORTER_H
# define KN_FBX_IMPORTER_H

# include <sig/gs_vars.h>
# include <sig/gs_table.h>
# include <sig/gs_string.h>

class KnSkeleton;
class KnMotion;
class KnJoint;
class KnSkin;
class KnFbxImporter;

//================================ KnFbxImporter ===================================

/*! A fbx loader */
class KnFbxImporter : public GsShareable
{  protected :
	struct Anim;
	struct Model
	{	gsuint id; GsString name, type; GsVec t, r; Model* parent; Anim* anim; KnJoint* j;
		Model() { id=0; parent=0; anim=0; j=0; }
	};
	GsTablePt<Model> M; // all parsed models
	GsArray<Model*> J;  // models representing joints

	struct Curve
	{	gsuint id; GsArray<double> times; GsArray<double> values; 
		Curve() { id=0; }
	};
	struct Anim
	{	gsuint id; char type; gsuint modelid; Curve *cx, *cy, *cz;
		Anim() { id=0; type='R'; modelid=0; cx=cy=cz=0; }
	};
	GsTablePt<Anim> A; // all parsed animation nodes
	GsTablePt<Curve> C; // all parsed animation curves

   public :

	/*! Constructor */
	KnFbxImporter ();

	/*! Destructor is public but pay attention to the use of ref()/unref() */
	virtual ~KnFbxImporter ();

	/*! Clears all internal data */
	void init ();

	/*! loads a .fbx file in ASCII format, returns true if data was loaded and false if error.
		Parameter options is a string where each letter, in any order, defines:
		s: simplify names by removing any namespaces
		If a null string is given, the default option string considered is: "s". */
	bool load ( const char* filename, const char* options=0 );

	bool get_skeleton ( KnSkeleton *sk );
	int num_motions () { return 0; }
	bool get_motion ( KnSkeleton *sk, KnMotion* m );

   protected :
	void read_properties ( GsInput& fin, Model* m );
	void read_anim ( GsInput& lin, GsInput& fin );
	void read_curve ( GsInput& lin, GsInput& fin );
	void read_connections ( GsInput& fin );
	void add_children ( KnSkeleton* sk, Model* pm, KnJoint* pj );
};

//================================ End of File =================================================

# endif  // KN_FBX_IMPORTER_H
