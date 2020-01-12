/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/ui_manager.h>
# include <sigogl/ui_radio_button.h>

//================================== UiRadioButton =========================================

UiRadioButton::UiRadioButton ( const char* l, int ev, bool val, int x, int y, int mw, int mh )
			  :UiCheckButton ( l, ev, val, x, y, mw, mh )
{
	_type = UiElement::RadioButton;
}

void UiRadioButton::clear_attached_panels ( UiPanel* p, int dir )
{
	if ( !p ) return;
	int i, s=p->elements();

	if ( dir!=0 )
	{	for ( i=0; i<s; i++ )
		{	if ( p->get(i)->type()==RadioButton ) ((UiRadioButton*)p->get(i))->value(0);
		}
	}

	// check if parent or next panel has more radio buttons to clear:
	if ( p->extended_radiobut_panel() )
	{
		UiElement* o = p->owner();
		if ( dir<=0 && o && o->owner() )
		{	o = o->owner();
			if ( o->type()==Panel || o->type()==Submenu ) clear_attached_panels ( ((UiPanel*)o), -1 );
		}

		if ( dir>=0 && p->get(s-1)->type()==Button )
		{	clear_attached_panels ( ((UiButton*)p->get(s-1))->submenu(), 1 ); }
	}
}

void UiRadioButton::make_value_unique ()
{
	if ( !in_panel() ) return;
	UiPanel* p = (UiPanel*)_owner;

	// find our position:
	int r, i, s=p->elements();
	for ( i=0; i<s; i++ ) if ( p->get(i)==this ) break;
	if ( i==s ) return; // not found

	// now adjust values in the adjacent radio buttons:
	r = i;
	bool val = !_value;
	for ( i=r-1; i>=0; i-- ) if ( p->get(i)->type()!=RadioButton ) break; else ((UiRadioButton*)p->get(i))->value(val);
	for ( i=r+1; i<s;  i++ ) if ( p->get(i)->type()!=RadioButton ) break; else ((UiRadioButton*)p->get(i))->value(val);

	// check if parent or next panel has more radio buttons to clear:
	if ( p->extended_radiobut_panel() )
	{	clear_attached_panels ( p );
	}
}

int UiRadioButton::handle ( const GsEvent& e, UiManager* uim )
{
	gsbyte origval = _value;
	int h = UiCheckButton::handle(e,uim);
	if ( h && origval!=_value ) // value changed
	{	if ( _type==RadioButton ) make_value_unique();
	}
	return h;
}

//================================ End of File =================================================
