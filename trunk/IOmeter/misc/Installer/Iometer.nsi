###############################################################################
##                                                                           ##
##  Iometer & Dynamo / Iometer.nsi                                           ##
##                                                                           ##
## ------------------------------------------------------------------------- ##
##                                                                           ##
##  Job .......: The NSIS Script file for the Windows Installer.             ##
##                                                                           ##
## ------------------------------------------------------------------------- ##
##                                                                           ##
##  This program is free software; you can redistribute it and/or modify it  ##
##  under the terms of the GNU General Public License as published by the    ##
##  Free Software Foundation; either version 2 of the License, or (at your   ##
##  option) any later version.                                               ##
##                                                                           ##
##  This program is distributed in the hope that it will be useful, but      ##
##  WITHOUT ANY WARRANTY; without even the implied warranty of               ##
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            ##
##  GNU General Public License for more details.                             ##
##                                                                           ##
##  You should have received a copy of the GNU General Public License along  ##
##  with this program; if not, write to the Free Software Foundation, Inc.,  ##
##  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.                 ##
##                                                                           ##
## ------------------------------------------------------------------------- ##
##                                                                           ##
##  Remarks ...: <none>                                                      ##
##                                                                           ##
## ------------------------------------------------------------------------- ##
##                                                                           ##
##  Changes ...: 2004-08-17 (daniel.scheibli@edelbyte.org)                   ##
##               - Modified script to reflect new version (2004.07.30).      ##
##               2004-03-24 (daniel.scheibli@edelbyte.org)                   ##
##               - Do not install Iometer.ico any more as the new icon       ##
##                 is now part of the executable itself.                     ##
##               2004-03-20 (daniel.scheibli@edelbyte.org)                   ##
##               - Initial setup of this script.                             ##
##                                                                           ##
###############################################################################



##### Includes
#------------------------------------------------------------------------------
!include "MUI.nsh"
#------------------------------------------------------------------------------



##### Global settings
#------------------------------------------------------------------------------

!define RELDATE "2014.05.28"
!define PLATF "win32"
!define ARCH "i386"
!define RELVER "1.1"

Name "Iometer ${RELVER}"

OutFile "iometer-${RELVER}.${PLATF}.${ARCH}-setup.exe"
InstallDir "$PROGRAMFILES\Iometer.org\Iometer ${RELVER}"

CRCCheck on
XPStyle on

!define MUI_HEADERIMAGE
!define MUI_COMPONENTSPAGE_SMALLDESC

!define MUI_ICON ".\iometer.ico"
!define MUI_ABORTWARNING

!define MUI_UNICON ".\uninst.ico"
!define MUI_UNABORTWARNING
#------------------------------------------------------------------------------



###############################################################################
##                                                                           ##
##   Pages                                                                   ##
##                                                                           ##
###############################################################################
#------------------------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE.TXT"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
#------------------------------------------------------------------------------
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
#------------------------------------------------------------------------------



###############################################################################
##                                                                           ##
##   Sections                                                                ##
##                                                                           ##
###############################################################################
#------------------------------------------------------------------------------
SubSection /e "Components"
	Section "Binaries (required)" SectionBinaries

		SectionIn RO
		SetOutPath $INSTDIR
		
		File ..\..\src\msvs11\Release\Win32\Iometer.exe
		File ..\..\src\msvs11\Release\Win32\Dynamo.exe

		WriteUninstaller "uninstall.exe"

	SectionEnd
	Section "Users Guide" SectionUsersGuide

		SetOutPath $INSTDIR
		
		File ..\..\Docs\Iometer.pdf

	SectionEnd
	Section /o "Import Wizard for MS Access" SectionImportWizard

		SetOutPath $INSTDIR
  
		File ..\Wizard.mdb

	SectionEnd
SubSectionEnd
#------------------------------------------------------------------------------
Section "Start Menu Shortcuts" SectionStartMenu

	CreateDirectory "$SMPROGRAMS\Iometer ${RELVER}"
	CreateShortCut  "$SMPROGRAMS\Iometer ${RELVER}\Iometer.lnk"                     "$INSTDIR\iometer.exe"   "" "$INSTDIR\iometer.exe"   0
        IfFileExists "$INSTDIR\iometer.pdf"   "" +2
		CreateShortCut "$SMPROGRAMS\Iometer ${RELVER}\Iometer Users Guide.lnk"  "$INSTDIR\iometer.pdf"   "" "$INSTDIR\iometer.pdf"   0
	CreateShortCut  "$SMPROGRAMS\Iometer ${RELVER}\Import Wizard for MS Access.lnk" "$INSTDIR\Wizard.mdb"    "" "$INSTDIR\Wizard.mdb"    0
	CreateShortCut  "$SMPROGRAMS\Iometer ${RELVER}\Uninstall Iometer.lnk"           "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

SectionEnd
#------------------------------------------------------------------------------
Section "Uninstall"
  
	Delete "$SMPROGRAMS\Iometer ${RELVER}\*.*"
	RMDir  "$SMPROGRAMS\Iometer ${RELVER}"

	Delete "$INSTDIR\Iometer.exe"
	Delete "$INSTDIR\Dynamo.exe"
	Delete "$INSTDIR\Iometer.pdf"
	Delete "$INSTDIR\Wizard.mdb"
	Delete "$INSTDIR\uninstall.exe"
	RMDir  "$INSTDIR"

SectionEnd
#------------------------------------------------------------------------------




###############################################################################
##                                                                           ##
##   Language settings                                                       ##
##                                                                           ##
###############################################################################
#------------------------------------------------------------------------------
!insertmacro MUI_LANGUAGE "English"

LangString DESC_SectionBinaries     ${LANG_ENGLISH} "The binaries (iometer.exe and dynamo.exe) needed to run Iometer."
LangString DESC_SectionUsersGuide   ${LANG_ENGLISH} "The complete Iometer Users Guide (pdf format)."
LangString DESC_SectionImportWizard ${LANG_ENGLISH} "Import Wizard to import Iometer result files (cvs format) into Microsoft Access."
LangString DESC_SectionStartMenu    ${LANG_ENGLISH} "Create an subfolder for Iometer in the Start Menu."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN

	!insertmacro MUI_DESCRIPTION_TEXT ${SectionBinaries}     $(DESC_SectionBinaries)
	!insertmacro MUI_DESCRIPTION_TEXT ${SectionUsersGuide}   $(DESC_SectionUsersGuide)
	!insertmacro MUI_DESCRIPTION_TEXT ${SectionImportWizard} $(DESC_SectionImportWizard)
	!insertmacro MUI_DESCRIPTION_TEXT ${SectionStartMenu}    $(DESC_SectionStartMenu)

!insertmacro MUI_FUNCTION_DESCRIPTION_END
#------------------------------------------------------------------------------



