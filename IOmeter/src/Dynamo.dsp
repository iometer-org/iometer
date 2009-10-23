# Microsoft Developer Studio Project File - Name="Dynamo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Dynamo - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Dynamo.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Dynamo.mak" CFG="Dynamo - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Dynamo - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Dynamo - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Dynamo - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "IOMTR_OSFAMILY_WINDOWS" /D "IOMTR_OS_WIN32" /D "IOMTR_CPU_I386" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "IOMTR_OSFAMILY_WINDOWS" /D "IOMTR_OS_WIN32" /D "IOMTR_CPU_I386" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "_PULSAR_" /D "IOMTR_OSFAMILY_WINDOWS" /D "IOMTR_OS_WIN32" /D "IOMTR_CPU_I386"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib mswsock.lib version.lib /nologo /subsystem:console /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Dynamo - Win32 Debug"

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
# ADD BASE CPP /nologo /MT /W3 /Gm /GX /ZI /Od /D "IOMTR_OSFAMILY_WINDOWS" /D "IOMTR_OS_WIN32" /D "IOMTR_CPU_I386" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "IOMTR_OSFAMILY_WINDOWS" /D "IOMTR_OS_WIN32" /D "IOMTR_CPU_I386" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /WX /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_PULSAR_" /D "IOMTR_OSFAMILY_WINDOWS" /D "IOMTR_OS_WIN32" /D "IOMTR_CPU_I386"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib mswsock.lib version.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "Dynamo - Win32 Release"
# Name "Dynamo - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ByteOrder.cpp
# End Source File
# Begin Source File

SOURCE=.\IOAccess.cpp
# End Source File
# Begin Source File

SOURCE=.\IOCompletionQ.cpp
# End Source File
# Begin Source File

SOURCE=.\IOCQAIO.cpp
# End Source File
# Begin Source File

SOURCE=.\IOCQVI.cpp
# End Source File
# Begin Source File

SOURCE=.\IOGlobals.cpp
# End Source File
# Begin Source File

SOURCE=.\IOGrunt.cpp
# End Source File
# Begin Source File

SOURCE=.\IOManager.cpp
# End Source File
# Begin Source File

SOURCE=.\IOManagerWin.cpp
# End Source File
# Begin Source File

SOURCE=.\IOPerformance.cpp
# End Source File
# Begin Source File

SOURCE=.\IOPort.cpp
# End Source File
# Begin Source File

SOURCE=.\IOPortTCP.cpp
# End Source File
# Begin Source File

SOURCE=.\IOTarget.cpp
# End Source File
# Begin Source File

SOURCE=.\IOTargetDisk.cpp
# End Source File
# Begin Source File

SOURCE=.\IOTargetTCP.cpp
# End Source File
# Begin Source File

SOURCE=.\IOTargetVI.cpp
# End Source File
# Begin Source File

SOURCE=.\IOTime.cpp
# End Source File
# Begin Source File

SOURCE=.\IOVIPL.cpp
# End Source File
# Begin Source File

SOURCE=.\NetTCP.cpp
# End Source File
# Begin Source File

SOURCE=.\NetVI.cpp
# End Source File
# Begin Source File

SOURCE=.\Network.cpp
# End Source File
# Begin Source File

SOURCE=.\Pulsar.cpp
# End Source File
# Begin Source File

SOURCE=.\Pulsar.rc
# End Source File
# Begin Source File

SOURCE=.\VINic.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
