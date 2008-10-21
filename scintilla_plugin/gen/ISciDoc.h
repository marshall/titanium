/*
 * WARNING this file was generated by Appcelerator's idl2npapi
 */
#ifndef _ISCIDOC_H
#define _ISCIDOC_H

#include <npapi.h>
#include <npruntime.h>
#include <string>



class ISciDoc : public NPObject
{
public:
	
	virtual int32 GetDocPointer() = 0;
	virtual void SetDocPointer(int32 value) = 0;

	virtual int32 GetStylingBits() = 0;
	virtual void SetStylingBits(int32 value) = 0;

	virtual int32 GetStylingBitsMask() = 0;
	virtual void SetStylingBitsMask(int32 value) = 0;

	virtual int32 GetEolMode() = 0;
	virtual void SetEolMode(int32 value) = 0;

	virtual int32 GetDbcsCodePage() = 0;
	virtual void SetDbcsCodePage(int32 value) = 0;

	virtual int32 GetTabInChars() = 0;
	virtual void SetTabInChars(int32 value) = 0;

	virtual int32 GetIndentInChars() = 0;
	virtual void SetIndentInChars(int32 value) = 0;

	virtual int32 GetActualIndentInChars() = 0;
	virtual void SetActualIndentInChars(int32 value) = 0;

	virtual bool GetUseTabs() = 0;
	virtual void SetUseTabs(bool value) = 0;

	virtual bool GetTabIndents() = 0;
	virtual void SetTabIndents(bool value) = 0;

	virtual bool GetBackspaceUnindents() = 0;
	virtual void SetBackspaceUnindents(bool value) = 0;

	virtual std::string GetText() = 0;
	virtual void SetText(std::string value) = 0;

	
	virtual int32 LineFromPosition (int32 pos) = 0;
	virtual int32 ClampPositionIntoDocument (int32 pos) = 0;
	virtual bool IsCrLf (int32 pos) = 0;
	virtual int32 LenChar (int32 pos) = 0;
	virtual int32 MovePositionOutsideChar (int32 pos, int32 moveDir, bool checkLineEnd) = 0;
	virtual void ModifiedAt (int32 pos) = 0;
	virtual bool DeleteChars (int32 pos, int32 len) = 0;
	virtual bool InsertString (int32 position, std::string s) = 0;
	virtual int32 Undo () = 0;
	virtual int32 Redo () = 0;
	virtual bool CanUndo () = 0;
	virtual bool CanRedo () = 0;
	virtual void DeleteUndoHistory () = 0;
	virtual bool SetUndoCollection (bool collectUndo) = 0;
	virtual bool IsCollectingUndo () = 0;
	virtual void BeginUndoAction () = 0;
	virtual void EndUndoAction () = 0;
	virtual void SetSavePoint () = 0;
	virtual bool IsSavePoint () = 0;
	virtual int32 GetLineIndentation (int32 line) = 0;
	virtual void SetLineIndentation (int32 line, int32 indent) = 0;
	virtual int32 GetLineIndentPosition (int32 line) = 0;
	virtual int32 GetColumn (int32 position) = 0;
	virtual int32 FindColumn (int32 line, int32 column) = 0;
	virtual void Indent (bool forwards, int32 lineBottom, int32 lineTop) = 0;
	virtual std::string TransformLineEnds (std::string s, int32 eolMode) = 0;
	virtual void ConvertLineEnds (int32 eolModeSet) = 0;
	virtual void SetReadOnly (bool set) = 0;
	virtual bool IsReadOnly () = 0;
	virtual bool InsertChar (int32 pos, char ch) = 0;
	virtual void ChangeChar (int32 pos, char ch) = 0;
	virtual void DelChar (int32 pos) = 0;
	virtual void DelCharBack (int32 pos) = 0;
	virtual char CharAt (int32 position) = 0;
	virtual std::string GetCharRange (int32 position, int32 lengthRetrieve) = 0;
	virtual char StyleAt (int32 position) = 0;
	virtual int32 GetMark (int32 line) = 0;
	virtual int32 AddMark (int32 line, int32 markerNum) = 0;
	virtual void AddMarkSet (int32 line, int32 valueSet) = 0;
	virtual void DeleteMark (int32 line, int32 markerNum) = 0;
	virtual void DeleteMarkFromHandle (int32 markerHandle) = 0;
	virtual void DeleteAllMarks (int32 markerNum) = 0;
	virtual int32 LineFromHandle (int32 markerHandle) = 0;
	virtual int32 LineStart (int32 line) = 0;
	virtual int32 LineEnd (int32 line) = 0;
	virtual int32 LineEndPosition (int32 position) = 0;
	virtual int32 VCHomePosition (int32 position) = 0;
	virtual int32 SetLevel (int32 line, int32 level) = 0;
	virtual int32 GetLevel (int32 line) = 0;
	virtual void ClearLevels () = 0;
	virtual int32 GetLastChild (int32 lineParent, int32 level) = 0;
	virtual int32 GetFoldParent (int32 line) = 0;
	virtual int32 ExtendWordSelect (int32 pos, int32 delta, bool onlyWordCharacters) = 0;
	virtual int32 NextWordStart (int32 pos, int32 delta) = 0;
	virtual int32 NextWordEnd (int32 pos, int32 delta) = 0;
	virtual int32 Length () = 0;
	virtual void Allocate (int32 newSize) = 0;
	virtual int32 FindText (int32 minPos, int32 maxPos, std::string s, bool caseSensitive, bool word, bool wordStart, bool regExp, bool posix) = 0;
	virtual std::string SubstituteByPosition (std::string text) = 0;
	virtual int32 LinesTotal () = 0;
	virtual void ChangeCase (int32 start, int32 end, bool makeUpperCase) = 0;
	virtual void SetDefaultCharClasses (bool includeWordClass) = 0;
	virtual void StartStyling (int32 position, char mask) = 0;
	virtual bool SetStyleFor (int32 length, char style) = 0;
	virtual bool SetStyles (int32 length, std::string styles) = 0;
	virtual int32 GetEndStyled () = 0;
	virtual void EnsureStyledTo (int32 pos) = 0;
	virtual int32 GetStyleClock () = 0;
	virtual void IncrementStyleClock () = 0;
	virtual void DecorationFillRange (int32 position, int32 value, int32 fillLength) = 0;
	virtual int32 SetLineState (int32 line, int32 state) = 0;
	virtual int32 GetLineState (int32 line) = 0;
	virtual int32 GetMaxLineState () = 0;
	virtual bool IsWordPartSeparator (char ch) = 0;
	virtual int32 WordPartLeft (int32 pos) = 0;
	virtual int32 WordPartRight (int32 pos) = 0;
	virtual int32 ExtendStyleRange (int32 pos, int32 delta, bool singleLine) = 0;
	virtual bool IsWhiteLine (int32 line) = 0;
	virtual int32 ParaUp (int32 pos) = 0;
	virtual int32 ParaDown (int32 pos) = 0;
	virtual int32 IndentSize () = 0;
	virtual int32 BraceMatch (int32 position, int32 maxReStyle) = 0;
	
};

#endif