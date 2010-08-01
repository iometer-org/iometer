/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / ICF_ifstream.cpp                                       ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the ICF_ifstream class, which       ## */
/* ##               extends the standard "ifstream" class with            ## */
/* ##               additional functionality for reading and writing      ## */
/* ##               Iometer's .ICF (Iometer Configuration File) files.    ## */
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
/* ##  Changes ...: 2003-04-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added the ExtractFirstIntVersion() method as a      ## */
/* ##                 copy of ExtractFirstInt() to enable parsing ICF     ## */
/* ##                 files that contains version strings with a          ## */
/* ##                 trailing dash (so "2003.12.16-post" for instance).  ## */
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

#include "stdafx.h"
#include "GalileoApp.h"
#include "ICF_ifstream.h"

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

//
// Reads the version header from the file,
// returns the version number as a long integer.
// Expects the file pointer to be at the BEGINNING of the file.
// 1998.10.08 becomes 19981008.
//
// Return value of -1 indicates an error.  The calling function
// should NOT report an error.  Error reporting is handled here.
//
long ICF_ifstream::GetVersion()
{
	CString version_string;
	int major, middle, minor;
	long version;

	version_string = GetNextLine();
	if (version_string.IsEmpty()) {
		ErrorMessage("File is improperly formatted.  Empty line found "
			     "where file version information was expected.");
		return -1;
	}

	if (!ExtractFirstIntVersion(version_string, major)
	    || !ExtractFirstIntVersion(version_string, middle)
	    || !ExtractFirstIntVersion(version_string, minor)) {
		ErrorMessage("File is improperly formatted.  " "Error retrieving file version information.");
		return -1;
	}

	version = (long)major *10000 + middle * 100 + minor;

	// Special case test for ancient version "3.26.97" OLTP.txt
	// file distributed with 1998.01.05.
	if (version == 32697)
		version = 19980105;

	if (version < 19980105) {
		ErrorMessage("Error restoring file.  "
			     "Version number earlier than 1998.01.05 or incorrectly formatted.");
		return -1;
	}

	return version;
}

//
// Looks through the whole stream for the given identifier on a line
// by itself, puts the fstream file pointer at the beginning of the
// line below the identifier.
// NOTE: Strips leading and trailing whitespace, ignores case.
//
// Return value of FALSE indicates that the requested identifier was
// not found in the file.  In this case, the caller should generally
// not report an error, nor return a failure status.  If a user
// is restoring information from a config file and a section
// chosen for loading is not present in the saved file, an
// error should not be reported.
//
BOOL ICF_ifstream::SkipTo(CString identifier)
{
	CString curline;

	identifier.TrimLeft();
	identifier.TrimRight();

	do {
		if (eof())
			return FALSE;

		curline = GetNextLine();

		curline.TrimLeft();
		curline.TrimRight();
	}
	while (identifier.CompareNoCase(curline.Left(identifier.GetLength())) != 0);
	// Loop until the identifier is matched.  (Ignore the rest of the line when
	// comparing.)

	return TRUE;
}

//
// Duplicates the functionality of istream's getline(), but does so using
// CStrings.  This hides all the "buffer" code required to get a char*
// from a CString.
//
// Error condition is not reported.  If returned string is empty (IsEmpty)
// and an empty string is not expected, the caller should report an error.
// The error reported here is only for bug identification.
//
CString ICF_ifstream::GetNextLine()
{
	CString curline;

	if (!is_open()) {
		// This should never happen.  This indicates an Iometer bug.
		ErrorMessage("A call was made to ICF_ifstream::GetNextLine() "
			     "with a closed file!  Please report this as an Iometer bug.");
		curline.Empty();
		return curline;
	}

	getline(curline.GetBuffer(MAX_ICF_LINE), MAX_ICF_LINE);
	curline.ReleaseBuffer();

	curline.TrimLeft();
	curline.TrimRight();

	return curline;
}

//
// Use GetPair retrieve the next comment and corresponding value from the
// Iometer config file.  (They are returned by reference in key and value.)
//
// Assumes the file pointer is at the beginning of a comment line (followed
// by zero or more additional comment lines).  If the first comment line is
// an end marker (end of a config file section), GetPair sets key equal to
// the comment and value empty.  If the first comment line is NOT an end
// marker, the remaining comment lines (if any) are skipped and the next
// line containing data is stored in value.
//
// Return value of FALSE indicates an error.  The calling function
// should report a specific error.
//
BOOL ICF_ifstream::GetPair(CString & key, CString & value)
{
	streampos placeholder;
	CString tempstring;

	if (!is_open()) {
		// This should never happen.  This indicates an Iometer bug.
		ErrorMessage("A call was made to ICF_ifstream::GetPair() "
			     "with a closed file!  Please report this as an Iometer bug.");
		return FALSE;
	}
	// If EOF, let the caller report an error.
	if (rdstate())
		return FALSE;

	// If first line isn't a comment, let the caller report an error.
	if (peek() != '\'')
		return FALSE;

	key = GetNextLine();

	// If this is an END or VERSION tag, there will be no data
	// corresponding to the comment on the following lines, so don't
	// look for any.
	//
	// Note that strlen is used (instead of a constant) to make the
	// connection between the string and its length obvious to someone
	// making changes to the code.
	if (key.Left((int)(strlen("'End"))).CompareNoCase("'End") == 0
	    || key.Left((int)(strlen("'Version"))).CompareNoCase("'Version") == 0) {
		value.Empty();
		return TRUE;
	}
	// Skip any extra comment lines before the data.
	while (peek() == '\'') {
		placeholder = tellg();
		tempstring = GetNextLine();

		if (tempstring.Left((int)(strlen("'End"))).CompareNoCase("'End") == 0
		    || tempstring.Left((int)(strlen("'Version"))).CompareNoCase("'Version") == 0) {
			// This only happens when there is an empty section, like:
			//      'Worker
			//      'End worker
			// Worker name is normally expected after the "'Worker" header,
			// but in this case, it's only an "'End worker" header.  The
			// "'End..." header must be returned in the NEXT call to GetPair.

			seekg(placeholder);	// back up the file pointer
			value.Empty();
			return TRUE;	// NOT an error condition
		}
	}

	// If EOF, let the caller report an error.
	if (rdstate()) {
		value.Empty();
		return FALSE;
	}
	// Get the data corresponding to the above key.
	value = GetNextLine();

	return TRUE;
}

//
// Extract the first integer in a CString and remove it from the CString.
// Both the string and the number parameters are modified.  Finds the
// first integer in a string and puts its value in the number parameter,
// then truncates the string parameter so it can be used in the next call
// to this function to extract the next integer, if there is one.
//
// Return value of FALSE indicates an error.  The calling function
// should report a specific error.
//
BOOL ICF_ifstream::ExtractFirstInt(CString & string, int &number)
{
	const CString backup_string = string;
	CString substring;
	int pos;

	number = 0;

	if ((pos = string.FindOneOf("-1234567890")) == -1) {
		ErrorMessage("File is improperly formatted.  Expected an " "integer value.");
		return FALSE;
	}
	// Cleave off everything before the first number.
	string = string.Right(string.GetLength() - pos);

	substring = string.SpanIncluding("-1234567890");	// get the int as a string
	string = string.Right(string.GetLength() - substring.GetLength());

	// If there are any negative signs after the first character, fail.
	if (substring.Right(substring.GetLength() - 1).Find('-') != -1) {
		ErrorMessage("File is improperly formatted.  An integer value "
			     "has a negative sign in the middle or on the end of it.");
		string = backup_string;	// restore string's old value
		return FALSE;
	}

	number = atoi((LPCTSTR) substring);

	// Prepare string for further processing.  Eat whitespace.
	string.TrimLeft();

	// If there's a trailing comma, eat it.
	if (!string.IsEmpty() && string.GetAt(0) == ',')
		string = string.Right(string.GetLength() - 1);

	// Eat any space following the comma.
	string.TrimLeft();

	return TRUE;
}

BOOL ICF_ifstream::ExtractFirstUInt64(CString & string, DWORDLONG &number)
{
	const CString backup_string = string;
	CString substring;
	int pos;

	number = 0;

	if ((pos = string.FindOneOf("-1234567890")) == -1) {
		ErrorMessage("File is improperly formatted.  Expected an " "integer value.");
		return FALSE;
	}
	// Cleave off everything before the first number.
	string = string.Right(string.GetLength() - pos);

	substring = string.SpanIncluding("-1234567890");	// get the int as a string
	string = string.Right(string.GetLength() - substring.GetLength());

	// If there are any negative signs after the first character, fail.
	if (substring.Right(substring.GetLength() - 1).Find('-') != -1) {
		ErrorMessage("File is improperly formatted.  An integer value "
			     "has a negative sign in the middle or on the end of it.");
		string = backup_string;	// restore string's old value
		return FALSE;
	}

	number = _strtoui64((LPCTSTR) substring, NULL, 10);

	// Prepare string for further processing.  Eat whitespace.
	string.TrimLeft();

	// If there's a trailing comma, eat it.
	if (!string.IsEmpty() && string.GetAt(0) == ',')
		string = string.Right(string.GetLength() - 1);

	// Eat any space following the comma.
	string.TrimLeft();

	return TRUE;
}



BOOL ICF_ifstream::ExtractFirstIntVersion(CString & string, int &number)
{
	const CString backup_string = string;
	CString substring;
	int pos;

	number = 0;

	if ((pos = string.FindOneOf("1234567890")) == -1) {
		ErrorMessage("File is improperly formatted.  Expected an " "integer value.");
		return FALSE;
	}
	// Cleave off everything before the first number.
	string = string.Right(string.GetLength() - pos);

	substring = string.SpanIncluding("1234567890");	// get the int as a string
	string = string.Right(string.GetLength() - substring.GetLength());

	number = atoi((LPCTSTR) substring);

	// Prepare string for further processing.  Eat whitespace.
	string.TrimLeft();

	// If there's a trailing comma, eat it.
	if (!string.IsEmpty() && string.GetAt(0) == ',')
		string = string.Right(string.GetLength() - 1);

	// Eat any space following the comma.
	string.TrimLeft();

	return TRUE;
}

//
// Extracts the first solid series of letters, numbers, underscores, and
// hyphens in a string.  Returns the substring, removes the token from the
// original string.
//
// If spaces parameter is TRUE, spaces are considered part of the token
// instead of delimiters.
//
// Error condition is not reported.  If returned string is empty (IsEmpty)
// and an empty string is not expected, the caller should report an error.
//
CString ICF_ifstream::ExtractFirstToken(CString & string, BOOL spaces /*=FALSE*/ )
{
	CString substring;
	int pos;
	CString token_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "abcdefghijklmnopqrstuvwxyz" "1234567890" "`~!@#$%^&*()-_=+[]{}\\:;\"'./<>?";

	if (spaces)
		token_chars += " ";

	if ((pos = string.FindOneOf(token_chars)) == -1) {
		substring.Empty();
		return substring;
	}

	string = string.Right(string.GetLength() - pos);

	substring = string.SpanIncluding(token_chars);

	string = string.Right(string.GetLength() - substring.GetLength());
	string.TrimLeft();
	string.TrimRight();

	return substring;
}
