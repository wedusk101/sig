/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_scandir.h>

# include <sigogl/ui_file_chooser.h>

//# define GS_USE_TRACE1 // file scanning
//# define GS_USE_TRACE2 // input callback
# include <sig/gs_trace.h>

//================================== UiFileChooser ====================================

static void format ( GsString& flist, const GsStrings& dirs, const GsStrings& files, const char* fstring )
{
	GsString s;
	int i;
	flist.len(0);

	for ( i=0; i<dirs.size(); i++ )
	{	s=dirs[i]; remove_path(s);
		if ( !fstring[0] || s.search(fstring)==0 ) flist<<s<<"/ ";
	}
	if (i>0) flist<<gsnl;
	for ( i=0; i<files.size(); i++ )
	{	s=files[i]; remove_path(s);
		if ( !fstring[0] || s.search(fstring)==0 ) flist<<s<<gspc;
	}
}

static void read_files ( const char* fstring, GsString& flist, const GsStrings& filters )
{
	GS_TRACE1 ( "Reading ["<<fstring<<"]" );
	if ( !fstring ) { flist.set(""); return; }

	GsString path ( fstring );
	GsString fname;
	extract_filename ( path, fname );
	if ( !has_path(path) ) path="";
	if ( !validate_path(path) ) path="./";
	GS_TRACE1 ( "Path:["<<path<<"] Filename:["<<fname<<"]" );
	GS_TRACE1 ( "Filters:["<<filters<<"]" );

	GsStrings dirs, files;
	gs_scandir ( path, dirs, files, filters );
	format ( flist, dirs, files, fstring );
}

static void inputcb ( UiInput* inp, const GsEvent& e, char delc, void* udata )
{
	GS_TRACE2 ( "Callback: "<<e.key<<gspc<<delc );
	UiFileChooser* fc = (UiFileChooser*)udata;
	fc->update_file_list ( inp->value() );
}

UiFileChooser::UiFileChooser ( const char* fname, const char* filters, int ev, int x, int y, int mw, int mh )
			  :UiPanel ( 0, Vertical, Top, x, y, mw, mh )
{
	_inp = new UiInput(">",ev,0,0,mw,0,fname); // ImprNote: event not used, should declare callback to retrive events
	_inp->nextspc ( 8 );
	_inp->label().left();
	_inp->callback ( inputcb, this );

	_out = new UiOutput(NULL,0,0,mw,mh); // null label
	_out->label().left();
	_out->color().bg=_inp->color().bg;
	_out->auto_wrap ( true );

	set_filters ( filters );
	update_file_list ( fname );

	UiPanel::add ( _inp );
	UiPanel::add ( _out );
	disable_dragging();
}

void UiFileChooser::set_filters ( const char* st )
{
	_filters.size(0);
	if ( !st || !st[0] ) return;
	GsString cur;
	for ( int i=0; st[i]; i++ ) // example format: "*.txt;*.cfg"
	{	if ( st[i]=='.' )
		{	cur.len(0);
		}
		else if ( st[i]==';' || st[i]==',' || st[i]=='|' )
		{	if ( cur.len()>0 ) _filters.push(cur.pt());
			cur.len(0);
		}
		else if ( st[i]!='*' && st[i]!='?' )
		{	cur.insert(cur.len(),1,st[i]);
		}
	}
	if ( cur.len()>0 ) _filters.push(cur.pt());
}

void UiFileChooser::update_file_list ( const char* fstring )
{
	//if ( !fstring || !fstring[0] ) return;
	read_files ( fstring, _out->text(), _filters );
}

int UiFileChooser::handle ( const GsEvent& e, UiManager* uim )
{
	return UiPanel::handle(e,uim);
}

//================================ EOF =================================================
