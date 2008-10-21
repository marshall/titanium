/* void addText(in long length, in wstring text); */
void SciMoz::AddText(int32 length, NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AddText\n");
#endif
	SendEditor(SCI_ADDTEXT, length, reinterpret_cast<long>(NPStringToString(text));
}

/* void addStyledText(in long length, in string c); */
void SciMoz::AddStyledText(int32 length, const char *c) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AddStyledText\n");
#endif
	SendEditor(SCI_ADDSTYLEDTEXT, length, reinterpret_cast<long>(c));
}

/* void insertText(in long pos, in wstring text); */
void SciMoz::InsertText(int32 pos, NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::InsertText\n");
#endif
	SendEditor(SCI_INSERTTEXT, pos, reinterpret_cast<long>(NPStringToString(text));
}

/* void clearAll(); */
void SciMoz::ClearAll() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ClearAll\n");
#endif
	SendEditor(SCI_CLEARALL, 0, 0);
}

/* void clearDocumentStyle(); */
void SciMoz::ClearDocumentStyle() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ClearDocumentStyle\n");
#endif
	SendEditor(SCI_CLEARDOCUMENTSTYLE, 0, 0);
}

/* readonly attribute long length; */
int32  SciMoz::GetLength() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLength\n");
#endif
	return SendEditor(SCI_GETLENGTH, 0, 0);
}

/* long getCharAt(in long pos); */
int32  SciMoz::GetCharAt(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCharAt\n");
#endif
	return SendEditor(SCI_GETCHARAT, pos, 0);
}

/* attribute long currentPos; */
int32  SciMoz::GetCurrentPos() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCurrentPos\n");
#endif
	return SendEditor(SCI_GETCURRENTPOS, 0, 0);
}

/* attribute long anchor; */
int32  SciMoz::GetAnchor() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetAnchor\n");
#endif
	return SendEditor(SCI_GETANCHOR, 0, 0);
}

/* long getStyleAt(in long pos); */
int32  SciMoz::GetStyleAt(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetStyleAt\n");
#endif
	return SendEditor(SCI_GETSTYLEAT, pos, 0);
}

/* void redo(); */
void SciMoz::Redo() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::Redo\n");
#endif
	SendEditor(SCI_REDO, 0, 0);
}

/* attribute boolean undoCollection; */
void SciMoz::SetUndoCollection(bool collectUndo) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetUndoCollection\n");
#endif
	SendEditor(SCI_SETUNDOCOLLECTION, collectUndo, 0);
}

/* void selectAll(); */
void SciMoz::SelectAll() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SelectAll\n");
#endif
	SendEditor(SCI_SELECTALL, 0, 0);
}

/* void setSavePoint(); */
void SciMoz::SetSavePoint() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetSavePoint\n");
#endif
	SendEditor(SCI_SETSAVEPOINT, 0, 0);
}

/* boolean canRedo(); */
bool  SciMoz::CanRedo() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CanRedo\n");
#endif
	return SendEditor(SCI_CANREDO, 0, 0);
}

/* long markerLineFromHandle(in long handle); */
int32  SciMoz::MarkerLineFromHandle(int32 handle) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerLineFromHandle\n");
#endif
	return SendEditor(SCI_MARKERLINEFROMHANDLE, handle, 0);
}

/* void markerDeleteHandle(in long handle); */
void SciMoz::MarkerDeleteHandle(int32 handle) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerDeleteHandle\n");
#endif
	SendEditor(SCI_MARKERDELETEHANDLE, handle, 0);
}

/* attribute boolean undoCollection; */
bool  SciMoz::GetUndoCollection() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetUndoCollection\n");
#endif
	return SendEditor(SCI_GETUNDOCOLLECTION, 0, 0);
}

/* attribute long viewWS; */
int32  SciMoz::GetViewWS() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetViewWS\n");
#endif
	return SendEditor(SCI_GETVIEWWS, 0, 0);
}

/* attribute long viewWS; */
void SciMoz::SetViewWS(int32 viewWS) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetViewWS\n");
#endif
	SendEditor(SCI_SETVIEWWS, viewWS, 0);
}

/* long positionFromPoint(in long x, in long y); */
int32  SciMoz::PositionFromPoint(int32 x, int32 y) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PositionFromPoint\n");
#endif
	return SendEditor(SCI_POSITIONFROMPOINT, x, y);
}

/* long positionFromPointClose(in long x, in long y); */
int32  SciMoz::PositionFromPointClose(int32 x, int32 y) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PositionFromPointClose\n");
#endif
	return SendEditor(SCI_POSITIONFROMPOINTCLOSE, x, y);
}

/* void gotoLine(in long line); */
void SciMoz::GotoLine(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GotoLine\n");
#endif
	SendEditor(SCI_GOTOLINE, line, 0);
}

/* void gotoPos(in long pos); */
void SciMoz::GotoPos(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GotoPos\n");
#endif
	SendEditor(SCI_GOTOPOS, pos, 0);
}

/* attribute long anchor; */
void SciMoz::SetAnchor(int32 posAnchor) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetAnchor\n");
#endif
	SendEditor(SCI_SETANCHOR, posAnchor, 0);
}

/* readonly attribute long endStyled; */
int32  SciMoz::GetEndStyled() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetEndStyled\n");
#endif
	return SendEditor(SCI_GETENDSTYLED, 0, 0);
}

/* void convertEOLs(in long eolMode); */
void SciMoz::ConvertEOLs(int32 eolMode) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ConvertEOLs\n");
#endif
	SendEditor(SCI_CONVERTEOLS, eolMode, 0);
}

/* attribute long eOLMode; */
int32  SciMoz::GetEOLMode() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetEOLMode\n");
#endif
	return SendEditor(SCI_GETEOLMODE, 0, 0);
}

/* attribute long eOLMode; */
void SciMoz::SetEOLMode(int32 eolMode) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetEOLMode\n");
#endif
	SendEditor(SCI_SETEOLMODE, eolMode, 0);
}

/* void startStyling(in long pos, in long mask); */
void SciMoz::StartStyling(int32 pos, int32 mask) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StartStyling\n");
#endif
	SendEditor(SCI_STARTSTYLING, pos, mask);
}

/* void setStyling(in long length, in long style); */
void SciMoz::SetStyling(int32 length, int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetStyling\n");
#endif
	SendEditor(SCI_SETSTYLING, length, style);
}

/* attribute boolean bufferedDraw; */
bool  SciMoz::GetBufferedDraw() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetBufferedDraw\n");
#endif
	return SendEditor(SCI_GETBUFFEREDDRAW, 0, 0);
}

/* attribute boolean bufferedDraw; */
void SciMoz::SetBufferedDraw(bool buffered) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetBufferedDraw\n");
#endif
	SendEditor(SCI_SETBUFFEREDDRAW, buffered, 0);
}

/* attribute long tabWidth; */
void SciMoz::SetTabWidth(int32 tabWidth) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetTabWidth\n");
#endif
	SendEditor(SCI_SETTABWIDTH, tabWidth, 0);
}

/* attribute long tabWidth; */
int32  SciMoz::GetTabWidth() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetTabWidth\n");
#endif
	return SendEditor(SCI_GETTABWIDTH, 0, 0);
}

/* attribute long codePage; */
void SciMoz::SetCodePage(int32 codePage) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCodePage\n");
#endif
	SendEditor(SCI_SETCODEPAGE, codePage, 0);
}

/* attribute boolean usePalette; */
void SciMoz::SetUsePalette(bool usePalette) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetUsePalette\n");
#endif
	SendEditor(SCI_SETUSEPALETTE, usePalette, 0);
}

/* void markerDefine(in long markerNumber, in long markerSymbol); */
void SciMoz::MarkerDefine(int32 markerNumber, int32 markerSymbol) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerDefine\n");
#endif
	SendEditor(SCI_MARKERDEFINE, markerNumber, markerSymbol);
}

/* void markerSetFore(in long markerNumber, in long fore); */
void SciMoz::MarkerSetFore(int32 markerNumber, int32 fore) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerSetFore\n");
#endif
	SendEditor(SCI_MARKERSETFORE, markerNumber, fore);
}

/* void markerSetBack(in long markerNumber, in long back); */
void SciMoz::MarkerSetBack(int32 markerNumber, int32 back) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerSetBack\n");
#endif
	SendEditor(SCI_MARKERSETBACK, markerNumber, back);
}

/* long markerAdd(in long line, in long markerNumber); */
int32  SciMoz::MarkerAdd(int32 line, int32 markerNumber) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerAdd\n");
#endif
	return SendEditor(SCI_MARKERADD, line, markerNumber);
}

/* void markerDelete(in long line, in long markerNumber); */
void SciMoz::MarkerDelete(int32 line, int32 markerNumber) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerDelete\n");
#endif
	SendEditor(SCI_MARKERDELETE, line, markerNumber);
}

/* void markerDeleteAll(in long markerNumber); */
void SciMoz::MarkerDeleteAll(int32 markerNumber) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerDeleteAll\n");
#endif
	SendEditor(SCI_MARKERDELETEALL, markerNumber, 0);
}

/* long markerGet(in long line); */
int32  SciMoz::MarkerGet(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerGet\n");
#endif
	return SendEditor(SCI_MARKERGET, line, 0);
}

/* long markerNext(in long lineStart, in long markerMask); */
int32  SciMoz::MarkerNext(int32 lineStart, int32 markerMask) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerNext\n");
#endif
	return SendEditor(SCI_MARKERNEXT, lineStart, markerMask);
}

/* long markerPrevious(in long lineStart, in long markerMask); */
int32  SciMoz::MarkerPrevious(int32 lineStart, int32 markerMask) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerPrevious\n");
#endif
	return SendEditor(SCI_MARKERPREVIOUS, lineStart, markerMask);
}

/* void markerDefinePixmap(in long markerNumber, in wstring pixmap); */
void SciMoz::MarkerDefinePixmap(int32 markerNumber, NPStringpixmap) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerDefinePixmap\n");
#endif
	SendEditor(SCI_MARKERDEFINEPIXMAP, markerNumber, reinterpret_cast<long>(NPStringToString(pixmap));
}

/* void markerAddSet(in long line, in long set); */
void SciMoz::MarkerAddSet(int32 line, int32 set) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerAddSet\n");
#endif
	SendEditor(SCI_MARKERADDSET, line, set);
}

/* void markerSetAlpha(in long markerNumber, in long alpha); */
void SciMoz::MarkerSetAlpha(int32 markerNumber, int32 alpha) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MarkerSetAlpha\n");
#endif
	SendEditor(SCI_MARKERSETALPHA, markerNumber, alpha);
}

/* void setMarginTypeN(in long margin, in long marginType); */
void SciMoz::SetMarginTypeN(int32 margin, int32 marginType) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetMarginTypeN\n");
#endif
	SendEditor(SCI_SETMARGINTYPEN, margin, marginType);
}

/* long getMarginTypeN(in long margin); */
int32  SciMoz::GetMarginTypeN(int32 margin) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetMarginTypeN\n");
#endif
	return SendEditor(SCI_GETMARGINTYPEN, margin, 0);
}

/* void setMarginWidthN(in long margin, in long pixelWidth); */
void SciMoz::SetMarginWidthN(int32 margin, int32 pixelWidth) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetMarginWidthN\n");
#endif
	SendEditor(SCI_SETMARGINWIDTHN, margin, pixelWidth);
}

/* long getMarginWidthN(in long margin); */
int32  SciMoz::GetMarginWidthN(int32 margin) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetMarginWidthN\n");
#endif
	return SendEditor(SCI_GETMARGINWIDTHN, margin, 0);
}

/* void setMarginMaskN(in long margin, in long mask); */
void SciMoz::SetMarginMaskN(int32 margin, int32 mask) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetMarginMaskN\n");
#endif
	SendEditor(SCI_SETMARGINMASKN, margin, mask);
}

/* long getMarginMaskN(in long margin); */
int32  SciMoz::GetMarginMaskN(int32 margin) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetMarginMaskN\n");
#endif
	return SendEditor(SCI_GETMARGINMASKN, margin, 0);
}

/* void setMarginSensitiveN(in long margin, in boolean sensitive); */
void SciMoz::SetMarginSensitiveN(int32 margin, bool sensitive) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetMarginSensitiveN\n");
#endif
	SendEditor(SCI_SETMARGINSENSITIVEN, margin, sensitive);
}

/* boolean getMarginSensitiveN(in long margin); */
bool  SciMoz::GetMarginSensitiveN(int32 margin) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetMarginSensitiveN\n");
#endif
	return SendEditor(SCI_GETMARGINSENSITIVEN, margin, 0);
}

/* void styleClearAll(); */
void SciMoz::StyleClearAll() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleClearAll\n");
#endif
	SendEditor(SCI_STYLECLEARALL, 0, 0);
}

/* void styleSetFore(in long style, in long fore); */
void SciMoz::StyleSetFore(int32 style, int32 fore) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetFore\n");
#endif
	SendEditor(SCI_STYLESETFORE, style, fore);
}

/* void styleSetBack(in long style, in long back); */
void SciMoz::StyleSetBack(int32 style, int32 back) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetBack\n");
#endif
	SendEditor(SCI_STYLESETBACK, style, back);
}

/* void styleSetBold(in long style, in boolean bold); */
void SciMoz::StyleSetBold(int32 style, bool bold) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetBold\n");
#endif
	SendEditor(SCI_STYLESETBOLD, style, bold);
}

/* void styleSetItalic(in long style, in boolean italic); */
void SciMoz::StyleSetItalic(int32 style, bool italic) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetItalic\n");
#endif
	SendEditor(SCI_STYLESETITALIC, style, italic);
}

/* void styleSetSize(in long style, in long sizePoints); */
void SciMoz::StyleSetSize(int32 style, int32 sizePoints) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetSize\n");
#endif
	SendEditor(SCI_STYLESETSIZE, style, sizePoints);
}

/* void styleSetFont(in long style, in wstring fontName); */
void SciMoz::StyleSetFont(int32 style, NPStringfontName) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetFont\n");
#endif
	SendEditor(SCI_STYLESETFONT, style, reinterpret_cast<long>(NPStringToString(fontName));
}

/* void styleSetEOLFilled(in long style, in boolean filled); */
void SciMoz::StyleSetEOLFilled(int32 style, bool filled) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetEOLFilled\n");
#endif
	SendEditor(SCI_STYLESETEOLFILLED, style, filled);
}

/* void styleResetDefault(); */
void SciMoz::StyleResetDefault() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleResetDefault\n");
#endif
	SendEditor(SCI_STYLERESETDEFAULT, 0, 0);
}

/* void styleSetUnderline(in long style, in boolean underline); */
void SciMoz::StyleSetUnderline(int32 style, bool underline) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetUnderline\n");
#endif
	SendEditor(SCI_STYLESETUNDERLINE, style, underline);
}

/* long styleGetFore(in long style); */
int32  SciMoz::StyleGetFore(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetFore\n");
#endif
	return SendEditor(SCI_STYLEGETFORE, style, 0);
}

/* long styleGetBack(in long style); */
int32  SciMoz::StyleGetBack(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetBack\n");
#endif
	return SendEditor(SCI_STYLEGETBACK, style, 0);
}

/* boolean styleGetBold(in long style); */
bool  SciMoz::StyleGetBold(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetBold\n");
#endif
	return SendEditor(SCI_STYLEGETBOLD, style, 0);
}

/* boolean styleGetItalic(in long style); */
bool  SciMoz::StyleGetItalic(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetItalic\n");
#endif
	return SendEditor(SCI_STYLEGETITALIC, style, 0);
}

/* long styleGetSize(in long style); */
int32  SciMoz::StyleGetSize(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetSize\n");
#endif
	return SendEditor(SCI_STYLEGETSIZE, style, 0);
}

/* long styleGetFont(in long style, out wstring fontName); */
int32  SciMoz::StyleGetFont(int32 style, PRUnichar **fontName) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetFont\n");
#endif
	static char _buffer[32 * 1024];
/*#ifdef NS_DEBUG
	static PRThread *myThread = nsnull;
	if (myThread == nsnull)
		myThread = PR_GetCurrentThread();
	// If this fires, caller should be using a proxy!  Scintilla is not free-threaded!
	NS_PRECONDITION(PR_GetCurrentThread()==myThread, "buffer (and Scintilla!) is not thread-safe!!!!");
#endif */ // NS_DEBUG
	_buffer[32 * 1024-1] = '\0';						short _buflen = static_cast<short>(sizeof(_buffer)-1);
	memcpy(_buffer, &_buflen, sizeof(_buflen));
	return SendEditor(SCI_STYLEGETFONT, style, reinterpret_cast<long>(_buffer));
	*fontName =  ToNewUnicode(NS_ConvertUTF8toUTF16(_buffer));
}

/* boolean styleGetEOLFilled(in long style); */
bool  SciMoz::StyleGetEOLFilled(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetEOLFilled\n");
#endif
	return SendEditor(SCI_STYLEGETEOLFILLED, style, 0);
}

/* boolean styleGetUnderline(in long style); */
bool  SciMoz::StyleGetUnderline(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetUnderline\n");
#endif
	return SendEditor(SCI_STYLEGETUNDERLINE, style, 0);
}

/* long styleGetCase(in long style); */
int32  SciMoz::StyleGetCase(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetCase\n");
#endif
	return SendEditor(SCI_STYLEGETCASE, style, 0);
}

/* long styleGetCharacterSet(in long style); */
int32  SciMoz::StyleGetCharacterSet(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetCharacterSet\n");
#endif
	return SendEditor(SCI_STYLEGETCHARACTERSET, style, 0);
}

/* boolean styleGetVisible(in long style); */
bool  SciMoz::StyleGetVisible(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetVisible\n");
#endif
	return SendEditor(SCI_STYLEGETVISIBLE, style, 0);
}

/* boolean styleGetChangeable(in long style); */
bool  SciMoz::StyleGetChangeable(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetChangeable\n");
#endif
	return SendEditor(SCI_STYLEGETCHANGEABLE, style, 0);
}

/* boolean styleGetHotSpot(in long style); */
bool  SciMoz::StyleGetHotSpot(int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleGetHotSpot\n");
#endif
	return SendEditor(SCI_STYLEGETHOTSPOT, style, 0);
}

/* void styleSetCase(in long style, in long caseForce); */
void SciMoz::StyleSetCase(int32 style, int32 caseForce) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetCase\n");
#endif
	SendEditor(SCI_STYLESETCASE, style, caseForce);
}

/* void styleSetCharacterSet(in long style, in long characterSet); */
void SciMoz::StyleSetCharacterSet(int32 style, int32 characterSet) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetCharacterSet\n");
#endif
	SendEditor(SCI_STYLESETCHARACTERSET, style, characterSet);
}

/* void styleSetHotSpot(in long style, in boolean hotspot); */
void SciMoz::StyleSetHotSpot(int32 style, bool hotspot) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetHotSpot\n");
#endif
	SendEditor(SCI_STYLESETHOTSPOT, style, hotspot);
}

/* void setSelFore(in boolean useSetting, in long fore); */
void SciMoz::SetSelFore(bool useSetting, int32 fore) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetSelFore\n");
#endif
	SendEditor(SCI_SETSELFORE, useSetting, fore);
}

/* void setSelBack(in boolean useSetting, in long back); */
void SciMoz::SetSelBack(bool useSetting, int32 back) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetSelBack\n");
#endif
	SendEditor(SCI_SETSELBACK, useSetting, back);
}

/* attribute long selAlpha; */
int32  SciMoz::GetSelAlpha() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetSelAlpha\n");
#endif
	return SendEditor(SCI_GETSELALPHA, 0, 0);
}

/* attribute long selAlpha; */
void SciMoz::SetSelAlpha(int32 alpha) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetSelAlpha\n");
#endif
	SendEditor(SCI_SETSELALPHA, alpha, 0);
}

/* attribute boolean selEOLFilled; */
bool  SciMoz::GetSelEOLFilled() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetSelEOLFilled\n");
#endif
	return SendEditor(SCI_GETSELEOLFILLED, 0, 0);
}

/* attribute boolean selEOLFilled; */
void SciMoz::SetSelEOLFilled(bool filled) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetSelEOLFilled\n");
#endif
	SendEditor(SCI_SETSELEOLFILLED, filled, 0);
}

/* attribute long caretFore; */
void SciMoz::SetCaretFore(int32 fore) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCaretFore\n");
#endif
	SendEditor(SCI_SETCARETFORE, fore, 0);
}

/* void clearAllCmdKeys(); */
void SciMoz::ClearAllCmdKeys() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ClearAllCmdKeys\n");
#endif
	SendEditor(SCI_CLEARALLCMDKEYS, 0, 0);
}

/* void setStylingEx(in long length, in wstring styles); */
void SciMoz::SetStylingEx(int32 length, NPStringstyles) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetStylingEx\n");
#endif
	SendEditor(SCI_SETSTYLINGEX, length, reinterpret_cast<long>(NPStringToString(styles));
}

/* void styleSetVisible(in long style, in boolean visible); */
void SciMoz::StyleSetVisible(int32 style, bool visible) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetVisible\n");
#endif
	SendEditor(SCI_STYLESETVISIBLE, style, visible);
}

/* attribute long caretPeriod; */
int32  SciMoz::GetCaretPeriod() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCaretPeriod\n");
#endif
	return SendEditor(SCI_GETCARETPERIOD, 0, 0);
}

/* attribute long caretPeriod; */
void SciMoz::SetCaretPeriod(int32 periodMilliseconds) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCaretPeriod\n");
#endif
	SendEditor(SCI_SETCARETPERIOD, periodMilliseconds, 0);
}

/* void setWordChars(in wstring characters); */
void SciMoz::SetWordChars(NPStringcharacters) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetWordChars\n");
#endif
	SendEditor(SCI_SETWORDCHARS, 0, reinterpret_cast<long>(NPStringToString(characters));
}

/* void beginUndoAction(); */
void SciMoz::BeginUndoAction() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::BeginUndoAction\n");
#endif
	SendEditor(SCI_BEGINUNDOACTION, 0, 0);
}

/* void endUndoAction(); */
void SciMoz::EndUndoAction() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::EndUndoAction\n");
#endif
	SendEditor(SCI_ENDUNDOACTION, 0, 0);
}

/* void indicSetStyle(in long indic, in long style); */
void SciMoz::IndicSetStyle(int32 indic, int32 style) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicSetStyle\n");
#endif
	SendEditor(SCI_INDICSETSTYLE, indic, style);
}

/* long indicGetStyle(in long indic); */
int32  SciMoz::IndicGetStyle(int32 indic) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicGetStyle\n");
#endif
	return SendEditor(SCI_INDICGETSTYLE, indic, 0);
}

/* void indicSetFore(in long indic, in long fore); */
void SciMoz::IndicSetFore(int32 indic, int32 fore) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicSetFore\n");
#endif
	SendEditor(SCI_INDICSETFORE, indic, fore);
}

/* long indicGetFore(in long indic); */
int32  SciMoz::IndicGetFore(int32 indic) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicGetFore\n");
#endif
	return SendEditor(SCI_INDICGETFORE, indic, 0);
}

/* void indicSetUnder(in long indic, in boolean under); */
void SciMoz::IndicSetUnder(int32 indic, bool under) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicSetUnder\n");
#endif
	SendEditor(SCI_INDICSETUNDER, indic, under);
}

/* boolean indicGetUnder(in long indic); */
bool  SciMoz::IndicGetUnder(int32 indic) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicGetUnder\n");
#endif
	return SendEditor(SCI_INDICGETUNDER, indic, 0);
}

/* void setWhitespaceFore(in boolean useSetting, in long fore); */
void SciMoz::SetWhitespaceFore(bool useSetting, int32 fore) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetWhitespaceFore\n");
#endif
	SendEditor(SCI_SETWHITESPACEFORE, useSetting, fore);
}

/* void setWhitespaceBack(in boolean useSetting, in long back); */
void SciMoz::SetWhitespaceBack(bool useSetting, int32 back) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetWhitespaceBack\n");
#endif
	SendEditor(SCI_SETWHITESPACEBACK, useSetting, back);
}

/* attribute long styleBits; */
void SciMoz::SetStyleBits(int32 bits) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetStyleBits\n");
#endif
	SendEditor(SCI_SETSTYLEBITS, bits, 0);
}

/* attribute long styleBits; */
int32  SciMoz::GetStyleBits() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetStyleBits\n");
#endif
	return SendEditor(SCI_GETSTYLEBITS, 0, 0);
}

/* void setLineState(in long line, in long state); */
void SciMoz::SetLineState(int32 line, int32 state) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetLineState\n");
#endif
	SendEditor(SCI_SETLINESTATE, line, state);
}

/* long getLineState(in long line); */
int32  SciMoz::GetLineState(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLineState\n");
#endif
	return SendEditor(SCI_GETLINESTATE, line, 0);
}

/* readonly attribute long maxLineState; */
int32  SciMoz::GetMaxLineState() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetMaxLineState\n");
#endif
	return SendEditor(SCI_GETMAXLINESTATE, 0, 0);
}

/* attribute boolean caretLineVisible; */
bool  SciMoz::GetCaretLineVisible() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCaretLineVisible\n");
#endif
	return SendEditor(SCI_GETCARETLINEVISIBLE, 0, 0);
}

/* attribute boolean caretLineVisible; */
void SciMoz::SetCaretLineVisible(bool show) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCaretLineVisible\n");
#endif
	SendEditor(SCI_SETCARETLINEVISIBLE, show, 0);
}

/* attribute long caretLineBack; */
int32  SciMoz::GetCaretLineBack() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCaretLineBack\n");
#endif
	return SendEditor(SCI_GETCARETLINEBACK, 0, 0);
}

/* attribute long caretLineBack; */
void SciMoz::SetCaretLineBack(int32 back) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCaretLineBack\n");
#endif
	SendEditor(SCI_SETCARETLINEBACK, back, 0);
}

/* void styleSetChangeable(in long style, in boolean changeable); */
void SciMoz::StyleSetChangeable(int32 style, bool changeable) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StyleSetChangeable\n");
#endif
	SendEditor(SCI_STYLESETCHANGEABLE, style, changeable);
}

/* void autoCShow(in long lenEntered, in wstring itemList); */
void SciMoz::AutoCShow(int32 lenEntered, NPStringitemList) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AutoCShow\n");
#endif
	SendEditor(SCI_AUTOCSHOW, lenEntered, reinterpret_cast<long>(NPStringToString(itemList));
}

/* void autoCCancel(); */
void SciMoz::AutoCCancel() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AutoCCancel\n");
#endif
	SendEditor(SCI_AUTOCCANCEL, 0, 0);
}

/* boolean autoCActive(); */
bool  SciMoz::AutoCActive() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AutoCActive\n");
#endif
	return SendEditor(SCI_AUTOCACTIVE, 0, 0);
}

/* long autoCPosStart(); */
int32  SciMoz::AutoCPosStart() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AutoCPosStart\n");
#endif
	return SendEditor(SCI_AUTOCPOSSTART, 0, 0);
}

/* void autoCComplete(); */
void SciMoz::AutoCComplete() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AutoCComplete\n");
#endif
	SendEditor(SCI_AUTOCCOMPLETE, 0, 0);
}

/* void autoCStops(in wstring characterSet); */
void SciMoz::AutoCStops(NPStringcharacterSet) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AutoCStops\n");
#endif
	SendEditor(SCI_AUTOCSTOPS, 0, reinterpret_cast<long>(NPStringToString(characterSet));
}

/* attribute long autoCSeparator; */
void SciMoz::SetAutoCSeparator(int32 separatorCharacter) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetAutoCSeparator\n");
#endif
	SendEditor(SCI_AUTOCSETSEPARATOR, separatorCharacter, 0);
}

/* attribute long autoCSeparator; */
int32  SciMoz::GetAutoCSeparator() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetAutoCSeparator\n");
#endif
	return SendEditor(SCI_AUTOCGETSEPARATOR, 0, 0);
}

/* void autoCSelect(in wstring text); */
void SciMoz::AutoCSelect(NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AutoCSelect\n");
#endif
	SendEditor(SCI_AUTOCSELECT, 0, reinterpret_cast<long>(NPStringToString(text));
}

/* attribute boolean autoCCancelAtStart; */
void SciMoz::SetAutoCCancelAtStart(bool cancel) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetAutoCCancelAtStart\n");
#endif
	SendEditor(SCI_AUTOCSETCANCELATSTART, cancel, 0);
}

/* attribute boolean autoCCancelAtStart; */
bool  SciMoz::GetAutoCCancelAtStart() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetAutoCCancelAtStart\n");
#endif
	return SendEditor(SCI_AUTOCGETCANCELATSTART, 0, 0);
}

/* void autoCSetFillUps(in wstring characterSet); */
void SciMoz::AutoCSetFillUps(NPStringcharacterSet) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AutoCSetFillUps\n");
#endif
	SendEditor(SCI_AUTOCSETFILLUPS, 0, reinterpret_cast<long>(NPStringToString(characterSet));
}

/* attribute boolean autoCChooseSingle; */
void SciMoz::SetAutoCChooseSingle(bool chooseSingle) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetAutoCChooseSingle\n");
#endif
	SendEditor(SCI_AUTOCSETCHOOSESINGLE, chooseSingle, 0);
}

/* attribute boolean autoCChooseSingle; */
bool  SciMoz::GetAutoCChooseSingle() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetAutoCChooseSingle\n");
#endif
	return SendEditor(SCI_AUTOCGETCHOOSESINGLE, 0, 0);
}

/* attribute boolean autoCIgnoreCase; */
void SciMoz::SetAutoCIgnoreCase(bool ignoreCase) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetAutoCIgnoreCase\n");
#endif
	SendEditor(SCI_AUTOCSETIGNORECASE, ignoreCase, 0);
}

/* attribute boolean autoCIgnoreCase; */
bool  SciMoz::GetAutoCIgnoreCase() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetAutoCIgnoreCase\n");
#endif
	return SendEditor(SCI_AUTOCGETIGNORECASE, 0, 0);
}

/* void userListShow(in long listType, in wstring itemList); */
void SciMoz::UserListShow(int32 listType, NPStringitemList) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::UserListShow\n");
#endif
	SendEditor(SCI_USERLISTSHOW, listType, reinterpret_cast<long>(NPStringToString(itemList));
}

/* attribute boolean autoCAutoHide; */
void SciMoz::SetAutoCAutoHide(bool autoHide) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetAutoCAutoHide\n");
#endif
	SendEditor(SCI_AUTOCSETAUTOHIDE, autoHide, 0);
}

/* attribute boolean autoCAutoHide; */
bool  SciMoz::GetAutoCAutoHide() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetAutoCAutoHide\n");
#endif
	return SendEditor(SCI_AUTOCGETAUTOHIDE, 0, 0);
}

/* attribute boolean autoCDropRestOfWord; */
void SciMoz::SetAutoCDropRestOfWord(bool dropRestOfWord) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetAutoCDropRestOfWord\n");
#endif
	SendEditor(SCI_AUTOCSETDROPRESTOFWORD, dropRestOfWord, 0);
}

/* attribute boolean autoCDropRestOfWord; */
bool  SciMoz::GetAutoCDropRestOfWord() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetAutoCDropRestOfWord\n");
#endif
	return SendEditor(SCI_AUTOCGETDROPRESTOFWORD, 0, 0);
}

/* void registerImage(in long type, in wstring xpmData); */
void SciMoz::RegisterImage(int32 type, NPStringxpmData) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::RegisterImage\n");
#endif
	SendEditor(SCI_REGISTERIMAGE, type, reinterpret_cast<long>(NPStringToString(xpmData));
}

/* void clearRegisteredImages(); */
void SciMoz::ClearRegisteredImages() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ClearRegisteredImages\n");
#endif
	SendEditor(SCI_CLEARREGISTEREDIMAGES, 0, 0);
}

/* attribute long autoCTypeSeparator; */
int32  SciMoz::GetAutoCTypeSeparator() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetAutoCTypeSeparator\n");
#endif
	return SendEditor(SCI_AUTOCGETTYPESEPARATOR, 0, 0);
}

/* attribute long autoCTypeSeparator; */
void SciMoz::SetAutoCTypeSeparator(int32 separatorCharacter) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetAutoCTypeSeparator\n");
#endif
	SendEditor(SCI_AUTOCSETTYPESEPARATOR, separatorCharacter, 0);
}

/* attribute long autoCMaxWidth; */
void SciMoz::SetAutoCMaxWidth(int32 characterCount) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetAutoCMaxWidth\n");
#endif
	SendEditor(SCI_AUTOCSETMAXWIDTH, characterCount, 0);
}

/* attribute long autoCMaxWidth; */
int32  SciMoz::GetAutoCMaxWidth() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetAutoCMaxWidth\n");
#endif
	return SendEditor(SCI_AUTOCGETMAXWIDTH, 0, 0);
}

/* attribute long autoCMaxHeight; */
void SciMoz::SetAutoCMaxHeight(int32 rowCount) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetAutoCMaxHeight\n");
#endif
	SendEditor(SCI_AUTOCSETMAXHEIGHT, rowCount, 0);
}

/* attribute long autoCMaxHeight; */
int32  SciMoz::GetAutoCMaxHeight() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetAutoCMaxHeight\n");
#endif
	return SendEditor(SCI_AUTOCGETMAXHEIGHT, 0, 0);
}

/* attribute long indent; */
void SciMoz::SetIndent(int32 indentSize) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetIndent\n");
#endif
	SendEditor(SCI_SETINDENT, indentSize, 0);
}

/* attribute long indent; */
int32  SciMoz::GetIndent() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetIndent\n");
#endif
	return SendEditor(SCI_GETINDENT, 0, 0);
}

/* attribute boolean useTabs; */
void SciMoz::SetUseTabs(bool useTabs) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetUseTabs\n");
#endif
	SendEditor(SCI_SETUSETABS, useTabs, 0);
}

/* attribute boolean useTabs; */
bool  SciMoz::GetUseTabs() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetUseTabs\n");
#endif
	return SendEditor(SCI_GETUSETABS, 0, 0);
}

/* void setLineIndentation(in long line, in long indentSize); */
void SciMoz::SetLineIndentation(int32 line, int32 indentSize) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetLineIndentation\n");
#endif
	SendEditor(SCI_SETLINEINDENTATION, line, indentSize);
}

/* long getLineIndentation(in long line); */
int32  SciMoz::GetLineIndentation(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLineIndentation\n");
#endif
	return SendEditor(SCI_GETLINEINDENTATION, line, 0);
}

/* long getLineIndentPosition(in long line); */
int32  SciMoz::GetLineIndentPosition(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLineIndentPosition\n");
#endif
	return SendEditor(SCI_GETLINEINDENTPOSITION, line, 0);
}

/* long getColumn(in long pos); */
int32  SciMoz::GetColumn(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetColumn\n");
#endif
	return SendEditor(SCI_GETCOLUMN, pos, 0);
}

/* attribute boolean hScrollBar; */
void SciMoz::SetHScrollBar(bool show) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetHScrollBar\n");
#endif
	SendEditor(SCI_SETHSCROLLBAR, show, 0);
}

/* attribute boolean hScrollBar; */
bool  SciMoz::GetHScrollBar() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetHScrollBar\n");
#endif
	return SendEditor(SCI_GETHSCROLLBAR, 0, 0);
}

/* attribute long indentationGuides; */
void SciMoz::SetIndentationGuides(int32 indentView) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetIndentationGuides\n");
#endif
	SendEditor(SCI_SETINDENTATIONGUIDES, indentView, 0);
}

/* attribute long indentationGuides; */
int32  SciMoz::GetIndentationGuides() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetIndentationGuides\n");
#endif
	return SendEditor(SCI_GETINDENTATIONGUIDES, 0, 0);
}

/* attribute long highlightGuide; */
void SciMoz::SetHighlightGuide(int32 column) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetHighlightGuide\n");
#endif
	SendEditor(SCI_SETHIGHLIGHTGUIDE, column, 0);
}

/* attribute long highlightGuide; */
int32  SciMoz::GetHighlightGuide() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetHighlightGuide\n");
#endif
	return SendEditor(SCI_GETHIGHLIGHTGUIDE, 0, 0);
}

/* long getLineEndPosition(in long line); */
int32  SciMoz::GetLineEndPosition(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLineEndPosition\n");
#endif
	return SendEditor(SCI_GETLINEENDPOSITION, line, 0);
}

/* attribute long codePage; */
int32  SciMoz::GetCodePage() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCodePage\n");
#endif
	return SendEditor(SCI_GETCODEPAGE, 0, 0);
}

/* attribute long caretFore; */
int32  SciMoz::GetCaretFore() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCaretFore\n");
#endif
	return SendEditor(SCI_GETCARETFORE, 0, 0);
}

/* attribute boolean usePalette; */
bool  SciMoz::GetUsePalette() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetUsePalette\n");
#endif
	return SendEditor(SCI_GETUSEPALETTE, 0, 0);
}

/* attribute boolean readOnly; */
bool  SciMoz::GetReadOnly() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetReadOnly\n");
#endif
	return SendEditor(SCI_GETREADONLY, 0, 0);
}

/* attribute long currentPos; */
void SciMoz::SetCurrentPos(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCurrentPos\n");
#endif
	SendEditor(SCI_SETCURRENTPOS, pos, 0);
}

/* attribute long selectionStart; */
void SciMoz::SetSelectionStart(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetSelectionStart\n");
#endif
	SendEditor(SCI_SETSELECTIONSTART, pos, 0);
}

/* attribute long selectionStart; */
int32  SciMoz::GetSelectionStart() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetSelectionStart\n");
#endif
	return SendEditor(SCI_GETSELECTIONSTART, 0, 0);
}

/* attribute long selectionEnd; */
void SciMoz::SetSelectionEnd(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetSelectionEnd\n");
#endif
	SendEditor(SCI_SETSELECTIONEND, pos, 0);
}

/* attribute long selectionEnd; */
int32  SciMoz::GetSelectionEnd() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetSelectionEnd\n");
#endif
	return SendEditor(SCI_GETSELECTIONEND, 0, 0);
}

/* attribute long printMagnification; */
void SciMoz::SetPrintMagnification(int32 magnification) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetPrintMagnification\n");
#endif
	SendEditor(SCI_SETPRINTMAGNIFICATION, magnification, 0);
}

/* attribute long printMagnification; */
int32  SciMoz::GetPrintMagnification() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetPrintMagnification\n");
#endif
	return SendEditor(SCI_GETPRINTMAGNIFICATION, 0, 0);
}

/* attribute long printColourMode; */
void SciMoz::SetPrintColourMode(int32 mode) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetPrintColourMode\n");
#endif
	SendEditor(SCI_SETPRINTCOLOURMODE, mode, 0);
}

/* attribute long printColourMode; */
int32  SciMoz::GetPrintColourMode() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetPrintColourMode\n");
#endif
	return SendEditor(SCI_GETPRINTCOLOURMODE, 0, 0);
}

/* readonly attribute long firstVisibleLine; */
int32  SciMoz::GetFirstVisibleLine() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetFirstVisibleLine\n");
#endif
	return SendEditor(SCI_GETFIRSTVISIBLELINE, 0, 0);
}

/* readonly attribute long lineCount; */
int32  SciMoz::GetLineCount() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLineCount\n");
#endif
	return SendEditor(SCI_GETLINECOUNT, 0, 0);
}

/* attribute long marginLeft; */
void SciMoz::SetMarginLeft(int32 pixelWidth) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetMarginLeft\n");
#endif
	SendEditor(SCI_SETMARGINLEFT, 0, pixelWidth);
}

/* attribute long marginLeft; */
int32  SciMoz::GetMarginLeft() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetMarginLeft\n");
#endif
	return SendEditor(SCI_GETMARGINLEFT, 0, 0);
}

/* attribute long marginRight; */
void SciMoz::SetMarginRight(int32 pixelWidth) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetMarginRight\n");
#endif
	SendEditor(SCI_SETMARGINRIGHT, 0, pixelWidth);
}

/* attribute long marginRight; */
int32  SciMoz::GetMarginRight() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetMarginRight\n");
#endif
	return SendEditor(SCI_GETMARGINRIGHT, 0, 0);
}

/* readonly attribute boolean modify; */
bool  SciMoz::GetModify() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetModify\n");
#endif
	return SendEditor(SCI_GETMODIFY, 0, 0);
}

/* void setSel(in long start, in long end); */
void SciMoz::SetSel(int32 start, int32 end) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetSel\n");
#endif
	SendEditor(SCI_SETSEL, start, end);
}

/* void hideSelection(in boolean normal); */
void SciMoz::HideSelection(bool normal) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::HideSelection\n");
#endif
	SendEditor(SCI_HIDESELECTION, normal, 0);
}

/* long pointXFromPosition(in long pos); */
int32  SciMoz::PointXFromPosition(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PointXFromPosition\n");
#endif
	return SendEditor(SCI_POINTXFROMPOSITION, 0, pos);
}

/* long pointYFromPosition(in long pos); */
int32  SciMoz::PointYFromPosition(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PointYFromPosition\n");
#endif
	return SendEditor(SCI_POINTYFROMPOSITION, 0, pos);
}

/* long lineFromPosition(in long pos); */
int32  SciMoz::LineFromPosition(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineFromPosition\n");
#endif
	return SendEditor(SCI_LINEFROMPOSITION, pos, 0);
}

/* long positionFromLine(in long line); */
int32  SciMoz::PositionFromLine(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PositionFromLine\n");
#endif
	return SendEditor(SCI_POSITIONFROMLINE, line, 0);
}

/* void lineScroll(in long columns, in long lines); */
void SciMoz::LineScroll(int32 columns, int32 lines) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineScroll\n");
#endif
	SendEditor(SCI_LINESCROLL, columns, lines);
}

/* void scrollCaret(); */
void SciMoz::ScrollCaret() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ScrollCaret\n");
#endif
	SendEditor(SCI_SCROLLCARET, 0, 0);
}

/* void replaceSel(in wstring text); */
void SciMoz::ReplaceSel(NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ReplaceSel\n");
#endif
	SendEditor(SCI_REPLACESEL, 0, reinterpret_cast<long>(NPStringToString(text));
}

/* attribute boolean readOnly; */
void SciMoz::SetReadOnly(bool readOnly) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetReadOnly\n");
#endif
	SendEditor(SCI_SETREADONLY, readOnly, 0);
}

/* boolean canPaste(); */
bool  SciMoz::CanPaste() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CanPaste\n");
#endif
	return SendEditor(SCI_CANPASTE, 0, 0);
}

/* boolean canUndo(); */
bool  SciMoz::CanUndo() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CanUndo\n");
#endif
	return SendEditor(SCI_CANUNDO, 0, 0);
}

/* void emptyUndoBuffer(); */
void SciMoz::EmptyUndoBuffer() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::EmptyUndoBuffer\n");
#endif
	SendEditor(SCI_EMPTYUNDOBUFFER, 0, 0);
}

/* void undo(); */
void SciMoz::Undo() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::Undo\n");
#endif
	SendEditor(SCI_UNDO, 0, 0);
}

/* void cut(); */
void SciMoz::Cut() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::Cut\n");
#endif
	SendEditor(SCI_CUT, 0, 0);
}

/* void copy(); */
void SciMoz::Copy() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::Copy\n");
#endif
	SendEditor(SCI_COPY, 0, 0);
}

/* void paste(); */
void SciMoz::Paste() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::Paste\n");
#endif
	SendEditor(SCI_PASTE, 0, 0);
}

/* void clear(); */
void SciMoz::Clear() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::Clear\n");
#endif
	SendEditor(SCI_CLEAR, 0, 0);
}

/* readonly attribute long textLength; */
int32  SciMoz::GetTextLength() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetTextLength\n");
#endif
	return SendEditor(SCI_GETTEXTLENGTH, 0, 0);
}

/* readonly attribute long directFunction; */
int32  SciMoz::GetDirectFunction() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetDirectFunction\n");
#endif
	return SendEditor(SCI_GETDIRECTFUNCTION, 0, 0);
}

/* readonly attribute long directPointer; */
int32  SciMoz::GetDirectPointer() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetDirectPointer\n");
#endif
	return SendEditor(SCI_GETDIRECTPOINTER, 0, 0);
}

/* attribute boolean overtype; */
void SciMoz::SetOvertype(bool overtype) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetOvertype\n");
#endif
	SendEditor(SCI_SETOVERTYPE, overtype, 0);
}

/* attribute boolean overtype; */
bool  SciMoz::GetOvertype() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetOvertype\n");
#endif
	return SendEditor(SCI_GETOVERTYPE, 0, 0);
}

/* attribute long caretWidth; */
void SciMoz::SetCaretWidth(int32 pixelWidth) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCaretWidth\n");
#endif
	SendEditor(SCI_SETCARETWIDTH, pixelWidth, 0);
}

/* attribute long caretWidth; */
int32  SciMoz::GetCaretWidth() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCaretWidth\n");
#endif
	return SendEditor(SCI_GETCARETWIDTH, 0, 0);
}

/* attribute long targetStart; */
void SciMoz::SetTargetStart(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetTargetStart\n");
#endif
	SendEditor(SCI_SETTARGETSTART, pos, 0);
}

/* attribute long targetStart; */
int32  SciMoz::GetTargetStart() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetTargetStart\n");
#endif
	return SendEditor(SCI_GETTARGETSTART, 0, 0);
}

/* attribute long targetEnd; */
void SciMoz::SetTargetEnd(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetTargetEnd\n");
#endif
	SendEditor(SCI_SETTARGETEND, pos, 0);
}

/* attribute long targetEnd; */
int32  SciMoz::GetTargetEnd() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetTargetEnd\n");
#endif
	return SendEditor(SCI_GETTARGETEND, 0, 0);
}

/* long replaceTarget(in long length, in wstring text); */
int32  SciMoz::ReplaceTarget(int32 length, NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ReplaceTarget\n");
#endif
	return SendEditor(SCI_REPLACETARGET, length, reinterpret_cast<long>(NPStringToString(text));
}

/* long replaceTargetRE(in long length, in wstring text); */
int32  SciMoz::ReplaceTargetRE(int32 length, NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ReplaceTargetRE\n");
#endif
	return SendEditor(SCI_REPLACETARGETRE, length, reinterpret_cast<long>(NPStringToString(text));
}

/* long searchInTarget(in long length, in wstring text); */
int32  SciMoz::SearchInTarget(int32 length, NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SearchInTarget\n");
#endif
	return SendEditor(SCI_SEARCHINTARGET, length, reinterpret_cast<long>(NPStringToString(text));
}

/* attribute long searchFlags; */
void SciMoz::SetSearchFlags(int32 flags) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetSearchFlags\n");
#endif
	SendEditor(SCI_SETSEARCHFLAGS, flags, 0);
}

/* attribute long searchFlags; */
int32  SciMoz::GetSearchFlags() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetSearchFlags\n");
#endif
	return SendEditor(SCI_GETSEARCHFLAGS, 0, 0);
}

/* void callTipShow(in long pos, in wstring definition); */
void SciMoz::CallTipShow(int32 pos, NPStringdefinition) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CallTipShow\n");
#endif
	SendEditor(SCI_CALLTIPSHOW, pos, reinterpret_cast<long>(NPStringToString(definition));
}

/* void callTipCancel(); */
void SciMoz::CallTipCancel() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CallTipCancel\n");
#endif
	SendEditor(SCI_CALLTIPCANCEL, 0, 0);
}

/* boolean callTipActive(); */
bool  SciMoz::CallTipActive() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CallTipActive\n");
#endif
	return SendEditor(SCI_CALLTIPACTIVE, 0, 0);
}

/* long callTipPosStart(); */
int32  SciMoz::CallTipPosStart() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CallTipPosStart\n");
#endif
	return SendEditor(SCI_CALLTIPPOSSTART, 0, 0);
}

/* void callTipSetHlt(in long start, in long end); */
void SciMoz::CallTipSetHlt(int32 start, int32 end) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CallTipSetHlt\n");
#endif
	SendEditor(SCI_CALLTIPSETHLT, start, end);
}

/* void callTipSetBack(in long back); */
void SciMoz::CallTipSetBack(int32 back) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CallTipSetBack\n");
#endif
	SendEditor(SCI_CALLTIPSETBACK, back, 0);
}

/* void callTipSetFore(in long fore); */
void SciMoz::CallTipSetFore(int32 fore) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CallTipSetFore\n");
#endif
	SendEditor(SCI_CALLTIPSETFORE, fore, 0);
}

/* void callTipSetForeHlt(in long fore); */
void SciMoz::CallTipSetForeHlt(int32 fore) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CallTipSetForeHlt\n");
#endif
	SendEditor(SCI_CALLTIPSETFOREHLT, fore, 0);
}

/* void callTipUseStyle(in long tabSize); */
void SciMoz::CallTipUseStyle(int32 tabSize) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CallTipUseStyle\n");
#endif
	SendEditor(SCI_CALLTIPUSESTYLE, tabSize, 0);
}

/* long visibleFromDocLine(in long line); */
int32  SciMoz::VisibleFromDocLine(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::VisibleFromDocLine\n");
#endif
	return SendEditor(SCI_VISIBLEFROMDOCLINE, line, 0);
}

/* long docLineFromVisible(in long lineDisplay); */
int32  SciMoz::DocLineFromVisible(int32 lineDisplay) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DocLineFromVisible\n");
#endif
	return SendEditor(SCI_DOCLINEFROMVISIBLE, lineDisplay, 0);
}

/* long wrapCount(in long line); */
int32  SciMoz::WrapCount(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WrapCount\n");
#endif
	return SendEditor(SCI_WRAPCOUNT, line, 0);
}

/* void setFoldLevel(in long line, in long level); */
void SciMoz::SetFoldLevel(int32 line, int32 level) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetFoldLevel\n");
#endif
	SendEditor(SCI_SETFOLDLEVEL, line, level);
}

/* long getFoldLevel(in long line); */
int32  SciMoz::GetFoldLevel(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetFoldLevel\n");
#endif
	return SendEditor(SCI_GETFOLDLEVEL, line, 0);
}

/* long getLastChild(in long line, in long level); */
int32  SciMoz::GetLastChild(int32 line, int32 level) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLastChild\n");
#endif
	return SendEditor(SCI_GETLASTCHILD, line, level);
}

/* long getFoldParent(in long line); */
int32  SciMoz::GetFoldParent(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetFoldParent\n");
#endif
	return SendEditor(SCI_GETFOLDPARENT, line, 0);
}

/* void showLines(in long lineStart, in long lineEnd); */
void SciMoz::ShowLines(int32 lineStart, int32 lineEnd) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ShowLines\n");
#endif
	SendEditor(SCI_SHOWLINES, lineStart, lineEnd);
}

/* void hideLines(in long lineStart, in long lineEnd); */
void SciMoz::HideLines(int32 lineStart, int32 lineEnd) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::HideLines\n");
#endif
	SendEditor(SCI_HIDELINES, lineStart, lineEnd);
}

/* boolean getLineVisible(in long line); */
bool  SciMoz::GetLineVisible(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLineVisible\n");
#endif
	return SendEditor(SCI_GETLINEVISIBLE, line, 0);
}

/* void setFoldExpanded(in long line, in boolean expanded); */
void SciMoz::SetFoldExpanded(int32 line, bool expanded) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetFoldExpanded\n");
#endif
	SendEditor(SCI_SETFOLDEXPANDED, line, expanded);
}

/* boolean getFoldExpanded(in long line); */
bool  SciMoz::GetFoldExpanded(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetFoldExpanded\n");
#endif
	return SendEditor(SCI_GETFOLDEXPANDED, line, 0);
}

/* void toggleFold(in long line); */
void SciMoz::ToggleFold(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ToggleFold\n");
#endif
	SendEditor(SCI_TOGGLEFOLD, line, 0);
}

/* void ensureVisible(in long line); */
void SciMoz::EnsureVisible(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::EnsureVisible\n");
#endif
	SendEditor(SCI_ENSUREVISIBLE, line, 0);
}

/* void setFoldFlags(in long flags); */
void SciMoz::SetFoldFlags(int32 flags) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetFoldFlags\n");
#endif
	SendEditor(SCI_SETFOLDFLAGS, flags, 0);
}

/* void ensureVisibleEnforcePolicy(in long line); */
void SciMoz::EnsureVisibleEnforcePolicy(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::EnsureVisibleEnforcePolicy\n");
#endif
	SendEditor(SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0);
}

/* attribute boolean tabIndents; */
void SciMoz::SetTabIndents(bool tabIndents) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetTabIndents\n");
#endif
	SendEditor(SCI_SETTABINDENTS, tabIndents, 0);
}

/* attribute boolean tabIndents; */
bool  SciMoz::GetTabIndents() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetTabIndents\n");
#endif
	return SendEditor(SCI_GETTABINDENTS, 0, 0);
}

/* attribute boolean backSpaceUnIndents; */
void SciMoz::SetBackSpaceUnIndents(bool bsUnIndents) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetBackSpaceUnIndents\n");
#endif
	SendEditor(SCI_SETBACKSPACEUNINDENTS, bsUnIndents, 0);
}

/* attribute boolean backSpaceUnIndents; */
bool  SciMoz::GetBackSpaceUnIndents() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetBackSpaceUnIndents\n");
#endif
	return SendEditor(SCI_GETBACKSPACEUNINDENTS, 0, 0);
}

/* attribute long mouseDwellTime; */
void SciMoz::SetMouseDwellTime(int32 periodMilliseconds) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetMouseDwellTime\n");
#endif
	SendEditor(SCI_SETMOUSEDWELLTIME, periodMilliseconds, 0);
}

/* attribute long mouseDwellTime; */
int32  SciMoz::GetMouseDwellTime() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetMouseDwellTime\n");
#endif
	return SendEditor(SCI_GETMOUSEDWELLTIME, 0, 0);
}

/* long wordStartPosition(in long pos, in boolean onlyWordCharacters); */
int32  SciMoz::WordStartPosition(int32 pos, bool onlyWordCharacters) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordStartPosition\n");
#endif
	return SendEditor(SCI_WORDSTARTPOSITION, pos, onlyWordCharacters);
}

/* long wordEndPosition(in long pos, in boolean onlyWordCharacters); */
int32  SciMoz::WordEndPosition(int32 pos, bool onlyWordCharacters) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordEndPosition\n");
#endif
	return SendEditor(SCI_WORDENDPOSITION, pos, onlyWordCharacters);
}

/* attribute long wrapMode; */
void SciMoz::SetWrapMode(int32 mode) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetWrapMode\n");
#endif
	SendEditor(SCI_SETWRAPMODE, mode, 0);
}

/* attribute long wrapMode; */
int32  SciMoz::GetWrapMode() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetWrapMode\n");
#endif
	return SendEditor(SCI_GETWRAPMODE, 0, 0);
}

/* attribute long wrapVisualFlags; */
void SciMoz::SetWrapVisualFlags(int32 wrapVisualFlags) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetWrapVisualFlags\n");
#endif
	SendEditor(SCI_SETWRAPVISUALFLAGS, wrapVisualFlags, 0);
}

/* attribute long wrapVisualFlags; */
int32  SciMoz::GetWrapVisualFlags() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetWrapVisualFlags\n");
#endif
	return SendEditor(SCI_GETWRAPVISUALFLAGS, 0, 0);
}

/* attribute long wrapVisualFlagsLocation; */
void SciMoz::SetWrapVisualFlagsLocation(int32 wrapVisualFlagsLocation) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetWrapVisualFlagsLocation\n");
#endif
	SendEditor(SCI_SETWRAPVISUALFLAGSLOCATION, wrapVisualFlagsLocation, 0);
}

/* attribute long wrapVisualFlagsLocation; */
int32  SciMoz::GetWrapVisualFlagsLocation() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetWrapVisualFlagsLocation\n");
#endif
	return SendEditor(SCI_GETWRAPVISUALFLAGSLOCATION, 0, 0);
}

/* attribute long wrapStartIndent; */
void SciMoz::SetWrapStartIndent(int32 indent) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetWrapStartIndent\n");
#endif
	SendEditor(SCI_SETWRAPSTARTINDENT, indent, 0);
}

/* attribute long wrapStartIndent; */
int32  SciMoz::GetWrapStartIndent() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetWrapStartIndent\n");
#endif
	return SendEditor(SCI_GETWRAPSTARTINDENT, 0, 0);
}

/* attribute long layoutCache; */
void SciMoz::SetLayoutCache(int32 mode) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetLayoutCache\n");
#endif
	SendEditor(SCI_SETLAYOUTCACHE, mode, 0);
}

/* attribute long layoutCache; */
int32  SciMoz::GetLayoutCache() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLayoutCache\n");
#endif
	return SendEditor(SCI_GETLAYOUTCACHE, 0, 0);
}

/* attribute long scrollWidth; */
void SciMoz::SetScrollWidth(int32 pixelWidth) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetScrollWidth\n");
#endif
	SendEditor(SCI_SETSCROLLWIDTH, pixelWidth, 0);
}

/* attribute long scrollWidth; */
int32  SciMoz::GetScrollWidth() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetScrollWidth\n");
#endif
	return SendEditor(SCI_GETSCROLLWIDTH, 0, 0);
}

/* attribute boolean scrollWidthTracking; */
void SciMoz::SetScrollWidthTracking(bool tracking) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetScrollWidthTracking\n");
#endif
	SendEditor(SCI_SETSCROLLWIDTHTRACKING, tracking, 0);
}

/* attribute boolean scrollWidthTracking; */
bool  SciMoz::GetScrollWidthTracking() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetScrollWidthTracking\n");
#endif
	return SendEditor(SCI_GETSCROLLWIDTHTRACKING, 0, 0);
}

/* long textWidth(in long style, in wstring text); */
int32  SciMoz::TextWidth(int32 style, NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::TextWidth\n");
#endif
	return SendEditor(SCI_TEXTWIDTH, style, reinterpret_cast<long>(NPStringToString(text));
}

/* attribute boolean endAtLastLine; */
void SciMoz::SetEndAtLastLine(bool endAtLastLine) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetEndAtLastLine\n");
#endif
	SendEditor(SCI_SETENDATLASTLINE, endAtLastLine, 0);
}

/* attribute boolean endAtLastLine; */
bool  SciMoz::GetEndAtLastLine() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetEndAtLastLine\n");
#endif
	return SendEditor(SCI_GETENDATLASTLINE, 0, 0);
}

/* long textHeight(in long line); */
int32  SciMoz::TextHeight(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::TextHeight\n");
#endif
	return SendEditor(SCI_TEXTHEIGHT, line, 0);
}

/* attribute boolean vScrollBar; */
void SciMoz::SetVScrollBar(bool show) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetVScrollBar\n");
#endif
	SendEditor(SCI_SETVSCROLLBAR, show, 0);
}

/* attribute boolean vScrollBar; */
bool  SciMoz::GetVScrollBar() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetVScrollBar\n");
#endif
	return SendEditor(SCI_GETVSCROLLBAR, 0, 0);
}

/* void appendText(in long length, in wstring text); */
void SciMoz::AppendText(int32 length, NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AppendText\n");
#endif
	SendEditor(SCI_APPENDTEXT, length, reinterpret_cast<long>(NPStringToString(text));
}

/* attribute boolean twoPhaseDraw; */
bool  SciMoz::GetTwoPhaseDraw() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetTwoPhaseDraw\n");
#endif
	return SendEditor(SCI_GETTWOPHASEDRAW, 0, 0);
}

/* attribute boolean twoPhaseDraw; */
void SciMoz::SetTwoPhaseDraw(bool twoPhase) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetTwoPhaseDraw\n");
#endif
	SendEditor(SCI_SETTWOPHASEDRAW, twoPhase, 0);
}

/* void targetFromSelection(); */
void SciMoz::TargetFromSelection() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::TargetFromSelection\n");
#endif
	SendEditor(SCI_TARGETFROMSELECTION, 0, 0);
}

/* void linesJoin(); */
void SciMoz::LinesJoin() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LinesJoin\n");
#endif
	SendEditor(SCI_LINESJOIN, 0, 0);
}

/* void linesSplit(in long pixelWidth); */
void SciMoz::LinesSplit(int32 pixelWidth) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LinesSplit\n");
#endif
	SendEditor(SCI_LINESSPLIT, pixelWidth, 0);
}

/* void setFoldMarginColour(in boolean useSetting, in long back); */
void SciMoz::SetFoldMarginColour(bool useSetting, int32 back) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetFoldMarginColour\n");
#endif
	SendEditor(SCI_SETFOLDMARGINCOLOUR, useSetting, back);
}

/* void setFoldMarginHiColour(in boolean useSetting, in long fore); */
void SciMoz::SetFoldMarginHiColour(bool useSetting, int32 fore) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetFoldMarginHiColour\n");
#endif
	SendEditor(SCI_SETFOLDMARGINHICOLOUR, useSetting, fore);
}

/* void lineDown(); */
void SciMoz::LineDown() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineDown\n");
#endif
	SendEditor(SCI_LINEDOWN, 0, 0);
}

/* void lineDownExtend(); */
void SciMoz::LineDownExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineDownExtend\n");
#endif
	SendEditor(SCI_LINEDOWNEXTEND, 0, 0);
}

/* void lineUp(); */
void SciMoz::LineUp() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineUp\n");
#endif
	SendEditor(SCI_LINEUP, 0, 0);
}

/* void lineUpExtend(); */
void SciMoz::LineUpExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineUpExtend\n");
#endif
	SendEditor(SCI_LINEUPEXTEND, 0, 0);
}

/* void charLeft(); */
void SciMoz::CharLeft() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CharLeft\n");
#endif
	SendEditor(SCI_CHARLEFT, 0, 0);
}

/* void charLeftExtend(); */
void SciMoz::CharLeftExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CharLeftExtend\n");
#endif
	SendEditor(SCI_CHARLEFTEXTEND, 0, 0);
}

/* void charRight(); */
void SciMoz::CharRight() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CharRight\n");
#endif
	SendEditor(SCI_CHARRIGHT, 0, 0);
}

/* void charRightExtend(); */
void SciMoz::CharRightExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CharRightExtend\n");
#endif
	SendEditor(SCI_CHARRIGHTEXTEND, 0, 0);
}

/* void wordLeft(); */
void SciMoz::WordLeft() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordLeft\n");
#endif
	SendEditor(SCI_WORDLEFT, 0, 0);
}

/* void wordLeftExtend(); */
void SciMoz::WordLeftExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordLeftExtend\n");
#endif
	SendEditor(SCI_WORDLEFTEXTEND, 0, 0);
}

/* void wordRight(); */
void SciMoz::WordRight() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordRight\n");
#endif
	SendEditor(SCI_WORDRIGHT, 0, 0);
}

/* void wordRightExtend(); */
void SciMoz::WordRightExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordRightExtend\n");
#endif
	SendEditor(SCI_WORDRIGHTEXTEND, 0, 0);
}

/* void home(); */
void SciMoz::Home() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::Home\n");
#endif
	SendEditor(SCI_HOME, 0, 0);
}

/* void homeExtend(); */
void SciMoz::HomeExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::HomeExtend\n");
#endif
	SendEditor(SCI_HOMEEXTEND, 0, 0);
}

/* void lineEnd(); */
void SciMoz::LineEnd() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineEnd\n");
#endif
	SendEditor(SCI_LINEEND, 0, 0);
}

/* void lineEndExtend(); */
void SciMoz::LineEndExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineEndExtend\n");
#endif
	SendEditor(SCI_LINEENDEXTEND, 0, 0);
}

/* void documentStart(); */
void SciMoz::DocumentStart() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DocumentStart\n");
#endif
	SendEditor(SCI_DOCUMENTSTART, 0, 0);
}

/* void documentStartExtend(); */
void SciMoz::DocumentStartExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DocumentStartExtend\n");
#endif
	SendEditor(SCI_DOCUMENTSTARTEXTEND, 0, 0);
}

/* void documentEnd(); */
void SciMoz::DocumentEnd() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DocumentEnd\n");
#endif
	SendEditor(SCI_DOCUMENTEND, 0, 0);
}

/* void documentEndExtend(); */
void SciMoz::DocumentEndExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DocumentEndExtend\n");
#endif
	SendEditor(SCI_DOCUMENTENDEXTEND, 0, 0);
}

/* void pageUp(); */
void SciMoz::PageUp() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PageUp\n");
#endif
	SendEditor(SCI_PAGEUP, 0, 0);
}

/* void pageUpExtend(); */
void SciMoz::PageUpExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PageUpExtend\n");
#endif
	SendEditor(SCI_PAGEUPEXTEND, 0, 0);
}

/* void pageDown(); */
void SciMoz::PageDown() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PageDown\n");
#endif
	SendEditor(SCI_PAGEDOWN, 0, 0);
}

/* void pageDownExtend(); */
void SciMoz::PageDownExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PageDownExtend\n");
#endif
	SendEditor(SCI_PAGEDOWNEXTEND, 0, 0);
}

/* void editToggleOvertype(); */
void SciMoz::EditToggleOvertype() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::EditToggleOvertype\n");
#endif
	SendEditor(SCI_EDITTOGGLEOVERTYPE, 0, 0);
}

/* void cancel(); */
void SciMoz::Cancel() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::Cancel\n");
#endif
	SendEditor(SCI_CANCEL, 0, 0);
}

/* void deleteBack(); */
void SciMoz::DeleteBack() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DeleteBack\n");
#endif
	SendEditor(SCI_DELETEBACK, 0, 0);
}

/* void tab(); */
void SciMoz::Tab() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::Tab\n");
#endif
	SendEditor(SCI_TAB, 0, 0);
}

/* void backTab(); */
void SciMoz::BackTab() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::BackTab\n");
#endif
	SendEditor(SCI_BACKTAB, 0, 0);
}

/* void newLine(); */
void SciMoz::NewLine() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::NewLine\n");
#endif
	SendEditor(SCI_NEWLINE, 0, 0);
}

/* void formFeed(); */
void SciMoz::FormFeed() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::FormFeed\n");
#endif
	SendEditor(SCI_FORMFEED, 0, 0);
}

/* void vCHome(); */
void SciMoz::VCHome() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::VCHome\n");
#endif
	SendEditor(SCI_VCHOME, 0, 0);
}

/* void vCHomeExtend(); */
void SciMoz::VCHomeExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::VCHomeExtend\n");
#endif
	SendEditor(SCI_VCHOMEEXTEND, 0, 0);
}

/* void zoomIn(); */
void SciMoz::ZoomIn() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ZoomIn\n");
#endif
	SendEditor(SCI_ZOOMIN, 0, 0);
}

/* void zoomOut(); */
void SciMoz::ZoomOut() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ZoomOut\n");
#endif
	SendEditor(SCI_ZOOMOUT, 0, 0);
}

/* void delWordLeft(); */
void SciMoz::DelWordLeft() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DelWordLeft\n");
#endif
	SendEditor(SCI_DELWORDLEFT, 0, 0);
}

/* void delWordRight(); */
void SciMoz::DelWordRight() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DelWordRight\n");
#endif
	SendEditor(SCI_DELWORDRIGHT, 0, 0);
}

/* void delWordRightEnd(); */
void SciMoz::DelWordRightEnd() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DelWordRightEnd\n");
#endif
	SendEditor(SCI_DELWORDRIGHTEND, 0, 0);
}

/* void lineCut(); */
void SciMoz::LineCut() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineCut\n");
#endif
	SendEditor(SCI_LINECUT, 0, 0);
}

/* void lineDelete(); */
void SciMoz::LineDelete() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineDelete\n");
#endif
	SendEditor(SCI_LINEDELETE, 0, 0);
}

/* void lineTranspose(); */
void SciMoz::LineTranspose() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineTranspose\n");
#endif
	SendEditor(SCI_LINETRANSPOSE, 0, 0);
}

/* void lineDuplicate(); */
void SciMoz::LineDuplicate() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineDuplicate\n");
#endif
	SendEditor(SCI_LINEDUPLICATE, 0, 0);
}

/* void lowerCase(); */
void SciMoz::LowerCase() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LowerCase\n");
#endif
	SendEditor(SCI_LOWERCASE, 0, 0);
}

/* void upperCase(); */
void SciMoz::UpperCase() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::UpperCase\n");
#endif
	SendEditor(SCI_UPPERCASE, 0, 0);
}

/* void lineScrollDown(); */
void SciMoz::LineScrollDown() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineScrollDown\n");
#endif
	SendEditor(SCI_LINESCROLLDOWN, 0, 0);
}

/* void lineScrollUp(); */
void SciMoz::LineScrollUp() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineScrollUp\n");
#endif
	SendEditor(SCI_LINESCROLLUP, 0, 0);
}

/* void deleteBackNotLine(); */
void SciMoz::DeleteBackNotLine() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DeleteBackNotLine\n");
#endif
	SendEditor(SCI_DELETEBACKNOTLINE, 0, 0);
}

/* void homeDisplay(); */
void SciMoz::HomeDisplay() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::HomeDisplay\n");
#endif
	SendEditor(SCI_HOMEDISPLAY, 0, 0);
}

/* void homeDisplayExtend(); */
void SciMoz::HomeDisplayExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::HomeDisplayExtend\n");
#endif
	SendEditor(SCI_HOMEDISPLAYEXTEND, 0, 0);
}

/* void lineEndDisplay(); */
void SciMoz::LineEndDisplay() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineEndDisplay\n");
#endif
	SendEditor(SCI_LINEENDDISPLAY, 0, 0);
}

/* void lineEndDisplayExtend(); */
void SciMoz::LineEndDisplayExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineEndDisplayExtend\n");
#endif
	SendEditor(SCI_LINEENDDISPLAYEXTEND, 0, 0);
}

/* void homeWrap(); */
void SciMoz::HomeWrap() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::HomeWrap\n");
#endif
	SendEditor(SCI_HOMEWRAP, 0, 0);
}

/* void homeWrapExtend(); */
void SciMoz::HomeWrapExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::HomeWrapExtend\n");
#endif
	SendEditor(SCI_HOMEWRAPEXTEND, 0, 0);
}

/* void lineEndWrap(); */
void SciMoz::LineEndWrap() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineEndWrap\n");
#endif
	SendEditor(SCI_LINEENDWRAP, 0, 0);
}

/* void lineEndWrapExtend(); */
void SciMoz::LineEndWrapExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineEndWrapExtend\n");
#endif
	SendEditor(SCI_LINEENDWRAPEXTEND, 0, 0);
}

/* void vCHomeWrap(); */
void SciMoz::VCHomeWrap() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::VCHomeWrap\n");
#endif
	SendEditor(SCI_VCHOMEWRAP, 0, 0);
}

/* void vCHomeWrapExtend(); */
void SciMoz::VCHomeWrapExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::VCHomeWrapExtend\n");
#endif
	SendEditor(SCI_VCHOMEWRAPEXTEND, 0, 0);
}

/* void lineCopy(); */
void SciMoz::LineCopy() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineCopy\n");
#endif
	SendEditor(SCI_LINECOPY, 0, 0);
}

/* void moveCaretInsideView(); */
void SciMoz::MoveCaretInsideView() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::MoveCaretInsideView\n");
#endif
	SendEditor(SCI_MOVECARETINSIDEVIEW, 0, 0);
}

/* long lineLength(in long line); */
int32  SciMoz::LineLength(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineLength\n");
#endif
	return SendEditor(SCI_LINELENGTH, line, 0);
}

/* void braceHighlight(in long pos1, in long pos2); */
void SciMoz::BraceHighlight(int32 pos1, int32 pos2) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::BraceHighlight\n");
#endif
	SendEditor(SCI_BRACEHIGHLIGHT, pos1, pos2);
}

/* void braceBadLight(in long pos); */
void SciMoz::BraceBadLight(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::BraceBadLight\n");
#endif
	SendEditor(SCI_BRACEBADLIGHT, pos, 0);
}

/* long braceMatch(in long pos); */
int32  SciMoz::BraceMatch(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::BraceMatch\n");
#endif
	return SendEditor(SCI_BRACEMATCH, pos, 0);
}

/* attribute boolean viewEOL; */
bool  SciMoz::GetViewEOL() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetViewEOL\n");
#endif
	return SendEditor(SCI_GETVIEWEOL, 0, 0);
}

/* attribute boolean viewEOL; */
void SciMoz::SetViewEOL(bool visible) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetViewEOL\n");
#endif
	SendEditor(SCI_SETVIEWEOL, visible, 0);
}

/* attribute long docPointer; */
int32  SciMoz::GetDocPointer() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetDocPointer\n");
#endif
	return SendEditor(SCI_GETDOCPOINTER, 0, 0);
}

/* attribute long docPointer; */
void SciMoz::SetDocPointer(int32 pointer) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetDocPointer\n");
#endif
	SendEditor(SCI_SETDOCPOINTER, 0, pointer);
}

/* attribute long modEventMask; */
void SciMoz::SetModEventMask(int32 mask) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetModEventMask\n");
#endif
	SendEditor(SCI_SETMODEVENTMASK, mask, 0);
}

/* attribute long edgeColumn; */
int32  SciMoz::GetEdgeColumn() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetEdgeColumn\n");
#endif
	return SendEditor(SCI_GETEDGECOLUMN, 0, 0);
}

/* attribute long edgeColumn; */
void SciMoz::SetEdgeColumn(int32 column) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetEdgeColumn\n");
#endif
	SendEditor(SCI_SETEDGECOLUMN, column, 0);
}

/* attribute long edgeMode; */
int32  SciMoz::GetEdgeMode() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetEdgeMode\n");
#endif
	return SendEditor(SCI_GETEDGEMODE, 0, 0);
}

/* attribute long edgeMode; */
void SciMoz::SetEdgeMode(int32 mode) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetEdgeMode\n");
#endif
	SendEditor(SCI_SETEDGEMODE, mode, 0);
}

/* attribute long edgeColour; */
int32  SciMoz::GetEdgeColour() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetEdgeColour\n");
#endif
	return SendEditor(SCI_GETEDGECOLOUR, 0, 0);
}

/* attribute long edgeColour; */
void SciMoz::SetEdgeColour(int32 edgeColour) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetEdgeColour\n");
#endif
	SendEditor(SCI_SETEDGECOLOUR, edgeColour, 0);
}

/* void searchAnchor(); */
void SciMoz::SearchAnchor() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SearchAnchor\n");
#endif
	SendEditor(SCI_SEARCHANCHOR, 0, 0);
}

/* long searchNext(in long flags, in wstring text); */
int32  SciMoz::SearchNext(int32 flags, NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SearchNext\n");
#endif
	return SendEditor(SCI_SEARCHNEXT, flags, reinterpret_cast<long>(NPStringToString(text));
}

/* long searchPrev(in long flags, in wstring text); */
int32  SciMoz::SearchPrev(int32 flags, NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SearchPrev\n");
#endif
	return SendEditor(SCI_SEARCHPREV, flags, reinterpret_cast<long>(NPStringToString(text));
}

/* readonly attribute long linesOnScreen; */
int32  SciMoz::GetLinesOnScreen() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLinesOnScreen\n");
#endif
	return SendEditor(SCI_LINESONSCREEN, 0, 0);
}

/* void usePopUp(in boolean allowPopUp); */
void SciMoz::UsePopUp(bool allowPopUp) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::UsePopUp\n");
#endif
	SendEditor(SCI_USEPOPUP, allowPopUp, 0);
}

/* readonly attribute boolean selectionIsRectangle; */
bool  SciMoz::GetSelectionIsRectangle() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetSelectionIsRectangle\n");
#endif
	return SendEditor(SCI_SELECTIONISRECTANGLE, 0, 0);
}

/* attribute long zoom; */
void SciMoz::SetZoom(int32 zoom) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetZoom\n");
#endif
	SendEditor(SCI_SETZOOM, zoom, 0);
}

/* attribute long zoom; */
int32  SciMoz::GetZoom() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetZoom\n");
#endif
	return SendEditor(SCI_GETZOOM, 0, 0);
}

/* long createDocument(); */
int32  SciMoz::CreateDocument() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CreateDocument\n");
#endif
	return SendEditor(SCI_CREATEDOCUMENT, 0, 0);
}

/* void addRefDocument(in long doc); */
void SciMoz::AddRefDocument(int32 doc) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AddRefDocument\n");
#endif
	SendEditor(SCI_ADDREFDOCUMENT, 0, doc);
}

/* void releaseDocument(in long doc); */
void SciMoz::ReleaseDocument(int32 doc) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ReleaseDocument\n");
#endif
	SendEditor(SCI_RELEASEDOCUMENT, 0, doc);
}

/* attribute long modEventMask; */
int32  SciMoz::GetModEventMask() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetModEventMask\n");
#endif
	return SendEditor(SCI_GETMODEVENTMASK, 0, 0);
}

/* attribute boolean focus; */
void SciMoz::SetFocus(bool focus) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetFocus\n");
#endif
	SendEditor(SCI_SETFOCUS, focus, 0);
}

/* attribute boolean focus; */
bool  SciMoz::GetFocus() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetFocus\n");
#endif
	return SendEditor(SCI_GETFOCUS, 0, 0);
}

/* attribute long status; */
void SciMoz::SetStatus(int32 statusCode) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetStatus\n");
#endif
	SendEditor(SCI_SETSTATUS, statusCode, 0);
}

/* attribute long status; */
int32  SciMoz::GetStatus() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetStatus\n");
#endif
	return SendEditor(SCI_GETSTATUS, 0, 0);
}

/* attribute boolean mouseDownCaptures; */
void SciMoz::SetMouseDownCaptures(bool captures) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetMouseDownCaptures\n");
#endif
	SendEditor(SCI_SETMOUSEDOWNCAPTURES, captures, 0);
}

/* attribute boolean mouseDownCaptures; */
bool  SciMoz::GetMouseDownCaptures() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetMouseDownCaptures\n");
#endif
	return SendEditor(SCI_GETMOUSEDOWNCAPTURES, 0, 0);
}

/* attribute long cursor; */
void SciMoz::SetCursor(int32 cursorType) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCursor\n");
#endif
	SendEditor(SCI_SETCURSOR, cursorType, 0);
}

/* attribute long cursor; */
int32  SciMoz::GetCursor() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCursor\n");
#endif
	return SendEditor(SCI_GETCURSOR, 0, 0);
}

/* attribute long controlCharSymbol; */
void SciMoz::SetControlCharSymbol(int32 symbol) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetControlCharSymbol\n");
#endif
	SendEditor(SCI_SETCONTROLCHARSYMBOL, symbol, 0);
}

/* attribute long controlCharSymbol; */
int32  SciMoz::GetControlCharSymbol() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetControlCharSymbol\n");
#endif
	return SendEditor(SCI_GETCONTROLCHARSYMBOL, 0, 0);
}

/* void wordPartLeft(); */
void SciMoz::WordPartLeft() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordPartLeft\n");
#endif
	SendEditor(SCI_WORDPARTLEFT, 0, 0);
}

/* void wordPartLeftExtend(); */
void SciMoz::WordPartLeftExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordPartLeftExtend\n");
#endif
	SendEditor(SCI_WORDPARTLEFTEXTEND, 0, 0);
}

/* void wordPartRight(); */
void SciMoz::WordPartRight() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordPartRight\n");
#endif
	SendEditor(SCI_WORDPARTRIGHT, 0, 0);
}

/* void wordPartRightExtend(); */
void SciMoz::WordPartRightExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordPartRightExtend\n");
#endif
	SendEditor(SCI_WORDPARTRIGHTEXTEND, 0, 0);
}

/* void setVisiblePolicy(in long visiblePolicy, in long visibleSlop); */
void SciMoz::SetVisiblePolicy(int32 visiblePolicy, int32 visibleSlop) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetVisiblePolicy\n");
#endif
	SendEditor(SCI_SETVISIBLEPOLICY, visiblePolicy, visibleSlop);
}

/* void delLineLeft(); */
void SciMoz::DelLineLeft() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DelLineLeft\n");
#endif
	SendEditor(SCI_DELLINELEFT, 0, 0);
}

/* void delLineRight(); */
void SciMoz::DelLineRight() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::DelLineRight\n");
#endif
	SendEditor(SCI_DELLINERIGHT, 0, 0);
}

/* attribute long xOffset; */
void SciMoz::SetXOffset(int32 newOffset) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetXOffset\n");
#endif
	SendEditor(SCI_SETXOFFSET, newOffset, 0);
}

/* attribute long xOffset; */
int32  SciMoz::GetXOffset() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetXOffset\n");
#endif
	return SendEditor(SCI_GETXOFFSET, 0, 0);
}

/* void chooseCaretX(); */
void SciMoz::ChooseCaretX() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ChooseCaretX\n");
#endif
	SendEditor(SCI_CHOOSECARETX, 0, 0);
}

/* void grabFocus(); */
void SciMoz::GrabFocus() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GrabFocus\n");
#endif
	SendEditor(SCI_GRABFOCUS, 0, 0);
}

/* void setXCaretPolicy(in long caretPolicy, in long caretSlop); */
void SciMoz::SetXCaretPolicy(int32 caretPolicy, int32 caretSlop) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetXCaretPolicy\n");
#endif
	SendEditor(SCI_SETXCARETPOLICY, caretPolicy, caretSlop);
}

/* void setYCaretPolicy(in long caretPolicy, in long caretSlop); */
void SciMoz::SetYCaretPolicy(int32 caretPolicy, int32 caretSlop) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetYCaretPolicy\n");
#endif
	SendEditor(SCI_SETYCARETPOLICY, caretPolicy, caretSlop);
}

/* attribute long printWrapMode; */
void SciMoz::SetPrintWrapMode(int32 mode) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetPrintWrapMode\n");
#endif
	SendEditor(SCI_SETPRINTWRAPMODE, mode, 0);
}

/* attribute long printWrapMode; */
int32  SciMoz::GetPrintWrapMode() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetPrintWrapMode\n");
#endif
	return SendEditor(SCI_GETPRINTWRAPMODE, 0, 0);
}

/* attribute boolean hotspotActiveFore; */
void SciMoz::SetHotspotActiveFore(bool useSetting, int32 fore) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetHotspotActiveFore\n");
#endif
	SendEditor(SCI_SETHOTSPOTACTIVEFORE, useSetting, fore);
}

/* attribute long hotspotActiveFore; */
int32  SciMoz::GetHotspotActiveFore() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetHotspotActiveFore\n");
#endif
	return SendEditor(SCI_GETHOTSPOTACTIVEFORE, 0, 0);
}

/* attribute boolean hotspotActiveBack; */
void SciMoz::SetHotspotActiveBack(bool useSetting, int32 back) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetHotspotActiveBack\n");
#endif
	SendEditor(SCI_SETHOTSPOTACTIVEBACK, useSetting, back);
}

/* attribute long hotspotActiveBack; */
int32  SciMoz::GetHotspotActiveBack() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetHotspotActiveBack\n");
#endif
	return SendEditor(SCI_GETHOTSPOTACTIVEBACK, 0, 0);
}

/* attribute boolean hotspotActiveUnderline; */
void SciMoz::SetHotspotActiveUnderline(bool underline) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetHotspotActiveUnderline\n");
#endif
	SendEditor(SCI_SETHOTSPOTACTIVEUNDERLINE, underline, 0);
}

/* attribute boolean hotspotActiveUnderline; */
bool  SciMoz::GetHotspotActiveUnderline() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetHotspotActiveUnderline\n");
#endif
	return SendEditor(SCI_GETHOTSPOTACTIVEUNDERLINE, 0, 0);
}

/* attribute boolean hotspotSingleLine; */
void SciMoz::SetHotspotSingleLine(bool singleLine) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetHotspotSingleLine\n");
#endif
	SendEditor(SCI_SETHOTSPOTSINGLELINE, singleLine, 0);
}

/* attribute boolean hotspotSingleLine; */
bool  SciMoz::GetHotspotSingleLine() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetHotspotSingleLine\n");
#endif
	return SendEditor(SCI_GETHOTSPOTSINGLELINE, 0, 0);
}

/* void paraDown(); */
void SciMoz::ParaDown() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ParaDown\n");
#endif
	SendEditor(SCI_PARADOWN, 0, 0);
}

/* void paraDownExtend(); */
void SciMoz::ParaDownExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ParaDownExtend\n");
#endif
	SendEditor(SCI_PARADOWNEXTEND, 0, 0);
}

/* void paraUp(); */
void SciMoz::ParaUp() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ParaUp\n");
#endif
	SendEditor(SCI_PARAUP, 0, 0);
}

/* void paraUpExtend(); */
void SciMoz::ParaUpExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ParaUpExtend\n");
#endif
	SendEditor(SCI_PARAUPEXTEND, 0, 0);
}

/* long positionBefore(in long pos); */
int32  SciMoz::PositionBefore(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PositionBefore\n");
#endif
	return SendEditor(SCI_POSITIONBEFORE, pos, 0);
}

/* long positionAfter(in long pos); */
int32  SciMoz::PositionAfter(int32 pos) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PositionAfter\n");
#endif
	return SendEditor(SCI_POSITIONAFTER, pos, 0);
}

/* void copyRange(in long start, in long end); */
void SciMoz::CopyRange(int32 start, int32 end) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CopyRange\n");
#endif
	SendEditor(SCI_COPYRANGE, start, end);
}

/* void copyText(in long length, in wstring text); */
void SciMoz::CopyText(int32 length, NPStringtext) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CopyText\n");
#endif
	SendEditor(SCI_COPYTEXT, length, reinterpret_cast<long>(NPStringToString(text));
}

/* attribute long selectionMode; */
void SciMoz::SetSelectionMode(int32 mode) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetSelectionMode\n");
#endif
	SendEditor(SCI_SETSELECTIONMODE, mode, 0);
}

/* attribute long selectionMode; */
int32  SciMoz::GetSelectionMode() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetSelectionMode\n");
#endif
	return SendEditor(SCI_GETSELECTIONMODE, 0, 0);
}

/* long getLineSelStartPosition(in long line); */
int32  SciMoz::GetLineSelStartPosition(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLineSelStartPosition\n");
#endif
	return SendEditor(SCI_GETLINESELSTARTPOSITION, line, 0);
}

/* long getLineSelEndPosition(in long line); */
int32  SciMoz::GetLineSelEndPosition(int32 line) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLineSelEndPosition\n");
#endif
	return SendEditor(SCI_GETLINESELENDPOSITION, line, 0);
}

/* void lineDownRectExtend(); */
void SciMoz::LineDownRectExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineDownRectExtend\n");
#endif
	SendEditor(SCI_LINEDOWNRECTEXTEND, 0, 0);
}

/* void lineUpRectExtend(); */
void SciMoz::LineUpRectExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineUpRectExtend\n");
#endif
	SendEditor(SCI_LINEUPRECTEXTEND, 0, 0);
}

/* void charLeftRectExtend(); */
void SciMoz::CharLeftRectExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CharLeftRectExtend\n");
#endif
	SendEditor(SCI_CHARLEFTRECTEXTEND, 0, 0);
}

/* void charRightRectExtend(); */
void SciMoz::CharRightRectExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CharRightRectExtend\n");
#endif
	SendEditor(SCI_CHARRIGHTRECTEXTEND, 0, 0);
}

/* void homeRectExtend(); */
void SciMoz::HomeRectExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::HomeRectExtend\n");
#endif
	SendEditor(SCI_HOMERECTEXTEND, 0, 0);
}

/* void vCHomeRectExtend(); */
void SciMoz::VCHomeRectExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::VCHomeRectExtend\n");
#endif
	SendEditor(SCI_VCHOMERECTEXTEND, 0, 0);
}

/* void lineEndRectExtend(); */
void SciMoz::LineEndRectExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LineEndRectExtend\n");
#endif
	SendEditor(SCI_LINEENDRECTEXTEND, 0, 0);
}

/* void pageUpRectExtend(); */
void SciMoz::PageUpRectExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PageUpRectExtend\n");
#endif
	SendEditor(SCI_PAGEUPRECTEXTEND, 0, 0);
}

/* void pageDownRectExtend(); */
void SciMoz::PageDownRectExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::PageDownRectExtend\n");
#endif
	SendEditor(SCI_PAGEDOWNRECTEXTEND, 0, 0);
}

/* void stutteredPageUp(); */
void SciMoz::StutteredPageUp() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StutteredPageUp\n");
#endif
	SendEditor(SCI_STUTTEREDPAGEUP, 0, 0);
}

/* void stutteredPageUpExtend(); */
void SciMoz::StutteredPageUpExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StutteredPageUpExtend\n");
#endif
	SendEditor(SCI_STUTTEREDPAGEUPEXTEND, 0, 0);
}

/* void stutteredPageDown(); */
void SciMoz::StutteredPageDown() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StutteredPageDown\n");
#endif
	SendEditor(SCI_STUTTEREDPAGEDOWN, 0, 0);
}

/* void stutteredPageDownExtend(); */
void SciMoz::StutteredPageDownExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StutteredPageDownExtend\n");
#endif
	SendEditor(SCI_STUTTEREDPAGEDOWNEXTEND, 0, 0);
}

/* void wordLeftEnd(); */
void SciMoz::WordLeftEnd() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordLeftEnd\n");
#endif
	SendEditor(SCI_WORDLEFTEND, 0, 0);
}

/* void wordLeftEndExtend(); */
void SciMoz::WordLeftEndExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordLeftEndExtend\n");
#endif
	SendEditor(SCI_WORDLEFTENDEXTEND, 0, 0);
}

/* void wordRightEnd(); */
void SciMoz::WordRightEnd() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordRightEnd\n");
#endif
	SendEditor(SCI_WORDRIGHTEND, 0, 0);
}

/* void wordRightEndExtend(); */
void SciMoz::WordRightEndExtend() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::WordRightEndExtend\n");
#endif
	SendEditor(SCI_WORDRIGHTENDEXTEND, 0, 0);
}

/* void setWhitespaceChars(in wstring characters); */
void SciMoz::SetWhitespaceChars(NPStringcharacters) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetWhitespaceChars\n");
#endif
	SendEditor(SCI_SETWHITESPACECHARS, 0, reinterpret_cast<long>(NPStringToString(characters));
}

/* void setCharsDefault(); */
void SciMoz::SetCharsDefault() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCharsDefault\n");
#endif
	SendEditor(SCI_SETCHARSDEFAULT, 0, 0);
}

/* long autoCGetCurrent(); */
int32  SciMoz::AutoCGetCurrent() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::AutoCGetCurrent\n");
#endif
	return SendEditor(SCI_AUTOCGETCURRENT, 0, 0);
}

/* void allocate(in long bytes); */
void SciMoz::Allocate(int32 bytes) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::Allocate\n");
#endif
	SendEditor(SCI_ALLOCATE, bytes, 0);
}

/* long targetAsUTF8(out wstring s); */
int32  SciMoz::TargetAsUTF8(PRUnichar **s) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::TargetAsUTF8\n");
#endif
	static char _buffer[32 * 1024];
/*#ifdef NS_DEBUG
	static PRThread *myThread = nsnull;
	if (myThread == nsnull)
		myThread = PR_GetCurrentThread();
	// If this fires, caller should be using a proxy!  Scintilla is not free-threaded!
	NS_PRECONDITION(PR_GetCurrentThread()==myThread, "buffer (and Scintilla!) is not thread-safe!!!!");
#endif */ // NS_DEBUG
	_buffer[32 * 1024-1] = '\0';						short _buflen = static_cast<short>(sizeof(_buffer)-1);
	memcpy(_buffer, &_buflen, sizeof(_buflen));
	return SendEditor(SCI_TARGETASUTF8, 0, reinterpret_cast<long>(_buffer));
	*s =  ToNewUnicode(NS_ConvertUTF8toUTF16(_buffer));
}

/* void setLengthForEncode(in long bytes); */
void SciMoz::SetLengthForEncode(int32 bytes) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetLengthForEncode\n");
#endif
	SendEditor(SCI_SETLENGTHFORENCODE, bytes, 0);
}

/* long encodedFromUTF8(in wstring utf8, out wstring encoded); */
int32  SciMoz::EncodedFromUTF8(NPStringutf8, PRUnichar **encoded) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::EncodedFromUTF8\n");
#endif
	static char _buffer[32 * 1024];
/*#ifdef NS_DEBUG
	static PRThread *myThread = nsnull;
	if (myThread == nsnull)
		myThread = PR_GetCurrentThread();
	// If this fires, caller should be using a proxy!  Scintilla is not free-threaded!
	NS_PRECONDITION(PR_GetCurrentThread()==myThread, "buffer (and Scintilla!) is not thread-safe!!!!");
#endif */ // NS_DEBUG
	_buffer[32 * 1024-1] = '\0';						short _buflen = static_cast<short>(sizeof(_buffer)-1);
	memcpy(_buffer, &_buflen, sizeof(_buflen));
	return SendEditor(SCI_ENCODEDFROMUTF8, reinterpret_cast<unsigned long>(NPStringToString(utf8), reinterpret_cast<long>(_buffer));
	*encoded =  ToNewUnicode(NS_ConvertUTF8toUTF16(_buffer));
}

/* long findColumn(in long line, in long column); */
int32  SciMoz::FindColumn(int32 line, int32 column) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::FindColumn\n");
#endif
	return SendEditor(SCI_FINDCOLUMN, line, column);
}

/* attribute boolean caretSticky; */
bool  SciMoz::GetCaretSticky() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCaretSticky\n");
#endif
	return SendEditor(SCI_GETCARETSTICKY, 0, 0);
}

/* attribute boolean caretSticky; */
void SciMoz::SetCaretSticky(bool useCaretStickyBehaviour) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCaretSticky\n");
#endif
	SendEditor(SCI_SETCARETSTICKY, useCaretStickyBehaviour, 0);
}

/* void toggleCaretSticky(); */
void SciMoz::ToggleCaretSticky() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::ToggleCaretSticky\n");
#endif
	SendEditor(SCI_TOGGLECARETSTICKY, 0, 0);
}

/* attribute boolean pasteConvertEndings; */
void SciMoz::SetPasteConvertEndings(bool convert) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetPasteConvertEndings\n");
#endif
	SendEditor(SCI_SETPASTECONVERTENDINGS, convert, 0);
}

/* attribute boolean pasteConvertEndings; */
bool  SciMoz::GetPasteConvertEndings() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetPasteConvertEndings\n");
#endif
	return SendEditor(SCI_GETPASTECONVERTENDINGS, 0, 0);
}

/* void selectionDuplicate(); */
void SciMoz::SelectionDuplicate() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SelectionDuplicate\n");
#endif
	SendEditor(SCI_SELECTIONDUPLICATE, 0, 0);
}

/* attribute long caretLineBackAlpha; */
void SciMoz::SetCaretLineBackAlpha(int32 alpha) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCaretLineBackAlpha\n");
#endif
	SendEditor(SCI_SETCARETLINEBACKALPHA, alpha, 0);
}

/* attribute long caretLineBackAlpha; */
int32  SciMoz::GetCaretLineBackAlpha() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCaretLineBackAlpha\n");
#endif
	return SendEditor(SCI_GETCARETLINEBACKALPHA, 0, 0);
}

/* attribute long caretStyle; */
void SciMoz::SetCaretStyle(int32 caretStyle) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCaretStyle\n");
#endif
	SendEditor(SCI_SETCARETSTYLE, caretStyle, 0);
}

/* attribute long caretStyle; */
int32  SciMoz::GetCaretStyle() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCaretStyle\n");
#endif
	return SendEditor(SCI_GETCARETSTYLE, 0, 0);
}

/* attribute long indicatorCurrent; */
void SciMoz::SetIndicatorCurrent(int32 indicator) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetIndicatorCurrent\n");
#endif
	SendEditor(SCI_SETINDICATORCURRENT, indicator, 0);
}

/* attribute long indicatorCurrent; */
int32  SciMoz::GetIndicatorCurrent() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetIndicatorCurrent\n");
#endif
	return SendEditor(SCI_GETINDICATORCURRENT, 0, 0);
}

/* attribute long indicatorValue; */
void SciMoz::SetIndicatorValue(int32 value) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetIndicatorValue\n");
#endif
	SendEditor(SCI_SETINDICATORVALUE, value, 0);
}

/* attribute long indicatorValue; */
int32  SciMoz::GetIndicatorValue() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetIndicatorValue\n");
#endif
	return SendEditor(SCI_GETINDICATORVALUE, 0, 0);
}

/* void indicatorFillRange(in long position, in long fillLength); */
void SciMoz::IndicatorFillRange(int32 position, int32 fillLength) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicatorFillRange\n");
#endif
	SendEditor(SCI_INDICATORFILLRANGE, position, fillLength);
}

/* void indicatorClearRange(in long position, in long clearLength); */
void SciMoz::IndicatorClearRange(int32 position, int32 clearLength) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicatorClearRange\n");
#endif
	SendEditor(SCI_INDICATORCLEARRANGE, position, clearLength);
}

/* long indicatorAllOnFor(in long position); */
int32  SciMoz::IndicatorAllOnFor(int32 position) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicatorAllOnFor\n");
#endif
	return SendEditor(SCI_INDICATORALLONFOR, position, 0);
}

/* long indicatorValueAt(in long indicator, in long position); */
int32  SciMoz::IndicatorValueAt(int32 indicator, int32 position) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicatorValueAt\n");
#endif
	return SendEditor(SCI_INDICATORVALUEAT, indicator, position);
}

/* long indicatorStart(in long indicator, in long position); */
int32  SciMoz::IndicatorStart(int32 indicator, int32 position) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicatorStart\n");
#endif
	return SendEditor(SCI_INDICATORSTART, indicator, position);
}

/* long indicatorEnd(in long indicator, in long position); */
int32  SciMoz::IndicatorEnd(int32 indicator, int32 position) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::IndicatorEnd\n");
#endif
	return SendEditor(SCI_INDICATOREND, indicator, position);
}

/* attribute long positionCache; */
void SciMoz::SetPositionCache(int32 size) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetPositionCache\n");
#endif
	SendEditor(SCI_SETPOSITIONCACHE, size, 0);
}

/* attribute long positionCache; */
int32  SciMoz::GetPositionCache() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetPositionCache\n");
#endif
	return SendEditor(SCI_GETPOSITIONCACHE, 0, 0);
}

/* void copyAllowLine(); */
void SciMoz::CopyAllowLine() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::CopyAllowLine\n");
#endif
	SendEditor(SCI_COPYALLOWLINE, 0, 0);
}

/* readonly attribute long characterPointer; */
int32  SciMoz::GetCharacterPointer() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetCharacterPointer\n");
#endif
	return SendEditor(SCI_GETCHARACTERPOINTER, 0, 0);
}

/* attribute boolean keysUnicode; */
void SciMoz::SetKeysUnicode(bool keysUnicode) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetKeysUnicode\n");
#endif
	SendEditor(SCI_SETKEYSUNICODE, keysUnicode, 0);
}

/* attribute boolean keysUnicode; */
bool  SciMoz::GetKeysUnicode() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetKeysUnicode\n");
#endif
	return SendEditor(SCI_GETKEYSUNICODE, 0, 0);
}

/* void startRecord(); */
void SciMoz::StartRecord() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StartRecord\n");
#endif
	SendEditor(SCI_STARTRECORD, 0, 0);
}

/* void stopRecord(); */
void SciMoz::StopRecord() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::StopRecord\n");
#endif
	SendEditor(SCI_STOPRECORD, 0, 0);
}

/* attribute long lexer; */
void SciMoz::SetLexer(int32 lexer) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetLexer\n");
#endif
	SendEditor(SCI_SETLEXER, lexer, 0);
}

/* attribute long lexer; */
int32  SciMoz::GetLexer() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetLexer\n");
#endif
	return SendEditor(SCI_GETLEXER, 0, 0);
}

/* void colourise(in long start, in long end); */
void SciMoz::Colourise(int32 start, int32 end) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::Colourise\n");
#endif
	SendEditor(SCI_COLOURISE, start, end);
}

/* void setProperty(in wstring key, in wstring value); */
void SciMoz::SetProperty(NPStringkey, NPStringvalue) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetProperty\n");
#endif
	SendEditor(SCI_SETPROPERTY, reinterpret_cast<unsigned long>(NPStringToString(key), reinterpret_cast<long>(NPStringToString(value));
}

/* void setKeyWords(in long keywordSet, in wstring keyWords); */
void SciMoz::SetKeyWords(int32 keywordSet, NPStringkeyWords) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetKeyWords\n");
#endif
	SendEditor(SCI_SETKEYWORDS, keywordSet, reinterpret_cast<long>(NPStringToString(keyWords));
}

/* void setLexerLanguage(in wstring language); */
void SciMoz::SetLexerLanguage(NPStringlanguage) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetLexerLanguage\n");
#endif
	SendEditor(SCI_SETLEXERLANGUAGE, 0, reinterpret_cast<long>(NPStringToString(language));
}

/* void loadLexerLibrary(in wstring path); */
void SciMoz::LoadLexerLibrary(NPStringpath) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::LoadLexerLibrary\n");
#endif
	SendEditor(SCI_LOADLEXERLIBRARY, 0, reinterpret_cast<long>(NPStringToString(path));
}

/* long getProperty(in wstring key, out wstring buf); */
int32  SciMoz::GetProperty(NPStringkey, PRUnichar **buf) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetProperty\n");
#endif
	static char _buffer[32 * 1024];
/*#ifdef NS_DEBUG
	static PRThread *myThread = nsnull;
	if (myThread == nsnull)
		myThread = PR_GetCurrentThread();
	// If this fires, caller should be using a proxy!  Scintilla is not free-threaded!
	NS_PRECONDITION(PR_GetCurrentThread()==myThread, "buffer (and Scintilla!) is not thread-safe!!!!");
#endif */ // NS_DEBUG
	_buffer[32 * 1024-1] = '\0';						short _buflen = static_cast<short>(sizeof(_buffer)-1);
	memcpy(_buffer, &_buflen, sizeof(_buflen));
	return SendEditor(SCI_GETPROPERTY, reinterpret_cast<unsigned long>(NPStringToString(key), reinterpret_cast<long>(_buffer));
	*buf =  ToNewUnicode(NS_ConvertUTF8toUTF16(_buffer));
}

/* long getPropertyExpanded(in wstring key, out wstring buf); */
int32  SciMoz::GetPropertyExpanded(NPStringkey, PRUnichar **buf) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetPropertyExpanded\n");
#endif
	static char _buffer[32 * 1024];
/*#ifdef NS_DEBUG
	static PRThread *myThread = nsnull;
	if (myThread == nsnull)
		myThread = PR_GetCurrentThread();
	// If this fires, caller should be using a proxy!  Scintilla is not free-threaded!
	NS_PRECONDITION(PR_GetCurrentThread()==myThread, "buffer (and Scintilla!) is not thread-safe!!!!");
#endif */ // NS_DEBUG
	_buffer[32 * 1024-1] = '\0';						short _buflen = static_cast<short>(sizeof(_buffer)-1);
	memcpy(_buffer, &_buflen, sizeof(_buflen));
	return SendEditor(SCI_GETPROPERTYEXPANDED, reinterpret_cast<unsigned long>(NPStringToString(key), reinterpret_cast<long>(_buffer));
	*buf =  ToNewUnicode(NS_ConvertUTF8toUTF16(_buffer));
}

/* long getPropertyInt(in wstring key); */
int32  SciMoz::GetPropertyInt(NPStringkey) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetPropertyInt\n");
#endif
	return SendEditor(SCI_GETPROPERTYINT, reinterpret_cast<unsigned long>(NPStringToString(key), 0);
}

/* readonly attribute long styleBitsNeeded; */
int32  SciMoz::GetStyleBitsNeeded() {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::GetStyleBitsNeeded\n");
#endif
	return SendEditor(SCI_GETSTYLEBITSNEEDED, 0, 0);
}

/* void setCaretPolicy(in long caretPolicy, in long caretSlop); */
void SciMoz::SetCaretPolicy(int32 caretPolicy, int32 caretSlop) {
#ifdef SCIMOZ_DEBUG
	printf("SciMoz::SetCaretPolicy\n");
#endif
	SendEditor(SCI_SETCARETPOLICY, caretPolicy, caretSlop);
}

