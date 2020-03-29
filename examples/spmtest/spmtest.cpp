/*=======================================================================
   Copyright (c) 2020 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_vars.h>
# include <sig/sn_poly_editor.h>
# include <sig/sn_lines.h>
# include <sigogl/gl_texture.h>
# include <sigogl/gl_resources.h>
# include <sigogl/ui_radio_button.h>
# include <sigogl/ws_viewer.h>
# include <sigogl/ws_run.h>

# include <sigspm/spm_manager.h>

class SpmViewer : public WsViewer
{  public: // ui:
	enum MenuEv { EvBuild, EvAutoBuild, EvMode, EvExit };
	UiRadioButton *_obstbut, *_sinkbut;
	UiCheckButton *_abbut;
   public: // scene:
	SnPolygons *_domain;
	SnPolyEditor *_sinks;
	SnPolyEditor *_obstacles;
	SnPlanarObjects* SpmPlane;
	SnLines *_path;
   public: // spm data:
	GlTexture SpmTexture;
	ShortestPathMap* Spm;
	ShortestPathMapManager SpmManager;
	std::vector<GsVec> SpmPath;
   public:
	SpmViewer ( int x, int y, int w, int h, const char* l );
	void refresh () { render(); ws_fast_check(); }
	void build ();
	void get_path ( float x, float y );
	virtual int uievent ( int e ) override;
	virtual int handle_scene_event ( const GsEvent& e ) override;
};

static void polyeditor_callback ( SnPolyEditor* pe, SnPolyEditor::Event e, int pid )
{
	SpmViewer* v = (SpmViewer*)pe->userdata();
	if ( !v->_abbut->value() ) return;
	if ( e==SnPolyEditor::PostMovement || e==SnPolyEditor::PostEdition || 
		 e==SnPolyEditor::PostInsertion || e==SnPolyEditor::PostRemoval )
	{	v->build();
		if ( !v->_path->empty() ) v->get_path( v->SpmPath[0].x, v->SpmPath[0].y );
	}
}

SpmViewer::SpmViewer ( int x, int y, int w, int h, const char* l ) : WsViewer( x, y, w, h, l )
{
	// Define domain boundaries:
	const float minx=-12, miny=-12, maxx=12, maxy=12;

	// Define a plane to display the spm as a texture:
	SnGroup* g = rootg();
	g->add ( SpmPlane = new SnPlanarObjects ); // will initialize plane after 1st spm is built
	GsRect rect ( minx, miny, maxx-minx, maxy-miny );
	SpmPlane->zcoordinate = -0.01f;
	SpmPlane->start_group ( SnPlanarObjects::Textured, 0 ); // texture id will be set after ogl is initialized
	SpmPlane->push_rect ( rect, GsColor::gray );
	SpmPlane->setT ( 0, GsPnt2( 0, 0 ), GsPnt2( 1, 0 ), GsPnt2( 1, 1 ), GsPnt2( 0, 1 ) );

	// Define the non-editable domain polygon:
	g->add ( _domain = new SnPolygons );
	_domain->push().rectangle ( minx, miny, maxx, maxy );
	_domain->draw_mode ( 0, 0 );
	_domain->ecolor ( GsColor::black );

	// Define an editable initial sink segment:
	g->add ( _sinks = new SnPolyEditor );
	_sinks->polygons()->push().setpoly ( "5 -5 5 5", true ); // the SPM sink
	_sinks->solid_drawing ( 0 );
	_sinks->polyline_mode ( true );
	_sinks->min_polygons ( 1 ); // need to always have a source
	_sinks->set_limits ( minx, maxx, miny, maxy );
	_sinks->mode ( SnPolyEditor::ModeNoEdition );
	_sinks->callback ( polyeditor_callback, this );

	// Define an editable obstacle:
	g->add ( _obstacles = new SnPolyEditor );
	_obstacles->solid_drawing ( 0 );
	_obstacles->set_limits ( minx, maxx, miny, maxy );
	_obstacles->callback ( polyeditor_callback, this );
	_obstacles->polygons()->push().setpoly ( "-2 -5 0 0 -2 5" ); // define one obstacle

	// Define scene node to show a path:
	g->add ( _path = new SnLines );

	// Initiaze Spm objects:
	Spm = 0;
	SpmManager.SetDomain ( _domain->cget(0) );
	SpmManager.SetEnv ( _obstacles->polygons(), _sinks->polygons() );
	GlResources::configuration()->add("oglfuncs",500); // need to load more OpenGL functions then the default number
	const char* shadersfolder = 0;//"../src/sigspm/shaders/"; // Use this to load external shaders
	if ( shadersfolder )
	{	SpmManager.SetShadersFolder ( shadersfolder ); // to load shaders instead of using the pre-defined ones
		if ( !SpmManager.CanLoadShaders() ) gsout.fatal("Cannot find spm shaders in: %s",shadersfolder);
	}

	// Build ui:
	UiPanel *p, *sp;
	p = uim()->add_panel ( 0, UiPanel::HorizLeft, UiPanel::Top );
	p->add ( new UiButton ( "build", EvBuild ) );
	p->add ( new UiButton ( "mode", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=sp;
		p->add( _obstbut=new UiRadioButton( "obstacles", EvMode, true ) );
		p->add( _sinkbut=new UiRadioButton( "sink", EvMode, false ) );
		p->add ( _abbut=new UiCheckButton ( "auto build", EvAutoBuild, true ) ); p->top()->separate();
	}
	p->add ( new UiButton ( "exit", EvExit ) ); p->top()->separate();

}

void SpmViewer::build ()
{
	activate_ogl_context();
	Spm = SpmManager.Compute ( glrenderer()->glcontext(), Spm );
	SpmTexture.data ( Spm->GetMapBuffer(), Spm->Width(), Spm->Height(), GlTexture::Linear );
	SpmPlane->G[0].texid = SpmTexture.id; // only needed for first time the id is created
}

void SpmViewer::get_path ( float x, float y )
{
	if ( !Spm ) return;
	Spm->GetShortestPath ( x, y, SpmPath );
	_path->init ();
	_path->line_width ( 2.0f );
	_path->color ( GsColor::red );
	_path->begin_polyline();
	for ( int i=0, s=SpmPath.size(); i<s; i++ ) _path->push(SpmPath[i]);
	_path->end_polyline();
}

int SpmViewer::uievent ( int e )
{
	switch ( e )
	{	case EvMode:
		_obstacles->mode ( _obstbut->value()? SnPolyEditor::ModeEdit : SnPolyEditor::ModeNoEdition );
		_sinks->mode ( _sinkbut->value()? SnPolyEditor::ModeEdit : SnPolyEditor::ModeNoEdition );
		break;

		case EvAutoBuild: if ( _abbut->value() ) { build(); refresh(); } break;

		case EvBuild: build(); refresh(); break;

		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}

int SpmViewer::handle_scene_event ( const GsEvent& e )
{
	if ( Spm && e.alt && e.button1 && (e.type==GsEvent::Push||e.type==GsEvent::Drag) )
	{	get_path ( e.mousep.x, e.mousep.y );
		render();
		return 1;
	}
	return WsViewer::handle_scene_event(e);
}

int main ( int argc, char** argv )
{
	SpmViewer* v = new SpmViewer( -1, -1, 800, 600, "SIG SPM!" );
	v->cmd ( WsViewer::VCmdPlanar );

	GsCamera cam;
	v->camera().eye.z = 25.0f;

	v->root()->visible(false); // do not show unitialized texture that will contain the spm
	v->show ();		// request window to show
	v->refresh ();	// forces window to show and OpenGL to initialize now
	v->root()->visible(true); // after initialization we can draw the scene
	v->build ();	// now we can use OpenGL to draw the first spm
	v->message ( "Press Esc to switch between polygon creation and edition mode. Alt-left click to query path." );

	ws_run ();
	return 1;
}
