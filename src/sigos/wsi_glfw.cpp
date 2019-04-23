/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

//================================= SIG-GLFW integration  ==========================================
//================================= and x11 port via glfw ==========================================

# include <sig/gs.h> // needed here to define macro below

# if defined (GS_LINUX) || defined (GS_GLFW)

# include <stdio.h>
# include <string.h>
# include <stdlib.h>

# include <sig/gs_array.h>
# include <sig/gs_buffer.h>
# include <sig/gs_output.h>
# include <sig/gs_string.h>
# include <sigogl/ws_window.h>
# include <sigogl/ws_osinterface.h>
# include <sigogl/gl_loader.h>
# include <sigogl/gl_resources.h>

//# define GS_USE_TRACE_COUNTER
//# define GS_USE_TRACE1 // trace main functions
//# define GS_USE_TRACE2 // trace more functions
//# define GS_USE_TRACE3 // trace more info
//# define GS_USE_TRACE4 // get ogl procedure

//# define GS_USE_TRACE5 // events
//# define GS_USE_TRACE6 // more events
//# define GS_USE_TRACE7 // mouse move event
# include <sig/gs_trace.h>

# ifdef GS_WINDOWS
# define WIN32_LEAN_AND_MEAN 1
# include <windows.h>
# include <Shlobj.h>
# include <Commdlg.h>
# define GLFW_INCLUDE_NONE
# endif // GS_WINDOWS

// This port to GLFW is not fully implemented yet

//#include <GL/glu.h>
//    #include <GL/glx.h>
//    #include <GL/glxext.h>
    //#define glXGetProcAddress(x) (*glXGetProcAddressARB)((const GLubyte*)x)

//# include <GL/gl.h> 

# include <GLFW/glfw3.h>

//===== Global Data ===========================================================================

struct SwSysWin; // fwd decl
static GsBuffer<SwSysWin*> AppWindows;
static gsint16		AppNumVisWindows=0;
static gsint16		DialogRunning=0;

//static int (*AppCallBack)( const GsEvent& ev, void* wdata )=0;

//===== Window Data ===========================================================================
struct SwSysWin
{	char*	 label;			// window label
	WsWindow* swin;			// sig window
	GsEvent  event;			// event being sent to window
	gscbool  justresized;	// flag to allow client area correction
	gscbool  needsinit;		// flag to call client init funtion before first paint
	gscbool  visible;		// visible state according to calls to show and hide
	gscbool	 isdialog;		// dialogs need special treatment
	gscbool  redrawcalled;	// dialogs need special treatment
	//gscbool  canbedestroyed;
	GLFWwindow* gwin;		// glfw window handle
	SwSysWin() { label=0; swin=0; }//canbedestroyed=1; }
   ~SwSysWin(); // destructor
};

SwSysWin::~SwSysWin()
{
	GS_TRACE1 ( "SwSysWin Destructor ["<<label<<']' );
	if ( gwin ) { wsi_win_hide(this); glfwDestroyWindow(gwin); gwin=0; } // no further callbacks will be called for this window
	delete swin; // destructor will call wsi_del_win()
	free ( label );
	if ( isdialog ) DialogRunning--;
	for ( int i=0, s=AppWindows.size(); i<s; i++ )
	{	if ( AppWindows[i]==this ) { AppWindows.remove(i); break; } }
}

//===== Functions =============================================================================

void wsi_del_win ( void *win ) // this function is called by the WsWindow destructor, possibly by the user
{
	GS_TRACE1 ( "wsi_del_win ["<<((SwSysWin*)win)->label<<"]..." );
	SwSysWin* sw = (SwSysWin*)win;
	if ( sw->isdialog ) { DialogRunning--; sw->isdialog=0; } // important to allow other windows to receive events

	if ( sw->gwin ) // the user called this function
	{	sw->swin=0; // if user deletes sw, this is being called before SwSysWin destructor to stop a 2nd delete
		delete sw;
	}
	else // this call came from SwSysWin desctructor triggered by a glfw event
	{	// nothing more to do
	}
}

static void _init_callbacks ( GLFWwindow* win ); // will set all glfw callbacks, defined later

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error: %s\n", description);
}

void* wsi_new_win ( int x, int y, int w, int h, const char* label, WsWindow* swin, int mode )
{
	GS_TRACE1 ( "wsi_new_win ["<<label<<"]..." );

	static bool notinit=true;
	if ( notinit ) // glfw initializations before creating first window
	{	glfwSetErrorCallback(error_callback);
		if ( !glfwInit() ) gsout.fatal("wsi_new_win: Could not init GLFW!");
	}

	GlResources::load_configuration_file (); // only overall 1st call will actually load config file

	SwSysWin* sw = new SwSysWin;
	# ifdef GS_WINDOWS
	sw->label = _strdup ( label );
	# else
	sw->label = strdup ( label );
	# endif
	sw->swin = swin;
	sw->justresized = 1;
	sw->needsinit = 1;
	sw->event.width = w;
	sw->event.height = h;
	sw->visible = 0;
	sw->isdialog = mode;
	sw->redrawcalled = 0;
	sw->gwin = 0;

	if ( mode>0 ) DialogRunning++;

	// update default sizes:
	const int minw=50, minh=50; // it seems windows may not always accept sizes smaller than about 100
	if ( w<minw ) w=minw;
	if ( h<minh ) h=minh;

	// dialog boxes are simulated with mode>0 (DialogBox() function does not return until callback function terminates)
	GS_TRACE1 ( "Creating window..." );
	glfwWindowHint ( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint ( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint ( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint ( GLFW_DOUBLEBUFFER, 1 );
	glfwWindowHint ( GLFW_RESIZABLE, 1 );
	glfwWindowHint ( GLFW_FOCUSED, mode>0? 1:0 );
	GLFWwindow* sharedwin = AppWindows.size()? AppWindows[0]->gwin:NULL;
	sw->gwin = glfwCreateWindow ( w, h, label, NULL, sharedwin );
	glfwSetWindowUserPointer ( sw->gwin, sw );

	if ( notinit ) // glfw initializations after creating first window
	{	glfwMakeContextCurrent ( sw->gwin );
		gl_load_and_initialize (); // only overall 1st call will actually load ogl
		notinit=false;
	}

	int scw, sch;
	wsi_screen_resolution ( scw, sch );
	if ( x<0 ) x = (scw-w)/2;
	if ( y<0 ) y = (sch-h)/2;
	glfwSetWindowPos ( sw->gwin, x, y );
//xxx	glfwSetWindowSizeLimits ( sw->gwin, 16, 8, GLFW_DONT_CARE, GLFW_DONT_CARE );

	if ( sw->gwin==NULL)
	{	gsout.fatal ( "Window Creation Failed !" );
	}

	if ( AppWindows.empty() ) // first windows being created
	{	_init_callbacks ( sw->gwin );
	}

	AppWindows.push(sw);
	return (void*) sw;
}

const char* wsi_win_label ( void* win )
{
	GS_TRACE2 ( "wsi_win_label ["<<((SwSysWin*)win)->label<<"] label requested." );
	return ((SwSysWin*)win)->label;
}

void wsi_win_label ( void* win, const char* label )
{
	GS_TRACE2 ( "wsi_win_label ["<<((SwSysWin*)win)->label<<"] to "<<label<<"..." );
	SwSysWin* sw = (SwSysWin*)win;
	free ( sw->label );
	# ifdef GS_WINDOWS
	sw->label = _strdup ( label );
	# else
	sw->label = strdup ( label );
	# endif
	glfwSetWindowTitle ( sw->gwin, sw->label );
}

void wsi_win_show ( void* win )
{
	GS_TRACE2 ( "wsi_win_show..." );
	SwSysWin* sw = (SwSysWin*)win;
	if ( !sw->visible ) { AppNumVisWindows++; sw->visible=1; }
	glfwShowWindow ( sw->gwin );
}

void wsi_win_hide ( void* win )
{
	GS_TRACE2 ( "wsi_win_hide..." );
	SwSysWin* sw = (SwSysWin*)win;
	if ( sw->visible ) { AppNumVisWindows--; sw->visible=0; }
	glfwHideWindow ( sw->gwin );
}

void wsi_win_move ( void* win, int x, int y, int w, int h )
{
	GS_TRACE2 ( "wsi_win_move..." );
	SwSysWin* sw = (SwSysWin*)win;
	sw->justresized = true;
	glfwSetWindowPos ( sw->gwin, x, y );
	glfwSetWindowSize( sw->gwin, w, h );
}

void wsi_win_pos ( void* win, int& x, int& y )
{
	GS_TRACE2 ( "wsi_win_pos..." );
	SwSysWin* sw = (SwSysWin*)win;
	glfwGetWindowPos ( sw->gwin, &x, &y );
}

void wsi_win_size ( void* win, int& w, int& h )
{
	GS_TRACE2 ( "wsi_win_size..." );
	SwSysWin* sw = (SwSysWin*)win;
	glfwGetWindowSize ( sw->gwin, &w, &h );
	GS_TRACE3 ( "got size "<<w<<"x"<<h );
}

bool wsi_win_visible ( void* win )
{
	GS_TRACE2 ( "wsi_win_visible..." );
	return true; // IsWindowVisible ( ((SwSysWin*)win)->gwin )==1;
}

bool wsi_win_minimized ( void* win )
{
	GS_TRACE2 ( "wsi_win_iconized..." );
	return false; // IsIconic ( ((SwSysWin*)win)->gwin )==1;
}

void wsi_activate_ogl_context ( void* win )
{
	GS_TRACE2 ( "wsi_set_ogl_context..." );
	glfwMakeContextCurrent ( ((SwSysWin*)win)->gwin );
}

int wsi_num_windows ()
{
	GS_TRACE2 ( "wsi_num_windows..." );
	return AppNumVisWindows;
}

void* wsi_get_ogl_procedure ( const char *name )
{
	GS_TRACE4 ( "wsi_get_ogl_procedure..." );
	void* pt = (void *)glfwGetProcAddress(name);
	return pt;
}

int wsi_check ()
{
	glfwPollEvents();
	return AppNumVisWindows;
}

void wsi_screen_resolution ( int& w, int& h )
{
	const GLFWvidmode* m = glfwGetVideoMode ( glfwGetPrimaryMonitor() );
	w = m->width;
    h = m->height;
}

//==== Callbacks ==============================================================================

// The following are inline friend functions of WsWindow:
inline void sysdraw ( WsWindow* win ) { win->draw(win->_glrenderer); }
inline void sysinit ( WsWindow* win, int w, int h ) { win->init(win->_glcontext,w,h); }
inline void sysresize ( WsWindow* win, int w, int h ) { win->resize(win->_glcontext,w,h); }
inline void sysevent ( WsWindow* win, GsEvent& e ) { win->handle(e); }

static void draw_cb ( GLFWwindow* gwin )
{
	GS_TRACE2 ( "draw_cb..." );
	glfwMakeContextCurrent ( gwin );
	SwSysWin* sw = (SwSysWin*)glfwGetWindowUserPointer(gwin);
	if (sw->needsinit)
	{	int w, h;
		glfwGetWindowSize ( sw->gwin, &w, &h );
		sysinit ( sw->swin, w, h );
		sw->needsinit=0;
	}
	sysdraw ( sw->swin ); // this will call the user's window draw method
	glfwSwapBuffers ( gwin );
	sw->redrawcalled=0;
}

void wsi_win_redraw ( void* win )
{
	GS_TRACE2 ( "wsi_win_redraw..." );
	SwSysWin* sw = (SwSysWin*)win;
	draw_cb(sw->gwin);
}

void resize_cb ( GLFWwindow* gwin, int w, int h )
{
	SwSysWin* sw = (SwSysWin*)glfwGetWindowUserPointer(gwin);
	WsWindow* swin = sw->swin;
	glfwMakeContextCurrent ( gwin );
	sysresize ( swin, w, h );
}

//==== Event Callbacks ==============================================================================

static void setmodifs ( GsEvent& e, int modifs )
{
	e.alt = modifs&GLFW_MOD_ALT? 1:0;
	e.ctrl = modifs&GLFW_MOD_CONTROL? 1:0;
	e.shift = modifs&GLFW_MOD_SHIFT? 1:0;
}

static void setkeycode ( GsEvent& e, int key, int modifs )
{
	//const gsbyte NS[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*', '(' };

	setmodifs ( e, modifs );
	if ( key<=162 ) e.key=key;

	if ( key>='A' && key<='Z') { e.character=(gsbyte)(e.shift?key:key-'A'+'a'); return; }

	e.character = 0;

	# define RET(x) e.key=GsEvent::x; return
	# define RET1(x) e.key=x; e.character=(gsbyte)x; return
	# define RET2(x,y) e.key=x; e.character=(gsbyte)(e.shift?y:x); return
	switch (key)
	{	case GLFW_KEY_LEFT	: RET(KeyLeft); case GLFW_KEY_RIGHT	: RET(KeyRight);
		case GLFW_KEY_UP	: RET(KeyUp);	case GLFW_KEY_DOWN	: RET(KeyDown);
		case GLFW_KEY_ESCAPE: RET(KeyEsc);	case GLFW_KEY_BACKSPACE	: RET(KeyBack);
		case GLFW_KEY_HOME	: RET(KeyHome); case GLFW_KEY_END	: RET(KeyEnd);
		case GLFW_KEY_TAB	: RET1('\t');	case GLFW_KEY_ENTER	: RET(KeyEnter);
		case GLFW_KEY_INSERT: RET(KeyIns);	case GLFW_KEY_DELETE: RET(KeyDel);
		case GLFW_KEY_PAGE_UP: RET(KeyPgUp); case GLFW_KEY_PAGE_DOWN: RET(KeyPgDn);

		case GLFW_KEY_KP_DIVIDE	: RET1('/'); case GLFW_KEY_KP_MULTIPLY: RET1('*');
		case GLFW_KEY_KP_ADD	: RET1('+'); case GLFW_KEY_KP_SUBTRACT: RET1('-');
		case 192: RET2('`','~'); case 189: RET2('-','_');  case 187: RET2('=','+');
		case 219: RET2('[','{'); case 221: RET2(']','}');  case 220: RET2('\\','|');
		case 186: RET2(';',':'); case 222: RET2('\'','"'); case 188: RET2(',','<');
		case 190: RET2('.','>'); case 191: RET2('/','?');
	}
	if ( key>=GLFW_KEY_F1 && key<=GLFW_KEY_F12 ) e.key=GsEvent::KeyF1+(key-GLFW_KEY_F1);
}

static void key_cb ( GLFWwindow* gwin, int key, int scancode, int action, int modifs )
{
	SwSysWin* sw = (SwSysWin*)glfwGetWindowUserPointer(gwin);
	GsEvent& e = sw->event;
	if ( key>=GLFW_KEY_LEFT_SHIFT && action==GLFW_REPEAT ) return; // do not send repeated modifier event
	e.type = action==GLFW_RELEASE? GsEvent::KeyRelease : GsEvent::Keyboard;
	e.wheelclicks = 0; 
	e.button = 0;
	setkeycode ( e, key, modifs );
	sysevent ( sw->swin, e );
}

static void setmouseev ( GLFWwindow* gwin, GsEvent& e, GsEvent::Type t )
{
	e.type = t;
	e.button = 0;
	e.wheelclicks = 0; 
	e.key = 0;
	glfwGetFramebufferSize ( gwin, &e.width, &e.height ); // alternative: glfwGetWindowSize
	e.button1 = glfwGetMouseButton(gwin,GLFW_MOUSE_BUTTON_LEFT);
	e.button2 = glfwGetMouseButton(gwin,GLFW_MOUSE_BUTTON_MIDDLE);
	e.button3 = glfwGetMouseButton(gwin,GLFW_MOUSE_BUTTON_RIGHT);
}

static void setmousepos ( GsEvent& e, gsint16 x, gsint16 y )
{
	e.lmousex=e.mousex;	e.lmousey=e.mousey;
	e.mousex=x;	e.mousey=y;
}

static void mouse_cb ( GLFWwindow* gwin, int but, int action, int modifs )
{
	SwSysWin* sw = (SwSysWin*)glfwGetWindowUserPointer(gwin);
	GsEvent& e = sw->event;
	setmouseev ( gwin, e, action==GLFW_PRESS? GsEvent::Push : GsEvent::Release );
	setmodifs ( e, modifs );
	e.button = but==GLFW_MOUSE_BUTTON_LEFT? 1 : but==GLFW_MOUSE_BUTTON_RIGHT? 3 : 2;
	double x, y;
	glfwGetCursorPos ( gwin, &x, &y );
	setmousepos ( e, (gsint16)x, (gsint16)y );
	sysevent ( sw->swin, e );
}

static void mousepos_cb ( GLFWwindow* gwin, double x, double y )
{
	SwSysWin* sw = (SwSysWin*)glfwGetWindowUserPointer(gwin);
	GsEvent& e = sw->event;
	setmouseev ( gwin, e, GsEvent::Move );
	if ( e.button1 || e.button3 ) e.type=GsEvent::Drag;
	setmousepos ( e, (gsint16)x, (gsint16)y );
	sysevent ( sw->swin, e );
}

static void close_cb ( GLFWwindow* gwin )
{
	SwSysWin* sw = (SwSysWin*)glfwGetWindowUserPointer(gwin);
	delete sw;
}

static void _init_callbacks ( GLFWwindow* gwin )
{
	glfwSetKeyCallback ( gwin, key_cb );
	glfwSetWindowRefreshCallback ( gwin, draw_cb );
	glfwSetWindowSizeCallback ( gwin, resize_cb );
	glfwSetMouseButtonCallback ( gwin, mouse_cb );
	glfwSetCursorPosCallback ( gwin, mousepos_cb );
	glfwSetWindowCloseCallback ( gwin, close_cb );

//TO LATER CALL:	glfwTerminate();

//
//GLFWAPI GLFWcharfun glfwSetCharCallback(GLFWwindow* window, GLFWcharfun cbfun);
//GLFWAPI GLFWcharmodsfun glfwSetCharModsCallback(GLFWwindow* window, GLFWcharmodsfun cbfun);
//
//GLFWAPI GLFWmonitorfun glfwSetMonitorCallback(GLFWmonitorfun cbfun);
//GLFWAPI GLFWwindowposfun glfwSetWindowPosCallback(GLFWwindow* window, GLFWwindowposfun cbfun);
//GLFWAPI GLFWwindowfocusfun glfwSetWindowFocusCallback(GLFWwindow* window, GLFWwindowfocusfun cbfun);
//GLFWAPI GLFWwindowiconifyfun glfwSetWindowIconifyCallback(GLFWwindow* window, GLFWwindowiconifyfun cbfun);
//GLFWAPI GLFWframebuffersizefun glfwSetFramebufferSizeCallback(GLFWwindow* window, GLFWframebuffersizefun cbfun);
//GLFWAPI GLFWcursorposfun glfwSetCursorPosCallback(GLFWwindow* window, GLFWcursorposfun cbfun);
//GLFWAPI GLFWcursorenterfun glfwSetCursorEnterCallback(GLFWwindow* window, GLFWcursorenterfun cbfun);
//GLFWAPI GLFWscrollfun glfwSetScrollCallback(GLFWwindow* window, GLFWscrollfun cbfun);
//GLFWAPI GLFWdropfun glfwSetDropCallback(GLFWwindow* window, GLFWdropfun cbfun);
//GLFWAPI GLFWjoystickfun glfwSetJoystickCallback(GLFWjoystickfun cbfun);
//
//GLFWAPI const char* glfwGetKeyName(int key, int scancode);
//GLFWAPI int glfwGetKey(GLFWwindow* window, int key);
//GLFWAPI int glfwGetMouseButton(GLFWwindow* window, int button);
//GLFWAPI void glfwGetCursorPos(GLFWwindow* window, double* xpos, double* ypos);
//

}


//
//static void setstate ( GsEvent& e )
//{
//	e.shift = GetKeyState(VK_SHIFT)&128? 1:0;
//	e.ctrl  = GetKeyState(VK_CONTROL)&128? 1:0;
//	e.alt   = GetKeyState(VK_MENU)&128? 1:0;
//	e.button1 = GetKeyState(VK_LBUTTON)&128? 1:0;
//	e.button2 = GetKeyState(VK_MBUTTON)&128? 1:0;
//	e.button3 = GetKeyState(VK_RBUTTON)&128? 1:0;
//}
//
//// this makes sure the client size is the specified one (and not the window size)
//static void fixsize ( SwSysWin* sw )
//{
//	RECT c;
//	GetClientRect ( sw->gwin, &c );
//	if ( c.right==sw->event.width && c.bottom==sw->event.height ) return;
//	GS_TRACE1 ( "FIXING SIZE..." );
//	RECT w;
//	GetWindowRect ( sw->gwin, &w );
//	GS_TRACE1 ( "Client: "<<c.right<<"x"<<c.bottom );
//	GS_TRACE1 ( "Window: "<<(w.bottom-w.top-1)<<"x"<<(w.right-w.left-1) );
//	int dw = (w.right-w.left)-c.right;
//	int dh = (w.bottom-w.top)-c.bottom;
//	int nw = sw->event.width + dw;
//	int nh = sw->event.height + dh;
//	MoveWindow ( sw->gwin, w.left, w.top, nw, nh, TRUE );
//}
//



//static LRESULT CALLBACK WndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
//{
//	GsEvent& e = sw->event;
//	e.type = GsEvent::None;
//	e.wheelclicks = 0;
//	e.button = 0;
//	e.key = 0;
//
//	switch(uMsg) // process other events
//	{
//		case WM_KEYDOWN: // (we do not read WM_CHAR events)
//		case WM_SYSKEYDOWN:
//			GS_TRACE5 ( "WM_KEYDOWN..." );
//			setstate ( e );
//			setkeycode ( e, wParam );
//			if ( e.key==0 ) return DefWindowProc ( hWnd, uMsg, wParam, lParam );
//			e.type = GsEvent::Keyboard;
//			break;
//
//		case WM_KEYUP  :
//		case WM_SYSKEYUP:
//			setstate ( e ); // update state in event
//			return DefWindowProc ( hWnd, uMsg, wParam, lParam ); // not currently in use
//			break;
//
//		// we call setstate below to have correct events in all cases with multiple windows
//		# define SETLMOUSE e.lmousex=e.mousex; e.lmousey=e.mousey; e.mousex=LOWORD(lParam); e.mousey=HIWORD(lParam);
//		# define MOUSEEV(t,i,but,bs) e.type=t; e.button=i; e.but=bs; SETLMOUSE; setstate(e)
//		case WM_LBUTTONDOWN :
//			GS_TRACE5 ( "WM_LBUTTONDOWN..." );
//			MOUSEEV ( GsEvent::Push, 1, button1, 1 );
//			break;
//		case WM_MBUTTONDOWN :
//			GS_TRACE5 ( "WM_MBUTTONDOWN..." );
//			MOUSEEV ( GsEvent::Push, 2, button2, 1 );
//			break;
//		case WM_RBUTTONDOWN :
//			GS_TRACE5 ( "WM_RBUTTONDOWN..." );
//			MOUSEEV ( GsEvent::Push, 3, button3, 1 );
//			break;
//		case WM_LBUTTONUP :
//			GS_TRACE5 ( "WM_LBUTTONUP..." );
//			MOUSEEV ( GsEvent::Release, 1, button1, 0 );
//			break;
//		case WM_MBUTTONUP :
//			GS_TRACE5 ( "WM_MBUTTONUP..." );
//			MOUSEEV ( GsEvent::Release, 2, button2, 0 );
//			break;
//		case WM_RBUTTONUP :
//			GS_TRACE5 ( "WM_RBUTTONUP..." );
//			MOUSEEV ( GsEvent::Release, 3, button3, 0 );
//			break;
//		case WM_MOUSEMOVE :
//			GS_TRACE7 ( "WM_MOUSEMOVE..." );
//			setstate(e);
//			e.type = e.button1||e.button3? GsEvent::Drag : GsEvent::Move;
//			SETLMOUSE;
//			if ( GetFocus()!=hWnd ) SetFocus ( hWnd );
//			break;
//		# undef MOUSEEV
//		# undef SETLMOUSE
//
//		case WM_MOUSEWHEEL:
//			GS_TRACE5 ( "WM_MOUSEWHEEL..." );
//			e.type = GsEvent::Wheel;
//			e.wheelclicks = GET_WHEEL_DELTA_WPARAM(wParam);
//			break;
//
//		case WM_SIZE :
//			GS_TRACE5 ( "WM_SIZE "<<sw->event.width<<"x"<<sw->event.height<<" to "<<LOWORD(lParam)<<"x"<<HIWORD(lParam)<<"..." );
//			if (sw->justresized)
//			{	sw->justresized=false; fixsize(sw);
//				RECT r; GetClientRect ( sw->gwin, &r ); // get client rect in case size was adjusted
//				e.width = r.right; // if not adjusted could use LOWORD(lParam)
//				e.height = r.bottom; // if not adjusted could use HIWORD(lParam)
//			}
//			else
//			{	e.width = LOWORD(lParam);
//				e.height = HIWORD(lParam);
//			}
//			if ( e.width==0 || e.height==0 ) return 0; // window is being minimized
//			// Only signal user's window if the window and OpenGL are initialized:
//			if ( !sw->needsinit )
//			{	wglMakeCurrent ( sw->gldevcontext, sw->glrendcontext );
//				sysresize ( sw->swin, e.width, e.height );
//				wglMakeCurrent ( NULL, NULL );
//			}
//			InvalidateRect ( sw->gwin, NULL, TRUE ); // force paint since a shrink will not do it
//			return 0;
//
//		case WM_DESTROY :
//			GS_TRACE5 ( "WM_DESTROY ["<<sw->label<<"]..." );
//			PostQuitMessage ( 0 );
//			if ( AppWindows.size()>1 ) // Do not delete last OpenGL Context because all shared resources would be lost
//			{	wglDeleteContext ( sw->glrendcontext ); sw->glrendcontext=NULL; }
//			ReleaseDC ( hWnd, sw->gldevcontext ); sw->gldevcontext=NULL;
//			delete sw;
//			return 0;
//
//		case WM_SYSCOMMAND:
//			switch ( wParam & 0xFFF0 ) // system commands that we want to ignore are detected here
//			{	case SC_KEYMENU: case SC_SCREENSAVE: return 0; // ignore
//				default: return DefWindowProc ( hWnd, uMsg, wParam, lParam ); // process the others
//			}
//			break;
//
//		default:
//			#ifdef GS_USE_TRACE6
//			if ( uMsg==WM_NCMOUSELEAVE ) gsout<<"WM_NCMOUSELEAVE\n";
//			gsout.putf("Event not handled: %0x\n",uMsg);
//			#endif
//			return DefWindowProc ( hWnd, uMsg, wParam, lParam );
//	}
//
//	sysevent ( sw->swin, e );
//	return 0;
//}

//=============================== native file dialogs ==========================================

# define FILEBUFSIZE 256
static GsString FileBuf;

// filter format:  "*.txt;*.log"
static const char* filedlg ( bool open, const char* msg, const char* file, const char* filter, GsArray<const char*>* multif )
{
	return 0;
 }

const char* wsi_open_file_dialog ( const char* msg, const char* file, const char* filter, GsArray<const char*>* multif )
{
	return filedlg ( true, msg, file, filter, multif );
}

const char* wsi_save_file_dialog ( const char* msg, const char* file, const char* filter )
{
	return filedlg ( false, msg, file, filter, 0 );
}

const char* wsi_select_folder ( const char* msg, const char* folder )
{
	return 0;
}

//=============================== misc windows functions ==========================================

// Main Function Arguments:
static GsBuffer<char*> AppArgs;

char** wsi_program_argv ()
{
	return &AppArgs[0];
}

int wsi_program_argc ()
{
	return AppArgs.size()-1;
}

// the main function entry point must be provided elsewhere:
extern int main ( int, char** );

# ifdef GS_WINDOWS
int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	GS_TRACE1 ( "WinMain..." );
	//AppInstance = hInstance;
	//AppPrevInstance = hPrevInstance;

	// Converting command line arguments to argv format:
	char* pt = GetCommandLine();
	if ( *pt=='"' ) pt++; // executable name may come between quotes, remove first one here
	while ( true )
	{	AppArgs.push(pt);
		while ( *pt && *pt!=' ' ) pt++; // skip argument
		if ( AppArgs.size()==1 && *(pt-1)=='"' ) *(pt-1)=0; // remove end quote in executable name
		if ( *pt==0 ) break;
		*pt++ = 0; // mark end of argument
		while ( *pt && *pt==' ' ) pt++; // skip any extra spaces
		if ( *pt==0 ) break;
	}
	AppArgs.push(0); // for compatibility last argv should be null

	// call the user-provided main:
	return main ( AppArgs.size()-1, &AppArgs[0] );

	// Reminder: SIG config file loading and OpenGL initialization are performed
	// automatically at the time of opening the first graphical window.
}
# endif



# endif // GS_LINUX

//================================ End of File =================================================

