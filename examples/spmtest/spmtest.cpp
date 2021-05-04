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
# include <sigogl/ui_dialogs.h>
# include <sigogl/ui_radio_button.h>
# include <sigogl/ui_slider.h>
# include <sigogl/ws_viewer.h>
# include <sigogl/ws_run.h>
# include <sigspm/sn_spm.h>
# include <sigspm/glr_spm.h>
# include <sigspm/spm_manager.h>

class SpmViewer : public WsViewer
{  public: // ui:
	enum MenuEv { EvSave, EvLoad, EvBuild, EvAutoBuild, EvMode,
	              EvViewContours, EvViewVectorField, EvViewDistanceField, EvContoursInterval, EvContoursThickness, EvBufferSize,
				  EvExit };
	UiRadioButton *_obstbut, *_sinkbut;
	UiCheckButton *_abbut, *_contourbut, *_distfieldbut, *_vecfieldbut;
	UiSlider *_intervalslider, *_thicknesslider, *_resolutionslider;
	GsString _lfname;
   public: // scene:
	SnPolygons *_domain;
	SnPolyEditor *_sinks;
	SnPolyEditor *_obstacles;
	SnLines2 *_path;
	SnLines2 *_vfield;
	SnSpm* _snspm;
   public: // spm data:
	GlTexture SpmTexture;
	ShortestPathMap* Spm;
	ShortestPathMapManager SpmManager;
	std::vector<GsVec2> SpmPath;
   public:
	SpmViewer ( int x, int y, int w, int h, const char* l );
	void refresh () { render(); ws_fast_check(); }
	void build ( bool loadFromGPU=false );
	void get_path ( float x, float y );
	void update_vector_field ();
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
		//if ( !v->_path->empty() ) v->get_path( v->SpmPath[0].x, v->SpmPath[0].y );
		if ( !v->_path->empty() ) v->_path->init();
		v->update_vector_field();
	}
}

SpmViewer::SpmViewer ( int x, int y, int w, int h, const char* l ) : WsViewer( x, y, w, h, l )
{
	// Define domain boundaries:
	const float minx=-12, miny=-12, maxx=12, maxy=12;

	SnGroup* g = rootg();

	// Use SnSpm to draw the spm:
	g->add ( _snspm = new SnSpm(&SpmManager) );
	_snspm->minx = minx;
	_snspm->miny = miny;
	_snspm->width = maxx-minx;
	_snspm->height = maxy-miny;

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

	// Define scene node to show path and vector field:
	g->add ( _path = new SnLines2 );
	g->add ( _vfield = new SnLines2 );

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
	UiPanel *p, *subp;
	p = uim()->add_panel ( 0, UiPanel::HorizLeft, UiPanel::Top );
	p->add ( new UiButton ( "file", subp = new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p = subp;
		p->add( new UiButton( "save", EvSave ) );
		p->add( new UiButton( "load", EvLoad ) );
	}
	p->add ( new UiButton ( "build", EvBuild ) );
	p->add ( new UiButton ( "mode", subp = new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p = subp;
		p->add( _obstbut = new UiRadioButton( "obstacles", EvMode, true ) );
		p->add( _sinkbut = new UiRadioButton( "sink", EvMode, false ) );
		p->add( _abbut = new UiCheckButton( "auto build", EvAutoBuild, true ) ); p->top()->separate();
	}
	p->add ( new UiButton ( "view", subp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p = subp;
		p->add( _vecfieldbut = new UiCheckButton ( "vector field", EvViewVectorField, false ) );
		p->add( _distfieldbut = new UiCheckButton ( "distance field", EvViewDistanceField, false ) );
		p->add( _contourbut = new UiCheckButton ( "contour lines", EvViewContours, true ) );

		p->add( new UiElement( "interval:" ) ); p->top()->separate();
		p->add( _intervalslider = new UiSlider( 0, EvContoursInterval, 0, 0, 120 ) );
		_intervalslider->configure ( 0.005f, 0.1f, 0.005f, 3, 3 );
		_intervalslider->value ( _snspm->contourInterval );
		_intervalslider->all_events ( true );

		p->add( new UiElement( "thickness:" ) );
		p->add( _thicknesslider = new UiSlider( 0, EvContoursThickness ) );
		_thicknesslider->configure ( 0.001f, 0.02f, 0.0005f, 3, 3 );
		_thicknesslider->value ( _snspm->contourThickness );
		_thicknesslider->all_events ( true );

		//SpmTodo: fix ability to change map resolution
		//p->add( new UiElement( "Buffer size:" ) ); p->top()->separate();
		//p->add( _resolutionslider = new UiSlider( "", EvBufferSize ) );
		//_bufferSizeSlider->configure( 100, 4000, 100, 1, 0 );
		//_bufferSizeSlider->value( 1000 );
	}
	p->add ( new UiButton ( "exit", EvExit ) ); p->top()->separate();
}

void SpmViewer::build ( bool loadFromGPU )
{
	activate_ogl_context();
	Spm = SpmManager.Compute ( glrenderer()->glcontext(), Spm, loadFromGPU );
}

void SpmViewer::get_path ( float x, float y )
{
	if ( !Spm ) return;
	if ( !Spm->IsReadyToQuery() ) build(true); // SpmTodo: fix bringing map from GPU without rebuilding it
	Spm->GetShortestPath ( x, y, SpmPath );
	_path->init ();
	_path->line_width ( 2.0f );
	_path->color ( GsColor::red );
	_path->begin_polyline();
	for ( size_t i=0, s=SpmPath.size(); i<s; i++ ) _path->push(SpmPath[i]);
	_path->end_polyline();
}

void SpmViewer::update_vector_field ()
{
	_vfield->init();
	_vfield->color ( GsColor::black );
	if ( !_vecfieldbut->value() ) return;
	if ( !Spm ) return;
	if ( !Spm->IsReadyToQuery() ) build(true);

	float spmw = _snspm->width;
	float spmh = _snspm->height;
	float x, incx=spmw/GS_ROUND(spmw);
	float y, incy=spmh/GS_ROUND(spmh);
	float maxx = _snspm->minx+spmw + incx/2.0f;
	float maxy = _snspm->miny+spmh + incy/2.0f;

	GsVec2 v;
	float vlen=0.75f, al=0.2f, aw=0.15f;

	for ( x=_snspm->minx; x<maxx; x+=incx )
	{	for ( y=_snspm->miny; y<maxy; y+=incy )
		{	if ( Spm->GetDirection(x,y,v,false) )
			{	float l = v.len();
				if ( l<vlen ) continue;
				v.len(vlen);
				_vfield->push_arrow ( GsPnt2(x,y), v+GsVec2(x,y), al, aw );
			}
		}
	}

	refresh();
}

int SpmViewer::uievent ( int e )
{
	switch( e )
	{	case EvSave:
		{	const char* fname = ui_input_file ( "File to save:", _lfname, "*.txt;*.spm" ); _lfname=fname;
			GsOutput out;
			if ( fname && out.open(fname) ) out<<*_obstacles->polygons()<<gsnl<<*_sinks->polygons()<<gsnl;
			break;
		}
		case EvLoad:
		{	const char* fname = ui_select_file ( "File to load:", _lfname, "*.txt;*.spm" ); _lfname=fname;
			GsInput inp;
			if ( !Spm || !fname || !inp.open(fname) ) break;
			inp>>*_obstacles->polygons()>>*_sinks->polygons();
			if ( !Spm->IsReadyToQuery() ) build(true);
			refresh();
			break;
		}

		case EvMode: {
			_obstacles->mode( _obstbut->value() ? SnPolyEditor::ModeEdit : SnPolyEditor::ModeNoEdition );
			_sinks->mode( _sinkbut->value() ? SnPolyEditor::ModeEdit : SnPolyEditor::ModeNoEdition );
			break;
		}

		case EvAutoBuild: if( _abbut->value() ) { build(); refresh(); } break;

		case EvBuild: build(); refresh(); break;

		case EvViewContours:	  _snspm->contourLines=_contourbut->value(); break;
		case EvContoursInterval:  _snspm->contourInterval=_intervalslider->value(); refresh(); break;
		case EvContoursThickness: _snspm->contourThickness=_thicknesslider->value(); refresh(); break;
		case EvViewDistanceField: _snspm->distanceField=_distfieldbut->value(); break;
		case EvViewVectorField:   update_vector_field(); break;

		case EvBufferSize: { // SpmTodo: resolution change is not working properly
			// rounds to closest 100
			//int width = ( _resolutionlider->valuei() + 50 ) / 100 * 100;
			//int height = width;
			//SpmManager.SetBufferDimensions( width, height );
			//refresh();
			break;
		}

		case EvExit: gs_exit();
	}
	return WsViewer::uievent( e );
}

int SpmViewer::handle_scene_event ( const GsEvent& e )
{
	// SpmTodo: revisit automatic SPM loading at mouse release button
	//if ( Spm && e.type==GsEvent::Release && _abbut->value()==false )
	//{
	//	Spm->LoadSPM();
	//	build();
	//	if ( !_path->empty() ) get_path( SpmPath[0].x, SpmPath[0].y );
	//}
	//else
	if ( Spm && e.alt && e.button1 && ( e.type==GsEvent::Push || e.type==GsEvent::Drag ) )
	{
		get_path( e.mousep.x, e.mousep.y );
		render();
		return 1;
	}

	return WsViewer::handle_scene_event( e );
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
	v->message ( "Press Esc to switch between polygon creation and edition mode. Alt + left-click to query path." );

	ws_run ();
	return 1;
}
