/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_font.h>
# include <sigogl/gl_resources.h>
# include <sigogl/ui_manager.h>
# include <sigogl/ui_output.h>
# include <sig/sn_lines2.h>
# include <sig/sn_planar_objects.h>

//# define GS_USE_TRACE1  // basic trace
//# define GS_USE_TRACE2  // text drawing
# include <sig/gs_trace.h>

//================================== UiOutput =========================================

void UiOutput::_init ( const char* l )
{
	if ( l && l[0] ) { _showlabel=1; } else { _showlabel=0; _label.set(""); }
	_rectclip = 1;
	set_style ( UiStyle::Current() );
}

UiOutput::UiOutput ( const char* l, int x, int y, int mw, int mh )
		 :UiElement ( UiElement::Output, l, x, y, mw, mh )
{
	_init(l);
}

UiOutput::UiOutput ( const char* l, const char* t, int x, int y, int mw, int mh )
		 :UiElement ( UiElement::Output, l, x, y, mw, mh )
{
	_init(l);
	_text=t;
}

GsString& UiOutput::text()
{ 
	changed(NeedsRedraw);
	return _text;
}

void UiOutput::set_style ( const UiStyle& s )
{
	_color.bg = s.color.output_bg;
	_color.fg = s.color.output_fg;
	_color.lb = s.color.element_label;
}

void UiOutput::change_style ( const UiStyle& s )
{
	UiElement::change_style(s);
	set_style(s);
}

void UiOutput::resize ( float w, float h )
{
	GS_TRACE1 ( "UiOutput::resize to "<<w<<","<<h );
	UiElement::resize(w,h);
}

int UiOutput::handle ( const GsEvent& e, UiManager* uim )
{
	return 0;
}

void UiOutput::build ()
{
	GS_TRACE1 ( "Build called");
	UiElement::build ();
	GS_TRACE1 ( "Built - Rect: "<<_rect );
}

void UiOutput::draw ( UiPanel* panel )
{
	GS_TRACE2 ( "UiOutput::draw()" );
	SnPlanarObjects* pobs = panel->pobs();
	_changed=0;

	GsRect r=_rect; // _rect and _label already account for _xspc and _yspc

	if ( _showlabel && _label.text() )
	{	UiElement::draw(panel);
		int d=_label.xp()+1; r.x+=d; r.w-=d;
	}

	if ( _color.bg.a>0 ) // rect of the text output area
	{	pobs->push_rect ( r, _color.bg );
	}

	if ( !_text.len() ) return; // empty text
	GS_TRACE2 ( "Text: "<<_text.pt() );

	const GsFontStyle& fs = _label.fs();

	if ( _autowrap )
	{ 	GS_TRACE2 ( "Word-wrapping..." );
		const GsFont* font = GlResources::get_gsfont ( fs ); // fast retrieval from id in fs
		int ini=0, cur=0;
		float w=0, h=0;
		while ( true )
		{	if ( !_text[cur] ) break;
			cur++;
			w = font->text_width ( fs, _text+ini, cur-ini+1 );
			if ( _text[ini+cur]=='\n' )
			{	w=0; h++; ini=cur; }
			else if ( w>rect().w )
			{	if (!_text[cur]) break; // UiDev: clip on height as well
				_text.insert(cur-2,"\n");
				w=0; h++; ini=cur-1;
			}
		}
	}

	// use drawing functionality of UiLabel:
	GS_TRACE2 ( "Drawing..." );
	UiLabel tl;
	tl.copy_spacing ( _label );
	tl.fs() = fs;
	float xmax = r.xp();
	tl.draw ( pobs, r.x-_lxs, r.y, _color.fg, _rectclip? &xmax:0, _text.pt() );
	// _lxs fix things here when several outputs are group-aligned (better use UiInput's implementation as guide)

	GS_TRACE2 ( "Done." );
}

//================================ End of File =================================================
