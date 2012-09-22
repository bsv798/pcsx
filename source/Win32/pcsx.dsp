# Microsoft Developer Studio Project File - Name="pcsx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PCSX - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pcsx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pcsx.mak" CFG="PCSX - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pcsx - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "pcsx - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pcsx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp16 /W3 /GX /O2 /Op /I "../" /I "./" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "__WIN32__" /D "_MSC_VER_" /D PCSX_VERSION=\"1.5\" /D "__i386__" /D "ENABLE_NLS" /D PACKAGE=\"pcsx\" /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x408 /d "NDEBUG"
# ADD RSC /l 0x408 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib zlib.lib gnu_gettext.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "pcsx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D PCSX_VERSION=\"1.3\" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /ZI /Od /I "../" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "__WIN32__" /D "__i386__" /D PCSX_VERSION=\"1.5\" /D "ENABLE_NLS" /D PACKAGE=\"pcsx\" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x408 /d "_DEBUG"
# ADD RSC /l 0x408 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib zlib.lib gnu_gettext.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "pcsx - Win32 Release"
# Name "pcsx - Win32 Debug"
# Begin Group "GUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AboutDlg.c
# End Source File
# Begin Source File

SOURCE=.\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Win32.h
# End Source File
# Begin Source File

SOURCE=.\WndMain.c
# End Source File
# End Group
# Begin Group "Bios"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\PsxBios.c
# End Source File
# Begin Source File

SOURCE=..\PsxBios.h
# End Source File
# End Group
# Begin Group "CdRom"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\CdRom.c
# End Source File
# Begin Source File

SOURCE=..\CdRom.h
# End Source File
# Begin Source File

SOURCE=..\Decode_XA.c
# End Source File
# Begin Source File

SOURCE=..\Decode_XA.h
# End Source File
# End Group
# Begin Group "GPU-SPU-SIO"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sio.c
# End Source File
# Begin Source File

SOURCE=..\Sio.h
# End Source File
# Begin Source File

SOURCE=..\Spu.c
# End Source File
# Begin Source File

SOURCE=..\Spu.h
# End Source File
# End Group
# Begin Group "Debugger"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Debug.h
# End Source File
# Begin Source File

SOURCE=..\DisR3000A.c
# End Source File
# End Group
# Begin Group "GTE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Gte.c
# End Source File
# Begin Source File

SOURCE=..\Gte.h
# End Source File
# End Group
# Begin Group "Memory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\PsxMem.c
# End Source File
# Begin Source File

SOURCE=..\PsxMem.h
# End Source File
# End Group
# Begin Group "Plugins system"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ConfigurePlugins.c
# End Source File
# Begin Source File

SOURCE=.\plugin.c
# End Source File
# Begin Source File

SOURCE=.\plugin.h
# End Source File
# Begin Source File

SOURCE=..\plugins.c
# End Source File
# Begin Source File

SOURCE=..\plugins.h
# End Source File
# End Group
# Begin Group "CPU"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\PsxInterpreter.c
# End Source File
# Begin Source File

SOURCE=..\R3000A.c
# End Source File
# Begin Source File

SOURCE=..\R3000A.h
# End Source File
# End Group
# Begin Group "Hardware registers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\PsxHw.c
# End Source File
# Begin Source File

SOURCE=..\PsxHw.h
# End Source File
# End Group
# Begin Group "DMA"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\PsxDma.c
# End Source File
# Begin Source File

SOURCE=..\PsxDma.h
# End Source File
# End Group
# Begin Group "Mdec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Mdec.c
# End Source File
# Begin Source File

SOURCE=..\Mdec.h
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Coff.h
# End Source File
# Begin Source File

SOURCE=..\Misc.c
# End Source File
# Begin Source File

SOURCE=..\Misc.h
# End Source File
# Begin Source File

SOURCE=..\NoPic.h
# End Source File
# Begin Source File

SOURCE=..\System.h
# End Source File
# End Group
# Begin Group "Counters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\PsxCounters.c
# End Source File
# Begin Source File

SOURCE=..\PsxCounters.h
# End Source File
# End Group
# Begin Group "ix86"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ix86\iGte.h
# End Source File
# Begin Source File

SOURCE=..\ix86\iR3000A.c
# End Source File
# Begin Source File

SOURCE=..\ix86\ix86.c
# End Source File
# Begin Source File

SOURCE=..\ix86\ix86.h
# End Source File
# End Group
# Begin Group "HLE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\PsxHLE.c
# End Source File
# Begin Source File

SOURCE=..\PsxHLE.h
# End Source File
# End Group
# Begin Group "Docs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Docs\ChangeLog.txt
# End Source File
# Begin Source File

SOURCE=..\Docs\CodingStyle.txt
# End Source File
# Begin Source File

SOURCE=..\Docs\License.txt
# End Source File
# Begin Source File

SOURCE=..\Docs\Readme.txt
# End Source File
# Begin Source File

SOURCE=..\Docs\ToDo.txt
# End Source File
# End Group
# Begin Source File

SOURCE=.\Cdrom02.ico
# End Source File
# Begin Source File

SOURCE=.\pcsx.rc
# End Source File
# Begin Source File

SOURCE=..\pcsxAbout.bmp
# End Source File
# Begin Source File

SOURCE=..\PSEmu_Plugin_Defs.h
# End Source File
# Begin Source File

SOURCE=..\PsxCommon.h
# End Source File
# End Target
# End Project
