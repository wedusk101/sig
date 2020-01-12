/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include "skelviewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sigogl/ui_dialogs.h>

# include <sigogl/ws_run.h>

# include <sigkin/kn_scene.h>

void ManipPCB ( KnIkManipulator* ikm, void* udata )
{
	((MySkelViewer*)udata)->iksolved(ikm);
}

MySkelViewer::MySkelViewer ( KnSkeleton* sk, int x, int y, int w, int h, const char* l, 
							 bool viewskel, bool viewfloor, float fy )
			 :WsViewer ( x, y, w, h, l )
{ 
	// create my scene:
	rootg()->add ( _sksc=new KnScene(sk) );

	// make floor: (extend to floor in any orientation, have floor be a vector)
	GsPrimitive floor; float floorth=1.0f;
	floor.box(200.0f,floorth,200.0f); floor.center.set(0,fy-(floorth/2.0f),0);
	_ground = new SnModel;
	_ground->model()->make_primitive ( floor, GsModel::UseNoMtl );
	_ground->color( GsColor::darkgreen );
	_ground->visible ( viewfloor );
	rootg()->add ( _ground );

	// set visualization:
	bool viewskin=false, viewvisg=false, viewcolg=false, viewaxis=false;
	if ( sk->skin() )
	{	viewskin=true;
	}
	else if ( sk->visgeos()==0 ) // if no geometries show the skeleton:
	{	viewskel=true; viewvisg=false;
	}
	else viewvisg=true;
	_sksc->set_visibility ( viewskin, viewskel/*skel*/, viewvisg/*visgeo*/, viewcolg/*colgeo*/, viewaxis/*axis*/ );

	// Extra settings:
	//sk->enforce_rot_limits ( false );

	_sk=sk;
	_seljoint=0;

	UiPanel *p, *sp;
	p = uim()->add_panel ( 0, UiPanel::HorizLeft );

	_effs.sizecap(0,4);
	int s=sk->joints().size();
	int max=24; // max entries per panel
	p->add ( new UiButton ( "joint", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_jointp=sp;
		for ( int i=0; i<s; i++ )
		{	p->add ( new UiRadioButton ( sk->joints()[i]->name(), EvJoint+i, i==0? true:false ) );
			if ( sk->joints()[i]->ik() ) _effs.push()=sk->joints()[i];
			if ( (i+1)%max==0 ) // too many entries: open new panel
			{	p->add ( new UiButton ( "more", sp=new UiPanel(0,UiPanel::Vertical) ) );
				p->extended_radiobut_panel(true);
				sp->extended_radiobut_panel(true);
				p = sp;
			}
		}
	}

	void rot_sliders_config ( UiSlider* s[4] );
	p->add ( new UiButton ( "values", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_valuesp=sp;
		for ( int i=0; i<7; i++ )
		{	_vsl[i] = (UiSlider*)p->add ( new UiSlider ( "val:", EvP1+i ) );
			_vsl[i]->all_events(true);
			if ( i==3 ) _vsl[i]->separate();
		}
		build_values_panel ( 0 );
	}

	p->add ( new UiButton ( "primitive", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_primp=sp;
		p->add ( new UiElement ( "(no joint selected)", 0,0,220 ) );
		_psl[0] = (UiSlider*)p->add ( new UiSlider ( "ra:", EvPrimitive ) );
		_psl[1] = (UiSlider*)p->add ( new UiSlider ( "rb:", EvPrimitive ) );
		_psl[2] = (UiSlider*)p->add ( new UiSlider ( "rc:", EvPrimitive ) );
		_psl[3] = (UiSlider*)p->add ( new UiSlider ( "nf:", EvPrimitive ) ); _psl[3]->separate();
		_psl[4] = (UiSlider*)p->add ( new UiSlider ( "cx:", EvPrimitive ) ); _psl[4]->separate();
		_psl[5] = (UiSlider*)p->add ( new UiSlider ( "cy:", EvPrimitive ) );
		_psl[6] = (UiSlider*)p->add ( new UiSlider ( "cz:", EvPrimitive ) );
		_psl[7] = (UiSlider*)p->add ( new UiSlider ( 0, EvPrimitive ) ); _psl[7]->separate();
		_psl[8] = (UiSlider*)p->add ( new UiSlider ( 0, EvPrimitive ) );
		_psl[9] = (UiSlider*)p->add ( new UiSlider ( 0, EvPrimitive ) );
		_psl[10] = (UiSlider*)p->add ( new UiSlider ( 0, EvPrimitive ) );
		rot_sliders_config ( _psl+7 );
		build_prim_panel ( 0 );
	}

	s=sk->postures().size();
	if ( s>0 )
	{	p->add ( new UiButton ( "postures", sp=new UiPanel(0,UiPanel::Vertical) ) );
		UiPanel* p=_posturesp=sp;
		for ( int i=0; i<s; i++ )
		{	p->add ( new UiRadioButton ( sk->postures()[i]->name(), EvPosture+i, i==0? true:false ) );
		}
	}

	s=_effs.size();
	if ( s>0 )
	{	rootg()->add( _mg=new SnGroup );
		p->add ( new UiButton ( "effectors", sp=new UiPanel(0,UiPanel::Vertical) ) );
		UiPanel* p=_endeffs=sp;
		for ( int i=0; i<s; i++ )
		{	p->add ( new UiButton ( _effs[i]->name(), EvEff+i ) );
			KnIkManipulator* ikm = new KnIkManipulator;
			ikm->init ( _effs[i]->ik() );
			ikm->solve_method ( KnIkManipulator::SearchOrbit, _sksc );
			ikm->post_callback ( ManipPCB, this );
			ikm->lines ( true );
			_mg->add ( ikm );
		}
	}

	p->add ( new UiButton ( "view", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_valuesp=sp;
		_vbut[0] = (UiCheckButton*)p->add ( new UiCheckButton ( "skin", EvView, viewskin ) );
		_vbut[1] = (UiCheckButton*)p->add ( new UiCheckButton ( "skeleton", EvView, viewskel ) );
		_vbut[2] = (UiCheckButton*)p->add ( new UiCheckButton ( "visgeo", EvView, viewvisg ) );
		_vbut[3] = (UiCheckButton*)p->add ( new UiCheckButton ( "colgeo", EvView, viewcolg ) );
		_vbut[4] = (UiCheckButton*)p->add ( new UiCheckButton ( "frames", EvView, viewaxis ) );
		_vbut[5] = (UiCheckButton*)p->add ( new UiCheckButton ( "floor", EvViewFloor, viewfloor ) ); p->top()->separate();
		_vbut[6] = (UiCheckButton*)p->add ( new UiCheckButton ( "motion slider", EvViewMotionSlider, false ) );
		_mslider = 0;
	}

	p->add ( new UiButton ( "info", EvInfo ) );

	p->add ( new UiButton ( "motions", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_motionsp=sp;
		p->add ( new UiButton ( "load", EvLoadMotion ) );
	}

	p->add ( new UiButton ( "exit", EvExit ) );
}

void MySkelViewer::joint_info ( int jid )
{
	if ( jid<0 || jid>=_sk->joints().size() ) return; // protection
	KnJoint* j = _sk->joints()[jid];

	message().len(0); GsOutput o; o.init(message());
	o << "Joint:[" << j->name() << "] ";
	o << "Parent:[" << (j->parent()?j->parent()->name():"null") << "] ";
	o << "Offset:[" << j->offset() << "] ";
	o << "PosDOFs:[" << (3-j->pos()->nfrozen()) << "] ";
	o << "Prepost:[" << (j->rot()->hasprepost()) << "] ";

	o << " Channels:[";
	const KnChannels& ch = _sk->channels();
	for ( int i=0, s=ch.size(); i<s; i++ )
	{	if ( j->name()==ch.cget(i).jname() )
		{	o << ch.cget(i).type_name();
			if ( i+1<s ) o<<gspc;
		}
	}
	o << "]";
}

void MySkelViewer::iksolved ( KnIkManipulator* ikm )
{
	message().set ( KnIk::message(ikm->result()) );
}

void rot_sliders_config ( UiSlider* s[4] )
{
	s[0]->label().set("rax:"); s[0]->range(-1.0f,1.0f); s[0]->value(1);
	s[1]->label().set("ray:"); s[1]->range(-1.0f,1.0f); s[1]->value(0);
	s[2]->label().set("raz:"); s[2]->range(-1.0f,1.0f); s[2]->value(0);
	s[3]->label().set("ang:"); s[3]->range(-gspi,gspi); s[3]->value(0);
	for ( int i=0; i<4; i++ ) s[i]->all_events ( true );
}

void rot_sliders_set ( UiSlider* s[4], GsQuat q )
{
	GsVec axis;
	float rad;
	q.get ( axis, rad );
	s[0]->value(axis.x);
	s[1]->value(axis.y);
	s[2]->value(axis.z);
	s[3]->value(rad);
}

void rot_sliders_get ( UiSlider* s[4], GsQuat& q )
{
	float ang = s[3]->value();
	if ( ang==0 )
		q = GsQuat::null;
	else
		q.set ( GsVec(s[0]->value(),s[1]->value(),s[2]->value()), ang );
}

void MySkelViewer::build_values_panel ( int jid )
{
	KnJoint* j = _sk->joints()[jid];

	float min=-4.0f, max=4.0f;
	for ( int i=0; i<3; i++ )
	{	float v = j->pos()->value(i);
		if ( j->pos()->frozen(0) )
		{	_vsl[i]->range(v,v); }
		else
		{	if ( j->pos()->limits(i) )
				_vsl[i]->range(j->pos()->lolim(i),j->pos()->uplim(i));
			else
				_vsl[i]->range(min,max);
			_vsl[i]->value(v);
		}
	}

	if ( j->rot_type()==KnJoint::TypeQuat )
	{
		rot_sliders_config ( _vsl+3 );
		rot_sliders_set ( _vsl+3, j->quat()->value() );
	}
	else if ( j->rot_type()==KnJoint::TypeEuler )
	{	KnJointEuler* e = j->euler();
		const char* l[] = { "rx:", "ry:", "rz:" };
		for ( int d=0; d<3; d++ )
		{	_vsl[d+3]->label().set(l[d]);
			_vsl[d+3]->range(e->lolim(d),e->uplim(d));
			_vsl[d+3]->value(e->value(d));
		}
		_vsl[6]->label().set("-"); _vsl[6]->range(0,0); _vsl[6]->value(0);
	}
	else if ( j->rot_type()==KnJoint::TypeST )
	{	KnJointST* st = j->st();
		_vsl[3]->label().set("sx:"); _vsl[3]->range(-gspi,gspi); _vsl[3]->value(st->swingx());
		_vsl[4]->label().set("sy:"); _vsl[4]->range(-gspi,gspi); _vsl[4]->value(st->swingy());
		_vsl[5]->label().set("tw:");
		_vsl[5]->range(st->twist_lolim(),st->twist_uplim());
		_vsl[5]->value(st->twist());
		_vsl[6]->label().set("-"); _vsl[6]->range(0,0); _vsl[6]->value(0);
	}

	// Depending on reconfiguratios may need:
	// UiPanel* p=_valuesp;
	// p->changed ( UiElement::NeedsRebuild );
}

void MySkelViewer::update_joint ( int si, int jid )
{
	KnJoint* j = _sk->joints()[_seljoint];
	if ( si<3 ) // position DOFs
	{	// to be completed
	}
	else // rotation DOFs
	{	if ( j->rot_type()==KnJoint::TypeQuat )
		{	GsQuat q;
			rot_sliders_get ( _vsl+3, q );
			j->rot()->value ( q );
		}
		else if ( j->rot_type()==KnJoint::TypeEuler )
		{	j->euler()->value ( si-3, _vsl[si]->value() );
		}
		else if ( j->rot_type()==KnJoint::TypeST )
		{	if ( si<5 )
				j->st()->swing ( _vsl[3]->value(), _vsl[4]->value() );
			else
				j->st()->twist ( _vsl[5]->value() );
		}
	}
}

void MySkelViewer::build_prim_panel ( int jid )
{
	KnJoint* j = _sk->joints()[jid];
	const GsPrimitive* p=0;
	if ( j->visgeo() ) p = j->visgeo()->primitive;
	_primp->get(0)->label().set( p? p->typestring():"(joint has no primitive)" );
	_primp->get(0)->changed( UiElement::NeedsRedraw );
	if ( !p )
	{	for ( int i=0; i<11; i++ ) _psl[i]->value(0);
		return;
	}

	// format sliders:
	float max=2.0f;
	for ( int i=0; i<3; i++ ) { GS_UPDMAX(max,p->r[i]+2.0f); }
	for ( int i=0; i<3; i++ )
	{	_psl[i]->range ( 0, max );
		_psl[i]->all_events ( true );
	}
	_psl[3]->range ( 0, 40 );
	_psl[3]->format ( 2, 0 );
	_psl[3]->all_events ( true );
	for ( int i=4; i<7; i++ ) { GS_UPDMAX(max,-p->center.e[i]); GS_UPDMAX(max,p->center.e[i]); }
	for ( int i=4; i<7; i++ )
	{	_psl[i]->range ( -max, max );
		_psl[i]->all_events ( true );
	}
	rot_sliders_set ( _psl+7, p->orientation );

	// set values:
	_psl[0]->value ( p->ra );
	_psl[1]->value ( p->rb );
	_psl[2]->value ( p->rc );
	_psl[3]->value ( p->nfaces );
	_psl[4]->value ( p->center.x );
	_psl[5]->value ( p->center.y );
	_psl[6]->value ( p->center.z );
}

void MySkelViewer::update_primitive ( int jid )
{
	KnJoint* j = _sk->joints()[jid];
	if ( !j->visgeo() ) return;
	GsPrimitive* p = j->visgeo()->primitive;
	if ( !p ) return;

	p->ra = _psl[0]->value();
	p->rb = _psl[1]->value();
	p->rc = _psl[2]->value();
	p->nfaces = _psl[3]->valuei();
	p->center.x = _psl[4]->value();
	p->center.y = _psl[5]->value();
	p->center.z = _psl[6]->value();

	rot_sliders_get ( _psl+7, p->orientation );

	j->visgeo()->make_primitive ( *p );
}

void MySkelViewer::apply_motion ( KnMotion* m, int n, float t )
{
	m->apply ( t );
	message().setf ( "%s:  n=%d  t=%4.2f  fps=%4.2f", m->name(), n, t, (t>0?float(n)/t:0.0f) );
	_sksc->update ();
	render ();
	ws_check ();
}

void MySkelViewer::load_motion ( const char* mfile, int applyfr )
{
	if ( !mfile ) mfile = ui_select_file ( "Selection motion file", 0, "*.sm;*.bvh" );
	if ( !mfile ) return;

	KnMotion* m = new KnMotion;
	if ( !m->load ( mfile ) ) { delete m; return; }
	_motions.push ( m );
	m->ref ();
	m->connect ( _sk );
	if ( applyfr>=0 ) { m->apply_frame(0); _sksc->update(); render(); }

	_motionsp->add ( new UiButton ( m->name(), EvMotion+_motions.size()-1 ) );
	_motionsp->build();
	if ( _motions.size()==1 ) _motionsp->top()->separate();
}

int MySkelViewer::uievent ( int e )
{
	if ( e>=EvP1 && e<=EvR4 )
	{	update_joint(e-EvP1,_seljoint);
		_sksc->update();
		render();
	}
	else if ( e>=EvPosture && e<EvPosture+NEV )
	{	KnPosture* p = _sk->postures()[e-EvPosture];
		p->apply();
		_sksc->update();
		message()=""; GsOutput o; o.init(message());
		p->output ( o, false, true );
		return 1;
	}
	else if ( e>=EvEff && e<EvEff+NEV )
	{	KnJoint* j = _effs[e-EvEff];
		_mg->get<KnIkManipulator>(e-EvEff)->lines( j->ik()->lines()? false:true );
		//_sksc->update();
		render ();
		return 1;
	}
	else if ( e>=EvJoint && e<EvJoint+NEV )
	{	_seljoint = e-EvJoint;
		joint_info ( _seljoint );
		build_values_panel ( _seljoint );
		build_prim_panel ( _seljoint );
		return 1;
	}
	else if ( e>=EvMotion && e<EvMotion+NEV )
	{	KnMotion* m = _motions[e-EvMotion];
		_motion = m;
		_frini = _frend = -1;
		double d = (double)m->duration();
		double t=0, t0=gs_time();
		int n=1;
		while ( t<d )
		{	apply_motion ( m, n++, (float)t );
			t = gs_time()-t0;
		}
		apply_motion ( m, n, (float)d );
		return 1;
	}
	else switch ( e )
	{	case EvLoadMotion: load_motion (); return 1;

		case EvPrimitive:
		{	update_primitive(_seljoint);
			_sksc->rebuild (_seljoint);
			render();
		} return 1;

		case EvView:
		{	_sksc->set_visibility ( _vbut[0]->value(), _vbut[1]->value(), _vbut[2]->value(), _vbut[3]->value(), _vbut[4]->value() );
			_sksc->update();
		} return 1;

		case EvViewFloor:
		{	_ground->visible ( _vbut[5]->value() );			
		} return 1;

		case EvViewMotionSlider: 
		{	if ( _motions.empty() ) return 1;
			if ( !_motion ) _motion=_motions[0];
			if ( !_mslider )
			{	UiPanel *p = new UiPanel ( 0, UiPanel::HorizLeft, 0, 25 );
				_mslider = (UiSlider*)p->add ( new UiSlider ( "fr:", EvMotionSlider ) );
				_mslider->minw ( 500 );
				_mslider->increment ( 1.0f );
				_mslider->format ( 3, 0 );
				_mslider->all_events(1);
				_msliderp = p;
				_msliderp->ref();
			}
			if ( _vbut[6]->value() )
			{	_mslider->range ( 0, float(_motion->frames()-1) );
				_mslider->value ( 0 );
				uim()->add(_msliderp);
			}
			else
			{	uim()->remove ( uim()->search(_msliderp) ); }
		} return 1;

		case EvMotionSlider: 
		{	if ( !_motion || !_mslider ) return 0;
			_motion->apply_frame ( _mslider->valuei() );
			_sksc->update();
		} return 1;

		case EvInfo: 
		{	message().setf ( "Skeleton:%s  Joints:%d  Channels:%d  Channels in Motions:", 
					_sk->name(), _sk->joints().size(), _sk->channels().size() );
			for ( int i=0; i<_motions.size(); i++ ) message()<<" "<<_motions[i]->channels()->size();
		} return 1;

		case EvExit: ws_exit(); return 1;
	}

	return WsViewer::uievent(e);
}

int MySkelViewer::handle_keyboard ( const GsEvent& e )
{
	switch ( e.key )
	{	
		case GsEvent::KeyF5 : // save skeleton
		{	GsString fname = _sk->name();
			fname << ".s";
			if ( !ui_confirm("Save skeleton using same name in current folder?") ) return 1;
			_sk->init_values();
			_sksc->update();
			bool ok = _sk->save ( fname, true, true );
			gsout << fname << ": " << ( ok? "saved.":"not saved!" ) << "\n";
			render ();
		} return 1;

		case GsEvent::KeyF6: _frini=_mslider? _mslider->valuei():-1; gsout<<"ini frame: "<<_frini<<gsnl; return 1;
		case GsEvent::KeyF7: _frend=_mslider? _mslider->valuei():-1; gsout<<"end frame: "<<_frend<<gsnl; return 1;

		case GsEvent::KeyF8 : // save optimized motion
		{	if ( _motions.empty() ) return 0;
			if ( !ui_confirm("Save motions to current folder?") ) return 1;
			GsString fname;
			for ( int i=0, s=_motions.size(); i<s; i++ )
			{	KnMotion* m = _motions[i];
				fname.setf ( "m%02d_%s.sm", i+1, m->name()? m->name():"noname" );
				bool ok = m->save ( fname, _frini, _frend, true );
				gsout << fname << ": " << ( ok? "saved.":"not saved!" ) << "\n";
				if(1) // save also in bvh
				{	fname.replace(".sm",".bvh");
					ok = m->save_bvh ( fname, 30 );
					gsout << fname << ": " << ( ok? "saved.":"not saved!" ) << "\n";
				}
			}
		} return 1;
	}

	return WsViewer::handle_keyboard(e);
}
