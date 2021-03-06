﻿/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : https://crystalmark.info/
//      License : The MIT License
/*---------------------------------------------------------------------------*/

#include "../stdafx.h"
#include "ButtonFx.h"

////------------------------------------------------
//   CButtonFx
////------------------------------------------------

CButtonFx::CButtonFx()
{
	// Control
	m_X = 0;
	m_Y = 0;
	m_RenderMode = SystemDraw;
	m_bHighContrast = FALSE;
	m_bDarkMode = FALSE;
	m_bDrawFrame = FALSE;

	// Glass
	m_GlassColor = RGB(255, 255, 255);
	m_GlassAlpha = 255;

	// Image
	m_ImageCount = 0;
	m_BkDC = NULL;
	m_bBkBitmapInit = FALSE;
	m_bBkLoad = FALSE;

	// Font
	m_TextAlign = BS_LEFT;
	m_TextColor = RGB(0, 0, 0);

	// Mouse
	m_bHover = FALSE;
	m_bFocas = FALSE;
	m_bTrackingNow = FALSE;
	m_bHandCursor = FALSE;
	m_bSelected = FALSE;
}

CButtonFx::~CButtonFx()
{
}

IMPLEMENT_DYNAMIC(CButtonFx, CButton)

BEGIN_MESSAGE_MAP(CButtonFx, CButton)
	//{{AFX_MSG_MAP(CButtonFx)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//------------------------------------------------
// Control
//------------------------------------------------

BOOL CButtonFx::InitControl(int x, int y, int width, int height, double zoomRatio, CDC* bkDC,
	LPCWSTR imagePath, int imageCount, DWORD textAlign, int renderMode, BOOL bHighContrast, BOOL bDarkMode)
{
	m_X = (int)(x * zoomRatio);
	m_Y = (int)(y * zoomRatio);
	m_CtrlSize.cx = (int)(width * zoomRatio);
	m_CtrlSize.cy = (int)(height * zoomRatio);
	MoveWindow(m_X, m_Y, m_CtrlSize.cx, m_CtrlSize.cy);

	m_BkDC = bkDC;
	m_ImagePath = imagePath;
	m_ImageCount = imageCount;
	m_RenderMode = renderMode;

	if (BS_LEFT <= textAlign && textAlign <= BS_CENTER)
	{
		m_TextAlign = textAlign;
	}

	if(m_ToolTip.m_hWnd != NULL)
	{
		if (m_ToolTip.GetToolCount() != 0)
		{
			m_ToolTip.DelTool(this, 1);
		}
		CRect rect;
		GetClientRect(rect);
		m_ToolTip.AddTool(this, m_ToolTipText, rect, 1);
	}

	m_bHighContrast = bHighContrast;
	m_bDarkMode = bDarkMode;

	if (m_bHighContrast)
	{
		ModifyStyle(BS_OWNERDRAW, m_TextAlign);

		return TRUE;
	}
	else if(renderMode & SystemDraw)
	{
		ModifyStyle(BS_OWNERDRAW, m_TextAlign);

		return TRUE;
	}
	else
	{
		SetBkReload();
		ModifyStyle(0, BS_OWNERDRAW);
	}

	if (renderMode & OwnerDrawImage)
	{
		if (!LoadBitmap(imagePath))
		{
			ModifyStyle(BS_OWNERDRAW, m_TextAlign);
		}
	}
	else if (renderMode & OwnerDrawGlass)
	{
		m_ImageCount = 3;
		m_CtrlImage.Destroy();
		m_CtrlImage.Create(m_CtrlSize.cx, m_CtrlSize.cy * m_ImageCount, 32);

		RECT rect;
		rect.left = 0;
		rect.right = m_CtrlSize.cx;
		
		CDC* pDC = CDC::FromHandle(m_CtrlImage.GetDC());
		rect.top = 0;
		rect.bottom = m_CtrlSize.cy;
		pDC->SetDCPenColor(RGB(128, 128, 128));
		pDC->SelectObject(GetStockObject(DC_PEN));
		pDC->Rectangle(&rect);

		rect.top = m_CtrlSize.cy;
		rect.bottom = m_CtrlSize.cy * 2;
		pDC->SetDCPenColor(RGB(64, 64, 255));
		pDC->SelectObject(GetStockObject(DC_PEN));
		pDC->Rectangle(&rect);

		rect.top = m_CtrlSize.cy * 2;
		rect.bottom = m_CtrlSize.cy * 3;
		pDC->SetDCPenColor(RGB(255, 64, 64));
		pDC->SelectObject(GetStockObject(DC_PEN));
		pDC->Rectangle(&rect);

		m_CtrlImage.ReleaseDC();

		m_CtrlBitmap.Detach();
		m_CtrlBitmap.Attach((HBITMAP)m_CtrlImage);

		DWORD length = m_CtrlSize.cx * m_CtrlSize.cy * m_ImageCount * 4;
		BYTE* bitmapBits = new BYTE[length];
		m_CtrlBitmap.GetBitmapBits(length, bitmapBits);

		BYTE r = (BYTE)GetRValue(m_GlassColor);
		BYTE g = (BYTE)GetGValue(m_GlassColor);
		BYTE b = (BYTE)GetBValue(m_GlassColor);
		BYTE a =  m_GlassAlpha;

		// for Debug
		// BYTE b0 = bitmapBits[0]; 128
		// BYTE b1 = bitmapBits[1]; 128
		// BYTE b2 = bitmapBits[2]; 128
		// BYTE b3 = bitmapBits[3];   0

		for (int i = 1; i <= m_ImageCount; i++)
		{
			for (int y = m_CtrlSize.cy * (i - 1) + 1; y < (int)(m_CtrlSize.cy * i) - 1; y++)
			{
				for (int x = 0 + 1 ; x < m_CtrlSize.cx - 1 ; x++)
				{
					bitmapBits[(y * m_CtrlSize.cx + x) * 4 + 0] = b;
					bitmapBits[(y * m_CtrlSize.cx + x) * 4 + 1] = g;
					bitmapBits[(y * m_CtrlSize.cx + x) * 4 + 2] = r;
				}
			}
		}

		for (int i = 1; i <= m_ImageCount; i++)
		{
			for (int y = m_CtrlSize.cy * (i - 1); y < (int)(m_CtrlSize.cy * i); y++)
			{
				for (int x = 0; x < m_CtrlSize.cx; x++)
				{
					bitmapBits[(y * m_CtrlSize.cx + x) * 4 + 3] = a;
				}
			}
		}

		m_CtrlBitmap.SetBitmapBits(length, bitmapBits);
		delete[] bitmapBits;

		SetDrawFrame(TRUE);
	}	
	else if (renderMode & OwnerDrawTransparent)
	{
		m_ImageCount = 1;
		m_CtrlImage.Destroy();
		m_CtrlImage.Create(m_CtrlSize.cx, m_CtrlSize.cy * m_ImageCount, 32);
		m_CtrlBitmap.Detach();
		m_CtrlBitmap.Attach((HBITMAP)m_CtrlImage);

		DWORD length = m_CtrlSize.cx * m_CtrlSize.cy * m_ImageCount * 4;
		BYTE* bitmapBits = new BYTE[length];
		m_CtrlBitmap.GetBitmapBits(length, bitmapBits);

		for (int y = 0; y < (int)(m_CtrlSize.cy * m_ImageCount); y++)
		{
			for (int x = 0; x < m_CtrlSize.cx; x++)
			{
				//	bitmapBits[(y * m_CtrlSize.cx + x) * 4 + 0] = 255;
				//	bitmapBits[(y * m_CtrlSize.cx + x) * 4 + 1] = 255;
				//	bitmapBits[(y * m_CtrlSize.cx + x) * 4 + 2] = 255;
				bitmapBits[(y * m_CtrlSize.cx + x) * 4 + 3] = (BYTE)0;
			}
		}

		m_CtrlBitmap.SetBitmapBits(length, bitmapBits);
		delete[] bitmapBits;
	}

	Invalidate();

	return TRUE;
}

BOOL CButtonFx::ReloadImage(LPCWSTR imagePath, UINT imageCount)
{
	if (imagePath != NULL && m_ImagePath.Compare(imagePath) == 0)
	{
		return FALSE;
	}

	m_ImagePath = imagePath;
	m_ImageCount = imageCount;

	LoadBitmap(imagePath);

	Invalidate();
	return TRUE;
}

void CButtonFx::SetMargin(int top, int left, int bottom, int right, double zoomRatio)
{
	m_Margin.top = (int)(top * zoomRatio);
	m_Margin.left = (int)(left * zoomRatio);
	m_Margin.bottom = (int)(bottom * zoomRatio);
	m_Margin.right = (int)(right * zoomRatio);
}

CSize CButtonFx::GetSize(void)
{
	return m_CtrlSize;
}

void CButtonFx::SetDrawFrame(BOOL bDrawFrame)
{
	if (bDrawFrame)
	{
		ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_DRAWFRAME);
	}
	else
	{
		ModifyStyleEx(WS_EX_STATICEDGE, 0, SWP_DRAWFRAME);
	}
}

void CButtonFx::SetDrawFrameEx(BOOL bDrawFrame, COLORREF frameColor)
{
	m_bDrawFrame = bDrawFrame;
	m_FrameColor = frameColor;
}

void CButtonFx::SetGlassColor(COLORREF glassColor, BYTE glassAlpha)
{
	m_GlassColor = glassColor;
	m_GlassAlpha = glassAlpha;
}

//------------------------------------------------
// Draw Control
//------------------------------------------------

void CButtonFx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (m_bHighContrast) { return CButton::DrawItem(lpDrawItemStruct); }

	CDC* drawDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	LoadCtrlBk(drawDC);

	if (IsWindowEnabled())
	{
		if (m_bSelected && m_ImageCount > ControlImageSelected)
		{
			DrawControl(drawDC, lpDrawItemStruct, m_CtrlBitmap, m_BkBitmap, ControlImageSelected);
		}
		else if ((lpDrawItemStruct->itemState & ODS_SELECTED || m_bHover) && m_ImageCount > ControlImageHover)
		{
			DrawControl(drawDC, lpDrawItemStruct, m_CtrlBitmap, m_BkBitmap, ControlImageHover);
		}
		else if ((lpDrawItemStruct->itemState & ODS_FOCUS || m_bFocas) && m_ImageCount > ControlImageFocus)
		{
			DrawControl(drawDC, lpDrawItemStruct, m_CtrlBitmap, m_BkBitmap, ControlImageFocus);
		}
		else
		{
			DrawControl(drawDC, lpDrawItemStruct, m_CtrlBitmap, m_BkBitmap, ControlImageNormal);
		}
	}
	else
	{
		if (m_ImageCount > ControlImageDisabled)
		{
			DrawControl(drawDC, lpDrawItemStruct, m_CtrlBitmap, m_BkBitmap, ControlImageDisabled);
		}
		else
		{
			DrawControl(drawDC, lpDrawItemStruct, m_CtrlBitmap, m_BkBitmap, ControlImageNormal);
		}
	}
}

void CButtonFx::DrawControl(CDC* drawDC, LPDRAWITEMSTRUCT lpDrawItemStruct, CBitmap& ctrlBitmap, CBitmap& bkBitmap, int no)
{
	CDC* pMemDC = new CDC;
	CBitmap* pOldMemBitmap;
	pMemDC->CreateCompatibleDC(drawDC);
	pOldMemBitmap = pMemDC->SelectObject(&ctrlBitmap);
	CDC* pBkDC = new CDC;
	CBitmap* pOldBkBitmap;
	pBkDC->CreateCompatibleDC(drawDC);
	pOldBkBitmap = pBkDC->SelectObject(&bkBitmap);

	if (drawDC->GetDeviceCaps(BITSPIXEL) * drawDC->GetDeviceCaps(PLANES) < 24)
	{
		drawDC->BitBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pMemDC, 0, m_CtrlSize.cy * no, SRCCOPY);
		DrawString(drawDC, lpDrawItemStruct);
	}
	else // Full Color (24/32bit)
	{
		CBitmap DrawBmp;
		DrawBmp.CreateCompatibleBitmap(drawDC, m_CtrlSize.cx, m_CtrlSize.cy);
		CDC* pDrawBmpDC = new CDC;
		CBitmap* pOldDrawBitmap;
		pDrawBmpDC->CreateCompatibleDC(drawDC);
		pOldDrawBitmap = pDrawBmpDC->SelectObject(&DrawBmp);

		if (!m_CtrlImage.IsNull())
		{
			if (m_CtrlImage.GetBPP() == 32)
			{
				BITMAP CtlBmpInfo, DstBmpInfo;
				bkBitmap.GetBitmap(&DstBmpInfo);
				DWORD DstLineBytes = DstBmpInfo.bmWidthBytes;
				DWORD DstMemSize = DstLineBytes * DstBmpInfo.bmHeight;
				ctrlBitmap.GetBitmap(&CtlBmpInfo);
				DWORD CtlLineBytes = CtlBmpInfo.bmWidthBytes;
				DWORD CtlMemSize = CtlLineBytes * CtlBmpInfo.bmHeight;

				BYTE* DstBuffer = new BYTE[DstMemSize];
				bkBitmap.GetBitmapBits(DstMemSize, DstBuffer);
				BYTE* CtlBuffer = new BYTE[CtlMemSize];
				ctrlBitmap.GetBitmapBits(CtlMemSize, CtlBuffer);

				int baseY = m_CtrlSize.cy * no;
				for (LONG py = 0; py < DstBmpInfo.bmHeight; py++)
				{
					int dn = py * DstLineBytes;
					int cn = (baseY + py) * CtlLineBytes;
					for (LONG px = 0; px < DstBmpInfo.bmWidth; px++)
					{
						BYTE a = CtlBuffer[cn + 3];
						BYTE na = 255 - a;
						DstBuffer[dn + 0] = (BYTE)((CtlBuffer[cn + 0] * a + DstBuffer[dn + 0] * na) / 255);
						DstBuffer[dn + 1] = (BYTE)((CtlBuffer[cn + 1] * a + DstBuffer[dn + 1] * na) / 255);
						DstBuffer[dn + 2] = (BYTE)((CtlBuffer[cn + 2] * a + DstBuffer[dn + 2] * na) / 255);
						dn += (DstBmpInfo.bmBitsPixel / 8);
						cn += (CtlBmpInfo.bmBitsPixel / 8);
					}
				}

				DrawBmp.SetBitmapBits(DstMemSize, DstBuffer);
				DrawString(pDrawBmpDC, lpDrawItemStruct);
				drawDC->BitBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pDrawBmpDC, 0, 0, SRCCOPY);

				delete[] DstBuffer;
				delete[] CtlBuffer;
			}
			else
			{
				pDrawBmpDC->BitBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pMemDC, 0, m_CtrlSize.cy * no, SRCCOPY);
				DrawString(pDrawBmpDC, lpDrawItemStruct);
				drawDC->BitBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pDrawBmpDC, 0, 0, SRCCOPY);
			}
		}
		else
		{
			pDrawBmpDC->BitBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pBkDC, 0, m_CtrlSize.cy * no, SRCCOPY);
			DrawString(pDrawBmpDC, lpDrawItemStruct);
			drawDC->BitBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pDrawBmpDC, 0, 0, SRCCOPY);
		}

		pDrawBmpDC->SelectObject(&pOldDrawBitmap);
		pDrawBmpDC->DeleteDC();
		delete pDrawBmpDC;
	}

	pMemDC->SelectObject(&pOldMemBitmap);
	pBkDC->SelectObject(&pOldBkBitmap);
	pMemDC->DeleteDC();
	pBkDC->DeleteDC();
	delete pMemDC;
	delete pBkDC;

	if (m_bDrawFrame)
	{
		CBrush brush;
		brush.CreateSolidBrush(m_FrameColor);
		drawDC->FrameRect(&(lpDrawItemStruct->rcItem), &brush);
		brush.DeleteObject();
	}
}

void CButtonFx::DrawString(CDC* drawDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CString title;
	GetWindowText(title);

	if (title.IsEmpty())
	{
		return;
	}

	drawDC->SetBkMode(TRANSPARENT);
	CRect rect = (CRect)(lpDrawItemStruct->rcItem);
	rect.top += m_Margin.top;
	rect.left += m_Margin.left;
	rect.bottom -= m_Margin.bottom;
	rect.right -= m_Margin.right;

	CArray<CString, CString> arr;
	arr.RemoveAll();

	CString resToken;
	int curPos = 0;
	resToken = title.Tokenize(L"\r\n", curPos);
	while (resToken != L"")
	{
		arr.Add(resToken);
		resToken = title.Tokenize(L"\r\n", curPos);
	}

	for (int i = 0; i < arr.GetCount(); i++)
	{
		CRect r;
		r.top = rect.top + (LONG)(((double)rect.Height()) / arr.GetCount() * i);
		r.bottom = rect.top + (LONG)(((double)rect.Height()) / arr.GetCount() * (i + 1));
		r.left = rect.left;
		r.right = rect.right;

		CRect rectI;
		CSize extent;
		HGDIOBJ oldFont = drawDC->SelectObject(m_Font);
		if ((m_RenderMode & OwnerDrawTransparent) && m_bDarkMode)
		{
			SetTextColor(drawDC->m_hDC, RGB(255, 255, 255));
		}
		else
		{
			SetTextColor(drawDC->m_hDC, m_TextColor);
		}
		GetTextExtentPoint32(drawDC->m_hDC, arr.GetAt(i), arr.GetAt(i).GetLength() + 1, &extent);

		if (m_TextAlign == BS_LEFT)
		{
			drawDC->DrawText(arr.GetAt(i), arr.GetAt(i).GetLength(), r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		else if (m_TextAlign == BS_RIGHT)
		{
			drawDC->DrawText(arr.GetAt(i), arr.GetAt(i).GetLength(), r, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}
		else
		{
			drawDC->DrawText(arr.GetAt(i), arr.GetAt(i).GetLength(), r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}

		drawDC->SelectObject(oldFont);
	}
}

//------------------------------------------------
// Image
//------------------------------------------------

BOOL CButtonFx::LoadBitmap(LPCTSTR fileName)
{
	if (m_bHighContrast) { return FALSE; }
	if (fileName == NULL) { return FALSE; }

	m_CtrlImage.Destroy();
	m_CtrlImage.Load(fileName);
	if (m_CtrlImage.IsNull()) { return FALSE; }

	return LoadBitmap((HBITMAP)m_CtrlImage);
}

BOOL CButtonFx::LoadBitmap(HBITMAP hBitmap)
{
	if (m_bHighContrast) { return FALSE; }

	m_CtrlBitmap.Detach();
	m_CtrlBitmap.Attach(hBitmap);

	return SetBitmap(m_CtrlBitmap);
}

void CButtonFx::SetBkReload(void)
{
	m_bBkBitmapInit = FALSE;
	m_bBkLoad = FALSE;
}

BOOL CButtonFx::SetBitmap(CBitmap& bitmap)
{
	if (m_bHighContrast) { return FALSE; }

	BITMAP	bitmapInfo;
	bitmap.GetBitmap(&bitmapInfo);

	if (m_CtrlSize.cx != bitmapInfo.bmWidth
	||  m_CtrlSize.cy != bitmapInfo.bmHeight / m_ImageCount)
	{
		ModifyStyle(BS_OWNERDRAW, 0);
		return FALSE;
	}
	else
	{
		ModifyStyle(0, BS_OWNERDRAW);
		return TRUE;
	}
}

void CButtonFx::LoadCtrlBk(CDC* drawDC)
{
	if (m_bHighContrast) { SetBkReload(); return; }

	if (m_BkBitmap.m_hObject != NULL)
	{
		BITMAP bitmapInfo;
		m_BkBitmap.GetBitmap(&bitmapInfo);
		if (bitmapInfo.bmBitsPixel != drawDC->GetDeviceCaps(BITSPIXEL))
		{
			SetBkReload();
		}
	}

	if (&m_CtrlBitmap != NULL)
	{
		if (!m_bBkBitmapInit)
		{
			m_BkBitmap.DeleteObject();
			m_BkBitmap.CreateCompatibleBitmap(drawDC, m_CtrlSize.cx, m_CtrlSize.cy);
			m_bBkBitmapInit = TRUE;
		}

		if (!m_bBkLoad)
		{
			CBitmap* pOldBitmap;
			CDC* pMemDC = new CDC;
			pMemDC->CreateCompatibleDC(drawDC);
			pOldBitmap = pMemDC->SelectObject(&m_BkBitmap);
			pMemDC->BitBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, m_BkDC, m_X, m_Y, SRCCOPY);
			pMemDC->SelectObject(pOldBitmap);
			pMemDC->DeleteDC();
			delete pMemDC;
			m_bBkLoad = TRUE;
		}
	}
}

//------------------------------------------------
// Font
//------------------------------------------------

void CButtonFx::SetFontEx(CString face, int size, int sizeToolTip, double zoomRatio, double fontRatio, COLORREF textColor, LONG fontWeight, BYTE fontRender)
{
	LOGFONT logFont = { 0 };
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfHeight = (LONG)(-1 * size * zoomRatio * fontRatio);
	logFont.lfQuality = fontRender;
	logFont.lfWeight = fontWeight;

	if (face.GetLength() < 32)
	{
		wsprintf(logFont.lfFaceName, L"%s", face.GetString());
	}
	else
	{
		wsprintf(logFont.lfFaceName, L"");
	}

	m_Font.DeleteObject();
	m_Font.CreateFontIndirect(&logFont);
	SetFont(&m_Font);

	logFont.lfHeight = (LONG)(-1 * sizeToolTip * zoomRatio);
	m_FontToolTip.DeleteObject();
	m_FontToolTip.CreateFontIndirect(&logFont);

	m_TextColor = textColor;

	if (m_ToolTip.m_hWnd != NULL)
	{
		m_ToolTip.SetFont(&m_FontToolTip);
	}
}

//------------------------------------------------
// Mouse
//------------------------------------------------

void CButtonFx::SetHandCursor(BOOL bHandCuror)
{
	m_bHandCursor = bHandCuror;
}

void CButtonFx::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bTrackingNow)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = 1;
		m_bTrackingNow = _TrackMouseEvent(&tme);
	}

	CButton::OnMouseMove(nFlags, point);
}

void CButtonFx::OnMouseHover(UINT nFlags, CPoint point)
{
	CButton::OnMouseHover(nFlags, point);

	m_bHover = TRUE;
	Invalidate();
}

void CButtonFx::OnMouseLeave()
{
	CButton::OnMouseLeave();

	m_bTrackingNow = FALSE;
	m_bHover = FALSE;
	Invalidate();
}

void CButtonFx::OnSetfocus()
{
	m_bFocas = TRUE;
	Invalidate();
}

void CButtonFx::OnKillfocus()
{
	m_bFocas = FALSE;
	Invalidate();
}

BOOL CButtonFx::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_bHandCursor)
	{
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
	}
	else
	{
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	}

	return TRUE;
}

void CButtonFx::SetSelected(BOOL bSelected)
{
	m_bSelected = bSelected;
	Invalidate();
}

//------------------------------------------------
// ToolTip
//------------------------------------------------

void CButtonFx::SetToolTipText(LPCTSTR text)
{
	if (text == NULL) { return; }

	InitToolTip();
	m_ToolTipText = text;
	if (m_ToolTip.GetToolCount() == 0)
	{
		CRect rect;
		GetClientRect(rect);
		m_ToolTip.AddTool(this, m_ToolTipText, rect, 1);
	}
	else
	{
		m_ToolTip.UpdateTipText(m_ToolTipText, this, 1);
	}

	SetToolTipActivate(TRUE);
}

void CButtonFx::SetToolTipActivate(BOOL bActivate)
{
	if (m_ToolTip.GetToolCount() == 0) { return; }
	m_ToolTip.Activate(bActivate);
}

void CButtonFx::SetToolTipWindowText(LPCTSTR text)
{
	SetToolTipText(text);
	SetWindowText(text);
}

CString CButtonFx::GetToolTipText()
{
	return m_ToolTipText;
}

void CButtonFx::InitToolTip()
{
	if (m_ToolTip.m_hWnd == NULL)
	{
		m_ToolTip.Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOANIMATE | TTS_NOFADE);
		m_ToolTip.Activate(FALSE);
		m_ToolTip.SetFont(&m_FontToolTip);
		m_ToolTip.SendMessageW(TTM_SETMAXTIPWIDTH, 0, 1024);
		m_ToolTip.SetDelayTime(TTDT_AUTOPOP, 8000);
		m_ToolTip.SetDelayTime(TTDT_INITIAL, 500);
		m_ToolTip.SetDelayTime(TTDT_RESHOW, 100);
	}
}

BOOL CButtonFx::PreTranslateMessage(MSG* pMsg)
{
	InitToolTip();
	m_ToolTip.RelayEvent(pMsg);

	return CButton::PreTranslateMessage(pMsg);
}

BOOL CButtonFx::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}