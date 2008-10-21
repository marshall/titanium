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

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef __nsSciMoz_h__
#define __nsSciMoz_h__

#include <stdio.h> 
#include <string.h> 

//#define SCIMOZ_DEBUG
//#define SCIDEBUG_REFS

#ifdef _WINDOWS
// with optimizations on, we crash "somewhere" in this file in a release build
// when we drag from scintilla into mozilla, over an tree
// komodo bugzilla bug 19186
// #pragma optimize("", off)
#else
#ifndef XP_MACOSX
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h> 
#include <gtk/gtk.h> 
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>

#ifdef GTK2
#include <gtk/gtkplug.h>
#else
#include <gtkmozbox.h>
#endif

/* Xlib/Xt stuff */
#ifdef MOZ_X11
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#endif
#endif
#endif 

/**
 * {3849EF46-AE99-45f7-BF8A-CC4B053A946B}
 */
#define SCI_MOZ_CID { 0x3849ef46, 0xae99, 0x45f7, { 0xbf, 0x8a, 0xcc, 0x4b, 0x5, 0x3a, 0x94, 0x6b } }
#define SCI_MOZ_PROGID "@mozilla.org/inline-plugin/application/x-scimoz-plugin"

#include "ISciMoz.h"
#include "ISciMozEvents.h"

#ifdef _WINDOWS
#include <windows.h>
#include <shellapi.h>
#include <richedit.h>
#undef FindText // conflicts with our definition of that name!
#endif

#ifdef XP_MACOSX
#include <Platform.h>
#include <ScintillaMacOSX.h>
#endif
#include <Scintilla.h>
#include "sendscintilla.h"
#include <SciLexer.h>

#define SCIMAX(a, b) (a > b ? a : b)
#define SCIMIN(a, b) (a < b ? a : b)
#define LONGFROMTWOSHORTS(a, b) ((a) | ((b) << 16))

// XXX also defined in ScintillaWin.cxx
#ifndef WM_UNICHAR
#define WM_UNICHAR                      0x0109
#endif


#include "SciMozEvents.h"

#ifdef XP_PC
static const char* gInstanceLookupString = "instance->pdata";

typedef struct _PlatformInstance {
	WNDPROC	fDefaultWindowProc;
	WNDPROC fDefaultChildWindowProc;
}
PlatformInstance;
#endif 

#if defined(XP_UNIX) && !defined(XP_MACOSX)
typedef struct _PlatformInstance {
	NPSetWindowCallbackStruct *ws_info;
	GtkWidget *moz_box;
}
PlatformInstance;
#define PLAT_GTK 1
#include "ScintillaWidget.h"
#endif 

#if defined(XP_MAC) || defined(XP_MACOSX)
#include <Carbon/Carbon.h>
typedef struct _PlatformInstance {
	WindowPtr	container;
#ifndef USE_CARBON //1.8 branch
	CGrafPtr    port;
#else
	CGContextRef port;
#endif
}
PlatformInstance;
#endif

class SciMoz : public ISciMoz
               //public nsClassInfoMixin,
               //public nsSupportsWeakReference
               
{
private:
    long _lastCharCodeAdded;
    
    // brace match support
    long bracesStyle;
    long bracesCheck;
    bool bracesSloppy;
    
    bool FindMatchingBracePosition(int &braceAtCaret, int &braceOpposite, bool sloppy);
    void BraceMatch();
    
public:
  SciMoz(PluginInstance* plugin);
  ~SciMoz();

#ifdef SCIDEBUG_REFS
public:
  int getRefCount() { return mRefCnt.get(); }
#endif
protected: 
    NPWindow* fWindow;
//    nsPluginMode fMode;
    PlatformInstance fPlatform;

    void *portMain;	// Native window in portable type
    WinID wMain;	// portMain cast into a native type
    WinID wEditor;
    WinID wParkingLot;  // temporary parent window while not visible.

#ifdef USE_SCIN_DIRECT	
    SciFnDirect fnEditor;
    long ptrEditor;
#endif

    bool initialised;
    bool parked;
    int width;
    int height;
    EventListeners listeners;
    //nsCOMPtr<nsIDOMWindowInternal> commandUpdateTarget;
    bool bCouldUndoLastTime;
    bool bCouldRedoLastTime;

    long SendEditor(unsigned int Msg, unsigned long wParam = 0, long lParam = 0);

    void Create(WinID hWnd);
    void PlatformCreate(WinID hWnd);
    void Notify(long lParam);
    void Resize();
    void _DoButtonUpDown(bool up, int32 x, int32 y, uint16 button, uint32 timeStamp, bool bShift, bool bCtrl, bool bAlt);

#ifdef XP_MACOSX
	void SetHIViewShowHide(bool disabled);
	static void NotifySignal(intptr_t windowid, unsigned int iMessage, uintptr_t wParam, uintptr_t lParam);
	Scintilla::ScintillaMacOSX *scintilla;
#endif
#ifdef XP_PC
    void LoadScintillaLibrary();
#endif

    // IME support
    int imeStartPos;
    bool imeComposing;
    bool imeActive;
	std::string mIMEString;
    void StartCompositing();
    void EndCompositing();
public:
	std::string name;
  // native methods callable from JavaScript
  //NS_DECL_ISUPPORTS
  //NS_DECL_ISCIMOZLITE
  //NS_DECL_ISCIMOZ

  void SetInstance(PluginInstance* plugin);

    void PlatformNew(void);

    // Destroy is always called as we destruct.
    int32 PlatformDestroy(void);

    // SetWindow is called as Mozilla gives us a window object.
    // If we are doing "window parking", we can attach
    // our existing Scintilla to the new Moz window.
    int32 PlatformSetWindow(NPWindow* window);

    // ResetWindow is called as the Mozilla window dies.
    // If we are doing "window parking", this is when we park.
    // Will also be called if Moz ever hands us a new window
    // while we already have one.
    int32 PlatformResetWindow();

    int32 PlatformHandleEvent(void* event);

//    void SetMode(nsPluginMode mode) { fMode = mode; }

#ifdef XP_PC
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ParkingLotWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#endif 

#if defined(XP_UNIX) && !defined(XP_MACOSX)
    int sInGrab;
    static void NotifySignal(GtkWidget *, gint wParam, gpointer lParam, SciMoz *scimoz);
#endif 

protected:
  PluginInstance* mPlugin;

// ********************************//
// ** Implementation of ISciMoz ** //
// *******************************//
public:
	virtual std::string GetName();
	virtual void SetName(std::string value);
	
	virtual std::string GetText();
	virtual void SetText(std::string value);
	
	virtual std::string GetSelText();
	
	virtual int32 GetLastCharCodeAdded();
	virtual void SetLastCharCodeAdded(int32 value);
	
	virtual bool GetIsOwned();
	
	virtual bool GetVisible();
	virtual void SetVisible(bool value);
	
	virtual bool GetIsTracking();
	
	virtual bool GetInDragSession();
	
	virtual bool GetUndoCollection();
	virtual void SetUndoCollection(bool value);
	
	virtual int32 GetViewWS();
	virtual void SetViewWS(int32 value);
	
	virtual int32 GetEndStyled();
	
	virtual int32 GetEOLMode();
	virtual void SetEOLMode(int32 value);
	
	virtual bool GetBufferedDraw();
	virtual void SetBufferedDraw(bool value);
	
	virtual int32 GetTabWidth();
	virtual void SetTabWidth(int32 value);
	
	virtual int32 GetSelAlpha();
	virtual void SetSelAlpha(int32 value);
	
	virtual bool GetSelEOLFilled();
	virtual void SetSelEOLFilled(bool value);
	
	virtual int32 GetCaretPeriod();
	virtual void SetCaretPeriod(int32 value);
	
	virtual int32 GetStyleBits();
	virtual void SetStyleBits(int32 value);
	
	virtual int32 GetMaxLineState();
	
	virtual bool GetCaretLineVisible();
	virtual void SetCaretLineVisible(bool value);
	
	virtual int32 GetCaretLineBack();
	virtual void SetCaretLineBack(int32 value);
	
	virtual int32 GetAutoCSeparator();
	virtual void SetAutoCSeparator(int32 value);
	
	virtual bool GetAutoCCancelAtStart();
	virtual void SetAutoCCancelAtStart(bool value);
	
	virtual bool GetAutoCChooseSingle();
	virtual void SetAutoCChooseSingle(bool value);
	
	virtual bool GetAutoCIgnoreCase();
	virtual void SetAutoCIgnoreCase(bool value);
	
	virtual bool GetAutoCAutoHide();
	virtual void SetAutoCAutoHide(bool value);
	
	virtual bool GetAutoCDropRestOfWord();
	virtual void SetAutoCDropRestOfWord(bool value);
	
	virtual int32 GetAutoCTypeSeparator();
	virtual void SetAutoCTypeSeparator(int32 value);
	
	virtual int32 GetAutoCMaxWidth();
	virtual void SetAutoCMaxWidth(int32 value);
	
	virtual int32 GetAutoCMaxHeight();
	virtual void SetAutoCMaxHeight(int32 value);
	
	virtual int32 GetIndent();
	virtual void SetIndent(int32 value);
	
	virtual bool GetUseTabs();
	virtual void SetUseTabs(bool value);
	
	virtual bool GetHScrollBar();
	virtual void SetHScrollBar(bool value);
	
	virtual int32 GetIndentationGuides();
	virtual void SetIndentationGuides(int32 value);
	
	virtual int32 GetHighlightGuide();
	virtual void SetHighlightGuide(int32 value);
	
	virtual int32 GetCodePage();
	virtual void SetCodePage(int32 value);
	
	virtual int32 GetCaretFore();
	virtual void SetCaretFore(int32 value);
	
	virtual bool GetUsePalette();
	virtual void SetUsePalette(bool value);
	
	virtual int32 GetPrintMagnification();
	virtual void SetPrintMagnification(int32 value);
	
	virtual int32 GetPrintColourMode();
	virtual void SetPrintColourMode(int32 value);
	
	virtual int32 GetFirstVisibleLine();
	
	virtual int32 GetLineCount();
	
	virtual int32 GetMarginLeft();
	virtual void SetMarginLeft(int32 value);
	
	virtual int32 GetMarginRight();
	virtual void SetMarginRight(int32 value);
	
	virtual bool GetModify();
	
	virtual int32 GetTextLength();
	
	virtual int32 GetDirectFunction();
	
	virtual int32 GetDirectPointer();
	
	virtual bool GetOvertype();
	virtual void SetOvertype(bool value);
	
	virtual int32 GetCaretWidth();
	virtual void SetCaretWidth(int32 value);
	
	virtual int32 GetTargetStart();
	virtual void SetTargetStart(int32 value);
	
	virtual int32 GetTargetEnd();
	virtual void SetTargetEnd(int32 value);
	
	virtual int32 GetSearchFlags();
	virtual void SetSearchFlags(int32 value);
	
	virtual bool GetTabIndents();
	virtual void SetTabIndents(bool value);
	
	virtual bool GetBackSpaceUnIndents();
	virtual void SetBackSpaceUnIndents(bool value);
	
	virtual int32 GetMouseDwellTime();
	virtual void SetMouseDwellTime(int32 value);
	
	virtual int32 GetWrapMode();
	virtual void SetWrapMode(int32 value);
	
	virtual int32 GetWrapVisualFlags();
	virtual void SetWrapVisualFlags(int32 value);
	
	virtual int32 GetWrapVisualFlagsLocation();
	virtual void SetWrapVisualFlagsLocation(int32 value);
	
	virtual int32 GetWrapStartIndent();
	virtual void SetWrapStartIndent(int32 value);
	
	virtual int32 GetLayoutCache();
	virtual void SetLayoutCache(int32 value);
	
	virtual bool GetScrollWidthTracking();
	virtual void SetScrollWidthTracking(bool value);
	
	virtual bool GetEndAtLastLine();
	virtual void SetEndAtLastLine(bool value);
	
	virtual bool GetVScrollBar();
	virtual void SetVScrollBar(bool value);
	
	virtual bool GetTwoPhaseDraw();
	virtual void SetTwoPhaseDraw(bool value);
	
	virtual bool GetViewEOL();
	virtual void SetViewEOL(bool value);
	
	virtual int32 GetDocPointer();
	virtual void SetDocPointer(int32 value);
	
	virtual int32 GetEdgeColumn();
	virtual void SetEdgeColumn(int32 value);
	
	virtual int32 GetEdgeMode();
	virtual void SetEdgeMode(int32 value);
	
	virtual int32 GetEdgeColour();
	virtual void SetEdgeColour(int32 value);
	
	virtual int32 GetLinesOnScreen();
	
	virtual bool GetSelectionIsRectangle();
	
	virtual int32 GetZoom();
	virtual void SetZoom(int32 value);
	
	virtual int32 GetModEventMask();
	virtual void SetModEventMask(int32 value);
	
	virtual bool GetFocus();
	virtual void SetFocus(bool value);
	
	virtual int32 GetStatus();
	virtual void SetStatus(int32 value);
	
	virtual bool GetMouseDownCaptures();
	virtual void SetMouseDownCaptures(bool value);
	
	virtual int32 GetCursor();
	virtual void SetCursor(int32 value);
	
	virtual int32 GetControlCharSymbol();
	virtual void SetControlCharSymbol(int32 value);
	
	virtual int32 GetPrintWrapMode();
	virtual void SetPrintWrapMode(int32 value);
	
	virtual int32 GetHotspotActiveFore();
	virtual void SetHotspotActiveFore(int32 value);
	
	virtual int32 GetHotspotActiveBack();
	virtual void SetHotspotActiveBack(int32 value);
	
	virtual bool GetHotspotActiveUnderline();
	virtual void SetHotspotActiveUnderline(bool value);
	
	virtual bool GetHotspotSingleLine();
	virtual void SetHotspotSingleLine(bool value);
	
	virtual int32 GetSelectionMode();
	virtual void SetSelectionMode(int32 value);
	
	virtual bool GetCaretSticky();
	virtual void SetCaretSticky(bool value);
	
	virtual bool GetPasteConvertEndings();
	virtual void SetPasteConvertEndings(bool value);
	
	virtual int32 GetCaretLineBackAlpha();
	virtual void SetCaretLineBackAlpha(int32 value);
	
	virtual int32 GetCaretStyle();
	virtual void SetCaretStyle(int32 value);
	
	virtual int32 GetIndicatorCurrent();
	virtual void SetIndicatorCurrent(int32 value);
	
	virtual int32 GetIndicatorValue();
	virtual void SetIndicatorValue(int32 value);
	
	virtual int32 GetPositionCache();
	virtual void SetPositionCache(int32 value);
	
	virtual int32 GetCharacterPointer();
	
	virtual bool GetKeysUnicode();
	virtual void SetKeysUnicode(bool value);
	
	virtual int32 GetLexer();
	virtual void SetLexer(int32 value);
	
	virtual int32 GetStyleBitsNeeded();
	
	virtual int32 GetLength();
	
	virtual int32 GetCurrentPos();
	virtual void SetCurrentPos(int32 value);
	
	virtual int32 GetAnchor();
	virtual void SetAnchor(int32 value);
	
	virtual bool GetReadOnly();
	virtual void SetReadOnly(bool value);
	
	virtual int32 GetSelectionStart();
	virtual void SetSelectionStart(int32 value);
	
	virtual int32 GetSelectionEnd();
	virtual void SetSelectionEnd(int32 value);
	
	virtual int32 GetScrollWidth();
	virtual void SetScrollWidth(int32 value);
	
	virtual int32 GetXOffset();
	virtual void SetXOffset(int32 value);
	
	
	virtual void hookEvents (ISciMozEvents* eventListener);
	virtual void hookEventsWithStrongReference (ISciMozEvents* eventListener);
	virtual void hookSomeEvents (ISciMozEvents* eventListener, uint32 mask);
	virtual void hookSomeEventsWithStrongReference (ISciMozEvents* eventListener, uint32 mask);
	virtual void unhookEvents (ISciMozEvents* eventListener);
	virtual void setCommandUpdateTarget (NPObject * window);
	virtual void sendUpdateCommands (std::string text);
	virtual void getStyledText (int32 min, int32 max, uint32 long, uint8_t str);
	virtual int32 getCurLine (std::string text);
	virtual void assignCmdKey (int32 key, int32 modifiers, int32 msg);
	virtual void clearCmdKey (int32 key, int32 modifiers);
	virtual std::string getTextRange (int32 min, int32 max);
	virtual void doBraceMatch ();
	virtual char getWCharAt (int32 pos);
	virtual int32 charPosAtPosition (int32 position);
	virtual void addChar (uint32 ch);
	virtual int32 getLine (int32 line, std::string text);
	virtual void endDrop ();
	virtual std::string handleTextEvent (NPObject * event);
	virtual void addStyledText (int32 length, std::string c);
	virtual void clearAll ();
	virtual void clearDocumentStyle ();
	virtual int32 getCharAt (int32 pos);
	virtual int32 getStyleAt (int32 pos);
	virtual void redo ();
	virtual void setSavePoint ();
	virtual bool canRedo ();
	virtual int32 markerLineFromHandle (int32 handle);
	virtual void markerDeleteHandle (int32 handle);
	virtual int32 positionFromPoint (int32 x, int32 y);
	virtual int32 positionFromPointClose (int32 x, int32 y);
	virtual void convertEOLs (int32 eolMode);
	virtual void markerDefine (int32 markerNumber, int32 markerSymbol);
	virtual void markerSetFore (int32 markerNumber, int32 fore);
	virtual void markerSetBack (int32 markerNumber, int32 back);
	virtual void markerDelete (int32 line, int32 markerNumber);
	virtual void markerDeleteAll (int32 markerNumber);
	virtual int32 markerGet (int32 line);
	virtual int32 markerPrevious (int32 lineStart, int32 markerMask);
	virtual void markerDefinePixmap (int32 markerNumber, std::string pixmap);
	virtual void markerAddSet (int32 line, int32 set);
	virtual void markerSetAlpha (int32 markerNumber, int32 alpha);
	virtual void setMarginTypeN (int32 margin, int32 marginType);
	virtual int32 getMarginTypeN (int32 margin);
	virtual void setMarginWidthN (int32 margin, int32 pixelWidth);
	virtual int32 getMarginWidthN (int32 margin);
	virtual void setMarginMaskN (int32 margin, int32 mask);
	virtual int32 getMarginMaskN (int32 margin);
	virtual void setMarginSensitiveN (int32 margin, bool sensitive);
	virtual bool getMarginSensitiveN (int32 margin);
	virtual void styleClearAll ();
	virtual void styleSetBack (int32 style, int32 back);
	virtual void styleSetBold (int32 style, bool bold);
	virtual void styleSetItalic (int32 style, bool italic);
	virtual void styleSetSize (int32 style, int32 sizePoints);
	virtual void styleSetFont (int32 style, std::string fontName);
	virtual void styleSetEOLFilled (int32 style, bool filled);
	virtual void styleResetDefault ();
	virtual void styleSetUnderline (int32 style, bool underline);
	virtual int32 styleGetFore (int32 style);
	virtual int32 styleGetBack (int32 style);
	virtual bool styleGetBold (int32 style);
	virtual bool styleGetItalic (int32 style);
	virtual int32 styleGetSize (int32 style);
	virtual int32 styleGetFont (int32 style, std::string fontName);
	virtual bool styleGetEOLFilled (int32 style);
	virtual bool styleGetUnderline (int32 style);
	virtual int32 styleGetCase (int32 style);
	virtual int32 styleGetCharacterSet (int32 style);
	virtual bool styleGetVisible (int32 style);
	virtual bool styleGetChangeable (int32 style);
	virtual bool styleGetHotSpot (int32 style);
	virtual void styleSetCase (int32 style, int32 caseForce);
	virtual void styleSetCharacterSet (int32 style, int32 characterSet);
	virtual void styleSetHotSpot (int32 style, bool hotspot);
	virtual void setSelFore (bool useSetting, int32 fore);
	virtual void setSelBack (bool useSetting, int32 back);
	virtual void clearAllCmdKeys ();
	virtual void setStylingEx (int32 length, std::string styles);
	virtual void styleSetVisible (int32 style, bool visible);
	virtual void setWordChars (std::string characters);
	virtual void beginUndoAction ();
	virtual void endUndoAction ();
	virtual void indicSetStyle (int32 indic, int32 style);
	virtual int32 indicGetStyle (int32 indic);
	virtual void indicSetFore (int32 indic, int32 fore);
	virtual int32 indicGetFore (int32 indic);
	virtual void indicSetUnder (int32 indic, bool under);
	virtual bool indicGetUnder (int32 indic);
	virtual void setWhitespaceFore (bool useSetting, int32 fore);
	virtual void setWhitespaceBack (bool useSetting, int32 back);
	virtual void setLineState (int32 line, int32 state);
	virtual int32 getLineState (int32 line);
	virtual void styleSetChangeable (int32 style, bool changeable);
	virtual void autoCShow (int32 lenEntered, std::string itemList);
	virtual void autoCCancel ();
	virtual bool autoCActive ();
	virtual int32 autoCPosStart ();
	virtual void autoCComplete ();
	virtual void autoCStops (std::string characterSet);
	virtual void autoCSelect (std::string text);
	virtual void autoCSetFillUps (std::string characterSet);
	virtual void userListShow (int32 listType, std::string itemList);
	virtual void registerImage (int32 type, std::string xpmData);
	virtual void clearRegisteredImages ();
	virtual void setLineIndentation (int32 line, int32 indentSize);
	virtual int32 getLineIndentation (int32 line);
	virtual int32 getLineIndentPosition (int32 line);
	virtual int32 getColumn (int32 pos);
	virtual int32 getLineEndPosition (int32 line);
	virtual void setSel (int32 start, int32 end);
	virtual int32 pointXFromPosition (int32 pos);
	virtual int32 pointYFromPosition (int32 pos);
	virtual int32 lineFromPosition (int32 pos);
	virtual int32 positionFromLine (int32 line);
	virtual void lineScroll (int32 columns, int32 lines);
	virtual void scrollCaret ();
	virtual bool canPaste ();
	virtual bool canUndo ();
	virtual void emptyUndoBuffer ();
	virtual void undo ();
	virtual void cut ();
	virtual void copy ();
	virtual void paste ();
	virtual void clear ();
	virtual int32 replaceTarget (int32 length, std::string text);
	virtual int32 replaceTargetRE (int32 length, std::string text);
	virtual int32 searchInTarget (int32 length, std::string text);
	virtual void callTipShow (int32 pos, std::string definition);
	virtual void callTipCancel ();
	virtual bool callTipActive ();
	virtual int32 callTipPosStart ();
	virtual void callTipSetHlt (int32 start, int32 end);
	virtual void callTipSetBack (int32 back);
	virtual void callTipSetFore (int32 fore);
	virtual void callTipSetForeHlt (int32 fore);
	virtual void callTipUseStyle (int32 tabSize);
	virtual int32 visibleFromDocLine (int32 line);
	virtual int32 docLineFromVisible (int32 lineDisplay);
	virtual int32 wrapCount (int32 line);
	virtual void setFoldLevel (int32 line, int32 level);
	virtual int32 getFoldLevel (int32 line);
	virtual int32 getLastChild (int32 line, int32 level);
	virtual int32 getFoldParent (int32 line);
	virtual void showLines (int32 lineStart, int32 lineEnd);
	virtual void hideLines (int32 lineStart, int32 lineEnd);
	virtual bool getLineVisible (int32 line);
	virtual void setFoldExpanded (int32 line, bool expanded);
	virtual bool getFoldExpanded (int32 line);
	virtual void toggleFold (int32 line);
	virtual void ensureVisible (int32 line);
	virtual void setFoldFlags (int32 flags);
	virtual void ensureVisibleEnforcePolicy (int32 line);
	virtual int32 wordStartPosition (int32 pos, bool onlyWordCharacters);
	virtual int32 wordEndPosition (int32 pos, bool onlyWordCharacters);
	virtual int32 textWidth (int32 style, std::string text);
	virtual int32 textHeight (int32 line);
	virtual void appendText (int32 length, std::string text);
	virtual void targetFromSelection ();
	virtual void linesJoin ();
	virtual void linesSplit (int32 pixelWidth);
	virtual void setFoldMarginColour (bool useSetting, int32 back);
	virtual void setFoldMarginHiColour (bool useSetting, int32 fore);
	virtual void lineDown ();
	virtual void lineDownExtend ();
	virtual void lineUp ();
	virtual void lineUpExtend ();
	virtual void charLeft ();
	virtual void charLeftExtend ();
	virtual void charRight ();
	virtual void charRightExtend ();
	virtual void wordLeft ();
	virtual void wordLeftExtend ();
	virtual void wordRight ();
	virtual void wordRightExtend ();
	virtual void homeExtend ();
	virtual void lineEnd ();
	virtual void lineEndExtend ();
	virtual void documentStart ();
	virtual void documentStartExtend ();
	virtual void documentEnd ();
	virtual void documentEndExtend ();
	virtual void pageUp ();
	virtual void pageUpExtend ();
	virtual void pageDown ();
	virtual void pageDownExtend ();
	virtual void editToggleOvertype ();
	virtual void cancel ();
	virtual void tab ();
	virtual void backTab ();
	virtual void formFeed ();
	virtual void vCHome ();
	virtual void vCHomeExtend ();
	virtual void zoomIn ();
	virtual void zoomOut ();
	virtual void delWordLeft ();
	virtual void delWordRight ();
	virtual void delWordRightEnd ();
	virtual void lineCut ();
	virtual void lineDelete ();
	virtual void lineTranspose ();
	virtual void lineDuplicate ();
	virtual void lowerCase ();
	virtual void upperCase ();
	virtual void lineScrollDown ();
	virtual void lineScrollUp ();
	virtual void deleteBackNotLine ();
	virtual void homeDisplay ();
	virtual void homeDisplayExtend ();
	virtual void lineEndDisplay ();
	virtual void lineEndDisplayExtend ();
	virtual void homeWrap ();
	virtual void homeWrapExtend ();
	virtual void lineEndWrap ();
	virtual void lineEndWrapExtend ();
	virtual void vCHomeWrap ();
	virtual void vCHomeWrapExtend ();
	virtual void lineCopy ();
	virtual void moveCaretInsideView ();
	virtual int32 lineLength (int32 line);
	virtual void braceHighlight (int32 pos1, int32 pos2);
	virtual void braceBadLight (int32 pos);
	virtual int32 braceMatch (int32 pos);
	virtual void searchAnchor ();
	virtual int32 searchNext (int32 flags, std::string text);
	virtual int32 searchPrev (int32 flags, std::string text);
	virtual void usePopUp (bool allowPopUp);
	virtual int32 createDocument ();
	virtual void addRefDocument (int32 doc);
	virtual void releaseDocument (int32 doc);
	virtual void wordPartLeft ();
	virtual void wordPartLeftExtend ();
	virtual void wordPartRight ();
	virtual void wordPartRightExtend ();
	virtual void setVisiblePolicy (int32 visiblePolicy, int32 visibleSlop);
	virtual void delLineLeft ();
	virtual void delLineRight ();
	virtual void chooseCaretX ();
	virtual void grabFocus ();
	virtual void setXCaretPolicy (int32 caretPolicy, int32 caretSlop);
	virtual void setYCaretPolicy (int32 caretPolicy, int32 caretSlop);
	virtual void paraDown ();
	virtual void paraDownExtend ();
	virtual void paraUp ();
	virtual void paraUpExtend ();
	virtual int32 positionBefore (int32 pos);
	virtual int32 positionAfter (int32 pos);
	virtual void copyRange (int32 start, int32 end);
	virtual void copyText (int32 length, std::string text);
	virtual int32 getLineSelStartPosition (int32 line);
	virtual int32 getLineSelEndPosition (int32 line);
	virtual void lineDownRectExtend ();
	virtual void lineUpRectExtend ();
	virtual void charLeftRectExtend ();
	virtual void charRightRectExtend ();
	virtual void homeRectExtend ();
	virtual void vCHomeRectExtend ();
	virtual void lineEndRectExtend ();
	virtual void pageUpRectExtend ();
	virtual void pageDownRectExtend ();
	virtual void stutteredPageUp ();
	virtual void stutteredPageUpExtend ();
	virtual void stutteredPageDown ();
	virtual void stutteredPageDownExtend ();
	virtual void wordLeftEnd ();
	virtual void wordLeftEndExtend ();
	virtual void wordRightEnd ();
	virtual void wordRightEndExtend ();
	virtual void setWhitespaceChars (std::string characters);
	virtual void setCharsDefault ();
	virtual int32 autoCGetCurrent ();
	virtual void allocate (int32 bytes);
	virtual int32 targetAsUTF8 (std::string s);
	virtual void setLengthForEncode (int32 bytes);
	virtual int32 encodedFromUTF8 (std::string utf8, std::string encoded);
	virtual int32 findColumn (int32 line, int32 column);
	virtual void toggleCaretSticky ();
	virtual void selectionDuplicate ();
	virtual void indicatorFillRange (int32 position, int32 fillLength);
	virtual void indicatorClearRange (int32 position, int32 clearLength);
	virtual int32 indicatorAllOnFor (int32 position);
	virtual int32 indicatorValueAt (int32 indicator, int32 position);
	virtual int32 indicatorStart (int32 indicator, int32 position);
	virtual int32 indicatorEnd (int32 indicator, int32 position);
	virtual void copyAllowLine ();
	virtual void startRecord ();
	virtual void stopRecord ();
	virtual void colourise (int32 start, int32 end);
	virtual void setProperty (std::string key, std::string value);
	virtual void setKeyWords (int32 keywordSet, std::string keyWords);
	virtual void setLexerLanguage (std::string language);
	virtual void loadLexerLibrary (std::string path);
	virtual int32 getProperty (std::string key, std::string buf);
	virtual int32 getPropertyExpanded (std::string key, std::string buf);
	virtual int32 getPropertyInt (std::string key);
	virtual void setCaretPolicy (int32 caretPolicy, int32 caretSlop);
	virtual void addText (int32 length, std::string text);
	virtual void insertText (int32 pos, std::string text);
	virtual void selectAll ();
	virtual void gotoLine (int32 line);
	virtual void gotoPos (int32 pos);
	virtual void startStyling (int32 pos, int32 mask);
	virtual void setStyling (int32 length, int32 style);
	virtual int32 markerAdd (int32 line, int32 markerNumber);
	virtual int32 markerNext (int32 lineStart, int32 markerMask);
	virtual void styleSetFore (int32 style, int32 fore);
	virtual void hideSelection (bool normal);
	virtual void replaceSel (std::string text);
	virtual void deleteBack ();
	virtual void newLine ();
	virtual void buttonDown (int32 x, int32 y, uint16 button, uint32 timeStamp, bool bShift, bool bCtrl, bool bAlt);
	virtual void buttonUp (int32 x, int32 y, uint16 button, uint32 timeStamp, bool bShift, bool bCtrl, bool bAlt);
	virtual void buttonMove (int32 x, int32 y);
	
};

// We use our own timeline macros so they can be switch on independently.
//#include "nsITimelineService.h"
#if !defined(MOZ_TIMELINE) && defined (SCIMOZ_TIMELINE)
#undef SCIMOZ_TIMELINE
#endif

#if defined (SCIMOZ_TIMELINE)

#define SCIMOZ_TIMELINE_MARK NS_TIMELINE_MARK
// NS_TIMELINE_MARKV is wrong!
#define SCIMOZ_TIMELINE_MARKV NS_TimelineMark
#define SCIMOZ_TIMELINE_INDENT NS_TIMELINE_INDENT
#define SCIMOZ_TIMELINE_OUTDENT NS_TIMELINE_OUTDENT
#define SCIMOZ_TIMELINE_ENTER NS_TIMELINE_ENTER
#define SCIMOZ_TIMELINE_LEAVE NS_TIMELINE_LEAVE
#define SCIMOZ_TIMELINE_START_TIMER NS_TIMELINE_START_TIMER
#define SCIMOZ_TIMELINE_STOP_TIMER NS_TIMELINE_STOP_TIMER
#define SCIMOZ_TIMELINE_MARK_TIMER NS_TIMELINE_MARK_TIMER
#define SCIMOZ_TIMELINE_RESET_TIMER NS_TIMELINE_RESET_TIMER
#define SCIMOZ_TIMELINE_MARK_TIMER1 NS_TIMELINE_MARK_TIMER1

#else
#define SCIMOZ_TIMELINE_MARK(text)
#define SCIMOZ_TIMELINE_MARKV(args)
#define SCIMOZ_TIMELINE_INDENT()
#define SCIMOZ_TIMELINE_OUTDENT()
#define SCIMOZ_TIMELINE_START_TIMER(timerName)
#define SCIMOZ_TIMELINE_STOP_TIMER(timerName)
#define SCIMOZ_TIMELINE_MARK_TIMER(timerName)
#define SCIMOZ_TIMELINE_RESET_TIMER(timerName)
#define SCIMOZ_TIMELINE_MARK_TIMER1(timerName, str)
#define SCIMOZ_TIMELINE_ENTER(text)
#define SCIMOZ_TIMELINE_LEAVE(text)
#define SCIMOZ_TIMELINE_MARK_URI(text, uri)
#define SCIMOZ_TIMELINE_MARK_FUNCTION(timer)
#define SCIMOZ_TIMELINE_TIME_FUNCTION(timer)
#define SCIMOZ_TIMELINE_MARK_CHANNEL(text, channel)
#define SCIMOZ_TIMELINE_MARK_LOADER(text, loader);
#endif // defined (SCIMOZ_TIMELINE)

#endif

