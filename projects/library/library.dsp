# Microsoft Developer Studio Project File - Name="library" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=library - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "library.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "library.mak" CFG="library - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "library - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "library - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "library - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "library - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "SV_MSOFT" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\svlib.lib"

!ENDIF 

# Begin Target

# Name "library - Win32 Release"
# Name "library - Win32 Debug"
# Begin Source File

SOURCE=..\..\Src\Arf.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Arpors.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Attrib.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Decision.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Environs.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Flag.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Geometry.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Interval.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Ivallist.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Light.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Model.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\niederreiter.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Picture.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Polygon.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Polynml.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Prim.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Qv.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Raytrace.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Read.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Read1.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Render.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\rotations.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Set.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Shade.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Sums.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Surface.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Sv_graph.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Sv_util.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Sve.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Svlis.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\U_attrib.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\U_prim.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\View.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Src\Voronoi.cxx
# End Source File
# End Target
# End Project
