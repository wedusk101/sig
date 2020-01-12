/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
# pragma once

# include <sig/sn_model.h>

# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_ik_manipulator.h>
# include <sigkin/kn_motion.h>

# include <sigogl/ui_panel.h>
# include <sigogl/ui_slider.h>
# include <sigogl/ui_check_button.h>

# include <sigogl/ws_viewer.h>

// Viewer for this example:
class MySkelViewer : public WsViewer
{  protected :
	# define NUMELPEV 400	// Number of elements possible per event type
	const int NEV=NUMELPEV; // shorter const int version of the above define

	enum MenuEv { EvInfo, EvView, EvExit, EvLoadMotion,
				  EvP1, EvP2, EvP3, EvR1, EvR2, EvR3, EvR4, // values slider
				  EvPrimitive, EvViewFloor, EvViewMotionSlider, EvMotionSlider, 
				  EvPosture=NUMELPEV, EvEff=NUMELPEV*2, EvJoint=NUMELPEV*3, EvMotion=NUMELPEV*4 };
	SnModel* _ground;
	SnGroup* _mg;
	KnSkeleton* _sk;
	KnScene* _sksc;
	KnMotion* _motion=0;
	GsArray<KnJoint*> _effs;
	UiCheckButton* _vbut[7];
	UiPanel* _jointp;
	UiPanel* _valuesp;
	UiSlider* _vsl[7];
	UiPanel* _primp;
	UiSlider* _psl[11]; // ra, rb, rc, nf, cx, cy, cz, r[4]
	UiPanel* _posturesp;
	UiPanel* _endeffs;
	UiPanel* _motionsp;
	UiSlider* _mslider=0;
	UiPanel* _msliderp=0;
	GsArray<KnMotion*> _motions;
	int _seljoint;
	int _frini=-1, _frend=-1;
   public :
	MySkelViewer ( KnSkeleton* sk, int x, int y, int w, int h, const char* l, bool viewskel, bool viewfloor, float fy );
	//void add_model ( GsModel* m ) { rootg()->add(new SnModel(m)); }
	void joint_info ( int jid );
	void iksolved ( KnIkManipulator* ikm );
	void build_values_panel ( int jid );
	void update_joint ( int si, int jid );
	void build_prim_panel ( int jid );
	void update_primitive ( int jid );
	void apply_motion ( KnMotion* m, int n, float t );
	void load_motion ( const char* mfile=0, int applyfr=-1 );
	virtual int uievent ( int e ) override;
	virtual int handle_keyboard ( const GsEvent& e ) override;
};

