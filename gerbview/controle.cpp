/****************/
/* controle.cpp */
/****************/

#include "fctsys.h"
#include "common.h"
#include "class_drawpanel.h"
#include "gerbview.h"


void WinEDA_GerberFrame::GeneralControle( wxDC* aDC, const wxPoint& aPosition )
{
    wxRealPoint gridSize;
    wxPoint     oldpos;
    int         hotkey = 0;
    wxPoint     pos = aPosition;

    pos = GetScreen()->GetNearestGridPosition( pos );

    if( GetScreen()->IsRefreshReq() )
    {
        DrawPanel->Refresh( );
        wxSafeYield();

        // We must return here, instead of proceeding.
        // If we let the cursor move during a refresh request,
        // the cursor be displayed in the wrong place
        // during delayed repaint events that occur when
        // you move the mouse when a message dialog is on
        // the screen, and then you dismiss the dialog by
        // typing the Enter key.
        return;
    }

    oldpos = GetScreen()->GetCrossHairPosition();
    gridSize = GetScreen()->GetGridSize();

    switch( g_KeyPressed )
    {
    case WXK_NUMPAD8:
    case WXK_UP:
        pos.y -= wxRound( gridSize.y );
        DrawPanel->MoveCursor( pos );
        break;

    case WXK_NUMPAD2:
    case WXK_DOWN:
        pos.y += wxRound( gridSize.y );
        DrawPanel->MoveCursor( pos );
        break;

    case WXK_NUMPAD4:
    case WXK_LEFT:
        pos.x -= wxRound( gridSize.x );
        DrawPanel->MoveCursor( pos );
        break;

    case WXK_NUMPAD6:
    case WXK_RIGHT:
        pos.x += wxRound( gridSize.x );
        DrawPanel->MoveCursor( pos );
        break;

    default:
        hotkey = g_KeyPressed;
        break;
    }

    GetScreen()->SetCrossHairPosition( pos );

    if( oldpos != GetScreen()->GetCrossHairPosition() )
    {
        pos = GetScreen()->GetCrossHairPosition();
        GetScreen()->SetCrossHairPosition( oldpos );
        DrawPanel->CrossHairOff( aDC );
        GetScreen()->SetCrossHairPosition( pos );
        DrawPanel->CrossHairOn( aDC );

        if( DrawPanel->IsMouseCaptured() )
        {
            DrawPanel->m_mouseCaptureCallback( DrawPanel, aDC, aPosition, true );
        }
    }

    if( hotkey )
    {
        OnHotKey( aDC, hotkey, NULL );
    }

    if( GetScreen()->IsRefreshReq() )
    {
        DrawPanel->Refresh( );
        wxSafeYield();
    }

    UpdateStatusBar();
}
