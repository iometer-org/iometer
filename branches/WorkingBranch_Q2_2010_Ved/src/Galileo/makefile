			
#############################################################################
#
#		Copyright (C) Microsoft Corporation 1995, 1996
#       All Rights Reserved.
#
#       MAKEFILE for FILTER directory
#
#############################################################################

!ifdef NTMAKEENV
!include $(NTMAKEENV)\makefile.def
!else
# The current makefile go here
!endif

!if "$(_BUILDARCH)" == "IA64"
C_DEFINES=$(C_DEFINES) -DIOMTR_OSFAMILY_WINDOWS -DIOMTR_OS_WIN64 -DIOMTR_CPU_IA64
!elseif "$(_BUILDARCH)" == "AMD64"
C_DEFINES=$(C_DEFINES) -DIOMTR_OSFAMILY_WINDOWS -DIOMTR_OS_WIN64 -DIOMTR_CPU_X86_64
!else
C_DEFINES=$(C_DEFINES) -DIOMTR_OSFAMILY_WINDOWS -DIOMTR_OS_WIN32 -DIOMTR_CPU_I386
!endif