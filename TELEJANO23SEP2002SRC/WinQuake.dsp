# Microsoft Developer Studio Project File - Name="winquake" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=WINQUAKE - WIN32 GL RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WinQuake.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinQuake.mak" CFG="WINQUAKE - WIN32 GL RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "winquake - Win32 GL Debug" (based on "Win32 (x86) Application")
!MESSAGE "winquake - Win32 GL Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\winquake"
# PROP BASE Intermediate_Dir ".\winquake"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\debug_gl"
# PROP Intermediate_Dir ".\debug_gl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /ML /GX /Zi /Od /I ".\scitech\include" /I ".\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /ML /W3 /GX /ZI /Od /I ".\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "GLQUAKE" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib wsock32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib .\scitech\lib\win32\vc\mgllt.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 .\dxsdk\sdk\lib\dxguid.lib comctl32.lib winmm.lib wsock32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mss.lib /nologo /subsystem:windows /debug /machine:I386 /out:"e:\quake\teiq2.exe"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\winquak0"
# PROP BASE Intermediate_Dir ".\winquak0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\release_gl"
# PROP Intermediate_Dir ".\release_gl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /GX /Ox /Ot /Ow /I ".\scitech\include" /I ".\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# SUBTRACT BASE CPP /Oa /Og
# ADD CPP /nologo /G5 /W3 /Zd /Ox /Ot /Oa /Og /Oi /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "GLQUAKE" /Fr /YX /FD /GA /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib wsock32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib .\scitech\lib\win32\vc\mgllt.lib /nologo /subsystem:windows /profile /machine:I386
# SUBTRACT BASE LINK32 /map /debug
# ADD LINK32 .\dxsdk\sdk\lib\dxguid.lib comctl32.lib winmm.lib wsock32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mss.lib libpng.lib zlib.lib jpeg.lib /nologo /subsystem:windows /machine:I386 /out:"d:\quake\telejano4.exe"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "winquake - Win32 GL Debug"
# Name "winquake - Win32 GL Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "Client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cl_demo.c
# End Source File
# Begin Source File

SOURCE=.\cl_input.c
# End Source File
# Begin Source File

SOURCE=.\cl_main.c
# End Source File
# Begin Source File

SOURCE=.\cl_parse.c
# End Source File
# Begin Source File

SOURCE=.\cl_staticfx.c
# End Source File
# Begin Source File

SOURCE=.\cl_tent.c
# End Source File
# End Group
# Begin Group "Server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sv_collision.c
# End Source File
# Begin Source File

SOURCE=.\sv_main.c
# End Source File
# Begin Source File

SOURCE=.\sv_move.c
# End Source File
# Begin Source File

SOURCE=.\sv_phys.c
# End Source File
# Begin Source File

SOURCE=.\sv_user.c
# End Source File
# End Group
# Begin Group "Host"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\host.c
# End Source File
# Begin Source File

SOURCE=.\host_cmd.c
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net_dgrm.c
# End Source File
# Begin Source File

SOURCE=.\net_loop.c
# End Source File
# Begin Source File

SOURCE=.\net_main.c
# End Source File
# Begin Source File

SOURCE=.\net_win.c
# End Source File
# Begin Source File

SOURCE=.\net_wins.c
# End Source File
# Begin Source File

SOURCE=.\net_wipx.c
# End Source File
# End Group
# Begin Group "Menu"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\menu.c
# End Source File
# Begin Source File

SOURCE=.\menu_help.c
# End Source File
# Begin Source File

SOURCE=.\menu_main.c
# End Source File
# Begin Source File

SOURCE=.\menu_multi.c
# End Source File
# Begin Source File

SOURCE=.\menu_options.c
# End Source File
# Begin Source File

SOURCE=.\menu_quit.c
# End Source File
# Begin Source File

SOURCE=.\menu_single.c
# End Source File
# Begin Source File

SOURCE=.\png_loader.c
# End Source File
# End Group
# Begin Group "OpenGL"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=.\gl_crosshair.c
# End Source File
# Begin Source File

SOURCE=.\gl_draw.c
# End Source File
# Begin Source File

SOURCE=.\gl_flares.c
# End Source File
# Begin Source File

SOURCE=.\gl_font.c
# End Source File
# Begin Source File

SOURCE=.\gl_hc_texes.c
# End Source File
# Begin Source File

SOURCE=.\gl_md2.c
# End Source File
# Begin Source File

SOURCE=.\gl_mdl.c
# End Source File
# Begin Source File

SOURCE=.\gl_mirror.c
# End Source File
# Begin Source File

SOURCE=.\gl_model.c
# End Source File
# Begin Source File

SOURCE=.\gl_part.c
# End Source File
# Begin Source File

SOURCE=.\gl_part2.c
# End Source File
# Begin Source File

SOURCE=.\gl_refrag.c
# End Source File
# Begin Source File

SOURCE=.\gl_rlight.c
# End Source File
# Begin Source File

SOURCE=.\gl_rmain.c
# End Source File
# Begin Source File

SOURCE=.\gl_rmisc.c
# End Source File
# Begin Source File

SOURCE=.\gl_rscript.c
# End Source File
# Begin Source File

SOURCE=.\gl_rsurf.c
# End Source File
# Begin Source File

SOURCE=.\gl_screen.c
# End Source File
# Begin Source File

SOURCE=.\gl_sky.c
# End Source File
# Begin Source File

SOURCE=.\gl_sprite.c
# End Source File
# Begin Source File

SOURCE=.\gl_texman.c
# End Source File
# Begin Source File

SOURCE=.\gl_vidnt.c
# End Source File
# Begin Source File

SOURCE=.\gl_warp.c
# End Source File
# End Group
# Begin Group "VM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pr_cmds.c
# End Source File
# Begin Source File

SOURCE=.\pr_edict.c
# End Source File
# Begin Source File

SOURCE=.\pr_exec.c
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cd_win.c
# End Source File
# Begin Source File

SOURCE=.\snd_dma.c
# End Source File
# Begin Source File

SOURCE=.\snd_mem.c
# End Source File
# Begin Source File

SOURCE=.\snd_mix.c
# End Source File
# Begin Source File

SOURCE=.\snd_win.c
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\chase.c
# End Source File
# Begin Source File

SOURCE=.\cmd.c
# End Source File
# Begin Source File

SOURCE=.\common.c
# End Source File
# Begin Source File

SOURCE=.\conproc.c
# End Source File
# Begin Source File

SOURCE=.\console.c
# End Source File
# Begin Source File

SOURCE=.\crc.c
# End Source File
# Begin Source File

SOURCE=.\cvar.c
# End Source File
# Begin Source File

SOURCE=.\in_win.c
# End Source File
# Begin Source File

SOURCE=.\keys.c
# End Source File
# Begin Source File

SOURCE=.\mathlib.c
# End Source File
# Begin Source File

SOURCE=.\sbar.c
# End Source File
# Begin Source File

SOURCE=.\sys_win.c
# End Source File
# Begin Source File

SOURCE=.\view.c
# End Source File
# Begin Source File

SOURCE=.\wad.c
# End Source File
# Begin Source File

SOURCE=.\winquake.rc
# End Source File
# Begin Source File

SOURCE=.\world.c
# End Source File
# Begin Source File

SOURCE=.\zone.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\anorm_dots.h
# End Source File
# Begin Source File

SOURCE=.\anorms.h
# End Source File
# Begin Source File

SOURCE=.\bsp_render.h
# End Source File
# Begin Source File

SOURCE=.\bspfile.h
# End Source File
# Begin Source File

SOURCE=.\client.h
# End Source File
# Begin Source File

SOURCE=.\cmd.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\conproc.h
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\crc.h
# End Source File
# Begin Source File

SOURCE=.\cvar.h
# End Source File
# Begin Source File

SOURCE=.\draw.h
# End Source File
# Begin Source File

SOURCE=.\gl_mirror.h
# End Source File
# Begin Source File

SOURCE=.\gl_model.h
# End Source File
# Begin Source File

SOURCE=.\gl_rscript.h
# End Source File
# Begin Source File

SOURCE=.\gl_warp_sin.h
# End Source File
# Begin Source File

SOURCE=.\glquake.h
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# Begin Source File

SOURCE=.\keys.h
# End Source File
# Begin Source File

SOURCE=.\mathlib.h
# End Source File
# Begin Source File

SOURCE=.\menu.h
# End Source File
# Begin Source File

SOURCE=.\modelgen.h
# End Source File
# Begin Source File

SOURCE=.\net.h
# End Source File
# Begin Source File

SOURCE=.\net_dgrm.h
# End Source File
# Begin Source File

SOURCE=.\net_loop.h
# End Source File
# Begin Source File

SOURCE=.\net_ser.h
# End Source File
# Begin Source File

SOURCE=.\net_wins.h
# End Source File
# Begin Source File

SOURCE=.\net_wipx.h
# End Source File
# Begin Source File

SOURCE=.\png.h
# End Source File
# Begin Source File

SOURCE=.\pngasmrd.h
# End Source File
# Begin Source File

SOURCE=.\pngconf.h
# End Source File
# Begin Source File

SOURCE=.\pr_comp.h
# End Source File
# Begin Source File

SOURCE=.\progs.h
# End Source File
# Begin Source File

SOURCE=.\protocol.h
# End Source File
# Begin Source File

SOURCE=.\quakedef.h
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\sbar.h
# End Source File
# Begin Source File

SOURCE=.\screen.h
# End Source File
# Begin Source File

SOURCE=.\server.h
# End Source File
# Begin Source File

SOURCE=.\sound.h
# End Source File
# Begin Source File

SOURCE=.\spritegn.h
# End Source File
# Begin Source File

SOURCE=.\sys.h
# End Source File
# Begin Source File

SOURCE=.\vid.h
# End Source File
# Begin Source File

SOURCE=.\view.h
# End Source File
# Begin Source File

SOURCE=.\wad.h
# End Source File
# Begin Source File

SOURCE=.\winquake.h
# End Source File
# Begin Source File

SOURCE=.\world.h
# End Source File
# Begin Source File

SOURCE=.\zone.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\bg1.bmp
# End Source File
# Begin Source File

SOURCE=.\blood.ico
# End Source File
# Begin Source File

SOURCE=.\colorfull.ico
# End Source File
# Begin Source File

SOURCE=.\colorfull2.ico
# End Source File
# Begin Source File

SOURCE=.\pure.ico
# End Source File
# Begin Source File

SOURCE=.\tech.ico
# End Source File
# Begin Source File

SOURCE=.\tq.ico
# End Source File
# End Target
# End Project
