//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Log Viewer Control
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <Clipbrd.hpp>

#include <stdlib.h>
#include "WideNativeFuncs.h"
#include "MsgIntf.h"
#include "TLogViewer.h"


//---------------------------------------------------------------------------
void TVPCopyToClipboard(const ttstr & unicode)
{
	TClipboard *cb = Clipboard();
	cb->Open();
	HGLOBAL ansihandle = NULL;
	HGLOBAL unicodehandle = NULL;
	try
	{
		// store ANSI string
		AnsiString ansistr = unicode.AsAnsiString();
		ansihandle = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE,
			ansistr.Length() + 1);
		if(!ansihandle) throw Exception("copying to clipboard failed.");

		char *mem = (char*)GlobalLock(ansihandle);
		if(mem) strcpy(mem, ansistr.c_str());
		GlobalUnlock(ansihandle);

		cb->SetAsHandle(CF_TEXT, (int)ansihandle);
		ansihandle = NULL;

		// store UNICODE string
		unicodehandle = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE,
			(unicode.GetLen() + 1) * sizeof(tjs_char));
		if(!unicodehandle)  throw Exception("copying to clipboard failed.");

		tjs_char *unimem = (tjs_char*)GlobalLock(unicodehandle);
		if(unimem) TJS_strcpy(unimem, unicode.c_str());
		GlobalUnlock(unicodehandle);

		cb->SetAsHandle(CF_UNICODETEXT, (int)unicodehandle);
		unicodehandle = NULL;

	}
	catch(...)
	{
		if(ansihandle) GlobalFree(ansihandle);
		if(unicodehandle) GlobalFree(unicodehandle);
		cb->Close();
		throw;
	}
	cb->Close();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
__fastcall TLogViewer::TLogViewer(TWinControl *owner) : TCustomControl(Owner)
{
	Brush->Style = bsClear;
	Color = clBlack;
	Cursor = crIBeam;

	FDataLength = 0;
	FFirstLine = 0;
	FMargin = 3;
	FLastClickedDataPos = 0;

	FSelStart = 0;
	FSelLength = 0;
	ScrollTimer = NULL;

	CharWidthMap = NULL;

	FMouseSelecting = false;
	FDoubleClickSelecting = false;
}
//---------------------------------------------------------------------------
__fastcall TLogViewer::~TLogViewer()
{
	if(CharWidthMap) delete [] CharWidthMap;
	if(ScrollTimer) delete ScrollTimer;
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::CreateParams(TCreateParams &params)
{
	// create window parameters
	inherited::CreateParams(params);
	params.Style |= WS_VSCROLL;
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::Paint()
{
	int start = Canvas->ClipRect.top / FLineHeight;
	int end = Canvas->ClipRect.bottom / FLineHeight;

	Brush->Style = bsSolid;
	Canvas->Brush->Color = clBlack;

	int sellim = FSelStart + FSelLength;

	for(int y = start; y <= end; y++)
	{
		int ys = y * FLineHeight;
		Canvas->Brush->Color = clBlack;
		Canvas->FillRect(TRect(0, ys, ClientWidth, ys + FLineHeight) );
		if((unsigned int)(FFirstLine + y) < FDisplayLineData.size())
		{
			const TDisplayLineData &data = FDisplayLineData[FFirstLine + y];

			Canvas->Brush->Color = clBlack;
			Canvas->Font->Color = clWhite;

			const tjs_char *data_start = FData.c_str() + data.Start;
			tjs_int data_len = data.Length;
			bool allocated = false;

			for(tjs_int i = 0; i < data_len; i++)
			{
				if(data_start[i] == TJS_W('\t'))
				{
					// includes tab
					const tjs_char *data_start_org = data_start;
					data_start = new tjs_char[data_len];
					allocated = true;
					memcpy(const_cast<tjs_char*>(data_start),
						data_start_org, sizeof(tjs_char)*data_len);
					for(; i < data_len; i++)
					{
						if(data_start[i] == TJS_W('\t'))
							const_cast<tjs_char*>(data_start)[i] = TJS_W(' ');
					}
					break;
				}
			}

			if(procTextOutW)
			{
				procTextOutW(Canvas->Handle, FMargin, ys + 1,
					data_start, data_len);
			}
			else
			{
				char *buf2 = new char[data_len * 3 + 1];

				const tjs_char *p = data_start;
				char *pp = buf2;
				for(int i = 0; i < data_len; i++)
				{
					TJS_wctomb(pp, 0);
					int narrowlen = TJS_wctomb(pp, p[i]);
					if(narrowlen != -1) pp += narrowlen;
				}
				*pp = 0;

				TextOut(Canvas->Handle, FMargin, ys + 1,
					buf2, pp - buf2);

				delete [] buf2;
			}

			if(allocated) delete [] data_start;

			if(FSelLength > 0)
			{
				int ss = data.Start < FSelStart ? FSelStart : data.Start;
				int dl = data.Start + data.Length;
				int se = dl < sellim ? dl : sellim;
				if(ss < se)
				{
					// invert selection
					RECT r;
					r.left = GetTextWidth(FData.c_str() + data.Start,
						ss - data.Start) + FMargin;
					r.right = GetTextWidth(FData.c_str() + data.Start,
						se - data.Start) + FMargin;
					r.top = ys;
					r.bottom = ys + FLineHeight;
					InvertRect(Canvas->Handle, &r);
				}
			}
		}
	}

	Brush->Style = bsClear;
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::InvalidateRange(int start, int len)
{
	if(len <= 0) return;

	int st = DataPosToLine(start) - FFirstLine;
	int ed = DataPosToLine(start + len) - FFirstLine;

	if(st < 0) st = 0;
	if(ed >= FViewLines) ed = FViewLines;

	if(ed >= st)
	{
		RECT r;
		r.left = 0;
		r.top = st * FLineHeight;
		r.right = ClientWidth;
		r.bottom = (ed+1) * FLineHeight;
		::InvalidateRect(Handle, &r, FALSE);
	}
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::CreateCharWidthMap()
{
	// create table for character size
	if(CharWidthMap) return;
	CharWidthMap = new tjs_uint16[65536];
	CharWidthMap[0] = 0;

	for(tjs_int i = 1; i < 65536; i++) CharWidthMap[i] = 0xffff;
}
//---------------------------------------------------------------------------
int __fastcall TLogViewer::GetTextWidth(const tjs_char *txt, int len)
{
	CreateCharWidthMap();
	int w = 0;
	while(len--)
	{
		if(CharWidthMap[*txt] == 0xffff)
		{
			if(!procGetTextExtentPoint32W)
			{
				char narrow[10 + 1];
				TJS_wctomb(narrow, 0);
				tjs_char wc = *txt;
				if(wc == TJS_W('\t')) wc = TJS_W(' ');
				int narrowlen = TJS_wctomb(narrow, wc);
				if(narrowlen == -1)
				{
					CharWidthMap[*txt] = 0;
				}
				else
				{
					SIZE s;
					GetTextExtentPoint32A(Canvas->Handle, narrow, narrowlen, &s);
					CharWidthMap[*txt] = s.cx;
				}
			}
			else
			{
				tjs_char wc = *txt;
				if(wc == TJS_W('\t')) wc = TJS_W(' ');
				SIZE s;
				procGetTextExtentPoint32W(Canvas->Handle, &wc, 1, &s);
				CharWidthMap[*txt] = s.cx;
			}
		}
		w += CharWidthMap[*txt];
		txt++;
	}
	return w;
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::InternalLayout(int start)
{
	int line = start ? DataPosToLine(start) : 0;

	int clientsize = ClientWidth - FMargin * 2;

	const tjs_char *p = FData.c_str();
	if(!p) return;
	const tjs_char *org_p = p;

	if(line > 0) p += FDisplayLineData[line].Start;
	FDisplayLineData.resize(line);


	const tjs_char *p_start = p;
	while(*p)
	{
		int w = 0;
		while(*p)
		{
			if(*p == '\n' || *p == '\r')
			{
				FDisplayLineData.push_back(
					TDisplayLineData(p_start - org_p, p - p_start));
				if(*p == '\r')
				{
					p++;
					if(*p == '\n') p++;
				}
				else
				{
					p++;
				}
				p_start = p;
				break;
			}
			w += GetTextWidth(p, 1);
			if(w >= clientsize)
			{
				if(p == p_start) p++;
				FDisplayLineData.push_back(
					TDisplayLineData(p_start - org_p, p - p_start));
				p_start = p;
				break;
			}
			p++;
		}
	}
	if(p - p_start)
	{
		FDisplayLineData.push_back(
			TDisplayLineData(p_start - org_p, p - p_start));
	}
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::RedoLayout()
{
	// layout all lines
	InternalLayout(0);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::SetFont(TFont * font)
{
	Canvas->Font->Assign(font);
	delete [] CharWidthMap;
	CharWidthMap = NULL;
	Resize();
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::SetText(const ttstr & str)
{
	// set text
	bool showinglast = IsShowingLast();
	FData = str;
	FDataLength = str.GetLen();
	RedoLayout();
	SetScrollRange();
	if(showinglast) ShowLast();
	Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::Append(const ttstr & str, bool appendcr)
{
	// append to log
	bool showinglast = IsShowingLast();
	int last = FDataLength;
	int strlen = str.GetLen();
	if(appendcr) strlen++;
	FData += str;
	if(appendcr) FData += TJS_W('\n');
	FDataLength += strlen;
	InternalLayout(last);
	SetScrollRange();
	if(showinglast) ShowLast();
	InvalidateRange(last, strlen);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::Trim(int maxlen, int trimlen)
{
	if(FDataLength < maxlen) return;
	if(FDataLength < trimlen) return;

	int surplus = FDataLength - trimlen;

	int tofirstline = DataPosToLine(surplus);
	if(tofirstline == 0) return;

	int ofs = FDisplayLineData[tofirstline].Start;

	int lim = FDisplayLineData.size() - tofirstline;

	for(int i = 0; i < lim; i++)
	{
		FDisplayLineData[i] = FDisplayLineData[i + tofirstline];
		FDisplayLineData[i].Start -= ofs;
	}

	FDisplayLineData.resize(lim);

	FData = ttstr(FData.c_str() + ofs);
	FDataLength -= ofs;

	FSelStart -= ofs;
	if(FSelStart < 0)
	{
		FSelLength += FSelStart;
		FSelStart = 0;
		if(FSelLength < 0) FSelLength = 0;
	}

	FLastClickedDataPos -= ofs;
	if(FLastClickedDataPos < 0) FLastClickedDataPos = 0;


	FLastDblClickedLineStart -= ofs;
	if(FLastDblClickedLineStart < 0)
	{
		FLastDblClickedLineLength += FLastDblClickedLineStart;
		FLastDblClickedLineStart = 0;
		if(FLastDblClickedLineLength < 0) FLastDblClickedLineLength = 0;
	}

	ScrollBy(-tofirstline);
	SetScrollRange();

	Invalidate();
}
//---------------------------------------------------------------------------
int __fastcall TLogViewer::ClickPosToDataPos(int x, int y)
{
	// compute data pos from clicked client position
	y /= FLineHeight;
	y += FFirstLine;
	x -= FMargin;

	if(y < 0) return 0;
	if((unsigned int)y >= FDisplayLineData.size()) return FDataLength;

	const TDisplayLineData &data = FDisplayLineData[y];

	const tjs_char * str = FData.c_str() + data.Start;
	int w = 0;
	int i;
	for(i = 0; i < data.Length; i++)
	{
		int cw = GetTextWidth(str + i, 1);
		w += cw;
		if(w > x)
		{
			w -= cw;
			if(x - w > (cw>>1)) i++;
			break;
		}
	}

	int result = i + data.Start;
	if(result < 0) result = 0;
	if(result > FDataLength) result = FDataLength;

	return result;
}
//---------------------------------------------------------------------------
int __fastcall TLogViewer::DataPosToLine(int pos)
{
	// convert data position to display line number
	int linecount = FDisplayLineData.size();

	if(!linecount) return 0;

	if(pos < 0) pos = 0;
	if(pos >= FDataLength) return linecount - 1;


	// do binary search
	int s = 0, e = linecount;
	while(true)
	{
		if(s == e) return s;

		int m = (s + e) /2;
		const TDisplayLineData &mdata = FDisplayLineData[m];

		int lim;
		if(m == linecount - 1)
			lim = FDataLength;
		else
			lim = FDisplayLineData[m+1].Start;

		if(pos >= mdata.Start && pos < lim) return m;

		if(mdata.Start < pos)
			s = m;
		else
			e = m;
	}
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::DataPosToLogicalLine(int pos, int &start, int &len)
{
	// convert data position to logical line

	// search backward
	const tjs_char * data = FData.c_str();
	int p = pos;
	p--;
	while(p >= 0)
	{
		if(data[p] == '\n') break;
		p--;
	}
	p++;

	start = p;

	// search forward
	p = pos;
	while(p < FDataLength)
	{
		if(data[p] == '\n') break;
		p++;
	}
	if(p < FDataLength && data[p] == '\n') p++;

	len = p - start;
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::SetSelection(int start, int len)
{
	if(FSelLength > 0)
	{
		if(len > 0)
		{
			int s, e;
			if(FSelStart > start)
				s = start, e = FSelStart;
			else
				s = FSelStart, e = start;
			InvalidateRange(s, e - s);
			if(FSelStart + FSelLength > start + len)
				s = start + len, e = FSelStart + FSelLength;
			else
				s = FSelStart + FSelLength, e = start + len;
			InvalidateRange(s, e - s);
		}
		else
		{
			InvalidateRange(FSelStart, FSelLength);
		}
	}
	else
	{
		if(len > 0)
		{
			InvalidateRange(start, len);
		}
	}

	FSelStart = start;
	FSelLength = len;
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::StartSelect()
{
	TPoint pt = GetMousePos();

	FLastClickedDataPos = ClickPosToDataPos(pt.x, pt.y);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::UpdateMouseSelect()
{
	TPoint pt = GetMousePos();

	int s, e;
	int pos = ClickPosToDataPos(pt.x, pt.y);
	if(FDoubleClickSelecting)
	{
		int st, len;
		DataPosToLogicalLine(pos, st, len);
		if(st < FLastDblClickedLineStart)
			s = st, e = FLastDblClickedLineStart + FLastDblClickedLineLength;
		else
			s = FLastDblClickedLineStart, e = st + len;
	}
	else
	{
		if(pos < FLastClickedDataPos)
			s = pos, e = FLastClickedDataPos;
		else
			s = FLastClickedDataPos, e = pos;
	}

	SetSelection(s, e - s);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::SelectAll()
{
	SetSelection(0, FDataLength);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::CopyToClipboard()
{
	TVPCopyToClipboard(ttstr(FData.c_str() + FSelStart, FSelLength));
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::DblClick()
{
	TPoint pt = GetMousePos();
	int pos = ClickPosToDataPos(pt.x, pt.y);
	int start, len;

	DataPosToLogicalLine(pos, start, len);

	SetSelection(start, len);

	FLastDblClickedLineStart = start;
	FLastDblClickedLineLength = len;

	FDoubleClickSelecting = true;

	inherited::DblClick();
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::MouseDown(TMouseButton button, TShiftState shift,
	int x, int y)
{
	if(button == mbLeft)
	{
		if(shift.Contains(ssShift))
		{
			UpdateMouseSelect();
		}
		else
		{
			StartScrollTimer();
			StartSelect();
			FMouseSelecting = true;
		}
	}
	inherited::MouseDown(button, shift, x, y);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::MouseUp(TMouseButton button, TShiftState shift,
	int x, int y)
{
	if(button == mbLeft)
	{
		if(FMouseSelecting && !FDoubleClickSelecting) UpdateMouseSelect();
		FMouseSelecting = false;
		FDoubleClickSelecting = false;
		EndScrollTimer();
	}
	inherited::MouseUp(button, shift, x, y);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::MouseMove(TShiftState shift, int x, int y)
{
	if(FMouseSelecting)
	{
		UpdateMouseSelect();
		ScrollTimerHandler(this);
	}

	inherited::MouseMove(shift, x, y);
}
//---------------------------------------------------------------------------
TPoint __fastcall TLogViewer::GetMousePos()
{
	POINT p;
	::GetCursorPos(&p);
	return ScreenToClient(TPoint(p));
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::Resize(void)
{
	bool showinglast = IsShowingLast();
	RecalcMetrics();
	RedoLayout();
	SetScrollRange();
	if(showinglast) ShowLast();
	inherited::Resize();
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::RecalcMetrics()
{
	FLineHeight = Canvas->TextHeight("|") + 2;
	FViewLines = Height / FLineHeight;
	if(FViewLines * FLineHeight < Height) FViewLines ++;
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::WMVScroll(TWMVScroll &msg)
{
	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(Handle, SB_VERT, &si);

	if(msg.ScrollCode == SB_THUMBTRACK)
	{
		si.nPos = si.nTrackPos;
	}
	else if(msg.ScrollCode == SB_LINELEFT)
	{
		si.nPos --;
		if(si.nPos<0) si.nPos=0;
	}
	else if(msg.ScrollCode == SB_LINERIGHT)
	{
		si.nPos ++;
		if(si.nPos >= si.nMax) si.nPos = si.nMax - 1;
	}
	else if(msg.ScrollCode == SB_PAGELEFT)
	{
		si.nPos = si.nPos - si.nPage;
		if(si.nPos<0) si.nPos=0;
	}
	else if(msg.ScrollCode == SB_PAGERIGHT)
	{
		si.nPos = si.nPos + si.nPage;
		if(si.nPos >=si.nMax) si.nPos = si.nMax - 1;
	}

	ScrollTo(si.nPos);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::ScrollTo(int pos)
{
	int org = FFirstLine;

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.nPos = pos;
	si.fMask = SIF_POS;

	SetScrollInfo(Handle, SB_VERT, &si, true);

	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(Handle, SB_VERT, &si);
	FFirstLine = si.nPos;

	RECT r;
	r.left = 0;
	r.top = 0;
	r.right = ClientWidth;
	r.bottom = ClientHeight;

	ScrollWindowEx(Handle, 0, (org -si.nPos) * FLineHeight,
		&r, NULL, NULL, NULL, SW_INVALIDATE);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::ScrollBy(int lines)
{
	ScrollTo(FFirstLine + lines);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::SetScrollRange()
{
	// set scroll range
	// enable scroll bar and set scroll range
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;
	si.nMin = 0;
	si.nMax = FDisplayLineData.size() + 1;
	si.nPage = FViewLines;
	si.nPos = 0;
	SetScrollInfo(Handle, SB_VERT, &si, TRUE);

	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(Handle, SB_VERT, &si);
	FFirstLine = si.nPos;
}
//---------------------------------------------------------------------------
bool __fastcall TLogViewer::IsShowingLast()
{
	if(FDisplayLineData.size() <= (unsigned int)FViewLines) return true;
	
	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(Handle, SB_VERT, &si);

	return (unsigned int)si.nPos >= (si.nMax - si.nPage - 1);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::ShowLast()
{
	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(Handle, SB_VERT, &si);

	ScrollTo(si.nMax - 1);
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::StartScrollTimer()
{
	if(!ScrollTimer)
	{
		ScrollTimer = new TTimer(this);
		ScrollTimer->OnTimer = ScrollTimerHandler;
		ScrollTimer->Interval = 100;
		ScrollTimer->Enabled = true;
	}
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::EndScrollTimer()
{
	if(ScrollTimer) delete ScrollTimer, ScrollTimer = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TLogViewer::ScrollTimerHandler(TObject *Sender)
{
	POINT p;
	::GetCursorPos(&p);

	TPoint pt;
	pt.x = 0;
	pt.y = 0;
	pt = ClientToScreen(pt);

	int m = 0;

	if(p.y < pt.y)
	{
		m = (pt.y - p.y) / FLineHeight;
		m++;
		if(m > 10) m = 10;
		m = -m;
	}
	else if(p.y > pt.y + ClientHeight)
	{
		m = (p.y - (pt.y + ClientHeight)) / FLineHeight;
		m++;
		if(m > 10) m = 10;
	}

	if(m)
	{
		ScrollBy(m);
		UpdateMouseSelect();
	}
}
//---------------------------------------------------------------------------

#pragma package(smart_init)
