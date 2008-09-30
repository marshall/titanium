// Scintilla source code edit control
// ScintillaGTK.cxx - GTK+ specific subclass of ScintillaBase
// Copyright 1998-2004 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "Platform.h"

#if PLAT_GTK_WIN32
#include "windows.h"
#endif

#include "Scintilla.h"
#include "ScintillaWidget.h"
#ifdef SCI_LEXER
#include "SciLexer.h"
#include "PropSet.h"
#include "Accessor.h"
#include "KeyWords.h"
#endif
#include "SVector.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "XPM.h"
#include "LineMarker.h"
#include "Style.h"
#include "AutoComplete.h"
#include "ViewStyle.h"
#include "Decoration.h"
#include "CharClassify.h"
#include "Document.h"
#include "PositionCache.h"
#include "Editor.h"
#include "SString.h"
#include "ScintillaBase.h"
#include "UniConversion.h"

#include "gtk/gtksignal.h"
#include "gtk/gtkmarshal.h"
#if GLIB_MAJOR_VERSION >= 2
#include "scintilla-marshal.h"
#endif

#ifdef SCI_LEXER
#include <glib.h>
#include <gmodule.h>
#include "ExternalLexer.h"
#endif

#define INTERNATIONAL_INPUT

#if !PLAT_GTK_WIN32 || GTK_MAJOR_VERSION >= 2
#define USE_CONVERTER
#endif

#ifdef USE_CONVERTER
#include "Converter.h"
#endif

#ifdef _MSC_VER
// Constant conditional expressions are because of GTK+ headers
#pragma warning(disable: 4127)
// Ignore unreferenced local functions in GTK+ headers
#pragma warning(disable: 4505)
#endif

#if GTK_CHECK_VERSION(2,6,0)
#define USE_GTK_CLIPBOARD
#endif

#if GLIB_MAJOR_VERSION < 2
#define OBJECT_CLASS GtkObjectClass
#else
#define OBJECT_CLASS GObjectClass
#endif

extern char *UTF8FromLatin1(const char *s, int &len);

class ScintillaGTK : public ScintillaBase {
	_ScintillaObject *sci;
	Window wText;
	Window scrollbarv;
	Window scrollbarh;
	GtkObject *adjustmentv;
	GtkObject *adjustmenth;
	int scrollBarWidth;
	int scrollBarHeight;

	// Because clipboard access is asynchronous, copyText is created by Copy
#ifndef USE_GTK_CLIPBOARD
	SelectionText copyText;
#endif

	SelectionText primary;

	GdkEventButton evbtn;
	bool capturedMouse;
	bool dragWasDropped;
	int lastKey;

	GtkWidgetClass *parentClass;

	static GdkAtom atomClipboard;
	static GdkAtom atomUTF8;
	static GdkAtom atomString;
	static GdkAtom atomUriList;
	static GdkAtom atomDROPFILES_DND;
	GdkAtom atomSought;

#if PLAT_GTK_WIN32
	CLIPFORMAT cfColumnSelect;
#endif

#ifdef INTERNATIONAL_INPUT
#if GTK_MAJOR_VERSION < 2
	// Input context used for supporting internationalized key entry
	GdkIC *ic;
	GdkICAttr *ic_attr;
#else
	Window wPreedit;
	Window wPreeditDraw;
	GtkIMContext *im_context;
#endif
#endif

	// Wheel mouse support
	unsigned int linesPerScroll;
	GTimeVal lastWheelMouseTime;
	gint lastWheelMouseDirection;
	gint wheelMouseIntensity;

	GdkRegion *rgnUpdate;

	// Private so ScintillaGTK objects can not be copied
	ScintillaGTK(const ScintillaGTK &) : ScintillaBase() {}
	ScintillaGTK &operator=(const ScintillaGTK &) { return * this; }

public:
	ScintillaGTK(_ScintillaObject *sci_);
	virtual ~ScintillaGTK();
	static void ClassInit(OBJECT_CLASS* object_class, GtkWidgetClass *widget_class, GtkContainerClass *container_class);
private:
	virtual void Initialise();
	virtual void Finalise();
	virtual void DisplayCursor(Window::Cursor c);
	virtual bool DragThreshold(Point ptStart, Point ptNow);
	virtual void StartDrag();
	int TargetAsUTF8(char *text);
	int EncodedFromUTF8(char *utf8, char *encoded);
	virtual bool ValidCodePage(int codePage) const;
public: 	// Public for scintilla_send_message
	virtual sptr_t WndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
private:
	virtual sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
	virtual void SetTicking(bool on);
	virtual bool SetIdle(bool on);
	virtual void SetMouseCapture(bool on);
	virtual bool HaveMouseCapture();
	virtual bool PaintContains(PRectangle rc);
	void FullPaint();
	virtual PRectangle GetClientRectangle();
	void SyncPaint(PRectangle rc);
	virtual void ScrollText(int linesToMove);
	virtual void SetVerticalScrollPos();
	virtual void SetHorizontalScrollPos();
	virtual bool ModifyScrollBars(int nMax, int nPage);
	void ReconfigureScrollBars();
	virtual void NotifyChange();
	virtual void NotifyFocus(bool focus);
	virtual void NotifyParent(SCNotification scn);
	void NotifyKey(int key, int modifiers);
	void NotifyURIDropped(const char *list);
	const char *CharacterSetID() const;
	virtual int KeyDefault(int key, int modifiers);
	virtual void CopyToClipboard(const SelectionText &selectedText);
	virtual void Copy();
	virtual void Paste();
	virtual void CreateCallTipWindow(PRectangle rc);
	virtual void AddToPopUp(const char *label, int cmd = 0, bool enabled = true);
	bool OwnPrimarySelection();
	virtual void ClaimSelection();
	void GetGtkSelectionText(GtkSelectionData *selectionData, SelectionText &selText);
	void ReceivedSelection(GtkSelectionData *selection_data);
	void ReceivedDrop(GtkSelectionData *selection_data);
	static void GetSelection(GtkSelectionData *selection_data, guint info, SelectionText *selected);
#ifdef USE_GTK_CLIPBOARD
	void StoreOnClipboard(SelectionText *clipText);
	static void ClipboardGetSelection(GtkClipboard* clip, GtkSelectionData *selection_data, guint info, void *data);
	static void ClipboardClearSelection(GtkClipboard* clip, void *data);
#endif

	void UnclaimSelection(GdkEventSelection *selection_event);
	void Resize(int width, int height);

	// Callback functions
	void RealizeThis(GtkWidget *widget);
	static void Realize(GtkWidget *widget);
	void UnRealizeThis(GtkWidget *widget);
	static void UnRealize(GtkWidget *widget);
	void MapThis();
	static void Map(GtkWidget *widget);
	void UnMapThis();
	static void UnMap(GtkWidget *widget);
	static gint CursorMoved(GtkWidget *widget, int xoffset, int yoffset, ScintillaGTK *sciThis);
	static gint FocusIn(GtkWidget *widget, GdkEventFocus *event);
	static gint FocusOut(GtkWidget *widget, GdkEventFocus *event);
	static void SizeRequest(GtkWidget *widget, GtkRequisition *requisition);
	static void SizeAllocate(GtkWidget *widget, GtkAllocation *allocation);
	gint Expose(GtkWidget *widget, GdkEventExpose *ose);
	static gint ExposeMain(GtkWidget *widget, GdkEventExpose *ose);
	static void Draw(GtkWidget *widget, GdkRectangle *area);
	void ForAll(GtkCallback callback, gpointer callback_data);
	static void MainForAll(GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data);

	static void ScrollSignal(GtkAdjustment *adj, ScintillaGTK *sciThis);
	static void ScrollHSignal(GtkAdjustment *adj, ScintillaGTK *sciThis);
	gint PressThis(GdkEventButton *event);
	static gint Press(GtkWidget *widget, GdkEventButton *event);
	static gint MouseRelease(GtkWidget *widget, GdkEventButton *event);
#if PLAT_GTK_WIN32 || (GTK_MAJOR_VERSION >= 2)
	static gint ScrollEvent(GtkWidget *widget, GdkEventScroll *event);
#endif
	static gint Motion(GtkWidget *widget, GdkEventMotion *event);
	gboolean KeyThis(GdkEventKey *event);
	static gboolean KeyPress(GtkWidget *widget, GdkEventKey *event);
	static gboolean KeyRelease(GtkWidget *widget, GdkEventKey *event);
#if GTK_MAJOR_VERSION >= 2
	static gboolean ExposePreedit(GtkWidget *widget, GdkEventExpose *ose, ScintillaGTK *sciThis);
	gboolean ExposePreeditThis(GtkWidget *widget, GdkEventExpose *ose);
	static void Commit(GtkIMContext *context, char *str, ScintillaGTK *sciThis);
	void CommitThis(char *str);
	static void PreeditChanged(GtkIMContext *context, ScintillaGTK *sciThis);
	void PreeditChangedThis();
#endif
	static gint StyleSetText(GtkWidget *widget, GtkStyle *previous, void*);
	static gint RealizeText(GtkWidget *widget, void*);
#if GLIB_MAJOR_VERSION < 2
	static void Destroy(GtkObject *object);
#else
	static void Destroy(GObject *object);
#endif
	static void SelectionReceived(GtkWidget *widget, GtkSelectionData *selection_data,
	                              guint time);
	static void SelectionGet(GtkWidget *widget, GtkSelectionData *selection_data,
	                         guint info, guint time);
	static gint SelectionClear(GtkWidget *widget, GdkEventSelection *selection_event);
#if GTK_MAJOR_VERSION < 2
	static gint SelectionNotify(GtkWidget *widget, GdkEventSelection *selection_event);
#endif
	static void DragBegin(GtkWidget *widget, GdkDragContext *context);
	gboolean DragMotionThis(GdkDragContext *context, gint x, gint y, guint dragtime);
	static gboolean DragMotion(GtkWidget *widget, GdkDragContext *context,
	                           gint x, gint y, guint dragtime);
	static void DragLeave(GtkWidget *widget, GdkDragContext *context,
	                      guint time);
	static void DragEnd(GtkWidget *widget, GdkDragContext *context);
	static gboolean Drop(GtkWidget *widget, GdkDragContext *context,
	                     gint x, gint y, guint time);
	static void DragDataReceived(GtkWidget *widget, GdkDragContext *context,
	                             gint x, gint y, GtkSelectionData *selection_data, guint info, guint time);
	static void DragDataGet(GtkWidget *widget, GdkDragContext *context,
	                        GtkSelectionData *selection_data, guint info, guint time);
	static gint TimeOut(ScintillaGTK *sciThis);
	static gint IdleCallback(ScintillaGTK *sciThis);
	static void PopUpCB(ScintillaGTK *sciThis, guint action, GtkWidget *widget);

	gint ExposeTextThis(GtkWidget *widget, GdkEventExpose *ose);
	static gint ExposeText(GtkWidget *widget, GdkEventExpose *ose, ScintillaGTK *sciThis);

	static gint ExposeCT(GtkWidget *widget, GdkEventExpose *ose, CallTip *ct);
	static gint PressCT(GtkWidget *widget, GdkEventButton *event, ScintillaGTK *sciThis);

	static sptr_t DirectFunction(ScintillaGTK *sciThis,
	                             unsigned int iMessage, uptr_t wParam, sptr_t lParam);
};

enum {
    COMMAND_SIGNAL,
    NOTIFY_SIGNAL,
    LAST_SIGNAL
};

static gint scintilla_signals[LAST_SIGNAL] = { 0 };
#if GLIB_MAJOR_VERSION < 2
static GtkWidgetClass *parent_class = NULL;
#endif

enum {
    TARGET_STRING,
    TARGET_TEXT,
    TARGET_COMPOUND_TEXT,
    TARGET_UTF8_STRING,
    TARGET_URI
};

GdkAtom ScintillaGTK::atomClipboard = 0;
GdkAtom ScintillaGTK::atomUTF8 = 0;
GdkAtom ScintillaGTK::atomString = 0;
GdkAtom ScintillaGTK::atomUriList = 0;
GdkAtom ScintillaGTK::atomDROPFILES_DND = 0;

static const GtkTargetEntry clipboardCopyTargets[] = {
	{ "UTF8_STRING", 0, TARGET_UTF8_STRING },
	{ "STRING", 0, TARGET_STRING },
};
static const gint nClipboardCopyTargets = sizeof(clipboardCopyTargets) / sizeof(clipboardCopyTargets[0]);

static const GtkTargetEntry clipboardPasteTargets[] = {
	{ "text/uri-list", 0, TARGET_URI },
	{ "UTF8_STRING", 0, TARGET_UTF8_STRING },
	{ "STRING", 0, TARGET_STRING },
};
static const gint nClipboardPasteTargets = sizeof(clipboardPasteTargets) / sizeof(clipboardPasteTargets[0]);

static GtkWidget *PWidget(Window &w) {
	return reinterpret_cast<GtkWidget *>(w.GetID());
}

static ScintillaGTK *ScintillaFromWidget(GtkWidget *widget) {
	ScintillaObject *scio = reinterpret_cast<ScintillaObject *>(widget);
	return reinterpret_cast<ScintillaGTK *>(scio->pscin);
}

ScintillaGTK::ScintillaGTK(_ScintillaObject *sci_) :
		adjustmentv(0), adjustmenth(0),
		scrollBarWidth(30), scrollBarHeight(30),
		capturedMouse(false), dragWasDropped(false),
		lastKey(0), parentClass(0),
#ifdef INTERNATIONAL_INPUT
#if GTK_MAJOR_VERSION < 2
		ic(NULL),
		ic_attr(NULL),
#else
		im_context(NULL),
#endif
#endif
		lastWheelMouseDirection(0),
		wheelMouseIntensity(0),
		rgnUpdate(0) {
	sci = sci_;
	wMain = GTK_WIDGET(sci);

#if PLAT_GTK_WIN32
 	// There does not seem to be a real standard for indicating that the clipboard
	// contains a rectangular selection, so copy Developer Studio.
	cfColumnSelect = static_cast<CLIPFORMAT>(
		::RegisterClipboardFormat("MSDEVColumnSelect"));

  	// Get intellimouse parameters when running on win32; otherwise use
	// reasonable default
#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &linesPerScroll, 0);
#else
	linesPerScroll = 4;
#endif
	lastWheelMouseTime.tv_sec = 0;
	lastWheelMouseTime.tv_usec = 0;

	Initialise();
}

ScintillaGTK::~ScintillaGTK() {
}

void ScintillaGTK::RealizeThis(GtkWidget *widget) {
	//Platform::DebugPrintf("ScintillaGTK::realize this\n");
	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);
	GdkWindowAttr attrs;
	attrs.window_type = GDK_WINDOW_CHILD;
	attrs.x = widget->allocation.x;
	attrs.y = widget->allocation.y;
	attrs.width = widget->allocation.width;
	attrs.height = widget->allocation.height;
	attrs.wclass = GDK_INPUT_OUTPUT;
	attrs.visual = gtk_widget_get_visual(widget);
	attrs.colormap = gtk_widget_get_colormap(widget);
	attrs.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;
	GdkCursor *cursor = gdk_cursor_new(GDK_XTERM);
	attrs.cursor = cursor;
	widget->window = gdk_window_new(gtk_widget_get_parent_window(widget), &attrs,
	                                GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP | GDK_WA_CURSOR);
	gdk_window_set_user_data(widget->window, widget);
	gdk_window_set_background(widget->window, &widget->style->bg[GTK_STATE_NORMAL]);
	gdk_window_show(widget->window);
	gdk_cursor_destroy(cursor);
	widget->style = gtk_style_attach(widget->style, widget->window);
#ifdef INTERNATIONAL_INPUT
#if GTK_MAJOR_VERSION < 2
	if (gdk_im_ready() && (ic_attr = gdk_ic_attr_new()) != NULL) {
		gint width, height;
		GdkColormap *colormap;
		GdkEventMask mask;
		GdkICAttr *attr = ic_attr;
		GdkICAttributesType attrmask = GDK_IC_ALL_REQ;
		GdkIMStyle style;
		GdkIMStyle supported_style = (GdkIMStyle) (GDK_IM_PREEDIT_NONE |
		                             GDK_IM_PREEDIT_NOTHING |
		                             GDK_IM_PREEDIT_POSITION |
		                             GDK_IM_STATUS_NONE |
		                             GDK_IM_STATUS_NOTHING);

		if (widget->style && widget->style->font->type != GDK_FONT_FONTSET)
			supported_style = (GdkIMStyle) ((int) supported_style & ~GDK_IM_PREEDIT_POSITION);

		attr->style = style = gdk_im_decide_style(supported_style);
		attr->client_window = widget->window;

		if ((colormap = gtk_widget_get_colormap (widget)) != gtk_widget_get_default_colormap ()) {
			attrmask = (GdkICAttributesType) ((int) attrmask | GDK_IC_PREEDIT_COLORMAP);
			attr->preedit_colormap = colormap;
		}

		switch (style & GDK_IM_PREEDIT_MASK) {
		case GDK_IM_PREEDIT_POSITION:
			if (widget->style && widget->style->font->type != GDK_FONT_FONTSET)	{
				g_warning("over-the-spot style requires fontset");
				break;
			}

			attrmask = (GdkICAttributesType) ((int) attrmask | GDK_IC_PREEDIT_POSITION_REQ);
			gdk_window_get_size(widget->window, &width, &height);
			attr->spot_location.x = 0;
			attr->spot_location.y = height;
			attr->preedit_area.x = 0;
			attr->preedit_area.y = 0;
			attr->preedit_area.width = width;
			attr->preedit_area.height = height;
			attr->preedit_fontset = widget->style->font;

			break;
		}
		ic = gdk_ic_new(attr, attrmask);

		if (ic == NULL) {
			g_warning("Can't create input context.");
		} else {
			mask = gdk_window_get_events(widget->window);
			mask = (GdkEventMask) ((int) mask | gdk_ic_get_events(ic));
			gdk_window_set_events(widget->window, mask);

			if (GTK_WIDGET_HAS_FOCUS(widget))
				gdk_im_begin(ic, widget->window);
		}
	}
#else
	wPreedit = gtk_window_new(GTK_WINDOW_POPUP);
	wPreeditDraw = gtk_drawing_area_new();
	GtkWidget *predrw = PWidget(wPreeditDraw);	// No code inside the G_OBJECT macro
	g_signal_connect(G_OBJECT(predrw), "expose_event",
			   G_CALLBACK(ExposePreedit), this);
	gtk_container_add(GTK_CONTAINER(PWidget(wPreedit)), predrw);
	gtk_widget_realize(PWidget(wPreedit));
	gtk_widget_realize(predrw);
	gtk_widget_show(predrw);

	im_context = gtk_im_multicontext_new();
	g_signal_connect(G_OBJECT(im_context), "commit",
			 G_CALLBACK(Commit), this);
	g_signal_connect(G_OBJECT(im_context), "preedit_changed",
			 G_CALLBACK(PreeditChanged), this);
	gtk_im_context_set_client_window(im_context, widget->window);
#endif
#endif
	GtkWidget *widtxt = PWidget(wText);	//	// No code inside the G_OBJECT macro
#if GLIB_MAJOR_VERSION < 2
	gtk_signal_connect_after(GTK_OBJECT(widtxt), "style_set",
				 GtkSignalFunc(ScintillaGTK::StyleSetText), NULL);
	gtk_signal_connect_after(GTK_OBJECT(widtxt), "realize",
				 GtkSignalFunc(ScintillaGTK::RealizeText), NULL);
#else
	g_signal_connect_after(G_OBJECT(widtxt), "style_set",
				 G_CALLBACK(ScintillaGTK::StyleSetText), NULL);
	g_signal_connect_after(G_OBJECT(widtxt), "realize",
				 G_CALLBACK(ScintillaGTK::RealizeText), NULL);
#endif
	gtk_widget_realize(widtxt);
	gtk_widget_realize(PWidget(scrollbarv));
	gtk_widget_realize(PWidget(scrollbarh));
}

void ScintillaGTK::Realize(GtkWidget *widget) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->RealizeThis(widget);
}

void ScintillaGTK::UnRealizeThis(GtkWidget *widget) {
	if (GTK_WIDGET_MAPPED(widget)) {
		gtk_widget_unmap(widget);
	}
	GTK_WIDGET_UNSET_FLAGS(widget, GTK_REALIZED);
	gtk_widget_unrealize(PWidget(wText));
	gtk_widget_unrealize(PWidget(scrollbarv));
	gtk_widget_unrealize(PWidget(scrollbarh));
#ifdef INTERNATIONAL_INPUT
#if GTK_MAJOR_VERSION < 2
	if (ic) {
		gdk_ic_destroy(ic);
		ic = NULL;
	}
	if (ic_attr) {
		gdk_ic_attr_destroy(ic_attr);
		ic_attr = NULL;
	}
#else
	gtk_widget_unrealize(PWidget(wPreedit));
	gtk_widget_unrealize(PWidget(wPreeditDraw));
	g_object_unref(im_context);
	im_context = NULL;
#endif
#endif
	if (GTK_WIDGET_CLASS(parentClass)->unrealize)
		GTK_WIDGET_CLASS(parentClass)->unrealize(widget);

	Finalise();
}

void ScintillaGTK::UnRealize(GtkWidget *widget) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->UnRealizeThis(widget);
}

static void MapWidget(GtkWidget *widget) {
	if (widget &&
	        GTK_WIDGET_VISIBLE(widget) &&
	        !GTK_WIDGET_MAPPED(widget)) {
		gtk_widget_map(widget);
	}
}

void ScintillaGTK::MapThis() {
	//Platform::DebugPrintf("ScintillaGTK::map this\n");
	GTK_WIDGET_SET_FLAGS(PWidget(wMain), GTK_MAPPED);
	MapWidget(PWidget(wText));
	MapWidget(PWidget(scrollbarh));
	MapWidget(PWidget(scrollbarv));
	wMain.SetCursor(Window::cursorArrow);
	scrollbarv.SetCursor(Window::cursorArrow);
	scrollbarh.SetCursor(Window::cursorArrow);
	ChangeSize();
	gdk_window_show(PWidget(wMain)->window);
}

void ScintillaGTK::Map(GtkWidget *widget) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->MapThis();
}

void ScintillaGTK::UnMapThis() {
	//Platform::DebugPrintf("ScintillaGTK::unmap this\n");
	GTK_WIDGET_UNSET_FLAGS(PWidget(wMain), GTK_MAPPED);
	DropGraphics();
	gdk_window_hide(PWidget(wMain)->window);
	gtk_widget_unmap(PWidget(wText));
	gtk_widget_unmap(PWidget(scrollbarh));
	gtk_widget_unmap(PWidget(scrollbarv));
}

void ScintillaGTK::UnMap(GtkWidget *widget) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->UnMapThis();
}

void ScintillaGTK::ForAll(GtkCallback callback, gpointer callback_data) {
	(*callback) (PWidget(wText), callback_data);
	(*callback) (PWidget(scrollbarv), callback_data);
	(*callback) (PWidget(scrollbarh), callback_data);
}

void ScintillaGTK::MainForAll(GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data) {
	ScintillaGTK *sciThis = ScintillaFromWidget((GtkWidget *)container);

	if (callback != NULL && include_internals) {
		sciThis->ForAll(callback, callback_data);
	}
}

#ifdef INTERNATIONAL_INPUT
#if GTK_MAJOR_VERSION < 2
gint ScintillaGTK::CursorMoved(GtkWidget *widget, int xoffset, int yoffset, ScintillaGTK *sciThis) {
	if (GTK_WIDGET_HAS_FOCUS(widget) && gdk_im_ready() && sciThis->ic &&
	        (gdk_ic_get_style(sciThis->ic) & GDK_IM_PREEDIT_POSITION)) {
		sciThis->ic_attr->spot_location.x = xoffset;
		sciThis->ic_attr->spot_location.y = yoffset;
		gdk_ic_set_attr(sciThis->ic, sciThis->ic_attr, GDK_IC_SPOT_LOCATION);
	}
	return FALSE;
}
#else
gint ScintillaGTK::CursorMoved(GtkWidget *, int xoffset, int yoffset, ScintillaGTK *sciThis) {
	GdkRectangle area;
	area.x = xoffset;
	area.y = yoffset;
	area.width = 1;
	area.height = 1;
	gtk_im_context_set_cursor_location(sciThis->im_context, &area);
	return FALSE;
}
#endif
#else
gint ScintillaGTK::CursorMoved(GtkWidget *, int, int, ScintillaGTK *) {
	return FALSE;
}
#endif

gint ScintillaGTK::FocusIn(GtkWidget *widget, GdkEventFocus * /*event*/) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("ScintillaGTK::focus in %x\n", sciThis);
	GTK_WIDGET_SET_FLAGS(widget, GTK_HAS_FOCUS);
	sciThis->SetFocusState(true);

#ifdef INTERNATIONAL_INPUT
#if GTK_MAJOR_VERSION < 2
	if (sciThis->ic)
		gdk_im_begin(sciThis->ic, widget->window);
#else
	if (sciThis->im_context != NULL) {
		gchar *str = NULL;
		gint cursor_pos;

		gtk_im_context_get_preedit_string(sciThis->im_context, &str, NULL, &cursor_pos);
		if (PWidget(sciThis->wPreedit) != NULL) {
			if (strlen(str) > 0) {
				gtk_widget_show(PWidget(sciThis->wPreedit));
			} else {
				gtk_widget_hide(PWidget(sciThis->wPreedit));
			}
		}
		g_free(str);
		gtk_im_context_focus_in(sciThis->im_context);
	}
#endif
#endif

	return FALSE;
}

gint ScintillaGTK::FocusOut(GtkWidget *widget, GdkEventFocus * /*event*/) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("ScintillaGTK::focus out %x\n", sciThis);
	GTK_WIDGET_UNSET_FLAGS(widget, GTK_HAS_FOCUS);
	sciThis->SetFocusState(false);

#ifdef INTERNATIONAL_INPUT
#if GTK_MAJOR_VERSION < 2
	gdk_im_end();
#else
	if (PWidget(sciThis->wPreedit) != NULL)
		gtk_widget_hide(PWidget(sciThis->wPreedit));
	if (sciThis->im_context != NULL)
		gtk_im_context_focus_out(sciThis->im_context);
#endif
#endif

	return FALSE;
}

void ScintillaGTK::SizeRequest(GtkWidget *widget, GtkRequisition *requisition) {
	requisition->width = 600;
	requisition->height = gdk_screen_height();
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	GtkRequisition child_requisition;
	gtk_widget_size_request(PWidget(sciThis->scrollbarh), &child_requisition);
	gtk_widget_size_request(PWidget(sciThis->scrollbarv), &child_requisition);
}

void ScintillaGTK::SizeAllocate(GtkWidget *widget, GtkAllocation *allocation) {
	widget->allocation = *allocation;
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	if (GTK_WIDGET_REALIZED(widget))
		gdk_window_move_resize(widget->window,
		                       widget->allocation.x,
		                       widget->allocation.y,
		                       widget->allocation.width,
		                       widget->allocation.height);

	sciThis->Resize(allocation->width, allocation->height);

#ifdef INTERNATIONAL_INPUT
#if GTK_MAJOR_VERSION < 2
	if (sciThis->ic && (gdk_ic_get_style(sciThis->ic) & GDK_IM_PREEDIT_POSITION)) {
		gint width, height;

		gdk_window_get_size(widget->window, &width, &height);
		sciThis->ic_attr->preedit_area.width = width;
		sciThis->ic_attr->preedit_area.height = height;

		gdk_ic_set_attr(sciThis->ic, sciThis->ic_attr, GDK_IC_PREEDIT_AREA);
	}
#endif
#endif
}

void ScintillaGTK::Initialise() {
	//Platform::DebugPrintf("ScintillaGTK::Initialise\n");
	parentClass = reinterpret_cast<GtkWidgetClass *>(
	                  gtk_type_class(gtk_container_get_type()));

	GTK_WIDGET_SET_FLAGS(PWidget(wMain), GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(GTK_WIDGET(PWidget(wMain)), GTK_SENSITIVE);
	gtk_widget_set_events(PWidget(wMain),
	                      GDK_EXPOSURE_MASK
	                      | GDK_STRUCTURE_MASK
	                      | GDK_KEY_PRESS_MASK
	                      | GDK_KEY_RELEASE_MASK
	                      | GDK_FOCUS_CHANGE_MASK
	                      | GDK_LEAVE_NOTIFY_MASK
	                      | GDK_BUTTON_PRESS_MASK
	                      | GDK_BUTTON_RELEASE_MASK
	                      | GDK_POINTER_MOTION_MASK
	                      | GDK_POINTER_MOTION_HINT_MASK);

	wText = gtk_drawing_area_new();
	gtk_widget_set_parent(PWidget(wText), PWidget(wMain));
	GtkWidget *widtxt = PWidget(wText);	// No code inside the G_OBJECT macro
	gtk_widget_show(widtxt);
#if GLIB_MAJOR_VERSION < 2
	gtk_signal_connect(GTK_OBJECT(widtxt), "expose_event",
			   GtkSignalFunc(ScintillaGTK::ExposeText), this);
#else
	g_signal_connect(G_OBJECT(widtxt), "expose_event",
			   G_CALLBACK(ScintillaGTK::ExposeText), this);
#endif
	gtk_widget_set_events(widtxt, GDK_EXPOSURE_MASK);
#if GTK_MAJOR_VERSION >= 2
	// Avoid background drawing flash
	gtk_widget_set_double_buffered(widtxt, FALSE);
#endif
	gtk_drawing_area_size(GTK_DRAWING_AREA(widtxt),
	                      100,100);
	adjustmentv = gtk_adjustment_new(0.0, 0.0, 201.0, 1.0, 20.0, 20.0);
	scrollbarv = gtk_vscrollbar_new(GTK_ADJUSTMENT(adjustmentv));
	GTK_WIDGET_UNSET_FLAGS(PWidget(scrollbarv), GTK_CAN_FOCUS);
#if GLIB_MAJOR_VERSION < 2
	gtk_signal_connect(adjustmentv, "value_changed",
			   GtkSignalFunc(ScrollSignal), this);
#else
	g_signal_connect(G_OBJECT(adjustmentv), "value_changed",
			   G_CALLBACK(ScrollSignal), this);
#endif
	gtk_widget_set_parent(PWidget(scrollbarv), PWidget(wMain));
	gtk_widget_show(PWidget(scrollbarv));

	adjustmenth = gtk_adjustment_new(0.0, 0.0, 101.0, 1.0, 20.0, 20.0);
	scrollbarh = gtk_hscrollbar_new(GTK_ADJUSTMENT(adjustmenth));
	GTK_WIDGET_UNSET_FLAGS(PWidget(scrollbarh), GTK_CAN_FOCUS);
#if GLIB_MAJOR_VERSION < 2
	gtk_signal_connect(adjustmenth, "value_changed",
			   GtkSignalFunc(ScrollHSignal), this);
#else
	g_signal_connect(G_OBJECT(adjustmenth), "value_changed",
			   G_CALLBACK(ScrollHSignal), this);
#endif
	gtk_widget_set_parent(PWidget(scrollbarh), PWidget(wMain));
	gtk_widget_show(PWidget(scrollbarh));

	gtk_widget_grab_focus(PWidget(wMain));

	gtk_selection_add_targets(GTK_WIDGET(PWidget(wMain)), GDK_SELECTION_PRIMARY,
	                          clipboardCopyTargets, nClipboardCopyTargets);

#ifndef USE_GTK_CLIPBOARD
	gtk_selection_add_targets(GTK_WIDGET(PWidget(wMain)), atomClipboard,
	                          clipboardPasteTargets, nClipboardPasteTargets);
#endif

	gtk_drag_dest_set(GTK_WIDGET(PWidget(wMain)),
	                  GTK_DEST_DEFAULT_ALL, clipboardPasteTargets, nClipboardPasteTargets,
	                  static_cast<GdkDragAction>(GDK_ACTION_COPY | GDK_ACTION_MOVE));

#if GLIB_MAJOR_VERSION >= 2
	// Set caret period based on GTK settings
	gboolean blinkOn = false;
	if (g_object_class_find_property(G_OBJECT_GET_CLASS(
			G_OBJECT(gtk_settings_get_default())), "gtk-cursor-blink")) {
		g_object_get(G_OBJECT(
			gtk_settings_get_default()), "gtk-cursor-blink", &blinkOn, NULL);
	}
	if (blinkOn &&
		g_object_class_find_property(G_OBJECT_GET_CLASS(
			G_OBJECT(gtk_settings_get_default())), "gtk-cursor-blink-time")) {
		gint value;
		g_object_get(G_OBJECT(
			gtk_settings_get_default()), "gtk-cursor-blink-time", &value, NULL);
		caret.period = gint(value / 1.75);
	} else {
		caret.period = 0;
	}
#endif

	SetTicking(true);
}

void ScintillaGTK::Finalise() {
	SetTicking(false);
	ScintillaBase::Finalise();
}

void ScintillaGTK::DisplayCursor(Window::Cursor c) {
	if (cursorMode == SC_CURSORNORMAL)
		wText.SetCursor(c);
	else
		wText.SetCursor(static_cast<Window::Cursor>(cursorMode));
}

bool ScintillaGTK::DragThreshold(Point ptStart, Point ptNow) {
#if GTK_MAJOR_VERSION < 2
	return Editor::DragThreshold(ptStart, ptNow);
#else
	return gtk_drag_check_threshold(GTK_WIDGET(PWidget(wMain)),
		ptStart.x, ptStart.y, ptNow.x, ptNow.y);
#endif
}

void ScintillaGTK::StartDrag() {
	dragWasDropped = false;
	inDragDrop = ddDragging;
	GtkTargetList *tl = gtk_target_list_new(clipboardCopyTargets, nClipboardCopyTargets);
	gtk_drag_begin(GTK_WIDGET(PWidget(wMain)),
	               tl,
	               static_cast<GdkDragAction>(GDK_ACTION_COPY | GDK_ACTION_MOVE),
	               evbtn.button,
	               reinterpret_cast<GdkEvent *>(&evbtn));
}

#ifdef USE_CONVERTER
static char *ConvertText(int *lenResult, char *s, size_t len, const char *charSetDest,
	const char *charSetSource, bool transliterations) {
	*lenResult = 0;
	char *destForm = 0;
	Converter conv(charSetDest, charSetSource, transliterations);
	if (conv) {
		destForm = new char[len*3+1];
		char *pin = s;
		size_t inLeft = len;
		char *pout = destForm;
		size_t outLeft = len*3+1;
		size_t conversions = conv.Convert(&pin, &inLeft, &pout, &outLeft);
		if (conversions == ((size_t)(-1))) {
fprintf(stderr, "iconv %s->%s failed for %s\n", charSetSource, charSetDest, static_cast<char *>(s));
			delete []destForm;
			destForm = 0;
		} else {
//fprintf(stderr, "iconv OK %s %d\n", destForm, pout - destForm);
			*pout = '\0';
			*lenResult = pout - destForm;
		}
	} else {
fprintf(stderr, "Can not iconv %s %s\n", charSetDest, charSetSource);
	}
	if (!destForm) {
		destForm = new char[1];
		destForm[0] = '\0';
		*lenResult = 0;
	}
	return destForm;
}
#endif

// Returns the target converted to UTF8.
// Return the length in bytes.
int ScintillaGTK::TargetAsUTF8(char *text) {
	int targetLength = targetEnd - targetStart;
	if (IsUnicodeMode()) {
		if (text) {
			pdoc->GetCharRange(text, targetStart, targetLength);
		}
	} else {
		// Need to convert
#ifdef USE_CONVERTER
		const char *charSetBuffer = CharacterSetID();
		if (*charSetBuffer) {
//~ fprintf(stderr, "AsUTF8 %s %d  %0d-%0d\n", charSetBuffer, targetLength, targetStart, targetEnd);
			char *s = new char[targetLength];
			if (s) {
				pdoc->GetCharRange(s, targetStart, targetLength);
//~ fprintf(stderr, "    \"%s\"\n", s);
				if (text) {
					char *tmputf = ConvertText(&targetLength, s, targetLength, "UTF-8", charSetBuffer, false);
					memcpy(text, tmputf, targetLength);
					delete []tmputf;
//~ fprintf(stderr, "    \"%s\"\n", text);
				}
				delete []s;
			}
		} else {
			if (text) {
				pdoc->GetCharRange(text, targetStart, targetLength);
			}
		}
#else
		// Fail
		return 0;
#endif
	}
//~ fprintf(stderr, "Length = %d bytes\n", targetLength);
	return targetLength;
}

// Translates a nul terminated UTF8 string into the document encoding.
// Return the length of the result in bytes.
int ScintillaGTK::EncodedFromUTF8(char *utf8, char *encoded) {
	int inputLength = (lengthForEncode >= 0) ? lengthForEncode : strlen(utf8);
	if (IsUnicodeMode()) {
		if (encoded) {
			memcpy(encoded, utf8, inputLength);
		}
		return inputLength;
	} else {
		// Need to convert
#ifdef USE_CONVERTER
		const char *charSetBuffer = CharacterSetID();
		if (*charSetBuffer) {
//~ fprintf(stderr, "Encode %s %d\n", charSetBuffer, inputLength);
			int outLength = 0;
			char *tmpEncoded = ConvertText(&outLength, utf8, inputLength, charSetBuffer, "UTF-8", true);
			if (tmpEncoded) {
//~ fprintf(stderr, "    \"%s\"\n", tmpEncoded);
				if (encoded) {
					memcpy(encoded, tmpEncoded, outLength);
				}
				delete []tmpEncoded;
			}
			return outLength;
		} else {
			if (encoded) {
				memcpy(encoded, utf8, inputLength);
			}
			return inputLength;
		}
#endif
	}
	// Fail
	return 0;
}

bool ScintillaGTK::ValidCodePage(int codePage) const {
	return codePage == 0 || codePage == SC_CP_UTF8 || codePage == SC_CP_DBCS;
}

sptr_t ScintillaGTK::WndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	switch (iMessage) {

	case SCI_GRABFOCUS:
		gtk_widget_grab_focus(PWidget(wMain));
		break;

	case SCI_GETDIRECTFUNCTION:
		return reinterpret_cast<sptr_t>(DirectFunction);

	case SCI_GETDIRECTPOINTER:
		return reinterpret_cast<sptr_t>(this);

#ifdef SCI_LEXER
	case SCI_LOADLEXERLIBRARY:
		LexerManager::GetInstance()->Load(reinterpret_cast<const char*>(wParam));
		break;
#endif
	case SCI_TARGETASUTF8:
		return TargetAsUTF8(reinterpret_cast<char*>(lParam));

	case SCI_ENCODEDFROMUTF8:
		return EncodedFromUTF8(reinterpret_cast<char*>(wParam),
			reinterpret_cast<char*>(lParam));

	default:
		return ScintillaBase::WndProc(iMessage, wParam, lParam);
	}
	return 0l;
}

sptr_t ScintillaGTK::DefWndProc(unsigned int, uptr_t, sptr_t) {
	return 0;
}

void ScintillaGTK::SetTicking(bool on) {
	if (timer.ticking != on) {
		timer.ticking = on;
		if (timer.ticking) {
			timer.tickerID = reinterpret_cast<TickerID>(gtk_timeout_add(timer.tickSize, (GtkFunction)TimeOut, this));
		} else {
			gtk_timeout_remove(GPOINTER_TO_UINT(timer.tickerID));
		}
	}
	timer.ticksToWait = caret.period;
}

bool ScintillaGTK::SetIdle(bool on) {
	if (on) {
		// Start idler, if it's not running.
		if (idler.state == false) {
			idler.state = true;
			idler.idlerID = reinterpret_cast<IdlerID>
				(gtk_idle_add((GtkFunction)IdleCallback, this));
		}
	} else {
		// Stop idler, if it's running
		if (idler.state == true) {
			idler.state = false;
			gtk_idle_remove(GPOINTER_TO_UINT(idler.idlerID));
		}
	}
	return true;
}

void ScintillaGTK::SetMouseCapture(bool on) {
	if (mouseDownCaptures) {
		if (on) {
			gtk_grab_add(GTK_WIDGET(PWidget(wMain)));
		} else {
			gtk_grab_remove(GTK_WIDGET(PWidget(wMain)));
		}
	}
	capturedMouse = on;
}

bool ScintillaGTK::HaveMouseCapture() {
	return capturedMouse;
}

bool ScintillaGTK::PaintContains(PRectangle rc) {
	bool contains = true;
	if (paintState == painting) {
		if (!rcPaint.Contains(rc)) {
			contains = false;
		} else if (rgnUpdate) {
			GdkRectangle grc = {rc.left, rc.top,
				rc.right - rc.left, rc.bottom - rc.top};
			if (gdk_region_rect_in(rgnUpdate, &grc) != GDK_OVERLAP_RECTANGLE_IN) {
				contains = false;
			}
		}
	}
	return contains;
}

// Redraw all of text area. This paint will not be abandoned.
void ScintillaGTK::FullPaint() {
#if GTK_MAJOR_VERSION < 2
	paintState = painting;
	rcPaint = GetClientRectangle();
	//Platform::DebugPrintf("ScintillaGTK::FullPaint %0d,%0d %0d,%0d\n",
	//	rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
	paintingAllText = true;
	if ((PWidget(wText))->window) {
		Surface *sw = Surface::Allocate();
		if (sw) {
			sw->Init(PWidget(wText)->window, PWidget(wText));
			Paint(sw, rcPaint);
			sw->Release();
			delete sw;
		}
	}
	paintState = notPainting;
#else
	wText.InvalidateAll();
#endif
}

PRectangle ScintillaGTK::GetClientRectangle() {
	PRectangle rc = wMain.GetClientPosition();
	if (verticalScrollBarVisible)
		rc.right -= scrollBarWidth;
	if (horizontalScrollBarVisible && (wrapState == eWrapNone))
		rc.bottom -= scrollBarHeight;
	// Move to origin
	rc.right -= rc.left;
	rc.bottom -= rc.top;
	rc.left = 0;
	rc.top = 0;
	return rc;
}

// Synchronously paint a rectangle of the window.
void ScintillaGTK::SyncPaint(PRectangle rc) {
	paintState = painting;
	rcPaint = rc;
	PRectangle rcClient = GetClientRectangle();
	paintingAllText = rcPaint.Contains(rcClient);
	if ((PWidget(wText))->window) {
		Surface *sw = Surface::Allocate();
		if (sw) {
			sw->Init(PWidget(wText)->window, PWidget(wText));
			Paint(sw, rc);
			sw->Release();
			delete sw;
		}
	}
	if (paintState == paintAbandoned) {
		// Painting area was insufficient to cover new styling or brace highlight positions
		FullPaint();
	}
	paintState = notPainting;
}

void ScintillaGTK::ScrollText(int linesToMove) {
	int diff = vs.lineHeight * -linesToMove;
	//Platform::DebugPrintf("ScintillaGTK::ScrollText %d %d %0d,%0d %0d,%0d\n", linesToMove, diff,
	//	rc.left, rc.top, rc.right, rc.bottom);
	GtkWidget *wi = PWidget(wText);

#if GTK_MAJOR_VERSION < 2
	PRectangle rc = GetClientRectangle();
	GdkGC *gc = gdk_gc_new(wi->window);

	// Set up gc so we get GraphicsExposures from gdk_draw_pixmap
	//  which calls XCopyArea
	gdk_gc_set_exposures(gc, TRUE);

	// Redraw exposed bit : scrolling upwards
	if (diff > 0) {
		gdk_draw_pixmap(wi->window,
		                gc, wi->window,
		                0, diff,
		                0, 0,
		                rc.Width()-1, rc.Height() - diff);
		SyncPaint(PRectangle(0, rc.Height() - diff,
		                     rc.Width(), rc.Height()+1));

	// Redraw exposed bit : scrolling downwards
	} else {
		gdk_draw_pixmap(wi->window,
		                gc, wi->window,
		                0, 0,
		                0, -diff,
		                rc.Width()-1, rc.Height() + diff);
		SyncPaint(PRectangle(0, 0, rc.Width(), -diff));
	}

	// Look for any graphics expose
	GdkEvent* event;
	while ((event = gdk_event_get_graphics_expose(wi->window)) != NULL) {
		gtk_widget_event(wi, event);
		if (event->expose.count == 0) {
			gdk_event_free(event);
			break;
		}
		gdk_event_free(event);
	}

	gdk_gc_unref(gc);
#else
	gdk_window_scroll(wi->window, 0, -diff);
	gdk_window_process_updates(wi->window, FALSE);
#endif
}

void ScintillaGTK::SetVerticalScrollPos() {
	DwellEnd(true);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adjustmentv), topLine);
}

void ScintillaGTK::SetHorizontalScrollPos() {
	DwellEnd(true);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adjustmenth), xOffset / 2);
}

bool ScintillaGTK::ModifyScrollBars(int nMax, int nPage) {
	bool modified = false;
	int pageScroll = LinesToScroll();

	if (GTK_ADJUSTMENT(adjustmentv)->upper != (nMax + 1) ||
	        GTK_ADJUSTMENT(adjustmentv)->page_size != nPage ||
	        GTK_ADJUSTMENT(adjustmentv)->page_increment != pageScroll) {
		GTK_ADJUSTMENT(adjustmentv)->upper = nMax + 1;
		GTK_ADJUSTMENT(adjustmentv)->page_size = nPage;
		GTK_ADJUSTMENT(adjustmentv)->page_increment = pageScroll;
		gtk_adjustment_changed(GTK_ADJUSTMENT(adjustmentv));
		modified = true;
	}

	PRectangle rcText = GetTextRectangle();
	int horizEndPreferred = scrollWidth;
	if (horizEndPreferred < 0)
		horizEndPreferred = 0;
	unsigned int pageWidth = rcText.Width();
	unsigned int pageIncrement = pageWidth / 3;
	unsigned int charWidth = vs.styles[STYLE_DEFAULT].aveCharWidth;
	if (GTK_ADJUSTMENT(adjustmenth)->upper != horizEndPreferred ||
	        GTK_ADJUSTMENT(adjustmenth)->page_size != pageWidth ||
	        GTK_ADJUSTMENT(adjustmenth)->page_increment != pageIncrement ||
	        GTK_ADJUSTMENT(adjustmenth)->step_increment != charWidth) {
		GTK_ADJUSTMENT(adjustmenth)->upper = horizEndPreferred;
		GTK_ADJUSTMENT(adjustmenth)->step_increment = charWidth;
		GTK_ADJUSTMENT(adjustmenth)->page_size = pageWidth;
		GTK_ADJUSTMENT(adjustmenth)->page_increment = pageIncrement;
		gtk_adjustment_changed(GTK_ADJUSTMENT(adjustmenth));
		modified = true;
	}
	return modified;
}

void ScintillaGTK::ReconfigureScrollBars() {
	PRectangle rc = wMain.GetClientPosition();
	Resize(rc.Width(), rc.Height());
}

void ScintillaGTK::NotifyChange() {
#if GLIB_MAJOR_VERSION < 2
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[COMMAND_SIGNAL],
	                Platform::LongFromTwoShorts(GetCtrlID(), SCEN_CHANGE), PWidget(wMain));
#else
	g_signal_emit(G_OBJECT(sci), scintilla_signals[COMMAND_SIGNAL], 0,
	                Platform::LongFromTwoShorts(GetCtrlID(), SCEN_CHANGE), PWidget(wMain));
#endif
}

void ScintillaGTK::NotifyFocus(bool focus) {
#if GLIB_MAJOR_VERSION < 2
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[COMMAND_SIGNAL],
	                Platform::LongFromTwoShorts
					(GetCtrlID(), focus ? SCEN_SETFOCUS : SCEN_KILLFOCUS), PWidget(wMain));
#else
	g_signal_emit(G_OBJECT(sci), scintilla_signals[COMMAND_SIGNAL], 0,
	                Platform::LongFromTwoShorts
					(GetCtrlID(), focus ? SCEN_SETFOCUS : SCEN_KILLFOCUS), PWidget(wMain));
#endif
}

void ScintillaGTK::NotifyParent(SCNotification scn) {
	scn.nmhdr.hwndFrom = PWidget(wMain);
	scn.nmhdr.idFrom = GetCtrlID();
#if GLIB_MAJOR_VERSION < 2
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[NOTIFY_SIGNAL],
	                GetCtrlID(), &scn);
#else
	g_signal_emit(G_OBJECT(sci), scintilla_signals[NOTIFY_SIGNAL], 0,
	                GetCtrlID(), &scn);
#endif
}

void ScintillaGTK::NotifyKey(int key, int modifiers) {
	SCNotification scn = {0};
	scn.nmhdr.code = SCN_KEY;
	scn.ch = key;
	scn.modifiers = modifiers;

	NotifyParent(scn);
}

void ScintillaGTK::NotifyURIDropped(const char *list) {
	SCNotification scn = {0};
	scn.nmhdr.code = SCN_URIDROPPED;
	scn.text = list;

	NotifyParent(scn);
}

const char *CharacterSetID(int characterSet);

const char *ScintillaGTK::CharacterSetID() const {
	return ::CharacterSetID(vs.styles[STYLE_DEFAULT].characterSet);
}

int ScintillaGTK::KeyDefault(int key, int modifiers) {
	if (!(modifiers & SCI_CTRL) && !(modifiers & SCI_ALT)) {
		if (key < 256) {
			NotifyKey(key, modifiers);
			return 0;
		} else {
			// Pass up to container in case it is an accelerator
			NotifyKey(key, modifiers);
			return 0;
		}
	} else {
		// Pass up to container in case it is an accelerator
		NotifyKey(key, modifiers);
		return 0;
	}
	//Platform::DebugPrintf("SK-key: %d %x %x\n",key, modifiers);
}

void ScintillaGTK::CopyToClipboard(const SelectionText &selectedText) {
#ifndef USE_GTK_CLIPBOARD
	copyText.Copy(selectedText);
	gtk_selection_owner_set(GTK_WIDGET(PWidget(wMain)),
				atomClipboard,
				GDK_CURRENT_TIME);
#else
	SelectionText *clipText = new SelectionText();
	clipText->Copy(selectedText);
	StoreOnClipboard(clipText);
#endif
}

void ScintillaGTK::Copy() {
	if (currentPos != anchor) {
#ifndef USE_GTK_CLIPBOARD
		CopySelectionRange(&copyText);
		gtk_selection_owner_set(GTK_WIDGET(PWidget(wMain)),
		                        atomClipboard,
		                        GDK_CURRENT_TIME);
#else
		SelectionText *clipText = new SelectionText();
		CopySelectionRange(clipText);
		StoreOnClipboard(clipText);
#endif
#if PLAT_GTK_WIN32
		if (selType == selRectangle) {
			::OpenClipboard(NULL);
			::SetClipboardData(cfColumnSelect, 0);
			::CloseClipboard();
		}
#endif
	}
}

void ScintillaGTK::Paste() {
	atomSought = atomUTF8;
	gtk_selection_convert(GTK_WIDGET(PWidget(wMain)),
	                      atomClipboard, atomSought, GDK_CURRENT_TIME);
}

void ScintillaGTK::CreateCallTipWindow(PRectangle rc) {
	if (!ct.wCallTip.Created()) {
		ct.wCallTip = gtk_window_new(GTK_WINDOW_POPUP);
		ct.wDraw = gtk_drawing_area_new();
		GtkWidget *widcdrw = PWidget(ct.wDraw);	//	// No code inside the G_OBJECT macro
		gtk_container_add(GTK_CONTAINER(PWidget(ct.wCallTip)), widcdrw);
#if GLIB_MAJOR_VERSION < 2
		gtk_signal_connect(GTK_OBJECT(widcdrw), "expose_event",
				   GtkSignalFunc(ScintillaGTK::ExposeCT), &ct);
		gtk_signal_connect(GTK_OBJECT(widcdrw), "button_press_event",
				   GtkSignalFunc(ScintillaGTK::PressCT), static_cast<void *>(this));
#else
		g_signal_connect(G_OBJECT(widcdrw), "expose_event",
				   G_CALLBACK(ScintillaGTK::ExposeCT), &ct);
		g_signal_connect(G_OBJECT(widcdrw), "button_press_event",
				   G_CALLBACK(ScintillaGTK::PressCT), static_cast<void *>(this));
#endif
		gtk_widget_set_events(widcdrw,
			GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);
	}
	gtk_drawing_area_size(GTK_DRAWING_AREA(PWidget(ct.wDraw)),
	                      rc.Width(), rc.Height());
	ct.wDraw.Show();
	if (PWidget(ct.wCallTip)->window) {
		gdk_window_resize(PWidget(ct.wCallTip)->window, rc.Width(), rc.Height());
	}
}

void ScintillaGTK::AddToPopUp(const char *label, int cmd, bool enabled) {
	char fulllabel[200];
	strcpy(fulllabel, "/");
	strcat(fulllabel, label);
	GtkItemFactoryCallback menuSig = GtkItemFactoryCallback(PopUpCB);
	GtkItemFactoryEntry itemEntry = {
	    fulllabel, NULL,
	    menuSig,
	    cmd,
	    const_cast<gchar *>(label[0] ? "<Item>" : "<Separator>"),
#if GTK_MAJOR_VERSION >= 2
	    NULL
#endif
	};
	gtk_item_factory_create_item(GTK_ITEM_FACTORY(popup.GetID()),
	                             &itemEntry, this, 1);
	if (cmd) {
		GtkWidget *item = gtk_item_factory_get_widget_by_action(
		                      reinterpret_cast<GtkItemFactory *>(popup.GetID()), cmd);
		if (item)
			gtk_widget_set_sensitive(item, enabled);
	}
}

bool ScintillaGTK::OwnPrimarySelection() {
	return ((gdk_selection_owner_get(GDK_SELECTION_PRIMARY)
		== GTK_WIDGET(PWidget(wMain))->window) &&
			(GTK_WIDGET(PWidget(wMain))->window != NULL));
}

void ScintillaGTK::ClaimSelection() {
	// X Windows has a 'primary selection' as well as the clipboard.
	// Whenever the user selects some text, we become the primary selection
	if (currentPos != anchor && GTK_WIDGET_REALIZED(GTK_WIDGET(PWidget(wMain)))) {
		primarySelection = true;
		gtk_selection_owner_set(GTK_WIDGET(PWidget(wMain)),
		                        GDK_SELECTION_PRIMARY, GDK_CURRENT_TIME);
		primary.Free();
	} else if (OwnPrimarySelection()) {
		primarySelection = true;
		if (primary.s == NULL)
			gtk_selection_owner_set(NULL, GDK_SELECTION_PRIMARY, GDK_CURRENT_TIME);
	} else {
		primarySelection = false;
		primary.Free();
	}
}

// Detect rectangular text, convert line ends to current mode, convert from or to UTF-8
void ScintillaGTK::GetGtkSelectionText(GtkSelectionData *selectionData, SelectionText &selText) {
	char *data = reinterpret_cast<char *>(selectionData->data);
	int len = selectionData->length;
	GdkAtom selectionType = selectionData->type;

	// Return empty string if selection is not a string
	if ((selectionType != GDK_TARGET_STRING) && (selectionType != atomUTF8)) {
		char *empty = new char[1];
		empty[0] = '\0';
		selText.Set(empty, 0, SC_CP_UTF8, 0, false, false);
		return;
	}

	// Check for "\n\0" ending to string indicating that selection is rectangular
	bool isRectangular;
#if PLAT_GTK_WIN32
	isRectangular = ::IsClipboardFormatAvailable(cfColumnSelect) != 0;
#else
	isRectangular = ((len > 2) && (data[len - 1] == 0 && data[len - 2] == '\n'));
#endif

	char *dest;
	if (selectionType == GDK_TARGET_STRING) {
		dest = Document::TransformLineEnds(&len, data, len, pdoc->eolMode);
		if (IsUnicodeMode()) {
			// Unknown encoding so assume in Latin1
			char *destPrevious = dest;
			dest = UTF8FromLatin1(dest, len);
			selText.Set(dest, len, SC_CP_UTF8, 0, selText.rectangular, false);
			delete []destPrevious;
		} else {
			// Assume buffer is in same encoding as selection
			selText.Set(dest, len, pdoc->dbcsCodePage,
				vs.styles[STYLE_DEFAULT].characterSet, isRectangular, false);
		}
	} else {	// UTF-8
		dest = Document::TransformLineEnds(&len, data, len, pdoc->eolMode);
		selText.Set(dest, len, SC_CP_UTF8, 0, isRectangular, false);
#ifdef USE_CONVERTER
		const char *charSetBuffer = CharacterSetID();
		if (!IsUnicodeMode() && *charSetBuffer) {
//fprintf(stderr, "Convert to locale %s\n", CharacterSetID());
				// Convert to locale
				dest = ConvertText(&len, selText.s, selText.len, charSetBuffer, "UTF-8", true);
				selText.Set(dest, len, pdoc->dbcsCodePage,
					vs.styles[STYLE_DEFAULT].characterSet, selText.rectangular, false);
		}
#endif
	}
}

void ScintillaGTK::ReceivedSelection(GtkSelectionData *selection_data) {
	if ((selection_data->selection == atomClipboard) ||
		(selection_data->selection == GDK_SELECTION_PRIMARY)) {
		if ((atomSought == atomUTF8) && (selection_data->length <= 0)) {
			atomSought = atomString;
			gtk_selection_convert(GTK_WIDGET(PWidget(wMain)),
					      selection_data->selection, atomSought, GDK_CURRENT_TIME);
		} else if ((selection_data->length > 0) &&
			((selection_data->type == GDK_TARGET_STRING) || (selection_data->type == atomUTF8))) {
			SelectionText selText;
			GetGtkSelectionText(selection_data, selText);

			pdoc->BeginUndoAction();
			if (selection_data->selection != GDK_SELECTION_PRIMARY) {
				ClearSelection();
			}
			int selStart = SelectionStart();

			if (selText.rectangular) {
				PasteRectangular(selStart, selText.s, selText.len);
			} else {
				pdoc->InsertString(currentPos, selText.s, selText.len);
				SetEmptySelection(currentPos + selText.len);
			}
			pdoc->EndUndoAction();
			EnsureCaretVisible();
		}
	}
//	else fprintf(stderr, "Target non string %d %d\n", (int)(selection_data->type),
//		(int)(atomUTF8));
	Redraw();
}

void ScintillaGTK::ReceivedDrop(GtkSelectionData *selection_data) {
	dragWasDropped = true;
	if (selection_data->type == atomUriList || selection_data->type == atomDROPFILES_DND) {
		char *ptr = new char[selection_data->length + 1];
		ptr[selection_data->length] = '\0';
		memcpy(ptr, selection_data->data, selection_data->length);
 		NotifyURIDropped(ptr);
		delete []ptr;
	} else if ((selection_data->type == GDK_TARGET_STRING) || (selection_data->type == atomUTF8)) {
		if (selection_data->length > 0) {
			SelectionText selText;
			GetGtkSelectionText(selection_data, selText);
			DropAt(posDrop, selText.s, false, selText.rectangular);
		}
	} else if (selection_data->length > 0) {
	    //~ fprintf(stderr, "ReceivedDrop other %p\n", static_cast<void *>(selection_data->type));
	}
	Redraw();
}



void ScintillaGTK::GetSelection(GtkSelectionData *selection_data, guint info, SelectionText *text) {
#if PLAT_GTK_WIN32
	// GDK on Win32 expands any \n into \r\n, so make a copy of
	// the clip text now with newlines converted to \n.  Use { } to hide symbols
	// from code below
	SelectionText *newline_normalized = NULL;
	{
		int tmpstr_len;
		char *tmpstr = Document::TransformLineEnds(&tmpstr_len, text->s, text->len, SC_EOL_LF);
		newline_normalized = new SelectionText();
		newline_normalized->Set(tmpstr, tmpstr_len, SC_CP_UTF8, 0, text->rectangular, false);
		text = newline_normalized;
	}
#endif

#if GTK_MAJOR_VERSION >= 2
	// Convert text to utf8 if it isn't already
	SelectionText *converted = 0;
	if ((text->codePage != SC_CP_UTF8) && (info == TARGET_UTF8_STRING)) {
		const char *charSet = ::CharacterSetID(text->characterSet);
		if (*charSet) {
			int new_len;
			char* tmputf = ConvertText(&new_len, text->s, text->len, "UTF-8", charSet, false);
			converted = new SelectionText();
			converted->Set(tmputf, new_len, SC_CP_UTF8, 0, text->rectangular, false);
			text = converted;
		}
	}

	// Here is a somewhat evil kludge.
	// As I can not work out how to store data on the clipboard in multiple formats
	// and need some way to mark the clipping as being stream or rectangular,
	// the terminating \0 is included in the length for rectangular clippings.
	// All other tested aplications behave benignly by ignoring the \0.
	// The #if is here because on Windows cfColumnSelect clip entry is used
	// instead as standard indicator of rectangularness (so no need to kludge)
	int len = strlen(text->s);
#if PLAT_GTK_WIN32 == 0
	if (text->rectangular)
		len++;
#endif

	if (info == TARGET_UTF8_STRING) {
		gtk_selection_data_set_text(selection_data, text->s, len);
	} else {
		gtk_selection_data_set(selection_data,
			static_cast<GdkAtom>(GDK_SELECTION_TYPE_STRING),
			8, reinterpret_cast<unsigned char *>(text->s), len);
	}
	delete converted;

#else /* Gtk 1 */
	char *selBuffer = text->s;

	char *tmputf = 0;
	if ((info == TARGET_UTF8_STRING) || (info == TARGET_STRING)) {
		int len = strlen(selBuffer);
#ifdef USE_CONVERTER
		// Possible character set conversion
		const char *charSetBuffer = ::CharacterSetID(text->characterSet);
		if (info == TARGET_UTF8_STRING) {
			//fprintf(stderr, "Copy to clipboard as UTF-8\n");
			if (text->codePage != SC_CP_UTF8) {
				// Convert to UTF-8
	//fprintf(stderr, "Convert to UTF-8 from %s\n", charSetBuffer);
				tmputf = ConvertText(&len, selBuffer, len, "UTF-8", charSetBuffer, false);
				selBuffer = tmputf;
			}
		} else if (info == TARGET_STRING) {
			if (text->codePage == SC_CP_UTF8) {
	//fprintf(stderr, "Convert to locale %s\n", charSetBuffer);
				// Convert to locale
				tmputf = ConvertText(&len, selBuffer, len, charSetBuffer, "UTF-8", true);
				selBuffer = tmputf;
			}
		}
#endif

		// Here is a somewhat evil kludge.
		// As I can not work out how to store data on the clipboard in multiple formats
		// and need some way to mark the clipping as being stream or rectangular,
		// the terminating \0 is included in the length for rectangular clippings.
		// All other tested aplications behave benignly by ignoring the \0.
		// The #if is here because on Windows cfColumnSelect clip entry is used
                // instead as standard indicator of rectangularness (so no need to kludge)
#if PLAT_GTK_WIN32 == 0
		if (text->rectangular)
			len++;
#endif
		gtk_selection_data_set(selection_data,
					(info == TARGET_STRING) ?
					static_cast<GdkAtom>(GDK_SELECTION_TYPE_STRING) : atomUTF8,
		                       8, reinterpret_cast<unsigned char *>(selBuffer),
		                       len);
	} else if ((info == TARGET_TEXT) || (info == TARGET_COMPOUND_TEXT)) {
		guchar *text;
		GdkAtom encoding;
		gint format;
		gint new_length;

		gdk_string_to_compound_text(reinterpret_cast<char *>(selBuffer),
		                            &encoding, &format, &text, &new_length);
		gtk_selection_data_set(selection_data, encoding, format, text, new_length);
		gdk_free_compound_text(text);
	}

	delete []tmputf;
#endif /* Gtk >= 2 */

#if PLAT_GTK_WIN32
	delete newline_normalized;
#endif
}

#ifdef USE_GTK_CLIPBOARD
void ScintillaGTK::StoreOnClipboard(SelectionText *clipText) {
	GtkClipboard *clipBoard =
		gtk_widget_get_clipboard(GTK_WIDGET(PWidget(wMain)), atomClipboard);
	if (clipBoard == NULL) // Occurs if widget isn't in a toplevel
		return;

	if (gtk_clipboard_set_with_data(clipBoard, clipboardCopyTargets, nClipboardCopyTargets,
				    ClipboardGetSelection, ClipboardClearSelection, clipText)) {
		gtk_clipboard_set_can_store(clipBoard, clipboardCopyTargets, nClipboardCopyTargets);
	}
}

void ScintillaGTK::ClipboardGetSelection(GtkClipboard *, GtkSelectionData *selection_data, guint info, void *data) {
	GetSelection(selection_data, info, static_cast<SelectionText*>(data));
}

void ScintillaGTK::ClipboardClearSelection(GtkClipboard *, void *data) {
	SelectionText *obj = static_cast<SelectionText*>(data);
	delete obj;
}
#endif

void ScintillaGTK::UnclaimSelection(GdkEventSelection *selection_event) {
	//Platform::DebugPrintf("UnclaimSelection\n");
	if (selection_event->selection == GDK_SELECTION_PRIMARY) {
		//Platform::DebugPrintf("UnclaimPrimarySelection\n");
		if (!OwnPrimarySelection()) {
			primary.Free();
			primarySelection = false;
			FullPaint();
		}
	}
}

void ScintillaGTK::Resize(int width, int height) {
	//Platform::DebugPrintf("Resize %d %d\n", width, height);
	//printf("Resize %d %d\n", width, height);

	// Not always needed, but some themes can have different sizes of scrollbars
	scrollBarWidth = GTK_WIDGET(PWidget(scrollbarv))->requisition.width;
	scrollBarHeight = GTK_WIDGET(PWidget(scrollbarh))->requisition.height;

	// These allocations should never produce negative sizes as they would wrap around to huge
	// unsigned numbers inside GTK+ causing warnings.
	bool showSBHorizontal = horizontalScrollBarVisible && (wrapState == eWrapNone);
	int horizontalScrollBarHeight = scrollBarHeight;
	if (!showSBHorizontal)
		horizontalScrollBarHeight = 0;
	int verticalScrollBarHeight = scrollBarWidth;
	if (!verticalScrollBarVisible)
		verticalScrollBarHeight = 0;

	GtkAllocation alloc;
	if (showSBHorizontal) {
		gtk_widget_show(GTK_WIDGET(PWidget(scrollbarh)));
		alloc.x = 0;
		alloc.y = height - scrollBarHeight;
		alloc.width = Platform::Maximum(1, width - scrollBarWidth) + 1;
		alloc.height = horizontalScrollBarHeight;
		gtk_widget_size_allocate(GTK_WIDGET(PWidget(scrollbarh)), &alloc);
	} else {
		gtk_widget_hide(GTK_WIDGET(PWidget(scrollbarh)));
	}

	if (verticalScrollBarVisible) {
		gtk_widget_show(GTK_WIDGET(PWidget(scrollbarv)));
		alloc.x = width - scrollBarWidth;
		alloc.y = 0;
		alloc.width = scrollBarWidth;
		alloc.height = Platform::Maximum(1, height - scrollBarHeight) + 1;
		if (!showSBHorizontal)
			alloc.height += scrollBarWidth-1;
		gtk_widget_size_allocate(GTK_WIDGET(PWidget(scrollbarv)), &alloc);
	} else {
		gtk_widget_hide(GTK_WIDGET(PWidget(scrollbarv)));
	}
	if (GTK_WIDGET_MAPPED(PWidget(wMain))) {
		ChangeSize();
	}

	alloc.x = 0;
	alloc.y = 0;
	alloc.width = Platform::Maximum(1, width - scrollBarWidth);
	alloc.height = Platform::Maximum(1, height - scrollBarHeight);
	if (!showSBHorizontal)
		alloc.height += scrollBarHeight;
	if (!verticalScrollBarVisible)
		alloc.width += scrollBarWidth;
	gtk_widget_size_allocate(GTK_WIDGET(PWidget(wText)), &alloc);
}

static void SetAdjustmentValue(GtkObject *object, int value) {
	GtkAdjustment *adjustment = GTK_ADJUSTMENT(object);
	int maxValue = static_cast<int>(
		adjustment->upper - adjustment->page_size);
	if (value > maxValue)
		value = maxValue;
	if (value < 0)
		value = 0;
	gtk_adjustment_set_value(adjustment, value);
}

gint ScintillaGTK::PressThis(GdkEventButton *event) {
	//Platform::DebugPrintf("Press %x time=%d state = %x button = %x\n",this,event->time, event->state, event->button);
	// Do not use GTK+ double click events as Scintilla has its own double click detection
	if (event->type != GDK_BUTTON_PRESS)
		return FALSE;

	evbtn = *event;
	Point pt;
	pt.x = int(event->x);
	pt.y = int(event->y);
	PRectangle rcClient = GetClientRectangle();
	//Platform::DebugPrintf("Press %0d,%0d in %0d,%0d %0d,%0d\n",
	//	pt.x, pt.y, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
	if ((pt.x > rcClient.right) || (pt.y > rcClient.bottom)) {
		Platform::DebugPrintf("Bad location\n");
		return FALSE;
	}

	bool ctrl = (event->state & GDK_CONTROL_MASK) != 0;

	gtk_widget_grab_focus(PWidget(wMain));
	if (event->button == 1) {
		// On X, instead of sending literal modifiers use control instead of alt
		// This is because most X window managers grab alt + click for moving
#if !PLAT_GTK_WIN32
		ButtonDown(pt, event->time,
				    (event->state & GDK_SHIFT_MASK) != 0,
				    (event->state & GDK_CONTROL_MASK) != 0,
				    (event->state & GDK_CONTROL_MASK) != 0);
#else
		ButtonDown(pt, event->time,
				    (event->state & GDK_SHIFT_MASK) != 0,
				    (event->state & GDK_CONTROL_MASK) != 0,
				    (event->state & GDK_MOD1_MASK) != 0);
#endif
	} else if (event->button == 2) {
		// Grab the primary selection if it exists
		Position pos = PositionFromLocation(pt);
		if (OwnPrimarySelection() && primary.s == NULL)
			CopySelectionRange(&primary);

		SetSelection(pos, pos);
		atomSought = atomUTF8;
		gtk_selection_convert(GTK_WIDGET(PWidget(wMain)), GDK_SELECTION_PRIMARY,
		                      atomSought, event->time);
	} else if (event->button == 3) {
		if (displayPopupMenu) {
			// PopUp menu
			// Convert to screen
			int ox = 0;
			int oy = 0;
			gdk_window_get_origin(PWidget(wMain)->window, &ox, &oy);
			ContextMenu(Point(pt.x + ox, pt.y + oy));
		} else {
			return FALSE;
		}
	} else if (event->button == 4) {
		// Wheel scrolling up (only GTK 1.x does it this way)
		if (ctrl)
			SetAdjustmentValue(adjustmenth, (xOffset / 2) - 6);
		else
			SetAdjustmentValue(adjustmentv, topLine - 3);
	} else if (event->button == 5) {
		// Wheel scrolling down (only GTK 1.x does it this way)
		if (ctrl)
			SetAdjustmentValue(adjustmenth, (xOffset / 2) + 6);
		else
			SetAdjustmentValue(adjustmentv, topLine + 3);
	}
#if GTK_MAJOR_VERSION >= 2
	return TRUE;
#else
	return FALSE;
#endif
}

gint ScintillaGTK::Press(GtkWidget *widget, GdkEventButton *event) {
	if (event->window != widget->window)
		return FALSE;
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	return sciThis->PressThis(event);
}

gint ScintillaGTK::MouseRelease(GtkWidget *widget, GdkEventButton *event) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Release %x %d %d\n",sciThis,event->time,event->state);
	if (!sciThis->HaveMouseCapture())
		return FALSE;
	if (event->button == 1) {
		Point pt;
		pt.x = int(event->x);
		pt.y = int(event->y);
		//Platform::DebugPrintf("Up %x %x %d %d %d\n",
		//	sciThis,event->window,event->time, pt.x, pt.y);
		if (event->window != PWidget(sciThis->wMain)->window)
			// If mouse released on scroll bar then the position is relative to the
			// scrollbar, not the drawing window so just repeat the most recent point.
			pt = sciThis->ptMouseLast;
		sciThis->ButtonUp(pt, event->time, (event->state & 4) != 0);
	}
	return FALSE;
}

// win32gtk and GTK >= 2 use SCROLL_* events instead of passing the
// button4/5/6/7 events to the GTK app
#if PLAT_GTK_WIN32 || (GTK_MAJOR_VERSION >= 2)
gint ScintillaGTK::ScrollEvent(GtkWidget *widget,
                               GdkEventScroll *event) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);

	if (widget == NULL || event == NULL)
		return FALSE;

	// Compute amount and direction to scroll (even tho on win32 there is
	// intensity of scrolling info in the native message, gtk doesn't
	// support this so we simulate similarly adaptive scrolling)
	// Note that this is disabled on OS X (Darwin) where the X11 server already has
	// and adaptive scrolling algorithm that fights with this one
	int cLineScroll;
#if defined(__MWERKS__) || defined(__APPLE_CPP__) || defined(__APPLE_CC__)
	cLineScroll = sciThis->linesPerScroll;
	if (cLineScroll == 0)
		cLineScroll = 4;
	sciThis->wheelMouseIntensity = cLineScroll;
#else
	int timeDelta = 1000000;
	GTimeVal curTime;
	g_get_current_time(&curTime);
	if (curTime.tv_sec == sciThis->lastWheelMouseTime.tv_sec)
		timeDelta = curTime.tv_usec - sciThis->lastWheelMouseTime.tv_usec;
	else if (curTime.tv_sec == sciThis->lastWheelMouseTime.tv_sec + 1)
		timeDelta = 1000000 + (curTime.tv_usec - sciThis->lastWheelMouseTime.tv_usec);
	if ((event->direction == sciThis->lastWheelMouseDirection) && (timeDelta < 250000)) {
		if (sciThis->wheelMouseIntensity < 12)
			sciThis->wheelMouseIntensity++;
		cLineScroll = sciThis->wheelMouseIntensity;
	} else {
		cLineScroll = sciThis->linesPerScroll;
		if (cLineScroll == 0)
			cLineScroll = 4;
		sciThis->wheelMouseIntensity = cLineScroll;
	}
#endif
	if (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_LEFT) {
		cLineScroll *= -1;
	}
	g_get_current_time(&sciThis->lastWheelMouseTime);
	sciThis->lastWheelMouseDirection = event->direction;

	// Note:  Unpatched versions of win32gtk don't set the 'state' value so
	// only regular scrolling is supported there.  Also, unpatched win32gtk
	// issues spurious button 2 mouse events during wheeling, which can cause
	// problems (a patch for both was submitted by archaeopteryx.com on 13Jun2001)

	// Data zoom not supported
	if (event->state & GDK_SHIFT_MASK) {
		return FALSE;
	}

	// Horizontal scrolling
	if (event->direction == GDK_SCROLL_LEFT || event->direction == GDK_SCROLL_RIGHT) {
		sciThis->HorizontalScrollTo(sciThis->xOffset + cLineScroll);

	// Text font size zoom
	} else if (event->state & GDK_CONTROL_MASK) {
		if (cLineScroll < 0) {
			sciThis->KeyCommand(SCI_ZOOMIN);
		} else {
			sciThis->KeyCommand(SCI_ZOOMOUT);
		}

	// Regular scrolling
	} else {
		sciThis->ScrollTo(sciThis->topLine + cLineScroll);
	}
	return TRUE;
}
#endif

gint ScintillaGTK::Motion(GtkWidget *widget, GdkEventMotion *event) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Motion %x %d\n",sciThis,event->time);
	if (event->window != widget->window)
		return FALSE;
	int x = 0;
	int y = 0;
	GdkModifierType state;
	if (event->is_hint) {
		gdk_window_get_pointer(event->window, &x, &y, &state);
	} else {
		x = static_cast<int>(event->x);
		y = static_cast<int>(event->y);
		state = static_cast<GdkModifierType>(event->state);
	}
	//Platform::DebugPrintf("Move %x %x %d %c %d %d\n",
	//	sciThis,event->window,event->time,event->is_hint? 'h' :'.', x, y);
	Point pt(x, y);
	sciThis->ButtonMove(pt);
	return FALSE;
}

// Map the keypad keys to their equivalent functions
static int KeyTranslate(int keyIn) {
	switch (keyIn) {
	case GDK_ISO_Left_Tab:
		return SCK_TAB;
	case GDK_KP_Down:
		return SCK_DOWN;
	case GDK_KP_Up:
		return SCK_UP;
	case GDK_KP_Left:
		return SCK_LEFT;
	case GDK_KP_Right:
		return SCK_RIGHT;
	case GDK_KP_Home:
		return SCK_HOME;
	case GDK_KP_End:
		return SCK_END;
	case GDK_KP_Page_Up:
		return SCK_PRIOR;
	case GDK_KP_Page_Down:
		return SCK_NEXT;
	case GDK_KP_Delete:
		return SCK_DELETE;
	case GDK_KP_Insert:
		return SCK_INSERT;
	case GDK_KP_Enter:
		return SCK_RETURN;

	case GDK_Down:
		return SCK_DOWN;
	case GDK_Up:
		return SCK_UP;
	case GDK_Left:
		return SCK_LEFT;
	case GDK_Right:
		return SCK_RIGHT;
	case GDK_Home:
		return SCK_HOME;
	case GDK_End:
		return SCK_END;
	case GDK_Page_Up:
		return SCK_PRIOR;
	case GDK_Page_Down:
		return SCK_NEXT;
	case GDK_Delete:
		return SCK_DELETE;
	case GDK_Insert:
		return SCK_INSERT;
	case GDK_Escape:
		return SCK_ESCAPE;
	case GDK_BackSpace:
		return SCK_BACK;
	case GDK_Tab:
		return SCK_TAB;
	case GDK_Return:
		return SCK_RETURN;
	case GDK_KP_Add:
		return SCK_ADD;
	case GDK_KP_Subtract:
		return SCK_SUBTRACT;
	case GDK_KP_Divide:
		return SCK_DIVIDE;
	case GDK_Super_L:
		return SCK_WIN;
	case GDK_Super_R:
		return SCK_RWIN;
	case GDK_Menu:
		return SCK_MENU;
	default:
		return keyIn;
	}
}

gboolean ScintillaGTK::KeyThis(GdkEventKey *event) {
	//fprintf(stderr, "SC-key: %d %x [%s]\n",
	//	event->keyval, event->state, (event->length > 0) ? event->string : "empty");
#if GTK_MAJOR_VERSION >= 2
	if (gtk_im_context_filter_keypress(im_context, event)) {
		return 1;
	}
#endif
	if (!event->keyval) {
		return true;
	}

	bool shift = (event->state & GDK_SHIFT_MASK) != 0;
	bool ctrl = (event->state & GDK_CONTROL_MASK) != 0;
	bool alt = (event->state & GDK_MOD1_MASK) != 0;
	guint key = event->keyval;
	if (ctrl && (key < 128))
		key = toupper(key);
	else if (!ctrl && (key >= GDK_KP_Multiply && key <= GDK_KP_9))
		key &= 0x7F;
	// Hack for keys over 256 and below command keys but makes Hungarian work.
	// This will have to change for Unicode
	else if (key >= 0xFE00)
		key = KeyTranslate(key);
#if GTK_MAJOR_VERSION < 2
	else if (!IsUnicodeMode() && (key >= 0x100) && (key < 0x1000))
		key &= 0xff;
#endif

	bool consumed = false;
	bool added = KeyDown(key, shift, ctrl, alt, &consumed) != 0;
	if (!consumed)
		consumed = added;
	//fprintf(stderr, "SK-key: %d %x %x\n",event->keyval, event->state, consumed);
	if (event->keyval == 0xffffff && event->length > 0) {
		ClearSelection();
		if (pdoc->InsertCString(CurrentPosition(), event->string)) {
			MovePositionTo(CurrentPosition() + event->length);
		}
	}
	return consumed;
}

gboolean ScintillaGTK::KeyPress(GtkWidget *widget, GdkEventKey *event) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	return sciThis->KeyThis(event);
}

gboolean ScintillaGTK::KeyRelease(GtkWidget *, GdkEventKey * /*event*/) {
	//Platform::DebugPrintf("SC-keyrel: %d %x %3s\n",event->keyval, event->state, event->string);
	return FALSE;
}

#if GTK_MAJOR_VERSION >= 2
gboolean ScintillaGTK::ExposePreedit(GtkWidget *widget, GdkEventExpose *ose, ScintillaGTK *sciThis) {
	return sciThis->ExposePreeditThis(widget, ose);
}

gboolean ScintillaGTK::ExposePreeditThis(GtkWidget *widget, GdkEventExpose *ose) {
	gchar *str;
	gint cursor_pos;
	PangoAttrList *attrs;

	gtk_im_context_get_preedit_string(im_context, &str, &attrs, &cursor_pos);
	PangoLayout *layout = gtk_widget_create_pango_layout(PWidget(wText), str);
	pango_layout_set_attributes(layout, attrs);

	GdkGC *gc = gdk_gc_new(widget->window);
	GdkColor color[2] = {   {0, 0x0000, 0x0000, 0x0000},
                            {0, 0xffff, 0xffff, 0xffff}};
	gdk_color_alloc(gdk_colormap_get_system(), color);
	gdk_color_alloc(gdk_colormap_get_system(), color + 1);

	gdk_gc_set_foreground(gc, color + 1);
	gdk_draw_rectangle(widget->window, gc, TRUE, ose->area.x, ose->area.y,
	                   ose->area.width, ose->area.height);

	gdk_gc_set_foreground(gc, color);
	gdk_gc_set_background(gc, color + 1);
	gdk_draw_layout(widget->window, gc, 0, 0, layout);

	gdk_gc_unref(gc);
	g_free(str);
	pango_attr_list_unref(attrs);
	g_object_unref(layout);
	return TRUE;
}

void ScintillaGTK::Commit(GtkIMContext *, char  *str, ScintillaGTK *sciThis) {
	sciThis->CommitThis(str);
}

void ScintillaGTK::CommitThis(char *utfVal) {
	//~ fprintf(stderr, "Commit '%s'\n", utfVal);
	if (IsUnicodeMode()) {
		AddCharUTF(utfVal,strlen(utfVal));
	} else {
		const char *source = CharacterSetID();
		if (*source) {
			Converter conv(source, "UTF-8", true);
			if (conv) {
				char localeVal[4]="\0\0\0";
				char *pin = utfVal;
				size_t inLeft = strlen(utfVal);
				char *pout = localeVal;
				size_t outLeft = sizeof(localeVal);
				size_t conversions = conv.Convert(&pin, &inLeft, &pout, &outLeft);
				if (conversions != ((size_t)(-1))) {
					*pout = '\0';
					for (int i=0; localeVal[i]; i++) {
						AddChar(localeVal[i]);
					}
				} else {
					fprintf(stderr, "Conversion failed '%s'\n", utfVal);
				}
			}
		}
	}
}

void ScintillaGTK::PreeditChanged(GtkIMContext *, ScintillaGTK *sciThis) {
	sciThis->PreeditChangedThis();
}

void ScintillaGTK::PreeditChangedThis() {
	gchar *str;
	PangoAttrList *attrs;
	gint cursor_pos;
	gtk_im_context_get_preedit_string(im_context, &str, &attrs, &cursor_pos);
	if (strlen(str) > 0){
		PangoLayout *layout = gtk_widget_create_pango_layout(PWidget(wText), str);
		pango_layout_set_attributes(layout, attrs);

		gint w, h;
		pango_layout_get_pixel_size(layout, &w, &h);
		g_object_unref(layout);

		gint x, y;
		gdk_window_get_origin((PWidget(wText))->window, &x, &y);

		Point pt = LocationFromPosition(currentPos);
		if (pt.x < 0)
			pt.x = 0;
		if (pt.y < 0)
			pt.y = 0;

		gtk_window_move(GTK_WINDOW(PWidget(wPreedit)), x+pt.x, y+pt.y);
		gtk_window_resize(GTK_WINDOW(PWidget(wPreedit)), w, h);
		gtk_widget_show(PWidget(wPreedit));
		gtk_widget_queue_draw_area(PWidget(wPreeditDraw), 0, 0, w, h);
	} else {
		gtk_widget_hide(PWidget(wPreedit));
	}
	g_free(str);
	pango_attr_list_unref(attrs);
}
#endif

gint ScintillaGTK::StyleSetText(GtkWidget *widget, GtkStyle *, void*) {
	if (widget->window != NULL)
		gdk_window_set_back_pixmap(widget->window, NULL, FALSE);
	return FALSE;
}

gint ScintillaGTK::RealizeText(GtkWidget *widget, void*) {
	if (widget->window != NULL)
		gdk_window_set_back_pixmap(widget->window, NULL, FALSE);
	return FALSE;
}

#if GLIB_MAJOR_VERSION < 2
void ScintillaGTK::Destroy(GtkObject *object)
#else
void ScintillaGTK::Destroy(GObject *object)
#endif
{
	ScintillaObject *scio = reinterpret_cast<ScintillaObject *>(object);
	// This avoids a double destruction
	if (!scio->pscin)
		return;
	ScintillaGTK *sciThis = reinterpret_cast<ScintillaGTK *>(scio->pscin);
	//Platform::DebugPrintf("Destroying %x %x\n", sciThis, object);
	sciThis->Finalise();

#if GLIB_MAJOR_VERSION < 2
	if (GTK_OBJECT_CLASS(parent_class)->destroy)
		(* GTK_OBJECT_CLASS(parent_class)->destroy)(object);
#else
	// IS ANYTHING NEEDED ?
#endif

	delete sciThis;
	scio->pscin = 0;
}

static void DrawChild(GtkWidget *widget, GdkRectangle *area) {
	GdkRectangle areaIntersect;
	if (widget &&
	        GTK_WIDGET_DRAWABLE(widget) &&
	        gtk_widget_intersect(widget, area, &areaIntersect)) {
		gtk_widget_draw(widget, &areaIntersect);
	}
}

void ScintillaGTK::Draw(GtkWidget *widget, GdkRectangle *area) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Draw %p %0d,%0d %0d,%0d\n", widget, area->x, area->y, area->width, area->height);
	PRectangle rcPaint(area->x, area->y, area->x + area->width, area->y + area->height);
	sciThis->SyncPaint(rcPaint);
	if (GTK_WIDGET_DRAWABLE(PWidget(sciThis->wMain))) {
		DrawChild(PWidget(sciThis->scrollbarh), area);
		DrawChild(PWidget(sciThis->scrollbarv), area);
	}

#ifdef INTERNATIONAL_INPUT
	Point pt = sciThis->LocationFromPosition(sciThis->currentPos);
	pt.y += sciThis->vs.lineHeight - 2;
	if (pt.x < 0) pt.x = 0;
	if (pt.y < 0) pt.y = 0;
	CursorMoved(widget, pt.x, pt.y, sciThis);
#endif
}

gint ScintillaGTK::ExposeTextThis(GtkWidget * /*widget*/, GdkEventExpose *ose) {
	paintState = painting;

	rcPaint.left = ose->area.x;
	rcPaint.top = ose->area.y;
	rcPaint.right = ose->area.x + ose->area.width;
	rcPaint.bottom = ose->area.y + ose->area.height;

	PLATFORM_ASSERT(rgnUpdate == NULL);
#if GTK_MAJOR_VERSION >= 2
	rgnUpdate = gdk_region_copy(ose->region);
#endif
	PRectangle rcClient = GetClientRectangle();
	paintingAllText = rcPaint.Contains(rcClient);
	Surface *surfaceWindow = Surface::Allocate();
	if (surfaceWindow) {
		surfaceWindow->Init(PWidget(wText)->window, PWidget(wText));
		Paint(surfaceWindow, rcPaint);
		surfaceWindow->Release();
		delete surfaceWindow;
	}
	if (paintState == paintAbandoned) {
		// Painting area was insufficient to cover new styling or brace highlight positions
		FullPaint();
	}
	paintState = notPainting;

	if (rgnUpdate) {
		gdk_region_destroy(rgnUpdate);
	}
	rgnUpdate = 0;

	return FALSE;
}

gint ScintillaGTK::ExposeText(GtkWidget *widget, GdkEventExpose *ose, ScintillaGTK *sciThis) {
	return sciThis->ExposeTextThis(widget, ose);
}

gint ScintillaGTK::ExposeMain(GtkWidget *widget, GdkEventExpose *ose) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Expose Main %0d,%0d %0d,%0d\n",
	//ose->area.x, ose->area.y, ose->area.width, ose->area.height);
	return sciThis->Expose(widget, ose);
}

gint ScintillaGTK::Expose(GtkWidget *, GdkEventExpose *ose) {
	//fprintf(stderr, "Expose %0d,%0d %0d,%0d\n",
	//ose->area.x, ose->area.y, ose->area.width, ose->area.height);

#if GTK_MAJOR_VERSION < 2

	paintState = painting;

	rcPaint.left = ose->area.x;
	rcPaint.top = ose->area.y;
	rcPaint.right = ose->area.x + ose->area.width;
	rcPaint.bottom = ose->area.y + ose->area.height;

	PRectangle rcClient = GetClientRectangle();
	paintingAllText = rcPaint.Contains(rcClient);
	Surface *surfaceWindow = Surface::Allocate();
	if (surfaceWindow) {
		surfaceWindow->Init(PWidget(wMain)->window, PWidget(wMain));

		// Fill the corner between the scrollbars
		if (verticalScrollBarVisible) {
			if (horizontalScrollBarVisible && (wrapState == eWrapNone)) {
				PRectangle rcCorner = wMain.GetClientPosition();
				rcCorner.left = rcCorner.right - scrollBarWidth + 1;
				rcCorner.top = rcCorner.bottom - scrollBarHeight + 1;
				//fprintf(stderr, "Corner %0d,%0d %0d,%0d\n",
				//rcCorner.left, rcCorner.top, rcCorner.right, rcCorner.bottom);
				surfaceWindow->FillRectangle(rcCorner,
					vs.styles[STYLE_LINENUMBER].back.allocated);
			}
		}

		//Paint(surfaceWindow, rcPaint);
		surfaceWindow->Release();
		delete surfaceWindow;
	}
	if (paintState == paintAbandoned) {
		// Painting area was insufficient to cover new styling or brace highlight positions
		FullPaint();
	}
	paintState = notPainting;

#else
	// For GTK+ 2, the text is painted in ExposeText
	gtk_container_propagate_expose(
		GTK_CONTAINER(PWidget(wMain)), PWidget(scrollbarh), ose);
	gtk_container_propagate_expose(
		GTK_CONTAINER(PWidget(wMain)), PWidget(scrollbarv), ose);
#endif

	return FALSE;
}

void ScintillaGTK::ScrollSignal(GtkAdjustment *adj, ScintillaGTK *sciThis) {
	sciThis->ScrollTo(static_cast<int>(adj->value), false);
}

void ScintillaGTK::ScrollHSignal(GtkAdjustment *adj, ScintillaGTK *sciThis) {
	sciThis->HorizontalScrollTo(static_cast<int>(adj->value * 2));
}

void ScintillaGTK::SelectionReceived(GtkWidget *widget,
                                     GtkSelectionData *selection_data, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Selection received\n");
	sciThis->ReceivedSelection(selection_data);
}

void ScintillaGTK::SelectionGet(GtkWidget *widget,
                                GtkSelectionData *selection_data, guint info, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Selection get\n");
	if (selection_data->selection == GDK_SELECTION_PRIMARY) {
		if (sciThis->primary.s == NULL) {
			sciThis->CopySelectionRange(&sciThis->primary);
		}
		sciThis->GetSelection(selection_data, info, &sciThis->primary);
	}
#ifndef USE_GTK_CLIPBOARD
	else {
		sciThis->GetSelection(selection_data, info, &sciThis->copyText);
	}
#endif
}

gint ScintillaGTK::SelectionClear(GtkWidget *widget, GdkEventSelection *selection_event) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Selection clear\n");
	sciThis->UnclaimSelection(selection_event);
	return gtk_selection_clear(widget, selection_event);
}

#if GTK_MAJOR_VERSION < 2
gint ScintillaGTK::SelectionNotify(GtkWidget *widget, GdkEventSelection *selection_event) {
	//Platform::DebugPrintf("Selection notify\n");
	return gtk_selection_notify(widget, selection_event);
}
#endif

void ScintillaGTK::DragBegin(GtkWidget *, GdkDragContext *) {
	//Platform::DebugPrintf("DragBegin\n");
}

gboolean ScintillaGTK::DragMotionThis(GdkDragContext *context,
                                 gint x, gint y, guint dragtime) {
	Point npt(x, y);
	SetDragPosition(PositionFromLocation(npt));
	GdkDragAction preferredAction = context->suggested_action;
	int pos = PositionFromLocation(npt);
	if ((inDragDrop == ddDragging) && (0 == PositionInSelection(pos))) {
		// Avoid dragging selection onto itself as that produces a move
		// with no real effect but which creates undo actions.
		preferredAction = static_cast<GdkDragAction>(0);
	} else if (context->actions == static_cast<GdkDragAction>
		(GDK_ACTION_COPY | GDK_ACTION_MOVE)) {
		preferredAction = GDK_ACTION_MOVE;
	}
	gdk_drag_status(context, preferredAction, dragtime);
	return FALSE;
}

gboolean ScintillaGTK::DragMotion(GtkWidget *widget, GdkDragContext *context,
                                 gint x, gint y, guint dragtime) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	return sciThis->DragMotionThis(context, x, y, dragtime);
}

void ScintillaGTK::DragLeave(GtkWidget *widget, GdkDragContext * /*context*/, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->SetDragPosition(invalidPosition);
	//Platform::DebugPrintf("DragLeave %x\n", sciThis);
}

void ScintillaGTK::DragEnd(GtkWidget *widget, GdkDragContext * /*context*/) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	// If drag did not result in drop here or elsewhere
	if (!sciThis->dragWasDropped)
		sciThis->SetEmptySelection(sciThis->posDrag);
	sciThis->SetDragPosition(invalidPosition);
	//Platform::DebugPrintf("DragEnd %x %d\n", sciThis, sciThis->dragWasDropped);
	sciThis->inDragDrop = ddNone;
}

gboolean ScintillaGTK::Drop(GtkWidget *widget, GdkDragContext * /*context*/,
                            gint, gint, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Drop %x\n", sciThis);
	sciThis->SetDragPosition(invalidPosition);
	return FALSE;
}

void ScintillaGTK::DragDataReceived(GtkWidget *widget, GdkDragContext * /*context*/,
                                    gint, gint, GtkSelectionData *selection_data, guint /*info*/, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->ReceivedDrop(selection_data);
	sciThis->SetDragPosition(invalidPosition);
}

void ScintillaGTK::DragDataGet(GtkWidget *widget, GdkDragContext *context,
                               GtkSelectionData *selection_data, guint info, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->dragWasDropped = true;
	if (sciThis->currentPos != sciThis->anchor) {
		sciThis->GetSelection(selection_data, info, &sciThis->drag);
	}
	if (context->action == GDK_ACTION_MOVE) {
		int selStart = sciThis->SelectionStart();
		int selEnd = sciThis->SelectionEnd();
		if (sciThis->posDrop > selStart) {
			if (sciThis->posDrop > selEnd)
				sciThis->posDrop = sciThis->posDrop - (selEnd - selStart);
			else
				sciThis->posDrop = selStart;
			sciThis->posDrop = sciThis->pdoc->ClampPositionIntoDocument(sciThis->posDrop);
		}
		sciThis->ClearSelection();
	}
	sciThis->SetDragPosition(invalidPosition);
}

int ScintillaGTK::TimeOut(ScintillaGTK *sciThis) {
	sciThis->Tick();
	return 1;
}

int ScintillaGTK::IdleCallback(ScintillaGTK *sciThis) {
	// Idler will be automatically stoped, if there is nothing
	// to do while idle.
	bool ret = sciThis->Idle();
	if (ret == false) {
		// FIXME: This will remove the idler from GTK, we don't want to
		// remove it as it is removed automatically when this function
		// returns false (although, it should be harmless).
		sciThis->SetIdle(false);
	}
	return ret;
}

void ScintillaGTK::PopUpCB(ScintillaGTK *sciThis, guint action, GtkWidget *) {
	if (action) {
		sciThis->Command(action);
	}
}

gint ScintillaGTK::PressCT(GtkWidget *widget, GdkEventButton *event, ScintillaGTK *sciThis) {
	if (event->window != widget->window)
		return FALSE;
	if (event->type != GDK_BUTTON_PRESS)
		return FALSE;
	Point pt;
	pt.x = int(event->x);
	pt.y = int(event->y);
	sciThis->ct.MouseClick(pt);
	sciThis->CallTipClick();
#if GTK_MAJOR_VERSION >= 2
	return TRUE;
#else
	return FALSE;
#endif
}

gint ScintillaGTK::ExposeCT(GtkWidget *widget, GdkEventExpose * /*ose*/, CallTip *ctip) {
	Surface *surfaceWindow = Surface::Allocate();
	if (surfaceWindow) {
		surfaceWindow->Init(widget->window, widget);
		ctip->PaintCT(surfaceWindow);
		surfaceWindow->Release();
		delete surfaceWindow;
	}
	return TRUE;
}

sptr_t ScintillaGTK::DirectFunction(
    ScintillaGTK *sciThis, unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	return sciThis->WndProc(iMessage, wParam, lParam);
}

sptr_t scintilla_send_message(ScintillaObject *sci, unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	ScintillaGTK *psci = reinterpret_cast<ScintillaGTK *>(sci->pscin);
	return psci->WndProc(iMessage, wParam, lParam);
}

static void scintilla_class_init(ScintillaClass *klass);
static void scintilla_init(ScintillaObject *sci);

extern void Platform_Initialise();
extern void Platform_Finalise();

#if GLIB_MAJOR_VERSION < 2
GtkType scintilla_get_type() {
	static GtkType scintilla_type = 0;

	if (!scintilla_type) {
		Platform_Initialise();
		static GtkTypeInfo scintilla_info = {
		    "Scintilla",
		    sizeof (ScintillaObject),
		    sizeof (ScintillaClass),
		    (GtkClassInitFunc) scintilla_class_init,
		    (GtkObjectInitFunc) scintilla_init,
		    (gpointer) NULL,
		    (gpointer) NULL,
		    0
		};

		scintilla_type = gtk_type_unique(gtk_container_get_type(), &scintilla_info);
	}

	return scintilla_type;
}
#else
GType scintilla_get_type() {
	static GType scintilla_type = 0;

	if (!scintilla_type) {
		scintilla_type = g_type_from_name("Scintilla");
		if (!scintilla_type) {
			static GTypeInfo scintilla_info = {
				(guint16) sizeof (ScintillaClass),
				NULL, //(GBaseInitFunc)
				NULL, //(GBaseFinalizeFunc)
				(GClassInitFunc) scintilla_class_init,
				NULL, //(GClassFinalizeFunc)
				NULL, //gconstpointer data
				(guint16) sizeof (ScintillaObject),
				0, //n_preallocs
				(GInstanceInitFunc) scintilla_init,
				NULL //(GTypeValueTable*)
			};

			scintilla_type = g_type_register_static(
				GTK_TYPE_CONTAINER, "Scintilla", &scintilla_info, (GTypeFlags) 0);
		}
	}

	return scintilla_type;
}
#endif

void ScintillaGTK::ClassInit(OBJECT_CLASS* object_class, GtkWidgetClass *widget_class, GtkContainerClass *container_class) {
#if GLIB_MAJOR_VERSION >= 2
	Platform_Initialise();
#endif
	atomClipboard = gdk_atom_intern("CLIPBOARD", FALSE);
	atomUTF8 = gdk_atom_intern("UTF8_STRING", FALSE);
	atomString = GDK_SELECTION_TYPE_STRING;
	atomUriList = gdk_atom_intern("text/uri-list", FALSE);
	atomDROPFILES_DND = gdk_atom_intern("DROPFILES_DND", FALSE);

	// Define default signal handlers for the class:  Could move more
	// of the signal handlers here (those that currently attached to wDraw
	// in Initialise() may require coordinate translation?)

#if GLIB_MAJOR_VERSION < 2
	object_class->destroy = Destroy;
#else
	object_class->finalize = Destroy;
#endif
	widget_class->size_request = SizeRequest;
	widget_class->size_allocate = SizeAllocate;
	widget_class->expose_event = ExposeMain;
#if GTK_MAJOR_VERSION < 2
	widget_class->draw = Draw;
#endif
	widget_class->motion_notify_event = Motion;
	widget_class->button_press_event = Press;
	widget_class->button_release_event = MouseRelease;
#if PLAT_GTK_WIN32 || (GTK_MAJOR_VERSION >= 2)
	widget_class->scroll_event = ScrollEvent;
#endif
	widget_class->key_press_event = KeyPress;
	widget_class->key_release_event = KeyRelease;
	widget_class->focus_in_event = FocusIn;
	widget_class->focus_out_event = FocusOut;
	widget_class->selection_received = SelectionReceived;
	widget_class->selection_get = SelectionGet;
	widget_class->selection_clear_event = SelectionClear;
#if GTK_MAJOR_VERSION < 2
	widget_class->selection_notify_event = SelectionNotify;
#endif

	widget_class->drag_data_received = DragDataReceived;
	widget_class->drag_motion = DragMotion;
	widget_class->drag_leave = DragLeave;
	widget_class->drag_end = DragEnd;
	widget_class->drag_drop = Drop;
	widget_class->drag_data_get = DragDataGet;

	widget_class->realize = Realize;
	widget_class->unrealize = UnRealize;
	widget_class->map = Map;
	widget_class->unmap = UnMap;

	container_class->forall = MainForAll;
}

#if GLIB_MAJOR_VERSION < 2
#define GTK_CLASS_TYPE(c) (c->type)
#define SIG_MARSHAL gtk_marshal_NONE__INT_POINTER
#define MARSHAL_ARGUMENTS GTK_TYPE_INT, GTK_TYPE_POINTER
#else
#define SIG_MARSHAL scintilla_marshal_NONE__INT_POINTER
#define MARSHAL_ARGUMENTS G_TYPE_INT, G_TYPE_POINTER
#endif

static void scintilla_class_init(ScintillaClass *klass) {
	OBJECT_CLASS *object_class = (OBJECT_CLASS*) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass*) klass;
	GtkContainerClass *container_class = (GtkContainerClass*) klass;

#if GLIB_MAJOR_VERSION < 2
	parent_class = (GtkWidgetClass*) gtk_type_class(gtk_container_get_type());

	scintilla_signals[COMMAND_SIGNAL] = gtk_signal_new(
	                                        "command",
	                                        GTK_RUN_LAST,
	                                        GTK_CLASS_TYPE(object_class),
	                                        GTK_SIGNAL_OFFSET(ScintillaClass, command),
	                                        SIG_MARSHAL,
	                                        GTK_TYPE_NONE,
	                                        2, MARSHAL_ARGUMENTS);

	scintilla_signals[NOTIFY_SIGNAL] = gtk_signal_new(
	                                       SCINTILLA_NOTIFY,
	                                       GTK_RUN_LAST,
	                                       GTK_CLASS_TYPE(object_class),
	                                       GTK_SIGNAL_OFFSET(ScintillaClass, notify),
	                                       SIG_MARSHAL,
	                                       GTK_TYPE_NONE,
	                                       2, MARSHAL_ARGUMENTS);
	gtk_object_class_add_signals(object_class,
	                             reinterpret_cast<unsigned int *>(scintilla_signals), LAST_SIGNAL);
#else
	GSignalFlags sigflags = GSignalFlags(G_SIGNAL_ACTION | G_SIGNAL_RUN_LAST);
	scintilla_signals[COMMAND_SIGNAL] = g_signal_new(
	                                       "command",
	                                       G_TYPE_FROM_CLASS(object_class),
	                                       sigflags,
	                                       G_STRUCT_OFFSET(ScintillaClass, command),
	                                       NULL, //(GSignalAccumulator)
	                                       NULL, //(gpointer)
	                                       SIG_MARSHAL,
	                                       G_TYPE_NONE,
	                                       2, MARSHAL_ARGUMENTS);

	scintilla_signals[NOTIFY_SIGNAL] = g_signal_new(
	                                       SCINTILLA_NOTIFY,
	                                       G_TYPE_FROM_CLASS(object_class),
	                                       sigflags,
	                                       G_STRUCT_OFFSET(ScintillaClass, notify),
	                                       NULL,
	                                       NULL,
	                                       SIG_MARSHAL,
	                                       G_TYPE_NONE,
	                                       2, MARSHAL_ARGUMENTS);
#endif
	klass->command = NULL;
	klass->notify = NULL;

	ScintillaGTK::ClassInit(object_class, widget_class, container_class);
}

static void scintilla_init(ScintillaObject *sci) {
	GTK_WIDGET_SET_FLAGS(sci, GTK_CAN_FOCUS);
	sci->pscin = new ScintillaGTK(sci);
}

GtkWidget* scintilla_new() {
#if GLIB_MAJOR_VERSION < 2
	return GTK_WIDGET(gtk_type_new(scintilla_get_type()));
#else
	return GTK_WIDGET(g_object_new(scintilla_get_type(), NULL));
#endif
}

void scintilla_set_id(ScintillaObject *sci, uptr_t id) {
	ScintillaGTK *psci = reinterpret_cast<ScintillaGTK *>(sci->pscin);
	psci->ctrlID = id;
}

void scintilla_release_resources(void) {
	Platform_Finalise();
}
