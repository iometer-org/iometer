/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / TextDisplay.cpp                                        ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the CTextDisplay class, which       ## */
/* ##               provides the large text in the "Presentation Meter"   ## */
/* ##               dialog.                                               ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Intel Open Source License                                          ## */
/* ##                                                                     ## */
/* ##  Copyright (c) 2001 Intel Corporation                               ## */
/* ##  All rights reserved.                                               ## */
/* ##  Redistribution and use in source and binary forms, with or         ## */
/* ##  without modification, are permitted provided that the following    ## */
/* ##  conditions are met:                                                ## */
/* ##                                                                     ## */
/* ##  Redistributions of source code must retain the above copyright     ## */
/* ##  notice, this list of conditions and the following disclaimer.      ## */
/* ##                                                                     ## */
/* ##  Redistributions in binary form must reproduce the above copyright  ## */
/* ##  notice, this list of conditions and the following disclaimer in    ## */
/* ##  the documentation and/or other materials provided with the         ## */
/* ##  distribution.                                                      ## */
/* ##                                                                     ## */
/* ##  Neither the name of the Intel Corporation nor the names of its     ## */
/* ##  contributors may be used to endorse or promote products derived    ## */
/* ##  from this software without specific prior written permission.      ## */
/* ##                                                                     ## */
/* ##  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             ## */
/* ##  CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,      ## */
/* ##  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           ## */
/* ##  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           ## */
/* ##  DISCLAIMED. IN NO EVENT SHALL THE INTEL OR ITS  CONTRIBUTORS BE    ## */
/* ##  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,   ## */
/* ##  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,           ## */
/* ##  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,    ## */
/* ##  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    ## */
/* ##  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR     ## */
/* ##  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT    ## */
/* ##  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY    ## */
/* ##  OF SUCH DAMAGE.                                                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "stdafx.h"
#include "TextDisplay.h"

// Needed for MFC Library support for assisting in finding memory leaks
//
// NOTE: Based on the documentation[1] I found, it should be enough to have
//       a "#define new DEBUG_NEW" statement for the case, that we are
//       running Windows. There should be no need for checking the _DEBUG
//       flag and no need for redefiniting the THIS_FILE string. Maybe there
//       will be a MFC hacker who could advice here.
//       [1] = http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_mfc_debug_new.asp
//
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#ifdef IOMTR_SETTING_MFC_MEMALLOC_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

IMPLEMENT_DYNAMIC(CTextDisplay, CWnd)

// Message mapping
    BEGIN_MESSAGE_MAP(CTextDisplay, CWnd)
    //{{AFX_MSG_MAP(CTextDisplay)
    ON_WM_PAINT()
    ON_WM_CREATE()
    //}}AFX_MSG_MAP
    END_MESSAGE_MAP()

// Static member initialization
BOOL CTextDisplay::registered = Register();

//
// Initialize class member variables.
//
CTextDisplay::CTextDisplay()
{
	text = "";
}

//
// Handles WM_CREATE message (Window is being created)
//
int CTextDisplay::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Create the window.
	VERIFY(CWnd::OnCreate(lpCreateStruct) == 0);

	// Store where the text box (rectangle) was placed.
	GetClientRect(&text_box);
	text_alignment = TA_LEFT;
	text_position = text_box.left;

	// Initialize default font settings.
	memset(&font_settings, 0, sizeof(LOGFONT));
	font_settings.lfCharSet = ANSI_CHARSET;
	font_settings.lfOutPrecision = OUT_TT_PRECIS;
	font_settings.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	font_settings.lfQuality = DEFAULT_QUALITY;
	font_settings.lfPitchAndFamily = DEFAULT_PITCH;

	return 0;
}

//
// Removes used memory.  Called after response to WM_NCDESTROY
//
void CTextDisplay::PostNcDestroy()
{
	delete this;
}

//
// Registers the class with windows
//
BOOL CTextDisplay::Register()
{
	WNDCLASS wc;

	// See if the class has already been registered.
	if (GetClassInfo(NULL, "CTextDisplay", &wc)) {
		// Name already registered - ok if it was us
		return (wc.lpfnWndProc == (WNDPROC) TextDisplayWndProc);
	}
	// Register the window class of the control.
	wc.style = CS_GLOBALCLASS | CS_OWNDC | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc = CTextDisplay::TextDisplayWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = NULL;
	wc.hIcon = NULL;
	wc.hCursor =::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "CTextDisplay";

	return RegisterClass(&wc);
}

//
// Window procedure for the "CTextDisplay" window class.  This global function
// handles the creation of new CTextDisplay objects and subclasses the 
// objects so the MFC framework passes messages along to the CTextDisplay 
// member functions.
//
LRESULT CALLBACK EXPORT CTextDisplay::TextDisplayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CWnd *pWnd;

	// See if we're creating the window.
	if (!(pWnd = CWnd::FromHandlePermanent(hWnd)) && message == WM_NCCREATE) {
		// Yes, create the object.
		pWnd = new CTextDisplay();
		pWnd->Attach(hWnd);
	}

	return AfxCallWndProc(pWnd, hWnd, message, wParam, lParam);
}

//
// Handles WM_PAINT message.  Displays the text in the custom control according
// to the pre-specified font settings.
//
void CTextDisplay::OnPaint()
{
	CPaintDC dc(this);	// device context for painting

	// Display the text using the store text settings.
	dc.SetTextAlign(text_alignment);
	dc.SelectObject(&font);
	dc.SetTextColor(text_color);
	dc.SetBkMode(TRANSPARENT);
	dc.TextOut(text_position, text_box.top, text);
}

//
// Initializing text display values.
//
void CTextDisplay::SetAll(CString * show_text, char *font_name, LONG font_size,
			  TextAlignment alignment, BOOL bold, BOOL italic, BOOL underline, COLORREF color)
{
	// Update all settings for the displayed text, but do not update the
	// window until everything has been updated.
	SetText(show_text, FALSE);
	SetFontType(font_name, FALSE);
	SetFontSize(font_size, FALSE);
	SetTextAlignment(alignment, FALSE);
	Bold(bold, FALSE);
	Italic(italic, FALSE);
	Underline(underline, FALSE);
	SetTextColor(color, FALSE);

	// Update the display.
	RedrawWindow();
}

//
// Setting what text should be displayed.  Updating the display if needed.
//
void CTextDisplay::SetText(CString * show_text, BOOL refresh)
{
	if (text == *show_text)
		return;

	text = *show_text;

	if (refresh)
		RedrawWindow();
}

//
// Updating the type of font used by the text display based on the recorded
// font settings.
//
void CTextDisplay::UpdateFont(BOOL refresh)
{
	font.DeleteObject();
	font.CreateFontIndirect(&font_settings);
	SetFont(&font);

	if (refresh)
		RedrawWindow();
}

//
// Setting the text's font.
//
void CTextDisplay::SetFontType(char *font_name, BOOL refresh)
{
	strcpy(font_settings.lfFaceName, font_name);
	UpdateFont();
}

//
// Setting the size of the text to display.
//
void CTextDisplay::SetFontSize(LONG size, BOOL refresh)
{
	font_settings.lfHeight = size;
	UpdateFont();
}

void CTextDisplay::SetTextColor(COLORREF color, BOOL refresh)
{
	text_color = color;

	if (refresh)
		RedrawWindow();
}

//
// Setting whether the text is bold.
//
void CTextDisplay::Bold(BOOL enable, BOOL refresh)
{
	if (enable)
		font_settings.lfWeight = FW_BOLD;
	else
		font_settings.lfWeight = FW_NORMAL;

	UpdateFont();
}

//
// Setting whether the text is italic.
//
void CTextDisplay::Italic(BOOL enable, BOOL refresh)
{
	if (enable)
		font_settings.lfItalic = TRUE;
	else
		font_settings.lfItalic = FALSE;

	UpdateFont();
}

//
// Setting whether the text is underlined.
//
void CTextDisplay::Underline(BOOL enable, BOOL refresh)
{
	if (enable)
		font_settings.lfUnderline = TRUE;
	else
		font_settings.lfUnderline = FALSE;

	UpdateFont();
}

//
// Setting the text's alignment in the custom control box.
//
void CTextDisplay::SetTextAlignment(TextAlignment alignment, BOOL refresh)
{
	switch (alignment) {
	case AlignLeft:
		text_alignment = TA_LEFT;
		text_position = text_box.left;
		break;
	case AlignRight:
		text_alignment = TA_RIGHT;
		text_position = text_box.right;
		break;
	case AlignCenter:
		text_alignment = TA_CENTER;
		text_position = text_box.right / 2;
		break;
	}

	if (refresh)
		RedrawWindow();
}
