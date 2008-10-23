/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 * 
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * The Original Code is Komodo code.
 * 
 * The Initial Developer of the Original Code is ActiveState Software Inc.
 * Portions created by ActiveState Software Inc are Copyright (C) 2000-2007
 * ActiveState Software Inc. All Rights Reserved.
 * 
 * Contributor(s):
 *   ActiveState Software Inc
 *   Appcelerator Inc
 * 
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 * 
 * ***** END LICENSE BLOCK ***** */

/* Appcelerator NPAPI-based implementation to plugin to WebKit */

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "Platform.h"

#include "Scintilla.h"
#include "SVector.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "CellBuffer.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "Document.h"

//#ifdef SCI_NAMESPACE
//using namespace Scintilla;
//#endif

#include "npSciDoc.h"
#include "npSupport.h"

SciDoc::SciDoc()
{
  /* member initializers and constructor code */
	documentPointer = new Scintilla::Document();
	if (documentPointer) {
		documentPointer->AddRef();
	}
}

SciDoc::~SciDoc()
{
    /* destructor code */
    if (documentPointer!=NULL) {
        documentPointer->Release();
    }
}

/* attribute long document; */
int32 SciDoc::GetDocPointer()
{
    if (documentPointer==NULL) return NULL;
    return reinterpret_cast<int32>(documentPointer);
}
void SciDoc::SetDocPointer(int32 aDocument)
{
    if (documentPointer!=NULL) {
        documentPointer->Release();
    }
    documentPointer = reinterpret_cast<Scintilla::Document *>(aDocument);
    if (documentPointer) {
            documentPointer->AddRef();
    }
    return;
}

std::string SciDoc::GetText()
{
	return GetCharRange(0, documentPointer->Length());
}
void SciDoc::SetText(std::string aText)
{
#ifdef SCIMOZ_DEBUG
	fprintf(stderr,"SciDoc::SetText\n");
#endif
        // SCI_SETTEXT from Editor.cxx
        documentPointer->BeginUndoAction();
        documentPointer->DeleteChars(0, documentPointer->Length());
        // SetEmptySelection(0);
	if (documentPointer->dbcsCodePage == 0) {
		//UTF16?
	    documentPointer->InsertCString(0, aText.c_str());
	} else {
	    documentPointer->InsertCString(0, aText.c_str());
	}
        documentPointer->EndUndoAction();
}

/* attribute int32 stylingBits; */
int32 SciDoc::GetStylingBits()
{
    if (documentPointer==NULL) return 0;
	return documentPointer->stylingBits;
}
void SciDoc::SetStylingBits(int32 aStylingBits)
{
    if (documentPointer==NULL) return;
    documentPointer->stylingBits = aStylingBits;
}

/* attribute int32 stylingBitsMask; */
int32 SciDoc::GetStylingBitsMask()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->stylingBitsMask;
}
void SciDoc::SetStylingBitsMask(int32 aStylingBitsMask)
{
    if (documentPointer==NULL) return;
    documentPointer->stylingBitsMask = aStylingBitsMask;
}

/* attribute int32 eolMode; */
int32 SciDoc::GetEolMode()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->eolMode;
}
void SciDoc::SetEolMode(int32 aEolMode)
{
    if (documentPointer==NULL) return;
    documentPointer->eolMode = aEolMode;
}

/* attribute int32 dbcsCodePage; */
int32 SciDoc::GetDbcsCodePage()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->dbcsCodePage;
}
void SciDoc::SetDbcsCodePage(int32 aDbcsCodePage)
{
    if (documentPointer==NULL) return;
    documentPointer->dbcsCodePage = aDbcsCodePage;
}

/* attribute int32 tabInChars; */
int32 SciDoc::GetTabInChars()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->tabInChars;
}
void SciDoc::SetTabInChars(int32 aTabInChars)
{
    if (documentPointer==NULL) return;
    documentPointer->tabInChars = aTabInChars;
}

/* attribute int32 indentInChars; */
int32 SciDoc::GetIndentInChars()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->indentInChars;
}
void SciDoc::SetIndentInChars(int32 aIndentInChars)
{
    if (documentPointer==NULL) return;
    documentPointer->indentInChars = aIndentInChars;
}

/* attribute int32 actualIndentInChars; */
int32 SciDoc::GetActualIndentInChars()
{
    if (documentPointer==NULL) return 0;
	return documentPointer->actualIndentInChars;
}
void SciDoc::SetActualIndentInChars(int32 aActualIndentInChars)
{
    if (documentPointer==NULL) return;
    documentPointer->actualIndentInChars = aActualIndentInChars;
}

/* attribute boolean useTabs; */
bool SciDoc::GetUseTabs()
{
    if (documentPointer==NULL) return 0;
	return documentPointer->useTabs;
}
void SciDoc::SetUseTabs(bool aUseTabs)
{
    if (documentPointer==NULL) return;
    documentPointer->useTabs = aUseTabs;
}

/* attribute boolean tabIndents; */
bool SciDoc::GetTabIndents()
{
    if (documentPointer==NULL) return 0;
	return documentPointer->tabIndents;
}
void SciDoc::SetTabIndents(bool aTabIndents)
{
    if (documentPointer==NULL) return;
    documentPointer->tabIndents = aTabIndents;
}

/* attribute boolean backspaceUnindents; */
bool SciDoc::GetBackspaceUnindents()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->backspaceUnindents;
}
void SciDoc::SetBackspaceUnindents(bool aBackspaceUnindents)
{
    if (documentPointer==NULL) return;
    documentPointer->backspaceUnindents = aBackspaceUnindents;
}

/* int32 LineFromPosition (in int32 pos); */
int32 SciDoc::LineFromPosition(int32 pos)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->LineFromPosition(pos);
}

/* int32 ClampPositionIntoDocument (in int32 pos); */
int32 SciDoc::ClampPositionIntoDocument(int32 pos)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->ClampPositionIntoDocument(pos);
}

/* boolean IsCrLf (in int32 pos); */
bool SciDoc::IsCrLf(int32 pos)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->IsCrLf(pos);
}

/* int32 LenChar (in int32 pos); */
int32 SciDoc::LenChar(int32 pos)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->LenChar(pos);
}

/* int32 MovePositionOutsideChar (in int32 pos, in int32 moveDir, in boolean checkLineEnd); */
int32 SciDoc::MovePositionOutsideChar(int32 pos, int32 moveDir, bool checkLineEnd)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->MovePositionOutsideChar(pos, moveDir, checkLineEnd);
}

/* void ModifiedAt (in int32 pos); */
void SciDoc::ModifiedAt(int32 pos)
{
    if (documentPointer==NULL) return;
    documentPointer->ModifiedAt(pos);
}

/* boolean DeleteChars (in int32 pos, in int32 len); */
bool SciDoc::DeleteChars(int32 pos, int32 len)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->DeleteChars(pos, len);
}

/* boolean InsertString (in int32 position, in wstring s); */
bool SciDoc::InsertString(int32 position, std::string text)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->InsertString(position,
                                             reinterpret_cast<const char *>(text.c_str()),
                                             text.length());
}

/* int32 Undo (); */
int32 SciDoc::Undo()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->Undo();
}

/* int32 Redo (); */
int32 SciDoc::Redo()
{
    if (documentPointer==NULL) return 0;
	return documentPointer->Redo();
}

/* boolean CanUndo (); */
bool SciDoc::CanUndo()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->CanUndo();
}

/* boolean CanRedo (); */
bool SciDoc::CanRedo()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->CanRedo();
}

/* void DeleteUndoHistory (); */
void SciDoc::DeleteUndoHistory()
{
    if (documentPointer==NULL) return;
    documentPointer->DeleteUndoHistory();
}

/* boolean SetUndoCollection (in boolean collectUndo); */
bool SciDoc::SetUndoCollection(bool collectUndo)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->SetUndoCollection(collectUndo);
}

/* boolean IsCollectingUndo (); */
bool SciDoc::IsCollectingUndo()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->IsCollectingUndo();
}

/* void BeginUndoAction (); */
void SciDoc::BeginUndoAction()
{
    if (documentPointer==NULL) return;
    documentPointer->BeginUndoAction();
}

/* void EndUndoAction (); */
void SciDoc::EndUndoAction()
{
    if (documentPointer==NULL) return;
    documentPointer->EndUndoAction();
}

/* void SetSavePoint (); */
void SciDoc::SetSavePoint()
{
    if (documentPointer==NULL) return;
    documentPointer->SetSavePoint();
}

/* boolean IsSavePoint (); */
bool SciDoc::IsSavePoint()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->IsSavePoint();
}

/* int32 GetLineIndentation (in int32 line); */
int32 SciDoc::GetLineIndentation(int32 line)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->GetLineIndentation(line);
}

/* void SetLineIndentation (in int32 line, in int32 indent); */
void SciDoc::SetLineIndentation(int32 line, int32 indent)
{
    if (documentPointer==NULL) return;
    documentPointer->SetLineIndentation(line, indent);
}

/* int32 GetLineIndentPosition (in int32 line); */
int32 SciDoc::GetLineIndentPosition(int32 line)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->GetLineIndentPosition(line);
}

/* int32 GetColumn (in int32 position); */
int32 SciDoc::GetColumn(int32 position)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->GetColumn(position);
}

/* int32 FindColumn (in int32 line, in int32 column); */
int32 SciDoc::FindColumn(int32 line, int32 column)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->FindColumn(line, column);
}

/* void Indent (in boolean forwards, in int32 lineBottom, in int32 lineTop); */
void SciDoc::Indent(bool forwards, int32 lineBottom, int32 lineTop)
{
    if (documentPointer==NULL) return;
    documentPointer->Indent(forwards, lineBottom, lineTop);
}

/* wstring TransformLineEnds (in wstring s, in int32 eolMode); */
std::string SciDoc::TransformLineEnds(std::string s, int32 eolMode)
{
    if (documentPointer==NULL) return 0;
    //nsCAutoString text = NS_ConvertUTF16toUTF8(s);
    int lenOut = 0;
    char *buffer = documentPointer->TransformLineEnds(&lenOut,
                                             reinterpret_cast<const char *>(s.c_str()),
                                             s.length(),
                                             eolMode);
    //if (documentPointer->dbcsCodePage == 0) {
		//return std::string(buffer);
        //return  ToNewUnicode(NS_ConvertASCIItoUTF16(buffer));
    //} else {
		return std::string(buffer);
        //return  ToNewUnicode(NS_ConvertUTF8toUTF16(buffer));
    //}
}

/* void ConvertLineEnds (in int32 eolModeSet); */
void SciDoc::ConvertLineEnds(int32 eolModeSet)
{
    if (documentPointer==NULL) return;
    documentPointer->ConvertLineEnds(eolModeSet);
}

/* void SetReadOnly (in boolean set); */
void SciDoc::SetReadOnly(bool set)
{
    if (documentPointer==NULL) return;
    documentPointer->SetReadOnly(set);
}

/* boolean IsReadOnly (); */
bool SciDoc::IsReadOnly()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->IsReadOnly();
}

/* boolean InsertChar (in int32 pos, in wchar ch); */
bool SciDoc::InsertChar(int32 pos, char ch)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->InsertChar(pos, ch);
}

/* void ChangeChar (in int32 pos, in wchar ch); */
void SciDoc::ChangeChar(int32 pos, char ch)
{
    if (documentPointer==NULL) return;
    documentPointer->InsertChar(pos, ch);
}

/* void DelChar (in int32 pos); */
void SciDoc::DelChar(int32 pos)
{
    if (documentPointer==NULL) return;
    documentPointer->DelChar(pos);
}

/* void DelCharBack (in int32 pos); */
void SciDoc::DelCharBack(int32 pos)
{
    if (documentPointer==NULL) return;
    documentPointer->DelCharBack(pos);
}

/* wchar CharAt (in int32 position); */
char SciDoc::CharAt(int32 position)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->CharAt(position);
}

/* wstring GetCharRange (in int32 position, in int32 lengthRetrieve); */
std::string SciDoc::GetCharRange(int32 position, int32 lengthRetrieve)
{
    if (documentPointer==NULL) return 0;
    char *buffer = new char[lengthRetrieve + 1];
    //if (!buffer)
    //        return NS_ERROR_OUT_OF_MEMORY;
    buffer[lengthRetrieve]=0;
    documentPointer->GetCharRange(buffer, position, lengthRetrieve);

    //assert(buffer[lengthRetrieve] == NULL);

	std::string returnval = std::string(buffer);
	
    if (documentPointer->dbcsCodePage == 0) {
		// ASCII ?
    } else {
		// UTF8?
    }

    delete []buffer;
    return returnval;
}

/* wchar StyleAt (in int32 position); */
char SciDoc::StyleAt(int32 position)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->StyleAt(position);
}

/* int32 GetMark (in int32 line); */
int32 SciDoc::GetMark(int32 line)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->GetMark(line);
}

/* int32 AddMark (in int32 line, in int32 markerNum); */
int32 SciDoc::AddMark(int32 line, int32 markerNum)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->AddMark(line, markerNum);
}

/* void AddMarkSet (in int32 line, in int32 valueSet); */
void SciDoc::AddMarkSet(int32 line, int32 valueSet)
{
    if (documentPointer==NULL) return;
    documentPointer->AddMarkSet(line, valueSet);
}

/* void DeleteMark (in int32 line, in int32 markerNum); */
void SciDoc::DeleteMark(int32 line, int32 markerNum)
{
    if (documentPointer==NULL) return;
    documentPointer->DeleteMark(line, markerNum);
}

/* void DeleteMarkFromHandle (in int32 markerHandle); */
void SciDoc::DeleteMarkFromHandle(int32 markerHandle)
{
    if (documentPointer==NULL) return;
    documentPointer->DeleteMarkFromHandle(markerHandle);
}

/* void DeleteAllMarks (in int32 markerNum); */
void SciDoc::DeleteAllMarks(int32 markerNum)
{
    if (documentPointer==NULL) return;
    documentPointer->DeleteAllMarks(markerNum);
}

/* int32 LineFromHandle (in int32 markerHandle); */
int32 SciDoc::LineFromHandle(int32 markerHandle)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->LineFromHandle(markerHandle);
}

/* int32 LineStart (in int32 line); */
int32 SciDoc::LineStart(int32 line)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->LineStart(line);
}

/* int32 LineEnd (in int32 line); */
int32 SciDoc::LineEnd(int32 line)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->LineEnd(line);
}

/* int32 LineEndPosition (in int32 position); */
int32 SciDoc::LineEndPosition(int32 position)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->LineEndPosition(position);
}

/* int32 VCHomePosition (in int32 position); */
int32 SciDoc::VCHomePosition(int32 position)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->VCHomePosition(position);
}

/* int32 SetLevel (in int32 line, in int32 level); */
int32 SciDoc::SetLevel(int32 line, int32 level)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->SetLevel(line, level);
}

/* int32 GetLevel (in int32 line); */
int32 SciDoc::GetLevel(int32 line)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->GetLevel(line);
}

/* void ClearLevels (); */
void SciDoc::ClearLevels()
{
    if (documentPointer==NULL) return;
    documentPointer->ClearLevels();
}

/* int32 GetLastChild (in int32 lineParent, in int32 level); */
int32 SciDoc::GetLastChild(int32 lineParent, int32 level)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->GetLastChild(lineParent, level);
}

/* int32 GetFoldParent (in int32 line); */
int32 SciDoc::GetFoldParent(int32 line)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->GetFoldParent(line);
}

/* int32 ExtendWordSelect (in int32 pos, in int32 delta, in boolean onlyWordCharacters); */
int32 SciDoc::ExtendWordSelect(int32 pos, int32 delta, bool onlyWordCharacters)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->ExtendWordSelect(pos, delta, onlyWordCharacters);
}

/* int32 NextWordStart (in int32 pos, in int32 delta); */
int32 SciDoc::NextWordStart(int32 pos, int32 delta)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->NextWordStart(pos, delta);
}

/* int32 NextWordEnd (in int32 pos, in int32 delta); */
int32 SciDoc::NextWordEnd(int32 pos, int32 delta)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->NextWordEnd(pos, delta);
}

/* int32 Length (); */
int32 SciDoc::Length()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->Length();
}

/* void Allocate (in int32 newSize); */
void SciDoc::Allocate(int32 newSize)
{
    if (documentPointer==NULL) return;
    documentPointer->Allocate(newSize);
}

int32 SciDoc::FindText(int32 minPos, int32 maxPos,
					   std::string text, bool caseSensitive,
                               bool word, bool wordStart, bool regExp, 
                               bool posix)
{
    if (documentPointer==NULL) return 0;
	//nsCAutoString text = NS_ConvertUTF16toUTF8(s);
    int length = text.length();
    return documentPointer->FindText(minPos, maxPos, reinterpret_cast<const char *>(text.c_str()),
                                         caseSensitive, word, wordStart, regExp, posix, &length);
}

/* wstring SubstituteByPosition (in wstring text); */
std::string SciDoc::SubstituteByPosition(std::string text)
{
    if (documentPointer==NULL) return 0;
    //nsCAutoString text = NS_ConvertUTF16toUTF8(s);
    int length = text.length();
    const char *buffer = documentPointer->SubstituteByPosition(reinterpret_cast<const char *>(text.c_str()), &length);
    /*if (documentPointer->dbcsCodePage == 0) {
        return  ToNewUnicode(NS_ConvertASCIItoUTF16(buffer));
    } else {
        return  ToNewUnicode(NS_ConvertUTF8toUTF16(buffer));
    }*/
	return std::string(buffer);
}

/* int32 LinesTotal (); */
int32 SciDoc::LinesTotal()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->LinesTotal();
}

/* void ChangeCase (in int32 start, in int32 end, in boolean makeUpperCase); */
void SciDoc::ChangeCase(int32 start, int32 end, bool makeUpperCase)
{
    if (documentPointer==NULL) return;
    documentPointer->ChangeCase(Scintilla::Range(start, end), makeUpperCase);
}

/* void SetDefaultCharClasses (in boolean includeWordClass); */
void SciDoc::SetDefaultCharClasses(bool includeWordClass)
{
    if (documentPointer==NULL) return ;
    documentPointer->SetDefaultCharClasses(includeWordClass);
}

/* void StartStyling (in int32 position, in wchar mask); */
void SciDoc::StartStyling(int32 position, char mask)
{
    if (documentPointer==NULL) return;
    documentPointer->StartStyling(position, mask);
}

/* boolean SetStyleFor (in int32 length, in wchar style); */
bool SciDoc::SetStyleFor(int32 length, char style)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->SetStyleFor(length, style);
}

/* boolean SetStyles (in int32 length, in wstring styles); */
bool SciDoc::SetStyles(int32 length, std::string styles)
{
    if (documentPointer==NULL) return 0;
    char *buffer = new char[length + 1];
    if (!buffer)
            return false;
    buffer[length]=0;
    memcpy(buffer, styles.c_str(), length);
    return documentPointer->SetStyles(length, buffer);
    delete buffer;
}

/* int32 GetEndStyled (); */
int32 SciDoc::GetEndStyled()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->GetEndStyled();
}

/* void EnsureStyledTo (in int32 pos); */
void SciDoc::EnsureStyledTo(int32 pos)
{
    if (documentPointer==NULL) return;
    documentPointer->EnsureStyledTo(pos);
}

/* int32 GetStyleClock (); */
int32 SciDoc::GetStyleClock()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->GetStyleClock();
}

/* void IncrementStyleClock (); */
void SciDoc::IncrementStyleClock()
{
    if (documentPointer==NULL) return;
    documentPointer->IncrementStyleClock();
}

/* void DecorationFillRange (in int32 position, in int32 value, in int32 fillLength); */
void SciDoc::DecorationFillRange(int32 position, int32 value, int32 fillLength)
{
    if (documentPointer==NULL) return;
    documentPointer->DecorationFillRange(position, value, fillLength);
}

/* int32 SetLineState (in int32 line, in int32 state); */
int32 SciDoc::SetLineState(int32 line, int32 state)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->SetLineState(line, state);
}

/* int32 GetLineState (in int32 line); */
int32 SciDoc::GetLineState(int32 line)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->GetLineState(line);
}

/* int32 GetMaxLineState (); */
int32 SciDoc::GetMaxLineState()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->GetMaxLineState();
}

/* boolean IsWordPartSeparator (in wchar ch); */
bool SciDoc::IsWordPartSeparator(char ch)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->IsWordPartSeparator(ch);
}

/* int32 WordPartLeft (in int32 pos); */
int32 SciDoc::WordPartLeft(int32 pos)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->WordPartLeft(pos);
}

/* int32 WordPartRight (in int32 pos); */
int32 SciDoc::WordPartRight(int32 pos)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->WordPartRight(pos);
}

/* int32 ExtendStyleRange (in int32 pos, in int32 delta, in boolean singleLine); */
int32 SciDoc::ExtendStyleRange(int32 pos, int32 delta, bool singleLine)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->ExtendStyleRange(pos, delta, singleLine);
}

/* boolean IsWhiteLine (in int32 line); */
bool SciDoc::IsWhiteLine(int32 line)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->WordPartRight(line);
}

/* int32 ParaUp (in int32 pos); */
int32 SciDoc::ParaUp(int32 pos)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->ParaUp(pos);
}

/* int32 ParaDown (in int32 pos); */
int32 SciDoc::ParaDown(int32 pos)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->ParaDown(pos);
}

/* int32 IndentSize (); */
int32 SciDoc::IndentSize()
{
    if (documentPointer==NULL) return 0;
    return documentPointer->IndentSize();
}

/* int32 BraceMatch (in int32 position, in int32 maxReStyle); */
int32 SciDoc::BraceMatch(int32 position, int32 maxReStyle)
{
    if (documentPointer==NULL) return 0;
    return documentPointer->BraceMatch(position, maxReStyle);
}