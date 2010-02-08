/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / MeterCtrl.cpp                                          ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the CMeterCtrl class, which         ## */
/* ##               provides the "speedometer" control in the             ## */
/* ##               "Presentation Meter" dialog.                          ## */
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
/* ##  Changes ...: 2006-08-04 (vedrand@yahoo.com)                        ## */
/* ##               - misc syntactical -- explicit pow() arg typecasts    ## */
/* ##               2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - Needed to cast parameters to some functions         ## */
/* ##                 because latest Visual Studio C++ library has        ## */
/* ##                 multiple entry points for those functions.          ## */
/* ##               2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include <math.h>
#include "stdafx.h"
#include "MeterCtrl.h"
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
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#define	PI				3.14159265

IMPLEMENT_DYNAMIC(CMeterCtrl, CWnd)
// Message mapping
    BEGIN_MESSAGE_MAP(CMeterCtrl, CWnd)
    //{{AFX_MSG_MAP(CMeterCtrl)
    ON_WM_PAINT()
    ON_WM_CREATE()
    //}}AFX_MSG_MAP
    END_MESSAGE_MAP()

// Static member initialization
// (This verifies that the "meter" window class is registered before CMeterCtrl is used.)
BOOL CMeterCtrl::is_registered = Register();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMeterCtrl::CMeterCtrl()
{
	// Initialize the meter.
	value = 0;
	min_range = 0;
	max_range = 100;

	// Setting the low watermark to -1 to indicate that it has not been set 
	// yet.  This prevents the low watermark from always being zero.
	low_value = -1;
	high_value = 0;
	show_watermark = FALSE;
}

//
// Handles WM_CREATE message (Window is being created)
//
int CMeterCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CRect control_box;
	POINT temp_point;

	// Allow the framework to create the window
	VERIFY(CWnd::OnCreate(lpCreateStruct) == 0);

	// Store the control's client area rectangle
	GetClientRect(&control_box);

	// Determine where the pivot point should be located.
	// Get the length of the longest radius that will fit inside the box.
	outer_radius = (int)min(control_box.Width() / 2, control_box.Height()
				/ (1 + cos(PIVOT_ARC_ANGLE * PI / 180)));
	inner_radius = (int)(outer_radius * .70);
	pivot_point.x = (int)(control_box.Width() / 2);
	pivot_point.y = outer_radius;

	// Set the size of the rectangle containing the dial.
	meter_box.left = pivot_point.x - outer_radius;
	meter_box.top = pivot_point.y - outer_radius;
	meter_box.right = pivot_point.x + outer_radius;
	meter_box.bottom = pivot_point.y + outer_radius;

	// Initialize the needle's center pivot circle.
	pivot_radius = inner_radius / 12;
	needle_pivot.SetRect(pivot_point.x - pivot_radius,
			     pivot_point.y - pivot_radius, pivot_point.x + pivot_radius, pivot_point.y + pivot_radius);

	// Set the points for the bounding pie wedge.
	CalculatePoint(0, inner_radius, &(min_point.x), &(min_point.y));
	max_point.x = pivot_point.x + (pivot_point.x - min_point.x);
	max_point.y = min_point.y;

	// Now set the needle to the resting position.
	shown_angle = 0;
	actual_angle = 0;
	SetNeedlePoints();

	// Create the text box for the scale display.
	CalculatePoint(0, inner_radius, &scale_box.right, &temp_point.y);
	CalculatePoint(0, outer_radius, &temp_point.x, &scale_box.bottom);
	CalculatePoint(NEEDLE_SWEEP, inner_radius, &scale_box.left, &scale_box.top);

	// Initialize information needed to display the scale and labels.
	scale_font.CreateFont((int)(outer_radius * .2), 0, 0, 0, FW_NORMAL, FALSE,
			      FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
			      DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
	label_font.CreateFont((int)(outer_radius * .18), 0, 0, 0, FW_NORMAL, FALSE,
			      FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
			      DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
	marker_pen.CreatePen(PS_SOLID, 2, 0x00FFFFFF);
	label_box_size = (int)(outer_radius * .1);
	label_radius = (int)(outer_radius * .8275);
	tick_mark_radius = (int)(outer_radius * .96);
	UpdateScaleInfo();
	UpdateLabelInfo();

	// Set the points for the rectangle surrounding the watermark arc.
	watermark_box.left = pivot_point.x - inner_radius - 2;
	watermark_box.top = pivot_point.y - inner_radius - 2;
	watermark_box.right = pivot_point.x + inner_radius + 2;
	watermark_box.bottom = pivot_point.y + inner_radius + 2;

	return 0;
}

//
// Set the range of the meter and updates the display.
//
void CMeterCtrl::SetRange(int range1, int range2)
{
	// Disallow setting the ranges to the same value.
	if (range1 == range2)
		return;

	min_range = ((range1 < range2) ? range1 : range2);
	max_range = ((range1 > range2) ? range1 : range2);

	// Make sure that the needle value is within the new range and update
	// the actual angle that the needle should be.
	if (value < min_range)
		actual_angle = 0;
	else if (value > max_range)
		actual_angle = NEEDLE_SWEEP;
	else
		actual_angle = (int)(value / (max_range - min_range) * NEEDLE_SWEEP);

	// Update the displayed scale and marker labels.
	UpdateScaleInfo();
	UpdateLabelInfo();

	RedrawWindow();
}

//
// Resetting information used to maintain watermark range.
//
void CMeterCtrl::ResetWatermark()
{
	// Setting the low watermark to -1 to indicate that it has not been set 
	// yet.  This prevents the low watermark from always being zero.
	low_value = -1;
	high_value = 0;

	RedrawWindow();
}

//
// Updating the information used to display the scale at the bottom of the 
// meter.
//
void CMeterCtrl::UpdateScaleInfo()
{
	int p, scale;

	// Only show text if entire scale cannot be shown by the label markers.
	if (max_range <= 10) {
		scale_text = "";
		return;
	}
	// Calculate the new scale.
	scale = (int)powf(10.0, floorf(log10f((float)max_range)));

	if ((max_range == (int)powf(10.0, floorf(log10f((float)max_range))))) {
		// max_range is a power of 10
		scale /= 10;
	}
	// Format the displayed scale text.
	scale_text.Format("x%d", scale);

	// Add commas into the number for easier reading.
	for (p = 3; p < scale_text.GetLength() - 1; p += 4) {
		scale_text = scale_text.Left(scale_text.GetLength() - p)
		    + "," + scale_text.Right(p);
	}
}

//
// Updating the information used to display the marker labels along the 
// outer edge of the meter.  Because a scale is used, we are ensured that
// marker labels will only display values from 0 to 10, inclusive.
//
void CMeterCtrl::UpdateLabelInfo()
{
	float range_diff;
	float display_range;	// range normalized to ##.##### for ranges > 10
	float label_increment;
	float display_value;
	int label_angle;
	LONG x, y;		// temporary point coordinates

	// Sets the range that the marker labels will span.
	range_diff = (float) (max_range - min_range);
	if ((range_diff == powf(10.0, floorf(log10f(range_diff)))) && (range_diff != 1)) {
		// range_diff is a power of 10, but is not exactly 1
		display_range = 10;
	} else {
		display_range = range_diff / powf(10.0, floorf(log10f(range_diff)));
	}

	// Set the increment from one label to the next.
	if (display_range <= 1)
		label_increment = (float) .1;
	else if (display_range <= 2)
		label_increment = (float) .2;
	else if (display_range <= 5)
		label_increment = (float) .5;
	else if (display_range <= 10)
		label_increment = 1;

	// Update the marker label text boxes.
	label_count = 0;
	display_value = (float) min_range;
	while (display_value <= display_range) {
		// Set the position of the text boxes.
		label_angle = (int)(display_value / display_range * NEEDLE_SWEEP);
		CalculatePoint(label_angle, label_radius, &x, &y);
		label_box[label_count].left = x - label_box_size;
		label_box[label_count].top = y - label_box_size;
		label_box[label_count].right = x + label_box_size;
		label_box[label_count].bottom = y + label_box_size;

		// Format the label's text.  
		if (display_value > 0.0 && display_value <= 0.9)
			label[label_count].Format(".%.1g\n", display_value * 10.0);
		else
			label[label_count].Format("%.2g\n", display_value);

		// Set the location for the tick marks.
		CalculatePoint(label_angle, tick_mark_radius,
			       &tick_mark[label_count][0].x, &tick_mark[label_count][0].y);
		CalculatePoint(label_angle, outer_radius, &tick_mark[label_count][1].x, &tick_mark[label_count][1].y);

		label_count++;
		display_value += label_increment;
	}
}

//
// Set the value being displayed by the meter.  It also checks to see if the
// display needs to be updated and does so.
//
void CMeterCtrl::SetValue(double new_value)
{
	BOOL update_watermark = FALSE;

	// If the value is the same as what we had, we don't need to do anything.
	if (new_value == value)
		return;

	// See if the new value falls outside the current watermark ranges.
	if (new_value < low_value || low_value == -1) {
		low_value = new_value;
		update_watermark = TRUE;
	}
	if (new_value > high_value) {
		high_value = new_value;
		update_watermark = TRUE;
	}

	value = new_value;

	// Make sure that the new value is within the current range and update
	// the actual angle that the needle should be.
	if (new_value < min_range)
		actual_angle = 0;
	else if (new_value > max_range)
		actual_angle = NEEDLE_SWEEP;
	else
		actual_angle = (int)(value / (max_range - min_range) * NEEDLE_SWEEP);

	// See if the needle would move significantly.  If not, we're done.
	if (fabs((double)actual_angle - shown_angle) < NEEDLE_SENSITIVITY)
		return;

	// Update the location and angle of the needle's point.
	DrawNeedle();
	if (show_watermark && update_watermark)
		DrawWatermark();
}

//
// Calculate a point on the display given an angle (in degrees) and the 
// length of the radius.  The point is offset based on the pivot point.
//
void CMeterCtrl::CalculatePoint(int angle, int radius, LONG * x, LONG * y)
{
	// The given angle is from the minimum value and does not include the
	// rotation in the display.
	angle += 90 + PIVOT_ARC_ANGLE;
	*x = pivot_point.x + (int)(cos(angle * PI / 180) * radius);
	*y = pivot_point.y + (int)(sin(angle * PI / 180) * radius);
}

//
// Sets the three points needed to draw the needle.
//
void CMeterCtrl::SetNeedlePoints()
{
	CalculatePoint(actual_angle, inner_radius, &(needle[0].x), &(needle[0].y));
	CalculatePoint(180 + actual_angle - 20, pivot_radius + 5, &(needle[1].x), &(needle[1].y));
	CalculatePoint(180 + actual_angle + 20, pivot_radius + 5, &(needle[2].x), &(needle[2].y));

	// Update the angle that the needle now shows.
	shown_angle = actual_angle;
}

//
// Updating the display of the needle image.
//
void CMeterCtrl::DrawNeedle()
{
	CClientDC dc(this);
	CPen black_pen(PS_SOLID, 1, (COLORREF) 0x00000000);
	CBrush black_brush((COLORREF) 0x00000000);
	CPen needle_pen(PS_SOLID, 1, (COLORREF) 0x0000000FF);
	CBrush needle_brush((COLORREF) 0x0000000FF);
	CBrush pivot_brush(PALETTERGB(170, 170, 170));

	// Clear the current needle by painting it black.
	dc.SelectObject(black_pen);
	dc.SelectObject(black_brush);
	dc.Polygon(needle, 3);

	// Draw the pivot circle.
	dc.SelectObject(black_pen);
	dc.SelectObject(pivot_brush);
	dc.Ellipse(&needle_pivot);

	// Draw the new needle.
	SetNeedlePoints();
	dc.SelectObject(needle_pen);
	dc.SelectObject(needle_brush);
	dc.Polygon(needle, 3);
}

//
// Update the display of the watermark ranges.
//
void CMeterCtrl::DrawWatermark()
{
	CClientDC dc(this);
	CPen watermark_pen(PS_SOLID, 2, 0x00FF0000);
	int low_angle, high_angle;
	POINT start, end;

	// Calculate the angle that the low watermark makes.
	if (low_value < min_range)
		low_angle = 0;
	else if (low_value > max_range)
		low_angle = NEEDLE_SWEEP;
	else
		low_angle = (int)(low_value / (max_range - min_range) * NEEDLE_SWEEP);

	// Calculate the angle that the high watermark makes.
	if (high_value < min_range)
		high_angle = 0;
	else if (high_value > max_range)
		high_angle = NEEDLE_SWEEP;
	else
		high_angle = (int)(high_value / (max_range - min_range) * NEEDLE_SWEEP);

	// If the two angles are the same, don't display the watermark.
	if (low_angle == high_angle)
		return;

	// Get the endpoints of the watermark arc.  The arc is drawn counter-
	// clockwise.
	CalculatePoint(high_angle, inner_radius + 2, &start.x, &start.y);
	CalculatePoint(low_angle, inner_radius + 2, &end.x, &end.y);

	// Draw the watermark range of the currently displayed value.
	dc.SelectObject(watermark_pen);
	dc.Arc(&watermark_box, start, end);
}

//
// Redraws the display from a WM_PAINT message.
//
void CMeterCtrl::OnPaint()
{
	CPaintDC dc(this);
	int i;

	// Draw the dial.
	dc.SelectStockObject(BLACK_PEN);
	dc.SelectStockObject(BLACK_BRUSH);
	dc.Ellipse(&meter_box);

	// Indicate the scale.
	dc.SelectObject(&scale_font);
	dc.SetBkColor(0x00000000);
	dc.SetTextColor(0x00FFFFFF);
	dc.DrawText(scale_text, -1, &scale_box, DT_CENTER);

	// Add the labels and tick marks.
	dc.SelectObject(&label_font);
	dc.SelectObject(&marker_pen);
	for (i = 0; i < label_count; i++) {
		dc.DrawText(label[i], -1, &label_box[i], DT_CENTER | DT_NOCLIP);
		dc.MoveTo(tick_mark[i][0]);
		dc.LineTo(tick_mark[i][1]);
	}

	DrawNeedle();
	if (show_watermark)
		DrawWatermark();
}

//
// Removes used memory.  Called after response to WM_NCDESTROY
//
void CMeterCtrl::PostNcDestroy()
{
	delete this;
}

//
// Registers the "meter" window class
//
BOOL CMeterCtrl::Register()
{
	WNDCLASS wc;

	// See if the class has already been registered.
	if (GetClassInfo(NULL, "CMeterCtrl", &wc)) {
		// Name already registered - ok if it was us
		return (wc.lpfnWndProc == (WNDPROC) CMeterCtrl::MeterCtrlWndProc);
	}
	// Register the window class of the control.
	wc.style = CS_GLOBALCLASS | CS_OWNDC | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc = CMeterCtrl::MeterCtrlWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = NULL;
	wc.hIcon = NULL;
	wc.hCursor =::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "CMeterCtrl";

	return RegisterClass(&wc);
}

//
// Window procedure for the "meter" window class.  This global function handles the
// creation of new CMeterCtrl objects and subclasses the objects so the MFC framework
// passes messages along to the CMeterCtrl member functions.
//
LRESULT CALLBACK EXPORT CMeterCtrl::MeterCtrlWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CWnd *pWnd;

	// See if we're creating the window.
	if (!(pWnd = CWnd::FromHandlePermanent(hWnd)) && message == WM_NCCREATE) {
		// Yes, create the object.
		pWnd = new CMeterCtrl();
		pWnd->Attach(hWnd);
	}

	return AfxCallWndProc(pWnd, hWnd, message, wParam, lParam);
}
