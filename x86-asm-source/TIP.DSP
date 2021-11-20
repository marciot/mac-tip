# Microsoft Developer Studio Project File - Name="TIP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=TIP - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TIP.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TIP.MAK" CFG="TIP - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TIP - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "TIP - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "TIP - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f TIP.MAK"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "TIP.EXE"
# PROP BASE Bsc_Name "TIP.BSC"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "NMAKE /f TIP.MAK"
# PROP Rebuild_Opt "/a"
# PROP Target_File "TIP.EXE"
# PROP Bsc_Name "TIP.BSC"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "TIP - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f TIP.MAK"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "TIP.EXE"
# PROP BASE Bsc_Name "TIP.BSC"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "NMAKE /f makefile"
# PROP Rebuild_Opt "/a"
# PROP Target_File "S:\Asm\TIP\tip.exe"
# PROP Bsc_Name "TIP.BSC"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "TIP - Win32 Release"
# Name "TIP - Win32 Debug"

!IF  "$(CFG)" == "TIP - Win32 Release"

!ELSEIF  "$(CFG)" == "TIP - Win32 Debug"

!ENDIF 

# Begin Source File

SOURCE=.\ASPI.ASM
# End Source File
# Begin Source File

SOURCE=..\Include\ASPI.INC
# End Source File
# Begin Source File

SOURCE=.\font.asm
# End Source File
# Begin Source File

SOURCE=.\MACROS.INC
# End Source File
# Begin Source File

SOURCE=.\MAKEFILE
# End Source File
# Begin Source File

SOURCE=.\New.asm
# End Source File
# Begin Source File

SOURCE=.\tip.asm
# End Source File
# Begin Source File

SOURCE=.\tip.cur
# End Source File
# Begin Source File

SOURCE=.\tip.dat
# End Source File
# Begin Source File

SOURCE=.\tip.ico
# End Source File
# Begin Source File

SOURCE=.\tip.rc
# End Source File
# Begin Source File

SOURCE=.\tip.txt
# End Source File
# End Target
# End Project
