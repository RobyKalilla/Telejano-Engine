# Microsoft Developer Studio Generated NMAKE File, Based on WinQuake.dsp
!IF "$(CFG)" == ""
CFG=WINQUAKE - WIN32 GL RELEASE
!MESSAGE No configuration specified. Defaulting to WINQUAKE - WIN32 GL RELEASE.
!ENDIF 

!IF "$(CFG)" != "winquake - Win32 GL Debug" && "$(CFG)" != "winquake - Win32 GL Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "winquake - Win32 GL Debug"

OUTDIR=.\debug_gl
INTDIR=.\debug_gl
# Begin Custom Macros
OutDir=.\debug_gl
# End Custom Macros

ALL : "..\..\..\quake\teiq2.exe" "$(OUTDIR)\WinQuake.bsc"


CLEAN :
	-@erase "$(INTDIR)\cd_win.obj"
	-@erase "$(INTDIR)\cd_win.sbr"
	-@erase "$(INTDIR)\chase.obj"
	-@erase "$(INTDIR)\chase.sbr"
	-@erase "$(INTDIR)\cl_demo.obj"
	-@erase "$(INTDIR)\cl_demo.sbr"
	-@erase "$(INTDIR)\cl_input.obj"
	-@erase "$(INTDIR)\cl_input.sbr"
	-@erase "$(INTDIR)\cl_main.obj"
	-@erase "$(INTDIR)\cl_main.sbr"
	-@erase "$(INTDIR)\cl_parse.obj"
	-@erase "$(INTDIR)\cl_parse.sbr"
	-@erase "$(INTDIR)\cl_staticfx.obj"
	-@erase "$(INTDIR)\cl_staticfx.sbr"
	-@erase "$(INTDIR)\cl_tent.obj"
	-@erase "$(INTDIR)\cl_tent.sbr"
	-@erase "$(INTDIR)\cmd.obj"
	-@erase "$(INTDIR)\cmd.sbr"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\common.sbr"
	-@erase "$(INTDIR)\conproc.obj"
	-@erase "$(INTDIR)\conproc.sbr"
	-@erase "$(INTDIR)\console.obj"
	-@erase "$(INTDIR)\console.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\cvar.obj"
	-@erase "$(INTDIR)\cvar.sbr"
	-@erase "$(INTDIR)\gl_crosshair.obj"
	-@erase "$(INTDIR)\gl_crosshair.sbr"
	-@erase "$(INTDIR)\gl_draw.obj"
	-@erase "$(INTDIR)\gl_draw.sbr"
	-@erase "$(INTDIR)\gl_flares.obj"
	-@erase "$(INTDIR)\gl_flares.sbr"
	-@erase "$(INTDIR)\gl_font.obj"
	-@erase "$(INTDIR)\gl_font.sbr"
	-@erase "$(INTDIR)\gl_hc_texes.obj"
	-@erase "$(INTDIR)\gl_hc_texes.sbr"
	-@erase "$(INTDIR)\gl_md2.obj"
	-@erase "$(INTDIR)\gl_md2.sbr"
	-@erase "$(INTDIR)\gl_mdl.obj"
	-@erase "$(INTDIR)\gl_mdl.sbr"
	-@erase "$(INTDIR)\gl_mirror.obj"
	-@erase "$(INTDIR)\gl_mirror.sbr"
	-@erase "$(INTDIR)\gl_model.obj"
	-@erase "$(INTDIR)\gl_model.sbr"
	-@erase "$(INTDIR)\gl_part.obj"
	-@erase "$(INTDIR)\gl_part.sbr"
	-@erase "$(INTDIR)\gl_part2.obj"
	-@erase "$(INTDIR)\gl_part2.sbr"
	-@erase "$(INTDIR)\gl_refrag.obj"
	-@erase "$(INTDIR)\gl_refrag.sbr"
	-@erase "$(INTDIR)\gl_rlight.obj"
	-@erase "$(INTDIR)\gl_rlight.sbr"
	-@erase "$(INTDIR)\gl_rmain.obj"
	-@erase "$(INTDIR)\gl_rmain.sbr"
	-@erase "$(INTDIR)\gl_rmisc.obj"
	-@erase "$(INTDIR)\gl_rmisc.sbr"
	-@erase "$(INTDIR)\gl_rscript.obj"
	-@erase "$(INTDIR)\gl_rscript.sbr"
	-@erase "$(INTDIR)\gl_rsurf.obj"
	-@erase "$(INTDIR)\gl_rsurf.sbr"
	-@erase "$(INTDIR)\gl_screen.obj"
	-@erase "$(INTDIR)\gl_screen.sbr"
	-@erase "$(INTDIR)\gl_sky.obj"
	-@erase "$(INTDIR)\gl_sky.sbr"
	-@erase "$(INTDIR)\gl_sprite.obj"
	-@erase "$(INTDIR)\gl_sprite.sbr"
	-@erase "$(INTDIR)\gl_texman.obj"
	-@erase "$(INTDIR)\gl_texman.sbr"
	-@erase "$(INTDIR)\gl_vidnt.obj"
	-@erase "$(INTDIR)\gl_vidnt.sbr"
	-@erase "$(INTDIR)\gl_warp.obj"
	-@erase "$(INTDIR)\gl_warp.sbr"
	-@erase "$(INTDIR)\host.obj"
	-@erase "$(INTDIR)\host.sbr"
	-@erase "$(INTDIR)\host_cmd.obj"
	-@erase "$(INTDIR)\host_cmd.sbr"
	-@erase "$(INTDIR)\in_win.obj"
	-@erase "$(INTDIR)\in_win.sbr"
	-@erase "$(INTDIR)\keys.obj"
	-@erase "$(INTDIR)\keys.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\menu.sbr"
	-@erase "$(INTDIR)\menu_help.obj"
	-@erase "$(INTDIR)\menu_help.sbr"
	-@erase "$(INTDIR)\menu_main.obj"
	-@erase "$(INTDIR)\menu_main.sbr"
	-@erase "$(INTDIR)\menu_multi.obj"
	-@erase "$(INTDIR)\menu_multi.sbr"
	-@erase "$(INTDIR)\menu_options.obj"
	-@erase "$(INTDIR)\menu_options.sbr"
	-@erase "$(INTDIR)\menu_quit.obj"
	-@erase "$(INTDIR)\menu_quit.sbr"
	-@erase "$(INTDIR)\menu_single.obj"
	-@erase "$(INTDIR)\menu_single.sbr"
	-@erase "$(INTDIR)\net_dgrm.obj"
	-@erase "$(INTDIR)\net_dgrm.sbr"
	-@erase "$(INTDIR)\net_loop.obj"
	-@erase "$(INTDIR)\net_loop.sbr"
	-@erase "$(INTDIR)\net_main.obj"
	-@erase "$(INTDIR)\net_main.sbr"
	-@erase "$(INTDIR)\net_win.obj"
	-@erase "$(INTDIR)\net_win.sbr"
	-@erase "$(INTDIR)\net_wins.obj"
	-@erase "$(INTDIR)\net_wins.sbr"
	-@erase "$(INTDIR)\net_wipx.obj"
	-@erase "$(INTDIR)\net_wipx.sbr"
	-@erase "$(INTDIR)\pr_cmds.obj"
	-@erase "$(INTDIR)\pr_cmds.sbr"
	-@erase "$(INTDIR)\pr_edict.obj"
	-@erase "$(INTDIR)\pr_edict.sbr"
	-@erase "$(INTDIR)\pr_exec.obj"
	-@erase "$(INTDIR)\pr_exec.sbr"
	-@erase "$(INTDIR)\sbar.obj"
	-@erase "$(INTDIR)\sbar.sbr"
	-@erase "$(INTDIR)\snd_dma.obj"
	-@erase "$(INTDIR)\snd_dma.sbr"
	-@erase "$(INTDIR)\snd_mem.obj"
	-@erase "$(INTDIR)\snd_mem.sbr"
	-@erase "$(INTDIR)\snd_mix.obj"
	-@erase "$(INTDIR)\snd_mix.sbr"
	-@erase "$(INTDIR)\snd_win.obj"
	-@erase "$(INTDIR)\snd_win.sbr"
	-@erase "$(INTDIR)\sv_main.obj"
	-@erase "$(INTDIR)\sv_main.sbr"
	-@erase "$(INTDIR)\sv_move.obj"
	-@erase "$(INTDIR)\sv_move.sbr"
	-@erase "$(INTDIR)\sv_phys.obj"
	-@erase "$(INTDIR)\sv_phys.sbr"
	-@erase "$(INTDIR)\sv_user.obj"
	-@erase "$(INTDIR)\sv_user.sbr"
	-@erase "$(INTDIR)\sys_win.obj"
	-@erase "$(INTDIR)\sys_win.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\view.obj"
	-@erase "$(INTDIR)\view.sbr"
	-@erase "$(INTDIR)\wad.obj"
	-@erase "$(INTDIR)\wad.sbr"
	-@erase "$(INTDIR)\winquake.res"
	-@erase "$(INTDIR)\world.obj"
	-@erase "$(INTDIR)\world.sbr"
	-@erase "$(INTDIR)\zone.obj"
	-@erase "$(INTDIR)\zone.sbr"
	-@erase "$(OUTDIR)\teiq2.pdb"
	-@erase "$(OUTDIR)\WinQuake.bsc"
	-@erase "..\..\..\quake\teiq2.exe"
	-@erase "..\..\..\quake\teiq2.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /ML /W3 /GX /ZI /Od /I ".\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "GLQUAKE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\WinQuake.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\winquake.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinQuake.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cl_demo.sbr" \
	"$(INTDIR)\cl_input.sbr" \
	"$(INTDIR)\cl_main.sbr" \
	"$(INTDIR)\cl_parse.sbr" \
	"$(INTDIR)\cl_staticfx.sbr" \
	"$(INTDIR)\cl_tent.sbr" \
	"$(INTDIR)\sv_main.sbr" \
	"$(INTDIR)\sv_move.sbr" \
	"$(INTDIR)\sv_phys.sbr" \
	"$(INTDIR)\sv_user.sbr" \
	"$(INTDIR)\host.sbr" \
	"$(INTDIR)\host_cmd.sbr" \
	"$(INTDIR)\net_dgrm.sbr" \
	"$(INTDIR)\net_loop.sbr" \
	"$(INTDIR)\net_main.sbr" \
	"$(INTDIR)\net_win.sbr" \
	"$(INTDIR)\net_wins.sbr" \
	"$(INTDIR)\net_wipx.sbr" \
	"$(INTDIR)\menu.sbr" \
	"$(INTDIR)\menu_help.sbr" \
	"$(INTDIR)\menu_main.sbr" \
	"$(INTDIR)\menu_multi.sbr" \
	"$(INTDIR)\menu_options.sbr" \
	"$(INTDIR)\menu_quit.sbr" \
	"$(INTDIR)\menu_single.sbr" \
	"$(INTDIR)\gl_crosshair.sbr" \
	"$(INTDIR)\gl_draw.sbr" \
	"$(INTDIR)\gl_flares.sbr" \
	"$(INTDIR)\gl_font.sbr" \
	"$(INTDIR)\gl_hc_texes.sbr" \
	"$(INTDIR)\gl_md2.sbr" \
	"$(INTDIR)\gl_mdl.sbr" \
	"$(INTDIR)\gl_mirror.sbr" \
	"$(INTDIR)\gl_model.sbr" \
	"$(INTDIR)\gl_part.sbr" \
	"$(INTDIR)\gl_part2.sbr" \
	"$(INTDIR)\gl_refrag.sbr" \
	"$(INTDIR)\gl_rlight.sbr" \
	"$(INTDIR)\gl_rmain.sbr" \
	"$(INTDIR)\gl_rmisc.sbr" \
	"$(INTDIR)\gl_rscript.sbr" \
	"$(INTDIR)\gl_rsurf.sbr" \
	"$(INTDIR)\gl_screen.sbr" \
	"$(INTDIR)\gl_sky.sbr" \
	"$(INTDIR)\gl_sprite.sbr" \
	"$(INTDIR)\gl_texman.sbr" \
	"$(INTDIR)\gl_vidnt.sbr" \
	"$(INTDIR)\gl_warp.sbr" \
	"$(INTDIR)\pr_cmds.sbr" \
	"$(INTDIR)\pr_edict.sbr" \
	"$(INTDIR)\pr_exec.sbr" \
	"$(INTDIR)\cd_win.sbr" \
	"$(INTDIR)\snd_dma.sbr" \
	"$(INTDIR)\snd_mem.sbr" \
	"$(INTDIR)\snd_mix.sbr" \
	"$(INTDIR)\snd_win.sbr" \
	"$(INTDIR)\chase.sbr" \
	"$(INTDIR)\cmd.sbr" \
	"$(INTDIR)\common.sbr" \
	"$(INTDIR)\conproc.sbr" \
	"$(INTDIR)\console.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\cvar.sbr" \
	"$(INTDIR)\in_win.sbr" \
	"$(INTDIR)\keys.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\sbar.sbr" \
	"$(INTDIR)\sys_win.sbr" \
	"$(INTDIR)\view.sbr" \
	"$(INTDIR)\wad.sbr" \
	"$(INTDIR)\world.sbr" \
	"$(INTDIR)\zone.sbr"

"$(OUTDIR)\WinQuake.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=.\dxsdk\sdk\lib\dxguid.lib comctl32.lib winmm.lib wsock32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mss.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\teiq2.pdb" /debug /machine:I386 /out:"e:\quake\teiq2.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cl_demo.obj" \
	"$(INTDIR)\cl_input.obj" \
	"$(INTDIR)\cl_main.obj" \
	"$(INTDIR)\cl_parse.obj" \
	"$(INTDIR)\cl_staticfx.obj" \
	"$(INTDIR)\cl_tent.obj" \
	"$(INTDIR)\sv_main.obj" \
	"$(INTDIR)\sv_move.obj" \
	"$(INTDIR)\sv_phys.obj" \
	"$(INTDIR)\sv_user.obj" \
	"$(INTDIR)\host.obj" \
	"$(INTDIR)\host_cmd.obj" \
	"$(INTDIR)\net_dgrm.obj" \
	"$(INTDIR)\net_loop.obj" \
	"$(INTDIR)\net_main.obj" \
	"$(INTDIR)\net_win.obj" \
	"$(INTDIR)\net_wins.obj" \
	"$(INTDIR)\net_wipx.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\menu_help.obj" \
	"$(INTDIR)\menu_main.obj" \
	"$(INTDIR)\menu_multi.obj" \
	"$(INTDIR)\menu_options.obj" \
	"$(INTDIR)\menu_quit.obj" \
	"$(INTDIR)\menu_single.obj" \
	"$(INTDIR)\gl_crosshair.obj" \
	"$(INTDIR)\gl_draw.obj" \
	"$(INTDIR)\gl_flares.obj" \
	"$(INTDIR)\gl_font.obj" \
	"$(INTDIR)\gl_hc_texes.obj" \
	"$(INTDIR)\gl_md2.obj" \
	"$(INTDIR)\gl_mdl.obj" \
	"$(INTDIR)\gl_mirror.obj" \
	"$(INTDIR)\gl_model.obj" \
	"$(INTDIR)\gl_part.obj" \
	"$(INTDIR)\gl_part2.obj" \
	"$(INTDIR)\gl_refrag.obj" \
	"$(INTDIR)\gl_rlight.obj" \
	"$(INTDIR)\gl_rmain.obj" \
	"$(INTDIR)\gl_rmisc.obj" \
	"$(INTDIR)\gl_rscript.obj" \
	"$(INTDIR)\gl_rsurf.obj" \
	"$(INTDIR)\gl_screen.obj" \
	"$(INTDIR)\gl_sky.obj" \
	"$(INTDIR)\gl_sprite.obj" \
	"$(INTDIR)\gl_texman.obj" \
	"$(INTDIR)\gl_vidnt.obj" \
	"$(INTDIR)\gl_warp.obj" \
	"$(INTDIR)\pr_cmds.obj" \
	"$(INTDIR)\pr_edict.obj" \
	"$(INTDIR)\pr_exec.obj" \
	"$(INTDIR)\cd_win.obj" \
	"$(INTDIR)\snd_dma.obj" \
	"$(INTDIR)\snd_mem.obj" \
	"$(INTDIR)\snd_mix.obj" \
	"$(INTDIR)\snd_win.obj" \
	"$(INTDIR)\chase.obj" \
	"$(INTDIR)\cmd.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\conproc.obj" \
	"$(INTDIR)\console.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cvar.obj" \
	"$(INTDIR)\in_win.obj" \
	"$(INTDIR)\keys.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\sbar.obj" \
	"$(INTDIR)\sys_win.obj" \
	"$(INTDIR)\view.obj" \
	"$(INTDIR)\wad.obj" \
	"$(INTDIR)\world.obj" \
	"$(INTDIR)\zone.obj" \
	"$(INTDIR)\winquake.res"

"..\..\..\quake\teiq2.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

OUTDIR=.\release_gl
INTDIR=.\release_gl
# Begin Custom Macros
OutDir=.\release_gl
# End Custom Macros

ALL : "d:\quake\telejano4.exe" "$(OUTDIR)\WinQuake.bsc"


CLEAN :
	-@erase "$(INTDIR)\cd_win.obj"
	-@erase "$(INTDIR)\cd_win.sbr"
	-@erase "$(INTDIR)\chase.obj"
	-@erase "$(INTDIR)\chase.sbr"
	-@erase "$(INTDIR)\cl_demo.obj"
	-@erase "$(INTDIR)\cl_demo.sbr"
	-@erase "$(INTDIR)\cl_input.obj"
	-@erase "$(INTDIR)\cl_input.sbr"
	-@erase "$(INTDIR)\cl_main.obj"
	-@erase "$(INTDIR)\cl_main.sbr"
	-@erase "$(INTDIR)\cl_parse.obj"
	-@erase "$(INTDIR)\cl_parse.sbr"
	-@erase "$(INTDIR)\cl_staticfx.obj"
	-@erase "$(INTDIR)\cl_staticfx.sbr"
	-@erase "$(INTDIR)\cl_tent.obj"
	-@erase "$(INTDIR)\cl_tent.sbr"
	-@erase "$(INTDIR)\cmd.obj"
	-@erase "$(INTDIR)\cmd.sbr"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\common.sbr"
	-@erase "$(INTDIR)\conproc.obj"
	-@erase "$(INTDIR)\conproc.sbr"
	-@erase "$(INTDIR)\console.obj"
	-@erase "$(INTDIR)\console.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\cvar.obj"
	-@erase "$(INTDIR)\cvar.sbr"
	-@erase "$(INTDIR)\gl_crosshair.obj"
	-@erase "$(INTDIR)\gl_crosshair.sbr"
	-@erase "$(INTDIR)\gl_draw.obj"
	-@erase "$(INTDIR)\gl_draw.sbr"
	-@erase "$(INTDIR)\gl_flares.obj"
	-@erase "$(INTDIR)\gl_flares.sbr"
	-@erase "$(INTDIR)\gl_font.obj"
	-@erase "$(INTDIR)\gl_font.sbr"
	-@erase "$(INTDIR)\gl_hc_texes.obj"
	-@erase "$(INTDIR)\gl_hc_texes.sbr"
	-@erase "$(INTDIR)\gl_md2.obj"
	-@erase "$(INTDIR)\gl_md2.sbr"
	-@erase "$(INTDIR)\gl_mdl.obj"
	-@erase "$(INTDIR)\gl_mdl.sbr"
	-@erase "$(INTDIR)\gl_mirror.obj"
	-@erase "$(INTDIR)\gl_mirror.sbr"
	-@erase "$(INTDIR)\gl_model.obj"
	-@erase "$(INTDIR)\gl_model.sbr"
	-@erase "$(INTDIR)\gl_part.obj"
	-@erase "$(INTDIR)\gl_part.sbr"
	-@erase "$(INTDIR)\gl_part2.obj"
	-@erase "$(INTDIR)\gl_part2.sbr"
	-@erase "$(INTDIR)\gl_refrag.obj"
	-@erase "$(INTDIR)\gl_refrag.sbr"
	-@erase "$(INTDIR)\gl_rlight.obj"
	-@erase "$(INTDIR)\gl_rlight.sbr"
	-@erase "$(INTDIR)\gl_rmain.obj"
	-@erase "$(INTDIR)\gl_rmain.sbr"
	-@erase "$(INTDIR)\gl_rmisc.obj"
	-@erase "$(INTDIR)\gl_rmisc.sbr"
	-@erase "$(INTDIR)\gl_rscript.obj"
	-@erase "$(INTDIR)\gl_rscript.sbr"
	-@erase "$(INTDIR)\gl_rsurf.obj"
	-@erase "$(INTDIR)\gl_rsurf.sbr"
	-@erase "$(INTDIR)\gl_screen.obj"
	-@erase "$(INTDIR)\gl_screen.sbr"
	-@erase "$(INTDIR)\gl_sky.obj"
	-@erase "$(INTDIR)\gl_sky.sbr"
	-@erase "$(INTDIR)\gl_sprite.obj"
	-@erase "$(INTDIR)\gl_sprite.sbr"
	-@erase "$(INTDIR)\gl_texman.obj"
	-@erase "$(INTDIR)\gl_texman.sbr"
	-@erase "$(INTDIR)\gl_vidnt.obj"
	-@erase "$(INTDIR)\gl_vidnt.sbr"
	-@erase "$(INTDIR)\gl_warp.obj"
	-@erase "$(INTDIR)\gl_warp.sbr"
	-@erase "$(INTDIR)\host.obj"
	-@erase "$(INTDIR)\host.sbr"
	-@erase "$(INTDIR)\host_cmd.obj"
	-@erase "$(INTDIR)\host_cmd.sbr"
	-@erase "$(INTDIR)\in_win.obj"
	-@erase "$(INTDIR)\in_win.sbr"
	-@erase "$(INTDIR)\keys.obj"
	-@erase "$(INTDIR)\keys.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\menu.sbr"
	-@erase "$(INTDIR)\menu_help.obj"
	-@erase "$(INTDIR)\menu_help.sbr"
	-@erase "$(INTDIR)\menu_main.obj"
	-@erase "$(INTDIR)\menu_main.sbr"
	-@erase "$(INTDIR)\menu_multi.obj"
	-@erase "$(INTDIR)\menu_multi.sbr"
	-@erase "$(INTDIR)\menu_options.obj"
	-@erase "$(INTDIR)\menu_options.sbr"
	-@erase "$(INTDIR)\menu_quit.obj"
	-@erase "$(INTDIR)\menu_quit.sbr"
	-@erase "$(INTDIR)\menu_single.obj"
	-@erase "$(INTDIR)\menu_single.sbr"
	-@erase "$(INTDIR)\net_dgrm.obj"
	-@erase "$(INTDIR)\net_dgrm.sbr"
	-@erase "$(INTDIR)\net_loop.obj"
	-@erase "$(INTDIR)\net_loop.sbr"
	-@erase "$(INTDIR)\net_main.obj"
	-@erase "$(INTDIR)\net_main.sbr"
	-@erase "$(INTDIR)\net_win.obj"
	-@erase "$(INTDIR)\net_win.sbr"
	-@erase "$(INTDIR)\net_wins.obj"
	-@erase "$(INTDIR)\net_wins.sbr"
	-@erase "$(INTDIR)\net_wipx.obj"
	-@erase "$(INTDIR)\net_wipx.sbr"
	-@erase "$(INTDIR)\pr_cmds.obj"
	-@erase "$(INTDIR)\pr_cmds.sbr"
	-@erase "$(INTDIR)\pr_edict.obj"
	-@erase "$(INTDIR)\pr_edict.sbr"
	-@erase "$(INTDIR)\pr_exec.obj"
	-@erase "$(INTDIR)\pr_exec.sbr"
	-@erase "$(INTDIR)\sbar.obj"
	-@erase "$(INTDIR)\sbar.sbr"
	-@erase "$(INTDIR)\snd_dma.obj"
	-@erase "$(INTDIR)\snd_dma.sbr"
	-@erase "$(INTDIR)\snd_mem.obj"
	-@erase "$(INTDIR)\snd_mem.sbr"
	-@erase "$(INTDIR)\snd_mix.obj"
	-@erase "$(INTDIR)\snd_mix.sbr"
	-@erase "$(INTDIR)\snd_win.obj"
	-@erase "$(INTDIR)\snd_win.sbr"
	-@erase "$(INTDIR)\sv_main.obj"
	-@erase "$(INTDIR)\sv_main.sbr"
	-@erase "$(INTDIR)\sv_move.obj"
	-@erase "$(INTDIR)\sv_move.sbr"
	-@erase "$(INTDIR)\sv_phys.obj"
	-@erase "$(INTDIR)\sv_phys.sbr"
	-@erase "$(INTDIR)\sv_user.obj"
	-@erase "$(INTDIR)\sv_user.sbr"
	-@erase "$(INTDIR)\sys_win.obj"
	-@erase "$(INTDIR)\sys_win.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\view.obj"
	-@erase "$(INTDIR)\view.sbr"
	-@erase "$(INTDIR)\wad.obj"
	-@erase "$(INTDIR)\wad.sbr"
	-@erase "$(INTDIR)\winquake.res"
	-@erase "$(INTDIR)\world.obj"
	-@erase "$(INTDIR)\world.sbr"
	-@erase "$(INTDIR)\zone.obj"
	-@erase "$(INTDIR)\zone.sbr"
	-@erase "$(OUTDIR)\WinQuake.bsc"
	-@erase "d:\quake\telejano4.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /ML /W3 /Zd /Ox /Ot /Oa /Og /Oi /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "GLQUAKE" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\WinQuake.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GA /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\winquake.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinQuake.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cl_demo.sbr" \
	"$(INTDIR)\cl_input.sbr" \
	"$(INTDIR)\cl_main.sbr" \
	"$(INTDIR)\cl_parse.sbr" \
	"$(INTDIR)\cl_staticfx.sbr" \
	"$(INTDIR)\cl_tent.sbr" \
	"$(INTDIR)\sv_main.sbr" \
	"$(INTDIR)\sv_move.sbr" \
	"$(INTDIR)\sv_phys.sbr" \
	"$(INTDIR)\sv_user.sbr" \
	"$(INTDIR)\host.sbr" \
	"$(INTDIR)\host_cmd.sbr" \
	"$(INTDIR)\net_dgrm.sbr" \
	"$(INTDIR)\net_loop.sbr" \
	"$(INTDIR)\net_main.sbr" \
	"$(INTDIR)\net_win.sbr" \
	"$(INTDIR)\net_wins.sbr" \
	"$(INTDIR)\net_wipx.sbr" \
	"$(INTDIR)\menu.sbr" \
	"$(INTDIR)\menu_help.sbr" \
	"$(INTDIR)\menu_main.sbr" \
	"$(INTDIR)\menu_multi.sbr" \
	"$(INTDIR)\menu_options.sbr" \
	"$(INTDIR)\menu_quit.sbr" \
	"$(INTDIR)\menu_single.sbr" \
	"$(INTDIR)\gl_crosshair.sbr" \
	"$(INTDIR)\gl_draw.sbr" \
	"$(INTDIR)\gl_flares.sbr" \
	"$(INTDIR)\gl_font.sbr" \
	"$(INTDIR)\gl_hc_texes.sbr" \
	"$(INTDIR)\gl_md2.sbr" \
	"$(INTDIR)\gl_mdl.sbr" \
	"$(INTDIR)\gl_mirror.sbr" \
	"$(INTDIR)\gl_model.sbr" \
	"$(INTDIR)\gl_part.sbr" \
	"$(INTDIR)\gl_part2.sbr" \
	"$(INTDIR)\gl_refrag.sbr" \
	"$(INTDIR)\gl_rlight.sbr" \
	"$(INTDIR)\gl_rmain.sbr" \
	"$(INTDIR)\gl_rmisc.sbr" \
	"$(INTDIR)\gl_rscript.sbr" \
	"$(INTDIR)\gl_rsurf.sbr" \
	"$(INTDIR)\gl_screen.sbr" \
	"$(INTDIR)\gl_sky.sbr" \
	"$(INTDIR)\gl_sprite.sbr" \
	"$(INTDIR)\gl_texman.sbr" \
	"$(INTDIR)\gl_vidnt.sbr" \
	"$(INTDIR)\gl_warp.sbr" \
	"$(INTDIR)\pr_cmds.sbr" \
	"$(INTDIR)\pr_edict.sbr" \
	"$(INTDIR)\pr_exec.sbr" \
	"$(INTDIR)\cd_win.sbr" \
	"$(INTDIR)\snd_dma.sbr" \
	"$(INTDIR)\snd_mem.sbr" \
	"$(INTDIR)\snd_mix.sbr" \
	"$(INTDIR)\snd_win.sbr" \
	"$(INTDIR)\chase.sbr" \
	"$(INTDIR)\cmd.sbr" \
	"$(INTDIR)\common.sbr" \
	"$(INTDIR)\conproc.sbr" \
	"$(INTDIR)\console.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\cvar.sbr" \
	"$(INTDIR)\in_win.sbr" \
	"$(INTDIR)\keys.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\sbar.sbr" \
	"$(INTDIR)\sys_win.sbr" \
	"$(INTDIR)\view.sbr" \
	"$(INTDIR)\wad.sbr" \
	"$(INTDIR)\world.sbr" \
	"$(INTDIR)\zone.sbr"

"$(OUTDIR)\WinQuake.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=.\dxsdk\sdk\lib\dxguid.lib comctl32.lib winmm.lib wsock32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mss.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\telejano4.pdb" /machine:I386 /out:"d:\quake\telejano4.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cl_demo.obj" \
	"$(INTDIR)\cl_input.obj" \
	"$(INTDIR)\cl_main.obj" \
	"$(INTDIR)\cl_parse.obj" \
	"$(INTDIR)\cl_staticfx.obj" \
	"$(INTDIR)\cl_tent.obj" \
	"$(INTDIR)\sv_main.obj" \
	"$(INTDIR)\sv_move.obj" \
	"$(INTDIR)\sv_phys.obj" \
	"$(INTDIR)\sv_user.obj" \
	"$(INTDIR)\host.obj" \
	"$(INTDIR)\host_cmd.obj" \
	"$(INTDIR)\net_dgrm.obj" \
	"$(INTDIR)\net_loop.obj" \
	"$(INTDIR)\net_main.obj" \
	"$(INTDIR)\net_win.obj" \
	"$(INTDIR)\net_wins.obj" \
	"$(INTDIR)\net_wipx.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\menu_help.obj" \
	"$(INTDIR)\menu_main.obj" \
	"$(INTDIR)\menu_multi.obj" \
	"$(INTDIR)\menu_options.obj" \
	"$(INTDIR)\menu_quit.obj" \
	"$(INTDIR)\menu_single.obj" \
	"$(INTDIR)\gl_crosshair.obj" \
	"$(INTDIR)\gl_draw.obj" \
	"$(INTDIR)\gl_flares.obj" \
	"$(INTDIR)\gl_font.obj" \
	"$(INTDIR)\gl_hc_texes.obj" \
	"$(INTDIR)\gl_md2.obj" \
	"$(INTDIR)\gl_mdl.obj" \
	"$(INTDIR)\gl_mirror.obj" \
	"$(INTDIR)\gl_model.obj" \
	"$(INTDIR)\gl_part.obj" \
	"$(INTDIR)\gl_part2.obj" \
	"$(INTDIR)\gl_refrag.obj" \
	"$(INTDIR)\gl_rlight.obj" \
	"$(INTDIR)\gl_rmain.obj" \
	"$(INTDIR)\gl_rmisc.obj" \
	"$(INTDIR)\gl_rscript.obj" \
	"$(INTDIR)\gl_rsurf.obj" \
	"$(INTDIR)\gl_screen.obj" \
	"$(INTDIR)\gl_sky.obj" \
	"$(INTDIR)\gl_sprite.obj" \
	"$(INTDIR)\gl_texman.obj" \
	"$(INTDIR)\gl_vidnt.obj" \
	"$(INTDIR)\gl_warp.obj" \
	"$(INTDIR)\pr_cmds.obj" \
	"$(INTDIR)\pr_edict.obj" \
	"$(INTDIR)\pr_exec.obj" \
	"$(INTDIR)\cd_win.obj" \
	"$(INTDIR)\snd_dma.obj" \
	"$(INTDIR)\snd_mem.obj" \
	"$(INTDIR)\snd_mix.obj" \
	"$(INTDIR)\snd_win.obj" \
	"$(INTDIR)\chase.obj" \
	"$(INTDIR)\cmd.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\conproc.obj" \
	"$(INTDIR)\console.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cvar.obj" \
	"$(INTDIR)\in_win.obj" \
	"$(INTDIR)\keys.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\sbar.obj" \
	"$(INTDIR)\sys_win.obj" \
	"$(INTDIR)\view.obj" \
	"$(INTDIR)\wad.obj" \
	"$(INTDIR)\world.obj" \
	"$(INTDIR)\zone.obj" \
	"$(INTDIR)\winquake.res"

"d:\quake\telejano4.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("WinQuake.dep")
!INCLUDE "WinQuake.dep"
!ELSE 
!MESSAGE Warning: cannot find "WinQuake.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "winquake - Win32 GL Debug" || "$(CFG)" == "winquake - Win32 GL Release"
SOURCE=.\cl_demo.c

"$(INTDIR)\cl_demo.obj"	"$(INTDIR)\cl_demo.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cl_input.c

"$(INTDIR)\cl_input.obj"	"$(INTDIR)\cl_input.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cl_main.c

"$(INTDIR)\cl_main.obj"	"$(INTDIR)\cl_main.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cl_parse.c

"$(INTDIR)\cl_parse.obj"	"$(INTDIR)\cl_parse.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cl_staticfx.c

"$(INTDIR)\cl_staticfx.obj"	"$(INTDIR)\cl_staticfx.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cl_tent.c

"$(INTDIR)\cl_tent.obj"	"$(INTDIR)\cl_tent.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sv_main.c

"$(INTDIR)\sv_main.obj"	"$(INTDIR)\sv_main.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sv_move.c

"$(INTDIR)\sv_move.obj"	"$(INTDIR)\sv_move.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sv_phys.c

"$(INTDIR)\sv_phys.obj"	"$(INTDIR)\sv_phys.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sv_user.c

"$(INTDIR)\sv_user.obj"	"$(INTDIR)\sv_user.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\host.c

"$(INTDIR)\host.obj"	"$(INTDIR)\host.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\host_cmd.c

"$(INTDIR)\host_cmd.obj"	"$(INTDIR)\host_cmd.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\net_dgrm.c

"$(INTDIR)\net_dgrm.obj"	"$(INTDIR)\net_dgrm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\net_loop.c

"$(INTDIR)\net_loop.obj"	"$(INTDIR)\net_loop.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\net_main.c

"$(INTDIR)\net_main.obj"	"$(INTDIR)\net_main.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\net_win.c

"$(INTDIR)\net_win.obj"	"$(INTDIR)\net_win.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\net_wins.c

"$(INTDIR)\net_wins.obj"	"$(INTDIR)\net_wins.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\net_wipx.c

"$(INTDIR)\net_wipx.obj"	"$(INTDIR)\net_wipx.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\menu.c

"$(INTDIR)\menu.obj"	"$(INTDIR)\menu.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\menu_help.c

"$(INTDIR)\menu_help.obj"	"$(INTDIR)\menu_help.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\menu_main.c

"$(INTDIR)\menu_main.obj"	"$(INTDIR)\menu_main.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\menu_multi.c

"$(INTDIR)\menu_multi.obj"	"$(INTDIR)\menu_multi.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\menu_options.c

"$(INTDIR)\menu_options.obj"	"$(INTDIR)\menu_options.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\menu_quit.c

"$(INTDIR)\menu_quit.obj"	"$(INTDIR)\menu_quit.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\menu_single.c

"$(INTDIR)\menu_single.obj"	"$(INTDIR)\menu_single.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_crosshair.c

"$(INTDIR)\gl_crosshair.obj"	"$(INTDIR)\gl_crosshair.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_draw.c

"$(INTDIR)\gl_draw.obj"	"$(INTDIR)\gl_draw.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_flares.c

"$(INTDIR)\gl_flares.obj"	"$(INTDIR)\gl_flares.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_font.c

"$(INTDIR)\gl_font.obj"	"$(INTDIR)\gl_font.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_hc_texes.c

"$(INTDIR)\gl_hc_texes.obj"	"$(INTDIR)\gl_hc_texes.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_md2.c

"$(INTDIR)\gl_md2.obj"	"$(INTDIR)\gl_md2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_mdl.c

"$(INTDIR)\gl_mdl.obj"	"$(INTDIR)\gl_mdl.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_mirror.c

"$(INTDIR)\gl_mirror.obj"	"$(INTDIR)\gl_mirror.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_model.c

"$(INTDIR)\gl_model.obj"	"$(INTDIR)\gl_model.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_part.c

"$(INTDIR)\gl_part.obj"	"$(INTDIR)\gl_part.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_part2.c

"$(INTDIR)\gl_part2.obj"	"$(INTDIR)\gl_part2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_refrag.c

"$(INTDIR)\gl_refrag.obj"	"$(INTDIR)\gl_refrag.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_rlight.c

"$(INTDIR)\gl_rlight.obj"	"$(INTDIR)\gl_rlight.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_rmain.c

"$(INTDIR)\gl_rmain.obj"	"$(INTDIR)\gl_rmain.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_rmisc.c

"$(INTDIR)\gl_rmisc.obj"	"$(INTDIR)\gl_rmisc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_rscript.c

"$(INTDIR)\gl_rscript.obj"	"$(INTDIR)\gl_rscript.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_rsurf.c

"$(INTDIR)\gl_rsurf.obj"	"$(INTDIR)\gl_rsurf.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_screen.c

"$(INTDIR)\gl_screen.obj"	"$(INTDIR)\gl_screen.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_sky.c

"$(INTDIR)\gl_sky.obj"	"$(INTDIR)\gl_sky.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_sprite.c

"$(INTDIR)\gl_sprite.obj"	"$(INTDIR)\gl_sprite.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_texman.c

"$(INTDIR)\gl_texman.obj"	"$(INTDIR)\gl_texman.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_vidnt.c

"$(INTDIR)\gl_vidnt.obj"	"$(INTDIR)\gl_vidnt.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gl_warp.c

"$(INTDIR)\gl_warp.obj"	"$(INTDIR)\gl_warp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pr_cmds.c

"$(INTDIR)\pr_cmds.obj"	"$(INTDIR)\pr_cmds.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pr_edict.c

"$(INTDIR)\pr_edict.obj"	"$(INTDIR)\pr_edict.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pr_exec.c

"$(INTDIR)\pr_exec.obj"	"$(INTDIR)\pr_exec.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cd_win.c

"$(INTDIR)\cd_win.obj"	"$(INTDIR)\cd_win.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\snd_dma.c

"$(INTDIR)\snd_dma.obj"	"$(INTDIR)\snd_dma.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\snd_mem.c

"$(INTDIR)\snd_mem.obj"	"$(INTDIR)\snd_mem.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\snd_mix.c

"$(INTDIR)\snd_mix.obj"	"$(INTDIR)\snd_mix.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\snd_win.c

"$(INTDIR)\snd_win.obj"	"$(INTDIR)\snd_win.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\chase.c

"$(INTDIR)\chase.obj"	"$(INTDIR)\chase.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cmd.c

"$(INTDIR)\cmd.obj"	"$(INTDIR)\cmd.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\common.c

"$(INTDIR)\common.obj"	"$(INTDIR)\common.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\conproc.c

"$(INTDIR)\conproc.obj"	"$(INTDIR)\conproc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\console.c

"$(INTDIR)\console.obj"	"$(INTDIR)\console.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\crc.c

"$(INTDIR)\crc.obj"	"$(INTDIR)\crc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cvar.c

"$(INTDIR)\cvar.obj"	"$(INTDIR)\cvar.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\in_win.c

"$(INTDIR)\in_win.obj"	"$(INTDIR)\in_win.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\keys.c

"$(INTDIR)\keys.obj"	"$(INTDIR)\keys.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mathlib.c

"$(INTDIR)\mathlib.obj"	"$(INTDIR)\mathlib.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sbar.c

"$(INTDIR)\sbar.obj"	"$(INTDIR)\sbar.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sys_win.c

"$(INTDIR)\sys_win.obj"	"$(INTDIR)\sys_win.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\view.c

"$(INTDIR)\view.obj"	"$(INTDIR)\view.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\wad.c

"$(INTDIR)\wad.obj"	"$(INTDIR)\wad.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\winquake.rc

"$(INTDIR)\winquake.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\world.c

"$(INTDIR)\world.obj"	"$(INTDIR)\world.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\zone.c

"$(INTDIR)\zone.obj"	"$(INTDIR)\zone.sbr" : $(SOURCE) "$(INTDIR)"



!ENDIF 

