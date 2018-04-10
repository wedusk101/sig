/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_dirs.h>
# include <sig/gs_image.h>
# include <sig/gs_model.h>
# include <sig/gs_euler.h>

# include <sigkin/kn_fbx_importer.h>
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_posture.h>
# include <sigkin/kn_motion.h>
# include <sigkin/kn_skin.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Models
//# define GS_USE_TRACE3 // Object types
//# define GS_USE_TRACE4 // Connections
//# define GS_USE_TRACE5 // Animation nodes
//# define GS_USE_TRACE6 // Animation curves
//# define GS_USE_TRACE7 // Joint connections
//# define GS_USE_TRACE8 // Curve connections
//# define GS_USE_TRACE9
# include <sig/gs_trace.h>

//======================================= KnFbxImporter ====================================

KnFbxImporter::KnFbxImporter ()
{
	GS_TRACE1 ( "Constructor" );
}

KnFbxImporter::~KnFbxImporter ()
{
	GS_TRACE1 ( "Destructor" );
}

void KnFbxImporter::init ()
{
	// tables:
	M.init ( 256 );
	A.init ( 256 );
	C.init ( 512 );

	// arrays:
	J.size ( 0 );
}

static const char* modelname ( const GsString& s, bool simplify )
{
	if ( s.len()==0 ) return "-";
	if ( !simplify ) return s.pt();
	const char* pt = s.pt()+s.len()-1;
	while ( pt!=s.pt() ) { pt--; if ( *pt==':' ) return pt+1; }
	return pt;
}

bool KnFbxImporter::load ( const char* filename, const char* options )
{
	GS_TRACE2("Loading ["<<filename<<"]...");
	GsInput fin;
	if ( !fin.open(filename) ) return false;
	init ();

	// defaut options:
	bool simpnames = false;

	// user options:
	if ( !options ) options="s";
	for ( int i=0; options[i]; i++ )
	{	if ( options[i]=='s' ) simpnames=true;
	}

	// parse file:
	GsString linebuf;
	GsInput lin;
	while (true)
	{	fin.readline(linebuf);
		if ( fin.end() ) break;
		lin.init(linebuf);
		lin.get();
		if ( lin.ltype()!=GsInput::String ) continue;

		const GsString& s = lin.ltoken();

		if ( s=="ObjectType" )
		{	fin.skipto ( "}", GsInput::Delimiter ); // skip section to avoid parsing conflicts
		}
		else if ( s=="Model" )
		{	lin.get(); // ':'
			lin.get(); // number
			if ( lin.ltype()==GsInput::Number )
			{	Model* m = M.insert ( lin.ltoken() );
				if ( m )
				{	m->id = lin.ltoken().atol();
					lin.get(); // ','
					lin.get();
					m->name = modelname(lin.ltoken(),simpnames); // name
					lin.get(); // ','
					lin.get();
					m->type = lin.ltoken(); // type: Mesh, LimbNode, Null, Camera, CameraSwitcher
					GS_TRACE2 ( "Model "<<m->id<<" : ["<<m->name<<"] ["<<m->type<<"]..." );
					if ( m->type=="LimbNode" || m->type=="Null" )
					{	J.push () = m; // joints
						read_properties ( fin, m );
					}
				}
			}
		}
		else if ( s=="ObjectType" )
		{	lin.get(); // ':'
			lin.get();
			GS_TRACE3 ( "ObjectType: [" << lin.ltoken() << "]..." );
			//if ( in.get()==GsInput::String )
			//{	gsout<<in.ltoken()<<"\n";
			//}
			
		}
		else if ( s=="Connections" )
		{	lin.get(); // ':'
			lin.get(); // '{'
			GS_TRACE4 ( "Connections..." );
			read_connections ( fin );
		}
		else if ( s=="AnimationCurveNode" )
		{	lin.get(); // ':'
			read_anim ( lin, fin );
		}
		else if ( s=="AnimationCurve" ) // only connected to one CurveNode
		{	lin.get(); // ':'
			read_curve ( lin, fin );
		}
		else if ( s=="end"  ) // this is not a fbx keyword - can be used to force stop
		{	break;
		}
	}

	return true;
}

void KnFbxImporter::read_properties ( GsInput& fin, Model* m )
{
	// advance to properties:
	do { fin.get(); } while ( !fin.end() && fin.ltoken()!="P" );
	if ( fin.end() ) return;

	// read fields
	while (true)
	{	fin.get();
		if ( fin.end() ) break;

		const GsString& s = fin.ltoken();

		if ( s=="Lcl Translation" )
		{	do { fin.get(); } while ( fin.ltype()!=GsInput::Number );
			m->t.x = fin.ltoken().atof();
			fin.get(); // ','
			fin >> m->t.y;
			fin.get(); // ','
			fin >> m->t.z;
		}
		else if ( s=="Lcl Rotation" )
		{	do { fin.get(); } while ( fin.ltype()!=GsInput::Number );
			m->r.x = fin.ltoken().atof();
			fin.get(); // ','
			fin >> m->r.y;
			fin.get(); // ','
			fin >> m->r.z;
			m->r *= gspi/180.0f; // convert to radians
		}
		else if ( s=='}' )
		{	break;
		}
	}
}

void KnFbxImporter::read_anim ( GsInput& lin, GsInput& fin )
{
	lin.get ();
	Anim* a = A.insert ( lin.ltoken() );
	if ( !a ) return;
	a->id = lin.ltoken().atol(); // id

	lin.get(); // ,
	lin.get(); // AnimCurveNode:T/R
	a->type = lin.ltoken().lchar(); // T or R

	// advance to properties:
	fin.skipto ( "P" );
	if ( fin.end() ) return;

	// read fields
	while (true)
	{	fin.get();
		if ( fin.end() ) break;
		const GsString& s = fin.ltoken();
		if ( s=="d|X" )
		{	// no useful property to read
		}
		else if ( fin.ltype()==GsInput::Number )
		{	// values can be found here
		}
		else if ( s[0]=='}' )
		{	break;
		}
	}
	GS_TRACE5 ( "AnimationCurveNode " << a->id << ": type=" << a->type );
}

static void read_numbers ( GsInput& fin, GsArray<double>& A, double mf=1.0 )
{
	while ( !fin.end() )
	{	fin.get();
		if ( fin.ltype()==GsInput::Number )
		{	A.push()=fin.ltoken().atod();
			A.top() *= mf;
		}
		else if ( fin.ltoken()[0]=='}' ) break;
	}
}

void KnFbxImporter::read_curve ( GsInput& lin, GsInput& fin )
{
	lin.get ();
	Curve* c = C.insert ( lin.ltoken() );
	if ( !c ) return;
	c->id = lin.ltoken().atol(); // id

	// read fields
	int level = 0;
	while (true)
	{	fin.get();
		if ( fin.end() ) break;
		const GsString& s = fin.ltoken();

		if ( s=="KeyTime" )
		{	fin.get(); // ':'
			fin.get(); // '*'
			int s = fin.geti();
			//gsout<<s<<gsnl; gsout.pause();
			fin.get(); // '{'
			c->times.reserve(s);
			read_numbers(fin,c->times,1.0E-11);
			//gsout<<c->times<<gsnl; gsout.pause();
		}
		else if ( s=="KeyValueFloat" )
		{	fin.get(); // ':'
			fin.get(); // '*'
			int s = fin.geti();
			fin.get(); // '{'
			c->values.reserve(s);
			read_numbers(fin,c->values);
		}
		else if ( s[0]=='{' )
		{	level++;
		}
		else if ( s[0]=='}' )
		{	if ( --level<0 ) break;
		}
	}
	GS_TRACE6 ( "AnimationCurve " << c->id << ": times=" << c->times.size()<<" values=" << c->values.size() );
}

void KnFbxImporter::read_connections ( GsInput& fin )
{
	char c;
	GsString obj1, obj2;
	GsString code, id1, id2;

	while ( !fin.end() )
	{
		do { fin.get(); c=fin.ltoken()[0]; } while ( c!='}' && c!=';' );
		if ( c=='}' ) break;
		if ( c==';' ) fin>>obj1;

		do { fin.get(); c=fin.ltoken()[0]; } while ( c!='}' && c!=',' );
		if ( c=='}' ) break;
		if ( c==',' ) fin>>obj2;

		do { fin.get(); c=fin.ltoken()[0]; } while ( c!='}' && fin.ltoken()!="C" );
		if ( c=='}' ) break;

		fin.get(); // ':'
		fin >> code; // "OO" or "OP"
		fin.get(); // ','
		fin >> id1;
		fin.get(); // ','
		fin >> id2;

		if ( obj1=="Model" && obj2=="Model" )
		{	Model* m = M.lookup(id1);
			Model* p = M.lookup(id2);
			if ( m && p )
			{	GS_TRACE7 ( p->name << " <- " << m->name );
				m->parent = p;
			}
		}
		else if ( obj1=="AnimCurveNode" && obj2=="Model" )
		{
			//id1: curve node just says: 3 rotations, id2: joint
			// put joint name in curvenode struct
		}
		else if ( obj1=="AnimCurve" && obj2=="AnimCurveNode" )
		{
			//will put curve data (id1) in joint of curvenode (id2)
		}
		else
		{	GS_TRACE4 ( "Skipping connection "<<obj1<<" - "<<obj2<<"..." );
		}
		//Not parsing layer information:
		//else if ( obj1=="AnimCurveNode" && obj2=="AnimLayer" )
		//{
		//}
	}
}

void KnFbxImporter::add_children ( KnSkeleton* sk, Model* pm, KnJoint* pj )
{
	int i, s=J.size();
	GsQuat q;
	for ( i=0; i<s; i++ )
	{	Model* m=J[i];
		if ( pm==m->parent )
		{	KnJoint* j = sk->add_joint ( KnJoint::TypeQuat, pj, m->name );
			m->j = j;
			if ( !pm ) // this is the root node
			{	j->pos()->thaw();
				j->pos()->value ( m->t.x, m->t.y, m->t.z );
			}
			else
			{	j->offset ( m->t );
			}

			j->rot()->thaw();
			gs_rot ( gsXYZ, q, m->r.x, m->r.y, m->r.z );
			j->rot()->setmode ( KnJointRot::FullMode );
			j->rot ()->value ( q );
			//j->rot()->prerot(q);
			//j->rot ()->value( GsQuat() );
			add_children ( sk, m, j );
			if ( !pm ) return; // there is only one root node
		}
	}
}

bool KnFbxImporter::get_skeleton ( KnSkeleton *sk )
{
	sk->init ();

	// here we could search for parent nodes in the joint
	// list already in the skeleton as it seems connections
	// come ordered by hierarchy; but to be generic we will
	// do a complete recursive search:
	add_children ( sk, 0, 0 );

	return true;
}

bool KnFbxImporter::get_motion ( KnSkeleton *sk, KnMotion* m )
{
	if ( C.elements()==0 ) return false;

	//check if ( c->values.size()!=K.size() )
	//{	gsout.warning("Animation curve values resized to match number of keytimes!");
	//}
	return true;
}

//================================ EOF =================================================
