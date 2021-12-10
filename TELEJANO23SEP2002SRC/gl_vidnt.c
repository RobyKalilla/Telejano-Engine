/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// gl_vidnt.c -- NT GL vid component

#include "quakedef.h"
#include "winquake.h"
#include "resource.h"
#include <commctrl.h>

#define MAX_MODE_LIST	30
#define VID_ROW_SIZE	3
#define WARP_WIDTH		320
#define WARP_HEIGHT		200
#define MAXWIDTH		10000
#define MAXHEIGHT		10000
#define BASEWIDTH		320
#define BASEHEIGHT		200

#define MODE_WINDOWED			0
#define NO_MODE					(MODE_WINDOWED - 1)
#define MODE_FULLSCREEN_DEFAULT	(MODE_WINDOWED + 1)

typedef struct 
{
	modestate_t	type;
	int			width;
	int			height;
	int			modenum;
	int			dib;
	int			fullscreen;
	int			bpp;
	int			halfscreen;
	char		modedesc[17];
} vmode_t;

typedef struct 
{
	int			width;
	int			height;
} lmode_t;

lmode_t	lowresmodes[] = 
{
	{320, 200},
	{320, 240},
	{400, 300},
	{512, 384},
};

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;

qboolean		DDActive;
qboolean		scr_skipupdate;

static vmode_t	modelist[MAX_MODE_LIST];
static int		nummodes;
static vmode_t	*pcurrentmode;
static vmode_t	badmode;

static DEVMODE	gdevmode;
static qboolean	vid_initialized = false;
static qboolean	windowed, leavecurrentmode;
static qboolean vid_canalttab = false;
static qboolean vid_wassuspended = false;
static int		windowed_mouse;
extern qboolean	mouseactive;  // from in_win.c
static HICON	hIcon;

int			DIBWidth, DIBHeight;
RECT		WindowRect;
DWORD		WindowStyle, ExWindowStyle;

HWND	mainwindow, dibwindow;

int			vid_modenum = NO_MODE;
int			vid_default = MODE_WINDOWED;
static int	windowed_default;
unsigned char	vid_curpal[256*3];

static float vid_gamma = 1.0;

HGLRC	baseRC;
HDC		maindc;

glvert_t glv;

HWND WINAPI InitializeWindow (HINSTANCE hInstance, int nCmdShow);

viddef_t	vid;				// global video state

unsigned short	d_8to16table[256];
unsigned		d_8to24table[256];
unsigned char	d_15to8table[65536];

modestate_t	modestate = MS_UNINIT;

LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AppActivate(BOOL fActive, BOOL minimize);
char *VID_GetModeDescription (int mode);
void ClearAllStates (void);
void VID_UpdateWindowStatus (void);
void GL_Init (void);

PROC glArrayElementEXT;
PROC glColorPointerEXT;
PROC glTexCoordPointerEXT;
PROC glVertexPointerEXT;

float		gldepthmin, gldepthmax;

qboolean gl_mtexable		= false;
qboolean console_enabled	= true; // Tomaz - Console Toggle
//====================================

cvar_t		vid_mode = {"vid_mode","0", false};
// Note that 0 is MODE_WINDOWED
cvar_t		_vid_default_mode = {"_vid_default_mode","0", true};
// Note that 3 is MODE_FULLSCREEN_DEFAULT
cvar_t		_vid_default_mode_win = {"_vid_default_mode_win","3", true};
cvar_t		vid_wait = {"vid_wait","0"};
cvar_t		vid_nopageflip = {"vid_nopageflip","0", true};
cvar_t		_vid_wait_override = {"_vid_wait_override", "0", true};
cvar_t		vid_config_x = {"vid_config_x","800", true};//8
cvar_t		vid_config_y = {"vid_config_y","600", true};//6
cvar_t		vid_stretch_by_2 = {"vid_stretch_by_2","1", true};
cvar_t		_windowed_mouse = {"_windowed_mouse","1", true};

int			window_center_x, window_center_y, window_x, window_y, window_width, window_height;
RECT		window_rect;

int MHz,CPUfamily,Mem,InstCache,DataCache,L2Cache;
char VendorID[16];
boolean SupportCMOVs,Support3DNow,Support3DNowExt,SupportMMX,SupportMMXext,SupportSSE;


#pragma warning(disable: 4035)
__inline __int64 GetCycleNumber()
{
	__asm 
	{
		RDTSC
	}
}

void CenterWindow(HWND hWndCenter, int width, int height, BOOL lefttopjustify)
{
    int     CenterX, CenterY;

	CenterX = (GetSystemMetrics(SM_CXSCREEN) - width) * 0.5;	// Tomaz - Speed
	CenterY = (GetSystemMetrics(SM_CYSCREEN) - height) * 0.5;	// Tomaz - Speed
	if (CenterX > CenterY*2)
		CenterX >>= 1;	// dual screens
	CenterX = (CenterX < 0) ? 0: CenterX;
	CenterY = (CenterY < 0) ? 0: CenterY;
	SetWindowPos (hWndCenter, NULL, CenterX, CenterY, 0, 0,	SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);
}

qboolean VID_SetWindowedMode (int modenum)
{
	HDC				hdc;
	int				lastmodestate, width, height;
	RECT			rect;

	lastmodestate		= modestate;

	WindowRect.top		= 0;
	WindowRect.left		= 0;
	WindowRect.right	= modelist[modenum].width;
	WindowRect.bottom	= modelist[modenum].height;

	DIBWidth = modelist[modenum].width;
	DIBHeight = modelist[modenum].height;

	WindowStyle = WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU |
				  WS_MINIMIZEBOX;
	ExWindowStyle = 0;

	rect = WindowRect;
	AdjustWindowRectEx(&rect, WindowStyle, FALSE, 0);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	// Create the DIB window
	dibwindow = CreateWindowEx (
		 ExWindowStyle,
		 "TeiQuake",
		 "TeiQuake",
		 WindowStyle,
		 rect.left, rect.top,
		 width,
		 height,
		 NULL,
		 NULL,
		 global_hInstance,
		 NULL);

	if (!dibwindow)
		Sys_Error ("Couldn't create DIB window");

	// Center and show the DIB window
	CenterWindow(dibwindow, WindowRect.right - WindowRect.left,
				 WindowRect.bottom - WindowRect.top, false);

	ShowWindow (dibwindow, SW_SHOWDEFAULT);
	UpdateWindow (dibwindow);

	modestate = MS_WINDOWED;

// because we have set the background brush for the window to NULL
// (to avoid flickering when re-sizing the window on the desktop),
// we clear the window to black when created, otherwise it will be
// empty while Quake starts up.
	hdc = GetDC(dibwindow);
	PatBlt(hdc,0,0,WindowRect.right,WindowRect.bottom,BLACKNESS);
	ReleaseDC(dibwindow, hdc);

	if ((signed)vid.conheight > modelist[modenum].height)
		vid.conheight = modelist[modenum].height;
	if ((signed)vid.conwidth > modelist[modenum].width)
		vid.conwidth = modelist[modenum].width;
	vid.width = vid.conwidth;
	vid.height = vid.conheight;

	vid.numpages = 2;

	mainwindow = dibwindow;

	SendMessage (mainwindow, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
	SendMessage (mainwindow, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);

	return true;
}

qboolean VID_SetFullDIBMode (int modenum)
{
	HDC				hdc;
	int				lastmodestate, width, height;
	RECT			rect;

	if (!leavecurrentmode)
	{
		gdevmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		gdevmode.dmBitsPerPel = modelist[modenum].bpp;
		gdevmode.dmPelsWidth = modelist[modenum].width <<
							   modelist[modenum].halfscreen;
		gdevmode.dmPelsHeight = modelist[modenum].height;
		gdevmode.dmSize = sizeof (gdevmode);

		if (ChangeDisplaySettings (&gdevmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			Sys_Error ("Couldn't set fullscreen DIB mode");
	}

	lastmodestate = modestate;
	modestate = MS_FULLDIB;

	WindowRect.top = WindowRect.left = 0;

	WindowRect.right = modelist[modenum].width;
	WindowRect.bottom = modelist[modenum].height;

	DIBWidth = modelist[modenum].width;
	DIBHeight = modelist[modenum].height;

	WindowStyle = WS_POPUP;
	ExWindowStyle = 0;

	rect = WindowRect;
	AdjustWindowRectEx(&rect, WindowStyle, FALSE, 0);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	// Create the DIB window
	dibwindow = CreateWindowEx (
		 ExWindowStyle,
		 //"TomazQuake",
		 "Telejano",
		 //"TomazQuake",
		 "Telejano",
		 WindowStyle,
		 rect.left, rect.top,
		 width,
		 height,
		 NULL,
		 NULL,
		 global_hInstance,
		 NULL);

	if (!dibwindow)
		Sys_Error ("Couldn't create DIB window");

	ShowWindow (dibwindow, SW_SHOWDEFAULT);
	UpdateWindow (dibwindow);

	// Because we have set the background brush for the window to NULL
	// (to avoid flickering when re-sizing the window on the desktop), we
	// clear the window to black when created, otherwise it will be
	// empty while Quake starts up.
	hdc = GetDC(dibwindow);
	PatBlt(hdc,0,0,WindowRect.right,WindowRect.bottom,BLACKNESS);
	ReleaseDC(dibwindow, hdc);

	if ((signed)vid.conheight > modelist[modenum].height)
		vid.conheight = modelist[modenum].height;
	if ((signed)vid.conwidth > modelist[modenum].width)
		vid.conwidth = modelist[modenum].width;
	vid.width = vid.conwidth;
	vid.height = vid.conheight;

	vid.numpages = 2;

// needed because we're not getting WM_MOVE messages fullscreen on NT
	window_x = 0;
	window_y = 0;

	mainwindow = dibwindow;

	SendMessage (mainwindow, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
	SendMessage (mainwindow, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);

	return true;
}


int VID_SetMode (int modenum, unsigned char *palette)
{
	int				original_mode, temp;
	qboolean		stat;
    MSG				msg;

	if ((windowed && (modenum != 0)) ||
		(!windowed && (modenum < 1)) ||
		(!windowed && (modenum >= nummodes)))
	{
		Sys_Error ("Bad video mode\n");
	}

// so Con_Printfs don't mess us up by forcing vid and snd updates
	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;

//	S_PauseMusic();

	if (vid_modenum == NO_MODE)
		original_mode = windowed_default;
	else
		original_mode = vid_modenum;

	// Set either the fullscreen or windowed mode
	if (modelist[modenum].type == MS_WINDOWED)
	{
		if (_windowed_mouse.value && key_dest == key_game)
		{
			stat = VID_SetWindowedMode(modenum);
			IN_ActivateMouse ();
			IN_HideMouse ();
		}
		else
		{
			IN_DeactivateMouse ();
			IN_ShowMouse ();
			stat = VID_SetWindowedMode(modenum);
		}
	}
	else if (modelist[modenum].type == MS_FULLDIB)
	{
		stat = VID_SetFullDIBMode(modenum);
		IN_ActivateMouse ();
		IN_HideMouse ();
	}
	else
	{
		Sys_Error ("VID_SetMode: Bad mode type in modelist");
	}

	window_width = DIBWidth;
	window_height = DIBHeight;
	VID_UpdateWindowStatus ();

//	S_ResumeMusic();
	scr_disabled_for_loading = temp;

	if (!stat)
	{
		Sys_Error ("Couldn't set video mode");
	}

// now we try to make sure we get the focus on the mode switch, because
// sometimes in some systems we don't.  We grab the foreground, then
// finish setting up, pump all our messages, and sleep for a little while
// to let messages finish bouncing around the system, then we put
// ourselves at the top of the z order, then grab the foreground again,
// Who knows if it helps, but it probably doesn't hurt
	SetForegroundWindow (mainwindow);
	VID_SetPalette (palette);
	vid_modenum = modenum;
	Cvar_SetValue ("vid_mode", (float)vid_modenum);

	while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
	{
      	TranslateMessage (&msg);
      	DispatchMessage (&msg);
	}

	Sleep (100);

	SetWindowPos (mainwindow, HWND_TOP, 0, 0, 0, 0,
				  SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW |
				  SWP_NOCOPYBITS);

	SetForegroundWindow (mainwindow);

// fix the leftover Alt from any Alt-Tab or the like that switched us away
	ClearAllStates ();

	if (!msg_suppress_1)
		Con_SafePrintf ("Video mode %s initialized.\n", VID_GetModeDescription (vid_modenum));

	VID_SetPalette (palette);

	vid.recalc_refdef = 1;

	return true;
}


/*
================
VID_UpdateWindowStatus
================
*/
void VID_UpdateWindowStatus (void)
{

	window_rect.left = window_x;
	window_rect.top = window_y;
	window_rect.right = window_x + window_width;
	window_rect.bottom = window_y + window_height;
	window_center_x = (window_rect.left + window_rect.right) * 0.5;	// Tomaz - Speed
	window_center_y = (window_rect.top + window_rect.bottom) * 0.5;	// Tomaz - Speed

	IN_UpdateClipCursor ();
}


//====================================

BINDTEXFUNCPTR bindTexFunc;

#define TEXTURE_EXT_STRING "GL_EXT_texture_object"


void CheckTextureExtensions (void)
{
	char		*tmp;
	qboolean	texture_ext;
	HINSTANCE	hInstGL;

	texture_ext = FALSE;
	/* check for texture extension */
	tmp = (unsigned char *)glGetString(GL_EXTENSIONS);
	while (*tmp)
	{
		if (strncmp((const char*)tmp, TEXTURE_EXT_STRING, strlen(TEXTURE_EXT_STRING)) == 0)
			texture_ext = TRUE;
		tmp++;
	}

	if (!texture_ext || COM_CheckParm ("-gl11") )
	{
		hInstGL = LoadLibrary("opengl32.dll");
		
		if (hInstGL == NULL)
			Sys_Error ("Couldn't load opengl32.dll\n");

		bindTexFunc = (void *)GetProcAddress(hInstGL,"glBindTexture");

		if (!bindTexFunc)
			Sys_Error ("No texture objects!");
		return;
	}

/* load library and get procedure adresses for texture extension API */
	if ((bindTexFunc = (BINDTEXFUNCPTR)
		wglGetProcAddress((LPCSTR) "glBindTextureEXT")) == NULL)
	{
		Sys_Error ("GetProcAddress for BindTextureEXT failed");
		return;
	}
}

void CheckArrayExtensions (void)
{
	char		*tmp;

	/* check for texture extension */
	tmp = (unsigned char *)glGetString(GL_EXTENSIONS);
	while (*tmp)
	{
		if (strncmp((const char*)tmp, "GL_EXT_vertex_array", strlen("GL_EXT_vertex_array")) == 0)
		{
			if (((glArrayElementEXT		= wglGetProcAddress("glArrayElementEXT"))		== NULL) ||
				((glColorPointerEXT		= wglGetProcAddress("glColorPointerEXT"))		== NULL) ||
				((glTexCoordPointerEXT	= wglGetProcAddress("glTexCoordPointerEXT"))	== NULL) ||
				((glVertexPointerEXT	= wglGetProcAddress("glVertexPointerEXT"))		== NULL) )
			{
				Sys_Error ("GetProcAddress for vertex extension failed");
				return;
			}
			return;
		}
		tmp++;
	}

	Sys_Error ("Vertex array extension not present");
}

int		texture_mode = GL_LINEAR;

int		texture_extension_number = 1;

GLenum TEXTURE0_SGIS_ARB;
GLenum TEXTURE1_SGIS_ARB;

void CheckMultiTextureExtensions(void) 
{
	if (COM_CheckParm("-nomtex")) 
	{
		Con_Printf("&f9001 Multitexture Disabled\n\n &r");
		return;
	}

	if (strstr(gl_extensions, "GL_ARB_multitexture ")) 
	{
		Con_Printf("&f9001 GL_ARB_multitexture enabled &r \n\n");
		qglMTexCoord2fSGIS_ARB		= (void *) wglGetProcAddress("glMultiTexCoord2fARB");
		qglSelectTextureSGIS_ARB	= (void *) wglGetProcAddress("glActiveTextureARB");
		gl_mtexable = true;
		TEXTURE0_SGIS_ARB = TEXTURE0_ARB;
		TEXTURE1_SGIS_ARB = TEXTURE1_ARB;	
		return;
	} 
	
	if (strstr(gl_extensions, "GL_SGIS_multitexture ")) 
	{
		Con_Printf("&f9001 GL_SGIS_multitexture enabled\n\n &r");
		qglMTexCoord2fSGIS_ARB		= (void *) wglGetProcAddress("glMTexCoord2fSGIS");
		qglSelectTextureSGIS_ARB	= (void *) wglGetProcAddress("glSelectTextureSGIS");
		gl_mtexable = true;
		TEXTURE0_SGIS_ARB = TEXTURE0_SGIS;
		TEXTURE1_SGIS_ARB = TEXTURE1_SGIS;
		return;
	}
	Con_Printf("\n&f9001 Multitexture Not Found\n &r");
}

void GetMHz(void)
{
	LARGE_INTEGER t1,t2,tf;
	__int64 c1,c2;

	QueryPerformanceFrequency(&tf);
	QueryPerformanceCounter(&t1);
	c1 = GetCycleNumber();
	_asm {
		MOV  EBX, 5000000
		WaitAlittle:
			DEC		EBX
		JNZ	WaitAlittle
	}
	QueryPerformanceCounter(&t2);
	c2 = GetCycleNumber();
	
	Con_Printf("\nCPU Speed: %d MHz\n", (int) ((c2 - c1) * tf.QuadPart / (t2.QuadPart - t1.QuadPart) / 1000000));
}

void GetSysInfo(){
	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	Mem = ms.dwTotalPhys >> 10;

	__asm {
		PUSHFD
		POP		EAX
		MOV		EBX, EAX
		XOR		EAX, 00200000h
		PUSH	EAX
		POPFD
		PUSHFD
		POP		EAX
		CMP		EAX, EBX
		JZ		ExitCpuTest

			XOR		EAX, EAX
			CPUID

			MOV		DWORD PTR [VendorID],		EBX
			MOV		DWORD PTR [VendorID + 4],	EDX
			MOV		DWORD PTR [VendorID + 8],	ECX
			MOV		DWORD PTR [VendorID + 12],	0

			MOV		EAX, 1
			CPUID
			TEST	EDX, 0x00008000
			SETNZ	AL
			MOV		SupportCMOVs, AL
			TEST	EDX, 0x00800000
			SETNZ	AL
			MOV		SupportMMX, AL
	
			TEST	EDX, 0x02000000
			SETNZ	AL
			MOV		SupportSSE, AL

			SHR		EAX, 8
			AND		EAX, 0x0000000F
			MOV		CPUfamily, EAX
	
			MOV		MHz, 0
			TEST	EDX, 0x00000008
			JZ		NoTimeStampCounter
				CALL	GetMHz
				MOV		MHz, EAX
			NoTimeStampCounter:

			MOV		Support3DNow, 0
			MOV		EAX, 80000000h
			CPUID
			CMP		EAX, 80000000h
			JBE		NoExtendedFunction
				MOV		EAX, 80000001h
				CPUID
				TEST	EDX, 80000000h
				SETNZ	AL
				MOV		Support3DNow, AL

				TEST	EDX, 40000000h
				SETNZ	AL
				MOV		Support3DNowExt, AL

				TEST	EDX, 0x00400000
				SETNZ	AL
				MOV		SupportMMXext, AL

				MOV		EAX, 80000005h
				CPUID
				SHR		ECX, 24
				MOV		DataCache, ECX
				SHR		EDX, 24
				MOV		InstCache, EDX
				
				MOV		EAX, 80000006h
				CPUID
				SHR		ECX, 16
				MOV		L2Cache, ECX

				
			JMP		ExitCpuTest

			NoExtendedFunction:
			MOV		EAX, 2
			CPUID

			MOV		ESI, 4
			TestCache:
				CMP		DL, 0x40
				JNA		NotL2
					MOV		CL, DL
					SUB		CL, 0x40
					SETZ	CH
					DEC		CH
					AND		CL, CH
					MOV		EBX, 64
					SHL		EBX, CL
					MOV		L2Cache, EBX
				NotL2:
				CMP		DL, 0x06
				JNE		Next1
					MOV		InstCache, 8
				Next1:
				CMP		DL, 0x08
				JNE		Next2
					MOV		InstCache, 16
				Next2:
				CMP		DL, 0x0A
				JNE		Next3
					MOV		DataCache, 8
				Next3:
				CMP		DL, 0x0C
				JNE		Next4
					MOV		DataCache, 16
				Next4:
				SHR		EDX, 8
				DEC		ESI
			JNZ	TestCache

		ExitCpuTest:
	}
}


/*
===============
GL_Init
===============
*/
extern char *QSG_EXTENSIONS;
void GL_Init (void)
{
	//
	// Request system info - useless, but funny
	//
	GetSysInfo();

	//
	// Print the info we just requested
	//
	Con_Printf("CPU Family: %i\n", CPUfamily);
	Con_Printf("CPU Vendor: %s\n", VendorID);
	Con_Printf("Memory: %i Kb\n\n", Mem);
	Con_Printf("Detecting CPU extensions:\n");
	if (Support3DNow)
	{
		Con_Printf(" *3DNow detected\n");
	}
	if (Support3DNowExt)
	{
		Con_Printf(" *3DNow extensions detected\n");
	}
	if (SupportMMX)
	{
		Con_Printf(" *MMX detected\n");
	}
	if (SupportMMXext)
	{
		Con_Printf(" *MMX extensions detected\n");
	}

	//
	// Video card info
	//
	gl_vendor = glGetString (GL_VENDOR);
	Con_Printf ("\nGL_VENDOR: %s\n", gl_vendor);
	gl_renderer = glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n\n", gl_version);
	gl_extensions = glGetString (GL_EXTENSIONS);
	Con_Printf ("GL_EXTENSIONS:\n%s\n", gl_extensions);

	Con_Printf ("\nQSG extensions: %s\n", QSG_EXTENSIONS);

	//
	// Tomaz - Console Toggle Begin
	//
	if (COM_CheckParm ("-console"))
         console_enabled = true;

	//
	// Check extensions
	//
	CheckTextureExtensions ();
	CheckMultiTextureExtensions ();

	glLineWidth(2);
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glClearColor (0,0,0,0);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DITHER);

	glAlphaFunc(GL_GREATER, 0);
	glDepthFunc (GL_LEQUAL);	

//	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel (GL_SMOOTH);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Changed
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//Changed
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable  (GL_CULL_FACE);
	// This can be done locally, but is a pain in the ass to change when it's already
	// removed from every function, so we'll just leave it here for now.
	glEnable  (GL_BLEND);
	glDisable (GL_ALPHA_TEST);

	Con_Printf ("Forcing glFinish\n");
	glFinish();
}

/*
=================
GL_BeginRendering
=================
*/
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	extern cvar_t gl_clear;

	*x = *y = 0;
	*width = WindowRect.right - WindowRect.left;
	*height = WindowRect.bottom - WindowRect.top;
}


void GL_EndRendering (void)
{
	if (!scr_skipupdate)
		SwapBuffers(maindc);

// handle the mouse state when windowed if that's changed
	if (modestate == MS_WINDOWED)
	{
		if (!_windowed_mouse.value) 
		{
			if (windowed_mouse)	
			{
				IN_DeactivateMouse ();
				IN_ShowMouse ();
				windowed_mouse = false;
			}
		} 
		else 
		{
			windowed_mouse = true;
			if (key_dest == key_game && !mouseactive && ActiveApp) 
			{
				IN_ActivateMouse ();
				IN_HideMouse ();
			} 
			else if (mouseactive && key_dest != key_game) 
			{
				IN_DeactivateMouse ();
				IN_ShowMouse ();
			}
		}
	}
}

void VID_SetPalette (unsigned char *palette)
{
	byte			*pal;
	unsigned		r,g,b;
	unsigned		v;
	int				r1,g1,b1;
	int				j,k,l;
	unsigned short	i;
	unsigned		*table;

//
// 8 8 8 encoding
//
	pal		= palette;
	table	= d_8to24table;
	for (i=0 ; i<256 ; i++)
	{
		r	= pal[0];
		g	= pal[1];
		b	= pal[2];
		pal += 3;
		
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
	d_8to24table[255] &= 0x000000;	// 255 is transparent

	// JACK: 3D distance calcs - k is last closest, l is the distance.
	// FIXME: Precalculate this and cache to disk.
	for (i=0; i < (1<<15); i++) 
	{
		/* Maps
			000000000000000
			000000000011111 = Red  = 0x1F
			000001111100000 = Blue = 0x03E0
			111110000000000 = Grn  = 0x7C00
		*/
		r = ((i & 0x1F) << 3)+4;
		g = ((i & 0x03E0) >> 2)+4;
		b = ((i & 0x7C00) >> 7)+4;
		pal = (unsigned char *)d_8to24table;
		for (v=0,k=0,l=100000000; v<256; v++,pal+=4)	// Tomaz Speed
		{
			r1 = r-pal[0];
			g1 = g-pal[1];
			b1 = b-pal[2];
			j = (r1*r1)+(g1*g1)+(b1*b1);
			if (j<l) 
			{
				k=v;
				l=j;
			}
		}
		d_15to8table[i]=k;
	}
}

void VID_SetDefaultMode (void)
{
	IN_DeactivateMouse ();
}

void	VID_Shutdown (void)
{
   	HGLRC hRC;
   	HDC	  hDC;
	//GLuint temp[8192];
	//int i;//Bug code?

	if (vid_initialized)
	{
		vid_canalttab = false;
		hRC = wglGetCurrentContext();
    	hDC = wglGetCurrentDC();

    	wglMakeCurrent(NULL, NULL);

		//Bug code?
		// LordHavoc: free textures before closing (may help NVIDIA)
		//for (i = 0;i < 8192;i++)
		//	temp[i] = i+1;
		//glDeleteTextures(8192, temp);
		//Bug code?

    	if (hRC)
    	    wglDeleteContext(hRC);

		if (hDC && dibwindow)
			ReleaseDC(dibwindow, hDC);

		if (modestate == MS_FULLDIB)
			ChangeDisplaySettings (NULL, 0);

		if (maindc && dibwindow)
			ReleaseDC (dibwindow, maindc);

		AppActivate(false, false);
	}
}


//==========================================================================


BOOL bSetupPixelFormat(HDC hDC)
{
    static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
	1,				// version number
	PFD_DRAW_TO_WINDOW 		// support window
	|  PFD_SUPPORT_OPENGL 	// support OpenGL
	|  PFD_DOUBLEBUFFER ,	// double buffered
	PFD_TYPE_RGBA,			// RGBA type
	24,				// 24-bit color depth
	0, 0, 0, 0, 0, 0,		// color bits ignored
	0,				// no alpha buffer
	0,				// shift bit ignored
	0,				// no accumulation buffer
	0, 0, 0, 0, 			// accum bits ignored
	32,				// 32-bit z-buffer	
	0,				// no stencil buffer
	0,				// no auxiliary buffer
	PFD_MAIN_PLANE,			// main layer
	0,				// reserved
	0, 0, 0				// layer masks ignored
    };
    int pixelformat;

    if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0 )
    {
        MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    if (SetPixelFormat(hDC, pixelformat, &pfd) == FALSE)
    {
        MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    return TRUE;
}



byte        scantokey[128] = 
					{ 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6', 
	'7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0 
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
	'o',    'p',    '[',    ']',    13 ,    K_CTRL,'a',  's',      // 1 
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', 
	'\'' ,    '`',    K_SHIFT,'\\',  'z',    'x',    'c',    'v',      // 2 
	'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT,'*', 
	K_ALT,' ',   0  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
	K_F6, K_F7, K_F8, K_F9, K_F10, K_PAUSE  ,    0  , K_HOME, 
	K_UPARROW,K_PGUP,'-',K_LEFTARROW,'5',K_RIGHTARROW,'+',K_END, //4 
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11, 
	K_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
					}; 

byte        shiftscantokey[128] = 
					{ 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
	0  ,    27,     '!',    '@',    '#',    '$',    '%',    '^', 
	'&',    '*',    '(',    ')',    '_',    '+',    K_BACKSPACE, 9, // 0 
	'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I', 
	'O',    'P',    '{',    '}',    13 ,    K_CTRL,'A',  'S',      // 1 
	'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':', 
	'"' ,    '~',    K_SHIFT,'|',  'Z',    'X',    'C',    'V',      // 2 
	'B',    'N',    'M',    '<',    '>',    '?',    K_SHIFT,'*', 
	K_ALT,' ',   0  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
	K_F6, K_F7, K_F8, K_F9, K_F10, K_PAUSE  ,    0  , K_HOME, 
	K_UPARROW,K_PGUP,'_',K_LEFTARROW,'%',K_RIGHTARROW,'+',K_END, //4 
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11, 
	K_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
					}; 


/*
=======
MapKey

Map from windows to quake keynums
=======
*/
int MapKey (int key)
{
	key = (key>>16)&255;
	if (key > 127)
		return 0;
	if (scantokey[key] == 0)
		Con_DPrintf("key 0x%02x has no translation\n", key);
	return scantokey[key];
}

/*
===================================================================

MAIN WINDOW

===================================================================
*/

/*
================
ClearAllStates
================
*/
void ClearAllStates (void)
{
	int		i;
	
// send an up event for each key, to make sure the server clears them all
	for (i=0 ; i<256 ; i++)
	{
		Key_Event (i, false);
	}

	Key_ClearStates ();
	IN_ClearStates ();
}

void AppActivate(BOOL fActive, BOOL minimize)
/****************************************************************************
*
* Function:     AppActivate
* Parameters:   fActive - True if app is activating
*
* Description:  If the application is activating, then swap the system
*               into SYSPAL_NOSTATIC mode so that our palettes will display
*               correctly.
*
****************************************************************************/
{
	static BOOL	sound_active;

	ActiveApp = fActive;
	Minimized = minimize;

// enable/disable sound on focus gain/loss
	if (!ActiveApp && sound_active)
	{
		S_BlockSound ();
		sound_active = false;
	}
	else if (ActiveApp && !sound_active)
	{
		S_UnblockSound ();
		sound_active = true;
	}

	if (fActive)
	{
		if (modestate == MS_FULLDIB)
		{
			IN_ActivateMouse ();
			IN_HideMouse ();
			if (vid_canalttab && vid_wassuspended) {
				vid_wassuspended = false;
				ChangeDisplaySettings (&gdevmode, CDS_FULLSCREEN);
				ShowWindow(mainwindow, SW_SHOWNORMAL);
				MoveWindow(mainwindow, 0, 0, gdevmode.dmPelsWidth, gdevmode.dmPelsHeight, false);	// Tomaz - Alt+Tab Fix
			}
		}
		else if ((modestate == MS_WINDOWED) && _windowed_mouse.value && key_dest == key_game)
		{
			IN_ActivateMouse ();
			IN_HideMouse ();
		}
	}

	if (!fActive)
	{
		if (modestate == MS_FULLDIB)
		{
			IN_DeactivateMouse ();
			IN_ShowMouse ();
			if (vid_canalttab) { 
				ChangeDisplaySettings (NULL, 0);
				vid_wassuspended = true;
			}
		}
		else if ((modestate == MS_WINDOWED) && _windowed_mouse.value)
		{
			IN_DeactivateMouse ();
			IN_ShowMouse ();
		}
	}
}


/* main window procedure */
LONG WINAPI MainWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    LONG    lRet = 1;
	int		fActive, fMinimized, temp;
	extern unsigned int uiWheelMessage;

	if ( uMsg == uiWheelMessage )
		uMsg = WM_MOUSEWHEEL;

    switch (uMsg)
    {
		case WM_KILLFOCUS:
			if (modestate == MS_FULLDIB)
				ShowWindow(mainwindow, SW_SHOWMINNOACTIVE);
			break;

		case WM_CREATE:
			break;

		case WM_MOVE:
			window_x = (int) LOWORD(lParam);
			window_y = (int) HIWORD(lParam);
			VID_UpdateWindowStatus ();
			break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			Key_Event (MapKey(lParam), true);
			break;
			
		case WM_KEYUP:
		case WM_SYSKEYUP:
			Key_Event (MapKey(lParam), false);
			break;

		case WM_SYSCHAR:
		// keep Alt-Space from happening
			break;

	// this is complicated because Win32 seems to pack multiple mouse events into
	// one update sometimes, so we always check all states and look for events
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
			temp = 0;

			if (wParam & MK_LBUTTON)
				temp |= 1;

			if (wParam & MK_RBUTTON)
				temp |= 2;

			if (wParam & MK_MBUTTON)
				temp |= 4;

			IN_MouseEvent (temp);

			break;

		// JACK: This is the mouse wheel with the Intellimouse
		// Its delta is either positive or neg, and we generate the proper
		// Event.
		case WM_MOUSEWHEEL: 
			if ((short) HIWORD(wParam) > 0) {
				Key_Event(K_MWHEELUP, true);
				Key_Event(K_MWHEELUP, false);
			} else {
				Key_Event(K_MWHEELDOWN, true);
				Key_Event(K_MWHEELDOWN, false);
			}
			break;

    	case WM_SIZE:
            break;

   	    case WM_CLOSE:
			if (MessageBox (mainwindow, "Are you sure you want to quit?", "Confirm Exit",
						MB_YESNO | MB_SETFOREGROUND | MB_ICONQUESTION) == IDYES)
			{
				Sys_Quit ();
			}

	        break;

		case WM_ACTIVATE:
			fActive = LOWORD(wParam);
			fMinimized = (BOOL) HIWORD(wParam);
			AppActivate(!(fActive == WA_INACTIVE), fMinimized);

		// fix the leftover Alt from any Alt-Tab or the like that switched us away
			ClearAllStates ();

			break;

   	    case WM_DESTROY:
        {
			if (dibwindow)
				DestroyWindow (dibwindow);

            PostQuitMessage (0);
        }
        break;

		case MM_MCINOTIFY:
//            lRet = CDAudio_MessageHandler (hWnd, uMsg, wParam, lParam);
			break;

    	default:
            /* pass all unhandled messages to DefWindowProc */
            lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
        break;
    }

    /* return 1 if handled message, 0 if not */
    return lRet;
}


/*
=================
VID_NumModes
=================
*/
int VID_NumModes (void)
{
	return nummodes;
}

	
/*
=================
VID_GetModePtr
=================
*/
vmode_t *VID_GetModePtr (int modenum)
{

	if ((modenum >= 0) && (modenum < nummodes))
		return &modelist[modenum];
	else
		return &badmode;
}


/*
=================
VID_GetModeDescription
=================
*/
char *VID_GetModeDescription (int mode)
{
	char		*pinfo;
	vmode_t		*pv;
	static char	temp[100];

	if ((mode < 0) || (mode >= nummodes))
		return NULL;

	if (!leavecurrentmode)
	{
		pv = VID_GetModePtr (mode);
		pinfo = pv->modedesc;
	}
	else
	{
		sprintf (temp, "Desktop resolution (%dx%d)",
				 modelist[MODE_FULLSCREEN_DEFAULT].width,
				 modelist[MODE_FULLSCREEN_DEFAULT].height);
		pinfo = temp;
	}

	return pinfo;
}


// KJB: Added this to return the mode driver name in description for console

char *VID_GetExtModeDescription (int mode)
{
	static char	pinfo[40];
	vmode_t		*pv;

	if ((mode < 0) || (mode >= nummodes))
		return NULL;

	pv = VID_GetModePtr (mode);
	if (modelist[mode].type == MS_FULLDIB)
	{
		if (!leavecurrentmode)
		{
			sprintf(pinfo,"%s fullscreen", pv->modedesc);
		}
		else
		{
			sprintf (pinfo, "Desktop resolution (%dx%d)",
					 modelist[MODE_FULLSCREEN_DEFAULT].width,
					 modelist[MODE_FULLSCREEN_DEFAULT].height);
		}
	}
	else
	{
		if (modestate == MS_WINDOWED)
			sprintf(pinfo, "%s windowed", pv->modedesc);
		else
			sprintf(pinfo, "windowed");
	}

	return pinfo;
}


/*
=================
VID_DescribeCurrentMode_f
=================
*/
void VID_DescribeCurrentMode_f (void)
{
	Con_Printf ("%s\n", VID_GetExtModeDescription (vid_modenum));
}


/*
=================
VID_NumModes_f
=================
*/
void VID_NumModes_f (void)
{

	if (nummodes == 1)
		Con_Printf ("%d video mode is available\n", nummodes);
	else
		Con_Printf ("%d video modes are available\n", nummodes);
}


/*
=================
VID_DescribeMode_f
=================
*/
void VID_DescribeMode_f (void)
{
	int		t, modenum;
	
	modenum = Q_atoi (Cmd_Argv(1));

	t = leavecurrentmode;
	leavecurrentmode = 0;

	Con_Printf ("%s\n", VID_GetExtModeDescription (modenum));

	leavecurrentmode = t;
}


/*
=================
VID_DescribeModes_f
=================
*/
void VID_DescribeModes_f (void)
{
	int			i, lnummodes, t;
	char		*pinfo;
	vmode_t		*pv;

	lnummodes = VID_NumModes ();

	t = leavecurrentmode;
	leavecurrentmode = 0;

	for (i=1 ; i<lnummodes ; i++)
	{
		pv = VID_GetModePtr (i);
		pinfo = VID_GetExtModeDescription (i);
		Con_Printf ("%2d: %s\n", i, pinfo);
	}

	leavecurrentmode = t;
}


// BramBo: new function to read the config/startup file
//#### place on top of file, or min above VID_InitDIB - sigh, quake code...
int cwidth, cbpp;
int filexists = 0;

void ReadConfig(void) 
{
	FILE	*f;

	if (!COM_FOpenFile("scripts/startup.txt",&f))
	{
		return; // return, file does not exist, use standard values
	}
	
	if(!f)
	{
		return;
	}

	filexists = 1; // the file is found, thus we can proceed loading the values

	fscanf (f, "%i", &cwidth); // read height value, assign to cheight
	fscanf (f, "%i", &cbpp); // read fullscreen value, assign to cfullscreen
	
	fclose (f); // close the file
}

// VID_InitDIB - BramBo: added config file reading, selecting resulution. Removed support for resulutions below 640X480
void VID_InitDIB (HINSTANCE hInstance)
{
	WNDCLASS		wc;

	/* Register the frame class */
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL; // was 0
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL; // was 0
    //wc.lpszClassName = "TomazQuake";
	wc.lpszClassName = "Telejano";


    if (!RegisterClass (&wc) )
		Sys_Error ("Couldn't register window class");

	modelist[0].type = MS_WINDOWED;
	
	ReadConfig(); // read the config, IF it exists

	// we already in windowed mode, so we don't have to check bpp

	if (COM_CheckParm("-width"))
	{
		modelist[0].width = Q_atoi(com_argv[COM_CheckParm("-width")+1]);
	}
	else
	{
		if (!filexists)
		{
			modelist[0].width = 640;
		}
		else
		{
			modelist[0].width = cwidth;
		}
	}

	if (COM_CheckParm("-height"))
	{
		modelist[0].height= Q_atoi(com_argv[COM_CheckParm("-height")+1]);
	}
	else
	{
		modelist[0].height = modelist[0].width * 3/4; // height is width * 0.75
	}

	sprintf (modelist[0].modedesc, "%dx%d",
			 modelist[0].width, modelist[0].height);

	modelist[0].modenum = MODE_WINDOWED;
	modelist[0].dib = 1;
	modelist[0].fullscreen = 0;
	modelist[0].halfscreen = 0;
	modelist[0].bpp = 0;

	nummodes = 1;
}

//### END ###

/*
=================
VID_InitFullDIB
=================
*/
void VID_InitFullDIB (HINSTANCE hInstance)
{
	DEVMODE	devmode;
	int		i, modenum, originalnummodes, existingmode, numlowresmodes;
	int		j, bpp, done;
	BOOL	stat;

// enumerate >8 bpp modes
	originalnummodes = nummodes;
	modenum = 0;

	do
	{
		stat = EnumDisplaySettings (NULL, modenum, &devmode);

		if ((devmode.dmBitsPerPel >= 15) &&
			(devmode.dmPelsWidth <= MAXWIDTH) &&
			(devmode.dmPelsHeight <= MAXHEIGHT) &&
			(nummodes < MAX_MODE_LIST))
		{
			devmode.dmFields = DM_BITSPERPEL |
							   DM_PELSWIDTH |
							   DM_PELSHEIGHT;

			if (ChangeDisplaySettings (&devmode, CDS_TEST | CDS_FULLSCREEN) ==
					DISP_CHANGE_SUCCESSFUL)
			{
				modelist[nummodes].type = MS_FULLDIB;
				modelist[nummodes].width = devmode.dmPelsWidth;
				modelist[nummodes].height = devmode.dmPelsHeight;
				modelist[nummodes].modenum = 0;
				modelist[nummodes].halfscreen = 0;
				modelist[nummodes].dib = 1;
				modelist[nummodes].fullscreen = 1;
				modelist[nummodes].bpp = devmode.dmBitsPerPel;
				sprintf (modelist[nummodes].modedesc, "%dx%dx%d",
						 devmode.dmPelsWidth, devmode.dmPelsHeight,
						 devmode.dmBitsPerPel);

			// if the width is more than twice the height, reduce it by half because this
			// is probably a dual-screen monitor
				if (!COM_CheckParm("-noadjustaspect"))
				{
					if (modelist[nummodes].width > (modelist[nummodes].height << 1))
					{
						modelist[nummodes].width >>= 1;
						modelist[nummodes].halfscreen = 1;
						sprintf (modelist[nummodes].modedesc, "%dx%dx%d",
								 modelist[nummodes].width,
								 modelist[nummodes].height,
								 modelist[nummodes].bpp);
					}
				}

				for (i=originalnummodes, existingmode = 0 ; i<nummodes ; i++)
				{
					if ((modelist[nummodes].width == modelist[i].width)   &&
						(modelist[nummodes].height == modelist[i].height) &&
						(modelist[nummodes].bpp == modelist[i].bpp))
					{
						existingmode = 1;
						break;
					}
				}

				if (!existingmode)
				{
					nummodes++;
				}
			}
		}

		modenum++;
	} while (stat);

// see if there are any low-res modes that aren't being reported
	numlowresmodes = sizeof(lowresmodes) / sizeof(lowresmodes[0]);
	bpp = 16;
	done = 0;

	do
	{
		for (j=0 ; (j<numlowresmodes) && (nummodes < MAX_MODE_LIST) ; j++)
		{
			devmode.dmBitsPerPel = bpp;
			devmode.dmPelsWidth = lowresmodes[j].width;
			devmode.dmPelsHeight = lowresmodes[j].height;
			devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			if (ChangeDisplaySettings (&devmode, CDS_TEST | CDS_FULLSCREEN) ==
					DISP_CHANGE_SUCCESSFUL)
			{
				modelist[nummodes].type = MS_FULLDIB;
				modelist[nummodes].width = devmode.dmPelsWidth;
				modelist[nummodes].height = devmode.dmPelsHeight;
				modelist[nummodes].modenum = 0;
				modelist[nummodes].halfscreen = 0;
				modelist[nummodes].dib = 1;
				modelist[nummodes].fullscreen = 1;
				modelist[nummodes].bpp = devmode.dmBitsPerPel;
				sprintf (modelist[nummodes].modedesc, "%dx%dx%d",
						 devmode.dmPelsWidth, devmode.dmPelsHeight,
						 devmode.dmBitsPerPel);

				for (i=originalnummodes, existingmode = 0 ; i<nummodes ; i++)
				{
					if ((modelist[nummodes].width == modelist[i].width)   &&
						(modelist[nummodes].height == modelist[i].height) &&
						(modelist[nummodes].bpp == modelist[i].bpp))
					{
						existingmode = 1;
						break;
					}
				}

				if (!existingmode)
				{
					nummodes++;
				}
			}
		}
		switch (bpp)
		{
			case 16:
				bpp = 32;
				break;

			case 32:
				bpp = 24;
				break;

			case 24:
				done = 1;
				break;
		}
	} while (!done);

	if (nummodes == originalnummodes)
		Con_SafePrintf ("No fullscreen DIB modes found\n");
}

#define GL_SHARED_TEXTURE_PALETTE_EXT 0x81FB

static void Check_Gamma (unsigned char *pal)
{
	float	f, inf;
	unsigned char	palette[768];
	int		i;

	vid_gamma = 1.0f;

	if (i = COM_CheckParm("-gamma"))
		vid_gamma = Q_atof(com_argv[i+1]);
	
	for (i=0 ; i<768 ; i++)
	{
		f = pow ( (pal[i]+1)/256.0 , vid_gamma );
		//inf = f*255 + lhrandom(0,10);//Tei gamma hack 
#if 1 //Telejano RC2 gamma lovegame
		inf = f*256;// Tei gamma hack for more contrast + 0.5 deleted here
#else
		inf = f*255 + 0.5; 
#endif
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		palette[i] = inf;
	}

	memcpy (pal, palette, sizeof(palette));
}

/*
===================
SaveCurrentVidMode
===================
*/
void SaveCurrentVidMode (void) 
{
	FILE	*f;
	int	i;

	Sys_mkdir (va("%s/scripts", com_gamedir));	

	f = fopen (va("%s/scripts/startup.txt",com_gamedir), "w"); // open/create file for writing
	if (!f)
	{
		// crash, when we don't have the right to write a file, or out of disk space
		Sys_Error ("Could not write startup-resolution file, check diskspace or user-rights");
	}

	i = vid_modenum; // assign our current video mode to i

	fprintf (f, "%i\n", modelist[i].width); // save current width
	fprintf (f, "%i\n", modelist[i].bpp); // save current bpp
	fprintf (f, "// file generated by Quake, do not modify"); // warning msg, can be removed

	fclose (f); // close the file
}

/*
===================
VID_Init
Edited by BramBo to support reading the default screen resolution from startup.cfg
===================
*/
void	VID_Init (unsigned char *palette)
{
	int		i, existingmode;
	int		basenummodes, width, height, bpp, findbpp, done;
	HDC		hdc;
	DEVMODE	devmode;

	memset(&devmode, 0, sizeof(devmode));

	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&vid_wait);
	Cvar_RegisterVariable (&vid_nopageflip);
	Cvar_RegisterVariable (&_vid_wait_override);
	Cvar_RegisterVariable (&_vid_default_mode);
	Cvar_RegisterVariable (&_vid_default_mode_win);
	Cvar_RegisterVariable (&vid_config_x);
	Cvar_RegisterVariable (&vid_config_y);
	Cvar_RegisterVariable (&vid_stretch_by_2);
	Cvar_RegisterVariable (&_windowed_mouse);

	Cmd_AddCommand ("vid_nummodes", VID_NumModes_f);
	Cmd_AddCommand ("vid_describecurrentmode", VID_DescribeCurrentMode_f);
	Cmd_AddCommand ("vid_describemode", VID_DescribeMode_f);
	Cmd_AddCommand ("vid_describemodes", VID_DescribeModes_f);
	Cmd_AddCommand ("vid_savecurrentmode", SaveCurrentVidMode); // command to write our current config to the startup file

	hIcon = LoadIcon (global_hInstance, MAKEINTRESOURCE (IDI_ICON2));

	InitCommonControls();

	VID_InitDIB (global_hInstance); // this is where our config file is read, so we don't have to do it twice
	basenummodes = nummodes = 1;

	VID_InitFullDIB (global_hInstance);

	if (!filexists)
	{ 
		//cbpp = 15; // HACK - default bpp to 15 if our startup config file doesn't exist.
		cbpp = 32; // HACK - default bpp to 15 if our startup config file doesn't exist.
		findbpp = 1;
	} 

	// decide wether to choose windowed or fullscreen
	if (COM_CheckParm("-window"))
	{
		hdc = GetDC (NULL);

		if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
		{
			Sys_Error ("Can't run in non-RGB mode");
		}

		ReleaseDC (NULL, hdc);

		windowed = true;

		vid_default = MODE_WINDOWED;
	}
	else
	{
			// user didn't set "-window", so check cbpp
			if (cbpp == 0) // if -window isn't set, but cbpp == 0, assume we're running windowed
			{
				// do the normal windowed code stuff
				hdc = GetDC (NULL);

				if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
				{
					Sys_Error ("Can't run in non-RGB mode");
				}

				ReleaseDC (NULL, hdc);
	
				windowed = true;
	
				vid_default = MODE_WINDOWED; // strangely enough, the windowed height/width isn't set here...
			}
			else  // else, do the normal fullscreen stuff
			{
				if (nummodes == 1)
				{
					Sys_Error ("No RGB fullscreen modes available");
				}

				windowed = false;

				if (COM_CheckParm("-mode"))
				{
					vid_default = Q_atoi(com_argv[COM_CheckParm("-mode")+1]);
				}
				else
				{
					if (COM_CheckParm("-current"))
					{
						modelist[MODE_FULLSCREEN_DEFAULT].width =
							GetSystemMetrics (SM_CXSCREEN);
						modelist[MODE_FULLSCREEN_DEFAULT].height =
							GetSystemMetrics (SM_CYSCREEN);
						vid_default = MODE_FULLSCREEN_DEFAULT;
						leavecurrentmode = 1;
					}
					else
					{
						// check to see if "-width" is set by the user
						if (COM_CheckParm("-width"))
						{
							// yes, set the value as our current width
							width = Q_atoi(com_argv[COM_CheckParm("-width")+1]);
						}
						else
						{
							// no, check if our config file exists
							if (!filexists)
							{
								width = 640; // config file doesn't exist, use the standard value
							}
							else
							{
								width = cwidth; //assign our config file width to the internal width
							}
						}
			
					// check to see if the "-bpp" is set by the user
					if (COM_CheckParm("-bpp"))
					{
						// yes, set the user value as our current value
						bpp = Q_atoi(com_argv[COM_CheckParm("-bpp")+1]);
						findbpp = 0; // don't search for a decent bpp
					}
					else
					{
						// no, again, check if our config file exists
						if (!filexists)
						{
							// config file doesn't exist, set bpp to standard value
							bpp = 15;
							findbpp = 1; // search for a bpp value which may be better
						}
						else
						{
							// config file exists, set our config value as bpp
							bpp = cbpp;
							findbpp = 0; // don't search for other bpp values
						}
					}

					if (COM_CheckParm("-height"))
					{
						height = Q_atoi(com_argv[COM_CheckParm("-height")+1]);
					}
					// if they want to force it, add the specified mode to the list
					if (COM_CheckParm("-force") && (nummodes < MAX_MODE_LIST))
					{
						modelist[nummodes].type = MS_FULLDIB;
						modelist[nummodes].width = width;
						modelist[nummodes].height = height;
						modelist[nummodes].modenum = 0;
						modelist[nummodes].halfscreen = 0;
						modelist[nummodes].dib = 1;
						modelist[nummodes].fullscreen = 1;
						modelist[nummodes].bpp = bpp;
						sprintf (modelist[nummodes].modedesc, "%dx%dx%d",
								 devmode.dmPelsWidth, devmode.dmPelsHeight,
								 devmode.dmBitsPerPel);

						for (i=nummodes, existingmode = 0 ; i<nummodes ; i++)
						{
							if ((modelist[nummodes].width == modelist[i].width)   &&
								(modelist[nummodes].height == modelist[i].height) &&
								(modelist[nummodes].bpp == modelist[i].bpp))
							{
								existingmode = 1;
								break;
							}
						}

						if (!existingmode)
						{
							nummodes++;
						}
					}

					done = 0;

					do
					{
						if (COM_CheckParm("-height"))
						{
							height = Q_atoi(com_argv[COM_CheckParm("-height")+1]);

							for (i=1, vid_default=0 ; i<nummodes ; i++)
							{
								if ((modelist[i].width == width) &&
									(modelist[i].height == height) &&
									(modelist[i].bpp == bpp))
								{
									vid_default = i;
									done = 1;
									break;
								}
							}
						}
						else
						{
						
							for (i=1, vid_default=0 ; i<nummodes ; i++)
							{
								if ((modelist[i].width == width) && (modelist[i].bpp == bpp))
								{
									vid_default = i;
									done = 1;
									break;
								}
							}
	
						}

						if (!done)
						{
							if (findbpp)
							{
								switch (bpp)
								{
								case 15:
									bpp = 16;
									break;
								case 16:
									bpp = 32;
									break;
								case 32:
									bpp = 24;
									break;
								case 24:
									done = 1;
									break;
								}
							}
							else
							{
								done = 1;
							}
						}
					} while (!done);

					if (!vid_default)
					{
						Sys_Error ("Specified video mode not available");
					}
				}
			}
		}
	}
	vid_initialized = true;

	if ((i = COM_CheckParm("-conwidth")) != 0)
		vid.conwidth = Q_atoi(com_argv[i+1]);
	else
		vid.conwidth = width;

	vid.conwidth &= 0xfff8; // make it a multiple of eight

	if (vid.conwidth < 320)
		vid.conwidth = 320;

	// pick a conheight that matches with correct aspect
	vid.conheight = vid.conwidth * 0.75;	// Tomaz - Speed

	if ((i = COM_CheckParm("-conheight")) != 0)
		vid.conheight = Q_atoi(com_argv[i+1]);
	if (vid.conheight < 200)
		vid.conheight = 200;

	vid.colormap = host_colormap;

	Check_Gamma(palette);
	VID_SetPalette (palette);

	DestroyWindow (hwnd_dialog);

	VID_SetMode (vid_default, palette);

    maindc = GetDC(mainwindow);
	bSetupPixelFormat(maindc);

    baseRC = wglCreateContext( maindc );
	if (!baseRC)
		Sys_Error ("Could not initialize GL (wglCreateContext failed).\n\nMake sure you in are 65535 color mode, and try running -window.");
    if (!wglMakeCurrent( maindc, baseRC ))
		Sys_Error ("wglMakeCurrent failed");

	GL_Init ();

	// Tomaz - Removed ms2

	strcpy (badmode.modedesc, "Bad mode");
	vid_canalttab = true;
}


