/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 ///////////////////////////////////////////////////////////////////////////// 
// WheelSplitterWnd.h


#if !defined(WheelSplitterWnd_H_INCLUDED)
#define WheelSplitterWnd_H_INCLUDED

#define WHEEL_MODE_VSCROLL_MSG		(0x01 << 0)		// WM_MOUSEWHEELをWM_VSCROLLに変換してVIEWに転送(デフォルト)
#define WHEEL_MODE_WHEEL_MSG		(0x01 << 1)		// WM_MOUSEWHEELをWSW_WM_WHEEL_TRANSFER_MSGにしてVIEWに転送

#define WHEEL_MODE_ACTIVE_VIEW		(0x01 << 2)		// Active Viewにメッセージを転送(デフォルト)
#define WHEEL_MODE_ON_CURSOR_VIEW	(0x01 << 3)		// マウスカーソルのあるViewにメッセージを転送

// CSplitterWndの拡張
//
// ホイール操作でASSERTエラーにならないクラス
//
class CWheelSplitterWnd : public CSplitterWnd
{
protected:
	DECLARE_DYNCREATE(CWheelSplitterWnd)

public:
	CWheelSplitterWnd();
	virtual ~CWheelSplitterWnd();

	void SetMaxCols(int col);
	int GetMaxCols() { return m_nMaxCols; }

	void SetDeletePaneMessage(int msg) { m_delete_pane_msg = msg; }

	virtual BOOL SplitRow( int cyBefore );
	virtual BOOL SplitColumn( int cxBefore );
	virtual void DeleteRow( int rowDelete );
	virtual void DeleteColumn( int colDelete );

	void SetWheelMode(int mode) { m_wheel_mode = mode; }

protected:
	//{{AFX_MSG(CWheelSplitterWnd)
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int		m_delete_pane_msg;

private:
	int		m_wheel_mode;
};

#endif // !defined(WheelSplitterWnd_H_INCLUDED)
