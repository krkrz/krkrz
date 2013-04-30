//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Log Viewer Control
//---------------------------------------------------------------------------
#ifndef TLogViewerH
#define TLogViewerH

#include <vector>

//---------------------------------------------------------------------------
void TVPCopyToClipboard(const ttstr & unicode);

//---------------------------------------------------------------------------
class TLogViewer : public TCustomControl
{
	typedef TCustomControl inherited;

	// data
	int FMargin; // left and right margin
	int FLineHeight; // height in pixel per a line
	int FViewLines; // line count in client area
	int FFirstLine; // first line in display lines

	int FSelStart; // selection start
	int FSelLength; // selection end

	int FLastClickedDataPos; // last clicked data pos
	int FLastDblClickedLineStart; // last double clicked line start
	int FLastDblClickedLineLength;

	bool FMouseSelecting; // selecting by mouse
	bool FDoubleClickSelecting;

	ttstr FData; // line data
	int FDataLength;

	struct TDisplayLineData
	{
		TDisplayLineData() { Start = Length = 0; }
		TDisplayLineData(int s, int l)
			{ Start = s, Length = l; }
		int Start;
		int Length;
	};

	std::vector<TDisplayLineData> FDisplayLineData;

	TTimer * ScrollTimer;

	tjs_uint16 *CharWidthMap;

public:
	// constructor/destructor and initializers
	__fastcall TLogViewer(TWinControl *owner);
	__fastcall ~TLogViewer();

	void __fastcall CreateParams(TCreateParams &params);


	// message map
protected:
BEGIN_MESSAGE_MAP
	VCL_MESSAGE_HANDLER(WM_VSCROLL , TWMVScroll , WMVScroll)
END_MESSAGE_MAP(TCustomControl)

	// painting
protected:
	void __fastcall Paint();
	void __fastcall InvalidateRange(int start, int len);

	// layout
private:
	void __fastcall CreateCharWidthMap();
protected:
	int __fastcall GetTextWidth(const tjs_char *txt, int len);

protected:
	void __fastcall InternalLayout(int start);
	void __fastcall RedoLayout();
public:
	void __fastcall SetFont(TFont * font);
	void __fastcall SetText(const ttstr & str);
	void __fastcall Append(const ttstr & str, bool appendcr = false);
	void __fastcall Trim(int maxlen, int trimlen);
protected:
	int __fastcall ClickPosToDataPos(int x, int y);
	int __fastcall DataPosToLine(int pos);
	void __fastcall DataPosToLogicalLine(int pos, int &start, int &len);

	// selection
protected:
	void __fastcall SetSelection(int start, int len);

	void __fastcall StartSelect();
	void __fastcall UpdateMouseSelect();

public:
	void __fastcall SelectAll();
	bool __fastcall CanCopy() { return FSelLength > 0; }
	void __fastcall CopyToClipboard();

	// mouse messages
protected:
	DYNAMIC void __fastcall DblClick();
	DYNAMIC void __fastcall MouseDown(TMouseButton button, TShiftState shift, int x, int y);
	DYNAMIC void __fastcall MouseUp(TMouseButton button, TShiftState shift, int x, int y);
	DYNAMIC void __fastcall MouseMove(TShiftState shift, int x, int y);
	TPoint __fastcall GetMousePos();

	// resizing
protected:
	DYNAMIC void __fastcall Resize(void);
	void __fastcall RecalcMetrics();

	// scrool bar management
protected:
	void __fastcall WMVScroll(TWMVScroll &msg);
	void __fastcall InternalScrollTo(int org, int pos);
public:
	void __fastcall ScrollTo(int pos);
	void __fastcall ScrollBy(int lines);
protected:
	void __fastcall SetScrollRange();
	bool __fastcall IsShowingLast();
public:
	void __fastcall ShowLast();
protected:
	void __fastcall StartScrollTimer();
	void __fastcall EndScrollTimer();
	void __fastcall ScrollTimerHandler(TObject *Sender);


	// properties
__published:
	__property Align;
	__property PopupMenu;

};
//---------------------------------------------------------------------------


#endif
