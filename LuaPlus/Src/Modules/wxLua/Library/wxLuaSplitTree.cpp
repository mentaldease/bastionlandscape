/////////////////////////////////////////////////////////////////////////////
// Name:        splittree.cpp
// Purpose:     Classes to achieve a remotely-scrolled tree in a splitter
//              window that can be scrolled by a scrolled window higher in the
//              hierarchy
// Author:      Julian Smart
// Created:     8/7/2000
// Modified by: J. Winwood, to fix a few bugs and also add a second
//              splitter and scrolled window (three columns rather than two).
// Copyright:   (c) Julian Smart
//              (c) 2002 Lomtick Software. All rights reserved.
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#ifdef __GNUG__
    #pragma implementation "wxLuaSplitTree.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <windows.h>
#include "wx/msw/winundef.h"
#endif

#include "wxLuaSplitTree.h"
#include <math.h>

static wxTreeItemId defaultTreeItemId;

/*
 * wxRemotelyScrolledTreeCtrl
 */

#if USE_GENERIC_TREECTRL
IMPLEMENT_CLASS(wxRemotelyScrolledTreeCtrl, wxGenericTreeCtrl)
#else
IMPLEMENT_CLASS(wxRemotelyScrolledTreeCtrl, wxTreeCtrl)
#endif

#if USE_GENERIC_TREECTRL
BEGIN_EVENT_TABLE(wxRemotelyScrolledTreeCtrl, wxGenericTreeCtrl)
#else
BEGIN_EVENT_TABLE(wxRemotelyScrolledTreeCtrl, wxTreeCtrl)
#endif
    EVT_SIZE(wxRemotelyScrolledTreeCtrl::OnSize)
    EVT_PAINT(wxRemotelyScrolledTreeCtrl::OnPaint)
    EVT_TREE_ITEM_EXPANDED(-1, wxRemotelyScrolledTreeCtrl::OnExpand)
    EVT_TREE_ITEM_COLLAPSED(-1, wxRemotelyScrolledTreeCtrl::OnExpand)
    EVT_SCROLLWIN(wxRemotelyScrolledTreeCtrl::OnScroll)
    EVT_TREE_SEL_CHANGED(-1, wxRemotelyScrolledTreeCtrl::OnSelChanged)
END_EVENT_TABLE()

wxRemotelyScrolledTreeCtrl::wxRemotelyScrolledTreeCtrl(wxWindow      *parent,
                                                       wxWindowID     id,
                                                       const wxPoint& pt,
                                                       const wxSize&  sz,
                                                       long           style)
    : wxTreeCtrl(parent, id, pt, sz, style & ~wxTR_ROW_LINES),
    m_lastTreeItemId(defaultTreeItemId)
{
    m_firstCompanionWindow = NULL;
    m_secondCompanionWindow = NULL;

    // We draw the row lines ourself so they match what's done
    // by the companion window.  That is why the flag is turned
    // off above, so wxGenericTreeCtrl doesn't draw them in a
    // different colour.
    m_drawRowLines = (style & wxTR_ROW_LINES) != 0;
}

wxRemotelyScrolledTreeCtrl::~wxRemotelyScrolledTreeCtrl()
{
}

void wxRemotelyScrolledTreeCtrl::HideVScrollbar()
{
#if defined(__WXMSW__)
#if USE_GENERIC_TREECTRL
    if (!IsKindOf(CLASSINFO(wxGenericTreeCtrl)))
#endif
    {
        ::ShowScrollBar((HWND) GetHWND(), SB_VERT, FALSE);
    }
#if USE_GENERIC_TREECTRL
    else
    {
        // Implicit in overriding SetScrollbars
    }
#endif
#endif
}

// Number of pixels per user unit (0 or -1 for no scrollbar)
// Length of virtual canvas in user units
// Length of page in user units
void wxRemotelyScrolledTreeCtrl::SetScrollbars(int pixelsPerUnitX,
                                               int pixelsPerUnitY,
                                               int noUnitsX,
                                               int noUnitsY,
                                               int xPos,
                                               int yPos,
                                               bool noRefresh)
{
#if USE_GENERIC_TREECTRL || !defined(__WXMSW__)
    if (IsKindOf(CLASSINFO(wxGenericTreeCtrl)))
    {
        wxGenericTreeCtrl* win = (wxGenericTreeCtrl*) this;
        win->wxGenericTreeCtrl::SetScrollbars(pixelsPerUnitX, pixelsPerUnitY, noUnitsX, 0, xPos, 0, /* noRefresh */ TRUE);

        wxScrolledWindow* scrolledWindow = GetScrolledWindow();
        if (scrolledWindow)
        {
            scrolledWindow->SetScrollbars(0, pixelsPerUnitY, 0, noUnitsY, 0, yPos, noRefresh);
            scrolledWindow->AdjustScrollbars();   // FIXME - this is for the broken wxWindow's wxScrolledWindow
        }
    }
#else
# ifdef UNREFERENCED_PARAMETER
        UNREFERENCED_PARAMETER(pixelsPerUnitX);
        UNREFERENCED_PARAMETER(pixelsPerUnitY);
        UNREFERENCED_PARAMETER(noUnitsX);
        UNREFERENCED_PARAMETER(noUnitsY);
        UNREFERENCED_PARAMETER(xPos);
        UNREFERENCED_PARAMETER(yPos);
        UNREFERENCED_PARAMETER(noRefresh);
# endif    
#endif
}

// In case we're using the generic tree control.
int wxRemotelyScrolledTreeCtrl::GetScrollPos(int orient) const
{
#if USE_GENERIC_TREECTRL || !defined(__WXMSW__)
    wxScrolledWindow* scrolledWindow = GetScrolledWindow();

    if (IsKindOf(CLASSINFO(wxGenericTreeCtrl)))
    {
        wxGenericTreeCtrl* win = (wxGenericTreeCtrl*) this;

        if (orient == wxHORIZONTAL)
            return win->wxGenericTreeCtrl::GetScrollPos(orient);
        else
            return scrolledWindow->GetScrollPos(orient);
    }
#else
# ifdef UNREFERENCED_PARAMETER
    UNREFERENCED_PARAMETER(orient);
# endif    
#endif
    return 0;
}


// In case we're using the generic tree control.
// Get the view start
void wxRemotelyScrolledTreeCtrl::GetViewStart(int *x, int *y) const
{
    wxScrolledWindow* scrolledWindow = GetScrolledWindow();

#if USE_GENERIC_TREECTRL || !defined(__WXMSW__)
    if (IsKindOf(CLASSINFO(wxGenericTreeCtrl)))
    {

        wxGenericTreeCtrl* win = (wxGenericTreeCtrl*) this;
        int x1, y1, x2, y2;
        win->wxGenericTreeCtrl::GetViewStart(& x1, & y1);
        * x = x1; * y = y1;
        if (!scrolledWindow)
            return;

        scrolledWindow->GetViewStart(& x2, & y2);
        * y = y2;
    }
    else            
#endif
    {
        // x is wrong since the horizontal scrollbar is controlled by the
        // tree control, but we probably don't need it.
        scrolledWindow->GetViewStart(x, y);
    }
}

// In case we're using the generic tree control.
void wxRemotelyScrolledTreeCtrl::PrepareDC(wxDC& dc)
{
#if USE_GENERIC_TREECTRL || !defined(__WXMSW__)
    if (IsKindOf(CLASSINFO(wxGenericTreeCtrl)))
    {
        wxScrolledWindow* scrolledWindow = GetScrolledWindow();

        wxGenericTreeCtrl* win = (wxGenericTreeCtrl*) this;

        int startX, startY;
        GetViewStart(& startX, & startY);

        int xppu1, yppu1, xppu2, yppu2;
        win->wxGenericTreeCtrl::GetScrollPixelsPerUnit(& xppu1, & yppu1);
        scrolledWindow->GetScrollPixelsPerUnit(& xppu2, & yppu2);

        dc.SetDeviceOrigin( -startX * xppu1, -startY * yppu2 );
        // dc.SetUserScale( win->GetScaleX(), win->GetScaleY() );
    }
#else
# ifdef UNREFERENCED_PARAMETER
    UNREFERENCED_PARAMETER(dc);
# endif    
#endif
}

// Scroll to the given line (in scroll units where each unit is
// the height of an item)
void wxRemotelyScrolledTreeCtrl::ScrollToLine(int WXUNUSED(posHoriz), int posVert)
{
#ifdef __WXMSW__
#if USE_GENERIC_TREECTRL
    if (!IsKindOf(CLASSINFO(wxGenericTreeCtrl)))
#endif
    {
        UINT sbCode = SB_THUMBPOSITION;
        HWND vertScrollBar = 0;
#if wxMINOR_VERSION < 5
        MSWDefWindowProc((WXUINT) WM_VSCROLL, MAKELONG(sbCode, posVert), (WXHWND) vertScrollBar);
#else
        MSWDefWindowProc((WXUINT) WM_VSCROLL, MAKELONG(sbCode, posVert), (WXLPARAM) vertScrollBar);
#endif
    }
#if USE_GENERIC_TREECTRL
    else
#endif
#endif
#if USE_GENERIC_TREECTRL || !defined(__WXMSW__)
    {
        wxGenericTreeCtrl* win = (wxGenericTreeCtrl*) this;
        win->Refresh();
    }
#endif
}

void wxRemotelyScrolledTreeCtrl::OnSize(wxSizeEvent& event)
{
    HideVScrollbar();
    AdjustRemoteScrollbars();
    if (m_firstCompanionWindow)
        m_firstCompanionWindow->Refresh(TRUE);
    if (m_secondCompanionWindow)
        m_secondCompanionWindow->Refresh(TRUE);
    event.Skip();
}

void wxRemotelyScrolledTreeCtrl::OnExpand(wxTreeEvent& event)
{
    AdjustRemoteScrollbars();
    event.Skip();

    // If we don't have this, we get some bits of lines still remaining
    if (event.GetEventType() == wxEVT_COMMAND_TREE_ITEM_COLLAPSED)
        Refresh();

    // Pass on the event
    if (m_secondCompanionWindow)
        m_secondCompanionWindow->GetEventHandler()->ProcessEvent(event);
    if (m_firstCompanionWindow)
        m_firstCompanionWindow->GetEventHandler()->ProcessEvent(event);
}

void wxRemotelyScrolledTreeCtrl::OnSelChanged(wxTreeEvent& event)
{
    const wxTreeItemId &itemId = GetFirstVisibleItem();
    if (m_lastTreeItemId != itemId)
    {
        AdjustRemoteScrollbars();
        event.Skip();

        Refresh();

        // Pass on the event
        if (m_secondCompanionWindow)
            m_secondCompanionWindow->GetEventHandler()->ProcessEvent(event);
        if (m_firstCompanionWindow)
            m_firstCompanionWindow->GetEventHandler()->ProcessEvent(event);

        m_lastTreeItemId = itemId;
    }
    else
        event.Skip();
}

void wxRemotelyScrolledTreeCtrl::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    wxTreeCtrl::OnPaint(event);

    if (! m_drawRowLines)
        return;

    // Reset the device origin since it may have been set
    dc.SetDeviceOrigin(0, 0);

    wxPen pen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT), 1, wxSOLID);
    dc.SetPen(pen);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);

    wxSize clientSize = GetClientSize();
    wxRect itemRect;
    int cy = 0;
    wxTreeItemId h, lastH;
    for(h = GetFirstVisibleItem(); h && IsVisible(h); h = GetNextVisible(h))
    {
        if (GetBoundingRect(h, itemRect))
        {
            cy = itemRect.GetTop();
            dc.DrawLine(0, cy, clientSize.x, cy);
            lastH = h;
        }
    }

    if (lastH.IsOk() && GetBoundingRect(lastH, itemRect))
    {
        cy = itemRect.GetBottom();
        dc.DrawLine(0, cy, clientSize.x, cy);
    }
}


// Adjust the containing wxScrolledWindow's scrollbars appropriately
void wxRemotelyScrolledTreeCtrl::AdjustRemoteScrollbars()
{
#if USE_GENERIC_TREECTRL || !defined(__WXMSW__)
    if (IsKindOf(CLASSINFO(wxGenericTreeCtrl)))
    {
        // This is for the generic tree control.
        // It calls SetScrollbars which has been overridden
        // to adjust the parent scrolled window vertical
        // scrollbar.
        ((wxGenericTreeCtrl*) this)->AdjustMyScrollbars();
        return;
    }
    else
#endif
    {
        // This is for the wxMSW tree control
        wxScrolledWindow* scrolledWindow = GetScrolledWindow();
        if (scrolledWindow)
        {
            wxRect itemRect;
            if (GetBoundingRect(GetFirstVisibleItem(), itemRect)) // newer method
            //if (GetBoundingRect(GetRootItem(), itemRect))
            {
                // Actually, the real height seems to be 1 less than reported
                // (e.g. 16 instead of 16)
                int itemHeight = itemRect.GetHeight() - 1;

                int w, h;
                GetClientSize(&w, &h);

                wxRect rect(0, 0, 0, 0);
                CalcTreeSize(rect);

                double f = ((double) (rect.GetHeight()) / (double) itemHeight)  ;
                int treeViewHeight = (int) ceil(f);

                int scrollPixelsPerLine = itemHeight;
                int scrollPos = - (itemRect.y / itemHeight);

                scrolledWindow->SetScrollbars(0, scrollPixelsPerLine, 0, treeViewHeight - 1, 0, scrollPos);

                // Ensure that when a scrollbar becomes hidden or visible,
                // the contained window sizes are right.
                // Problem: this is called too early (?)
                wxSizeEvent event(scrolledWindow->GetSize(), scrolledWindow->GetId());
                scrolledWindow->GetEventHandler()->ProcessEvent(event);
            }
        }
    }
}

void wxRemotelyScrolledTreeCtrl::DeleteAllItems()
{
    wxTreeCtrl::DeleteAllItems();
    if (m_firstCompanionWindow)
        m_firstCompanionWindow->Refresh(TRUE);
    if (m_secondCompanionWindow)
        m_secondCompanionWindow->Refresh(TRUE);
}

// Calculate the area that contains both rectangles
static wxRect CombineRectangles(const wxRect& rect1, const wxRect& rect2)
{
    wxRect rect;

    int right1 = rect1.GetRight();
    int bottom1 = rect1.GetBottom();
    int right2 = rect2.GetRight();
    int bottom2 = rect2.GetBottom();

    wxPoint topLeft = wxPoint(wxMin(rect1.x, rect2.x), wxMin(rect1.y, rect2.y));
    wxPoint bottomRight = wxPoint(wxMax(right1, right2), wxMax(bottom1, bottom2));

    rect.x = topLeft.x; rect.y = topLeft.y;
    rect.SetRight(bottomRight.x);
    rect.SetBottom(bottomRight.y);

    return rect;
}

// Calculate the tree overall size so we can set the scrollbar
// correctly
void wxRemotelyScrolledTreeCtrl::CalcTreeSize(wxRect& rect)
{
    CalcTreeSize(GetRootItem(), rect);
}

void wxRemotelyScrolledTreeCtrl::CalcTreeSize(const wxTreeItemId& id, wxRect& rect)
{
    // More efficient implementation would be to find the last item (but how?)
    // Q: is the bounding rect relative to the top of the virtual tree workspace
    // or the top of the window? How would we convert?
    wxRect itemSize;
    if (GetBoundingRect(id, itemSize))
    {
        rect = CombineRectangles(rect, itemSize);
    }

    wxTreeItemIdValue cookie;
    wxTreeItemId childId = GetFirstChild(id, cookie);
    while (childId != wxTreeItemId((long)0))
    {
        CalcTreeSize(childId, rect);
        childId = GetNextChild(childId, cookie);
    }
}

// Find the scrolled window that contains this control
wxScrolledWindow* wxRemotelyScrolledTreeCtrl::GetScrolledWindow() const
{
    wxWindow* parent = wxWindow::GetParent();
    while (parent)
    {
        if (parent->IsKindOf(CLASSINFO(wxScrolledWindow)))
            return (wxScrolledWindow*) parent;
        parent = parent->GetParent();
    }
    return NULL;
}

void wxRemotelyScrolledTreeCtrl::OnScroll(wxScrollWinEvent& event)
{
    int orient = event.GetOrientation();
    if (orient == wxHORIZONTAL)
    {
        event.Skip();
        return;
    }

    wxScrolledWindow* scrollWin = GetScrolledWindow();
    if (!scrollWin)
        return;

    int x, y;
    scrollWin->GetViewStart(& x, & y);

    ScrollToLine(-1, y);

    // Pass on the event
    if (m_secondCompanionWindow)
        m_secondCompanionWindow->GetEventHandler()->ProcessEvent(event);
    if (m_firstCompanionWindow)
        m_firstCompanionWindow->GetEventHandler()->ProcessEvent(event);
}

/*
 * wxTreeCompanionWindow
 *
 * A window displaying values associated with tree control items.
 */

IMPLEMENT_CLASS(wxTreeCompanionWindow, wxWindow)

BEGIN_EVENT_TABLE(wxTreeCompanionWindow, wxWindow)
    EVT_PAINT(wxTreeCompanionWindow::OnPaint)
    EVT_SCROLLWIN(wxTreeCompanionWindow::OnScroll)
    EVT_TREE_ITEM_EXPANDED(-1, wxTreeCompanionWindow::OnExpand)
    EVT_TREE_ITEM_COLLAPSED(-1, wxTreeCompanionWindow::OnExpand)
    EVT_TREE_SEL_CHANGED(-1, wxTreeCompanionWindow::OnSelChanged)
END_EVENT_TABLE()

wxTreeCompanionWindow::wxTreeCompanionWindow(wxWindow* parent, wxWindowID id,
      const wxPoint& pos,
      const wxSize& sz,
      long  style):
    wxWindow(parent, id, pos, sz, style)
{
    m_treeControl = NULL;
}

wxTreeCompanionWindow::~wxTreeCompanionWindow()
{
}

void wxTreeCompanionWindow::DrawItem(wxDC& dc, wxTreeItemId id, const wxRect& rect)
{
    if (m_treeControl)
    {
        wxString text = m_treeControl->GetItemText(id);

        dc.SetTextForeground(*wxBLACK);
        dc.SetBackgroundMode(wxTRANSPARENT);

        int textW, textH;
        dc.GetTextExtent(text, &textW, &textH);

        int x = 5;
        int y = rect.GetY() + wxMax(0, (rect.GetHeight() - textH) / 2);

        dc.DrawText(text, x, y);
    }
}

void wxTreeCompanionWindow::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    if (m_treeControl == NULL)
        return;

    wxMemoryDC tempDC;
    wxSize clientSize = GetClientSize();

    wxBitmap tempBitmap(clientSize.x, clientSize.y);

    tempDC.SelectObject(tempBitmap);

    tempDC.Clear();

    wxPen pen(wxSystemSettings::GetSystemColour(wxSYS_COLOUR_3DLIGHT), 1, wxSOLID);
    tempDC.SetPen(pen);
    tempDC.SetBrush(*wxTRANSPARENT_BRUSH);
    wxFont font(wxSystemSettings::GetSystemFont(wxSYS_DEFAULT_GUI_FONT));
    tempDC.SetFont(font);

    wxRect itemRect;
    int cy = 0;
    wxTreeItemId h, lastH;
    for(h = m_treeControl->GetFirstVisibleItem();
        h && m_treeControl->IsVisible(h);
        h = m_treeControl->GetNextVisible(h))
    {
        if (m_treeControl->GetBoundingRect(h, itemRect))
        {
            cy = itemRect.GetTop();
            wxRect drawItemRect(0, cy, clientSize.x, itemRect.GetHeight());

            lastH = h;

            // Draw the actual item
            DrawItem(tempDC, h, drawItemRect);
            tempDC.DrawLine(0, cy, clientSize.x, cy);
        }
    }

    if (lastH.IsOk() && m_treeControl->GetBoundingRect(lastH, itemRect))
    {
        cy = itemRect.GetBottom();
        tempDC.DrawLine(0, cy, clientSize.x, cy);
    }

    wxPaintDC dc(this);
    dc.Blit(0, 0, clientSize.x, clientSize.y, &tempDC, 0, 0);
    tempDC.SelectObject(wxNullBitmap);

}

void wxTreeCompanionWindow::OnScroll(wxScrollWinEvent& event)
{
    int orient = event.GetOrientation();
    if (orient == wxHORIZONTAL)
    {
        event.Skip();
        return;
    }

    if (!m_treeControl)
        return;

    // TODO: scroll the window physically instead of just refreshing.
    Refresh(TRUE);
}

void wxTreeCompanionWindow::OnExpand(wxTreeEvent& WXUNUSED(event))
{
    // TODO: something more optimized than simply refresh the whole
    // window when the tree is expanded/collapsed. Tricky.
    Refresh();
}

void wxTreeCompanionWindow::OnSelChanged(wxTreeEvent& WXUNUSED(event))
{
    Refresh();
}

/*
 * wxThinSplitterWindow
 */

IMPLEMENT_CLASS(wxThinSplitterWindow, wxSplitterWindow)

BEGIN_EVENT_TABLE(wxThinSplitterWindow, wxSplitterWindow)
    EVT_SIZE(wxThinSplitterWindow::OnSize)
END_EVENT_TABLE()

wxThinSplitterWindow::wxThinSplitterWindow(wxWindow* parent, wxWindowID id,
      const wxPoint& pos,
      const wxSize& sz,
      long style):
      wxSplitterWindow(parent, id, pos, sz, style)
{
}

void wxThinSplitterWindow::SizeWindows()
{
    // The client size may have changed inbetween
    // the sizing of the first window and the sizing of
    // the second. So repeat SizeWindows.
    wxSplitterWindow::SizeWindows();
    wxSplitterWindow::SizeWindows();
}

// Tests for x, y over sash
bool wxThinSplitterWindow::SashHitTest(int x, int y, int WXUNUSED(tolerance))
{
    return wxSplitterWindow::SashHitTest(x, y, 4);
}

void wxThinSplitterWindow::DrawSash(wxDC& dc)
{
    if ( m_sashPosition == 0 || !m_windowTwo)
        return;
    if (GetWindowStyle() & wxSP_NOSASH)
        return;

    int w, h;
    GetClientSize(&w, &h);

    wxColour faceColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    wxPen facePen(faceColour, 1, wxSOLID);
    wxBrush faceBrush(faceColour, wxSOLID);

    if ( m_splitMode == wxSPLIT_VERTICAL )
    {
        dc.SetPen(facePen);
        dc.SetBrush(faceBrush);
        int h1 = h-1;
        int y1 = 0;
        if ( (GetWindowStyleFlag() & wxSP_BORDER) != wxSP_BORDER && (GetWindowStyleFlag() & wxSP_3DBORDER) != wxSP_3DBORDER )
            h1 += 1; // Not sure why this is necessary...
        if ( (GetWindowStyleFlag() & wxSP_3DBORDER) == wxSP_3DBORDER)
        {
            y1 = 2; h1 -= 3;
        }
        dc.DrawRectangle(m_sashPosition, y1, 3, h1); // 3 == m_sashSize now gone in wxWidgets >= 2.5
    }
    else
    {
        dc.SetPen(facePen);
        dc.SetBrush(faceBrush);
        int w1 = w-1;
        int x1 = 0;
        if ( (GetWindowStyleFlag() & wxSP_BORDER) != wxSP_BORDER && (GetWindowStyleFlag() & wxSP_3DBORDER) != wxSP_3DBORDER )
            w1 ++;
        if ( (GetWindowStyleFlag() & wxSP_3DBORDER) == wxSP_3DBORDER)
        {
            x1 = 2; w1 -= 3;
        }
        dc.DrawRectangle(x1, m_sashPosition, w1, 3);
    }

    dc.SetPen(wxNullPen);
    dc.SetBrush(wxNullBrush);
}

void wxThinSplitterWindow::OnSize(wxSizeEvent& event)
{
    wxSplitterWindow::OnSize(event);
}

/*
 * wxSplitterScrolledWindow
 */

IMPLEMENT_CLASS(wxSplitterScrolledWindow, wxScrolledWindow)

BEGIN_EVENT_TABLE(wxSplitterScrolledWindow, wxScrolledWindow)
    EVT_SCROLLWIN(wxSplitterScrolledWindow::OnScroll)
    EVT_SIZE(wxSplitterScrolledWindow::OnSize)
END_EVENT_TABLE()

wxSplitterScrolledWindow::wxSplitterScrolledWindow(wxWindow* parent, wxWindowID id,
      const wxPoint& pos,
      const wxSize& sz,
      long style):
      wxScrolledWindow(parent, id, pos, sz, style)
{
}

void wxSplitterScrolledWindow::OnSize(wxSizeEvent& WXUNUSED(event))
{
    wxSize sz = GetClientSize();
    if (GetChildren().GetFirst())
    {
        ((wxWindow*) GetChildren().GetFirst()->GetData())->SetSize(0, 0, sz.x, sz.y);
    }
}

void wxSplitterScrolledWindow::OnScroll(wxScrollWinEvent& event)
{
    // Ensure that events being propagated back up the window hierarchy
    // don't cause an infinite loop
    static bool inOnScroll = FALSE;
    if (inOnScroll)
    {
        event.Skip();
        return;
    }
    inOnScroll = TRUE;

    int orient = event.GetOrientation();

    int nScrollInc = CalcScrollInc(event);
    if (nScrollInc == 0)
    {
        inOnScroll = FALSE;
        return;
    }

    if (orient == wxHORIZONTAL)
    {
        inOnScroll = FALSE;
        event.Skip();
        return;
    }
    else
    {
        int newPos = m_yScrollPosition + nScrollInc;
        SetScrollPos(wxVERTICAL, newPos, TRUE );
    }

    if (orient == wxHORIZONTAL)
    {
        m_xScrollPosition += nScrollInc;
    }
    else
    {
        m_yScrollPosition += nScrollInc;
    }

    // Find targets in splitter window and send the event to them
    wxWindowListNode* node = GetChildren().GetFirst();
    while (node)
    {
        wxWindow* child = (wxWindow*) node->GetData();
        if (child->IsKindOf(CLASSINFO(wxSplitterWindow)))
        {
            wxSplitterWindow* splitter = (wxSplitterWindow*) child;
            if (splitter->GetWindow1())
                splitter->GetWindow1()->ProcessEvent(event);
            if (splitter->GetWindow2())
                splitter->GetWindow2()->ProcessEvent(event);
            break;
        }
        node = node->GetNext();
    }

#ifdef __WXMAC__
    m_targetWindow->MacUpdateImmediately() ;
#endif

    inOnScroll = FALSE;
}
