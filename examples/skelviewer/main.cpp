/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include "skelviewer.h"

# include <sigkin/kn_skin.h>
# include <sigkin/kn_fbx_importer.h>
# include <sigogl/ui_dialogs.h>
# include <sigogl/ws_dialog.h>
# include <sigogl/ws_run.h>

static SnGroup* get_skin_models ( KnFbxImporter& fbximp );

int main ( int argc, char **argv )
{
	const char* skfile=0;
	const char* skmotion=0;

	// Choose initial parameters:
	//skfile = "../data/characters/tubeguy.s";
	//skmotion = "../data/characters/walkfwd.sm";
	bool viewskel = false;
	bool viewfloor = false;
	float floory = 0;

	// Select skeleton file from disk if not specified above:
	if ( !skfile ) skfile = ui_select_file ( "Select skeleton file to load:", "../data/characters/", "*.s;*.bvh;*.fbx" );
	if ( !skfile ) return 1;

	// Create and load skeleton:
	KnSkeleton* sk = new KnSkeleton;
	SnGroup* g=0;
	if ( gs_compare(gs_extension(skfile),"fbx")==0 ) // import fbx
	{	KnFbxImporter fbximp;
		if ( !fbximp.load(skfile) ) gsout.fatal("could not load %s",skfile);
		fbximp.get_skeleton(sk);
		// g = get_skin_models(fbximp); // use this to inspect skin models
	}
	else
	{	if ( !sk->load(skfile) ) gsout.fatal("could not load %s",skfile);
	}

	// Create viewer:
	MySkelViewer* viewer = new MySkelViewer(sk,-1,-1,800,600,"SIG Skelviewer",viewskel,viewfloor,floory);
	if ( g ) viewer->rootg()->add ( g );

	// Motions and other settings have to be added after viewer is created:
	if ( skmotion ) viewer->load_motion ( skmotion, 0 );

	// Finalize and run:
	viewer->view_all ();
	viewer->show();
	ws_run();

	return 1;
}

static SnGroup* get_skin_models ( KnFbxImporter& fbximp )
{
	const GsArraySh<GsModel>& m = fbximp.models();
	if ( m.empty() ) return 0;
	SnGroup* g = new SnGroup;
	for ( int i=0; i<m.size(); i++ )
	{	g->add ( new SnModel ( m[i] ) );
	}
	return g;
}
