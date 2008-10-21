/*
 *  npSciDoc.h
 *  scintilla_plugin
 *
 *  Created by Marshall on 9/19/08.
 *  Copyright 2008 Appclerator, Inc. All rights reserved.
 *
 */

#ifndef _NPSCIDOC_H_
#define _NPSCIDOC_H_
#include "ISciDoc.h"

class SciDoc : public ISciDoc
{
public:
	SciDoc();
	Scintilla::Document *documentPointer;
	
private:
	~SciDoc();

public:
	virtual int32 GetDocPointer();
	virtual void SetDocPointer(int32 value);
	
	virtual int32 GetStylingBits();
	virtual void SetStylingBits(int32 value);
	
	virtual int32 GetStylingBitsMask();
	virtual void SetStylingBitsMask(int32 value);
	
	virtual int32 GetEolMode();
	virtual void SetEolMode(int32 value);
	
	virtual int32 GetDbcsCodePage();
	virtual void SetDbcsCodePage(int32 value);
	
	virtual int32 GetTabInChars();
	virtual void SetTabInChars(int32 value);
	
	virtual int32 GetIndentInChars();
	virtual void SetIndentInChars(int32 value);
	
	virtual int32 GetActualIndentInChars();
	virtual void SetActualIndentInChars(int32 value);
	
	virtual bool GetUseTabs();
	virtual void SetUseTabs(bool value);
	
	virtual bool GetTabIndents();
	virtual void SetTabIndents(bool value);
	
	virtual bool GetBackspaceUnindents();
	virtual void SetBackspaceUnindents(bool value);
	
	virtual std::string GetText();
	virtual void SetText(std::string value);
	
	
	virtual int32 LineFromPosition (int32 pos);
	virtual int32 ClampPositionIntoDocument (int32 pos);
	virtual bool IsCrLf (int32 pos);
	virtual int32 LenChar (int32 pos);
	virtual int32 MovePositionOutsideChar (int32 pos, int32 moveDir, bool checkLineEnd);
	virtual int32 GetBytePositionForCharOffset (int32 bytePos, int32 charOffset, bool checkLineEnd);
	virtual void ModifiedAt (int32 pos);
	virtual bool DeleteChars (int32 pos, int32 len);
	virtual bool InsertString (int32 position, std::string s);
	virtual int32 Undo ();
	virtual int32 Redo ();
	virtual bool CanUndo ();
	virtual bool CanRedo ();
	virtual void DeleteUndoHistory ();
	virtual bool SetUndoCollection (bool collectUndo);
	virtual bool IsCollectingUndo ();
	virtual void BeginUndoAction ();
	virtual void EndUndoAction ();
	virtual void SetSavePoint ();
	virtual bool IsSavePoint ();
	virtual int32 GetLineIndentation (int32 line);
	virtual void SetLineIndentation (int32 line, int32 indent);
	virtual int32 GetLineIndentPosition (int32 line);
	virtual int32 GetColumn (int32 position);
	virtual int32 FindColumn (int32 line, int32 column);
	virtual void Indent (bool forwards, int32 lineBottom, int32 lineTop);
	virtual std::string TransformLineEnds (std::string s, int32 eolMode);
	virtual void ConvertLineEnds (int32 eolModeSet);
	virtual void SetReadOnly (bool set);
	virtual bool IsReadOnly ();
	virtual bool InsertChar (int32 pos, char ch);
	virtual void ChangeChar (int32 pos, char ch);
	virtual void DelChar (int32 pos);
	virtual void DelCharBack (int32 pos);
	virtual char CharAt (int32 position);
	virtual std::string GetCharRange (int32 position, int32 lengthRetrieve);
	virtual char StyleAt (int32 position);
	virtual int32 GetMark (int32 line);
	virtual int32 AddMark (int32 line, int32 markerNum);
	virtual void AddMarkSet (int32 line, int32 valueSet);
	virtual void DeleteMark (int32 line, int32 markerNum);
	virtual void DeleteMarkFromHandle (int32 markerHandle);
	virtual void DeleteAllMarks (int32 markerNum);
	virtual int32 LineFromHandle (int32 markerHandle);
	virtual int32 LineStart (int32 line);
	virtual int32 LineEnd (int32 line);
	virtual int32 LineEndPosition (int32 position);
	virtual int32 VCHomePosition (int32 position);
	virtual int32 SetLevel (int32 line, int32 level);
	virtual int32 GetLevel (int32 line);
	virtual void ClearLevels ();
	virtual int32 GetLastChild (int32 lineParent, int32 level);
	virtual int32 GetFoldParent (int32 line);
	virtual int32 ExtendWordSelect (int32 pos, int32 delta, bool onlyWordCharacters);
	virtual int32 NextWordStart (int32 pos, int32 delta);
	virtual int32 NextWordEnd (int32 pos, int32 delta);
	virtual int32 Length ();
	virtual void Allocate (int32 newSize);
	virtual int32 FindText (int32 minPos, int32 maxPos, std::string s, bool caseSensitive, bool word, bool wordStart, bool regExp, bool posix);
	virtual std::string SubstituteByPosition (std::string text);
	virtual int32 LinesTotal ();
	virtual void ChangeCase (int32 start, int32 end, bool makeUpperCase);
	virtual void SetDefaultCharClasses (bool includeWordClass);
	virtual void StartStyling (int32 position, char mask);
	virtual bool SetStyleFor (int32 length, char style);
	virtual bool SetStyles (int32 length, std::string styles);
	virtual int32 GetEndStyled ();
	virtual void EnsureStyledTo (int32 pos);
	virtual int32 GetStyleClock ();
	virtual void IncrementStyleClock ();
	virtual void DecorationFillRange (int32 position, int32 value, int32 fillLength);
	virtual int32 SetLineState (int32 line, int32 state);
	virtual int32 GetLineState (int32 line);
	virtual int32 GetMaxLineState ();
	virtual bool IsWordPartSeparator (char ch);
	virtual int32 WordPartLeft (int32 pos);
	virtual int32 WordPartRight (int32 pos);
	virtual int32 ExtendStyleRange (int32 pos, int32 delta, bool singleLine);
	virtual bool IsWhiteLine (int32 line);
	virtual int32 ParaUp (int32 pos);
	virtual int32 ParaDown (int32 pos);
	virtual int32 IndentSize ();
	virtual int32 BraceMatch (int32 position, int32 maxReStyle);
};
#endif