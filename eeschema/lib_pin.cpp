/***************/
/* lib_pin.cpp */
/***************/

#include "fctsys.h"
#include "appl_wxstruct.h"
#include "gr_basic.h"
#include "macros.h"
#include "trigo.h"
#include "class_drawpanel.h"
#include "drawtxt.h"
#include "plot_common.h"
#include "wxEeschemaStruct.h"

#include "general.h"
#include "protos.h"
#include "libeditframe.h"
#include "class_libentry.h"
#include "lib_pin.h"
#include "transform.h"

#include "bitmaps.h"

/**
 * Note: The following name lists are sentence capitalized per the GNOME UI
 *       standards for list controls.  Please do not change the capitalization
 *       of these strings unless the GNOME UI standards are changed.
 */
static const wxString pin_orientation_names[] =
{
    _( "Right" ),
    _( "Left" ),
    _( "Up" ),
    _( "Down" )
};
// bitmaps to show pins orientations in dialog editor
// must have same order than pin_orientation_names
static const char ** s_icons_Pins_Orientations[] =
{
    pinorient_right_xpm,
    pinorient_left_xpm,
    pinorient_up_xpm,
    pinorient_down_xpm,
};

static const int pin_orientation_codes[] =
{
    PIN_RIGHT,
    PIN_LEFT,
    PIN_UP,
    PIN_DOWN
};


#define PIN_ORIENTATION_CNT  ( sizeof( pin_orientation_names ) / \
                               sizeof( wxString ) )


static const wxString pin_style_names[] =
{
    _( "Line" ),
    _( "Inverted" ),
    _( "Clock" ),
    _( "Inverted clock" ),
    _( "Input low" ),
    _( "Clock low" ),
    _( "Output low" ),
    _( "Falling edge clock" ),
    _( "NonLogic" )
};

// bitmaps to show pins shapes in dialog editor
// must have same order than pin_style_names
static const char ** s_icons_Pins_Shapes[] =
{
    pinshape_normal_xpm,
    pinshape_invert_xpm,
    pinshape_clock_normal_xpm,
    pinshape_clock_invert_xpm,
    pinshape_active_low_input_xpm,
    pinshape_clock_active_low_xpm,
    pinshape_active_low_output_xpm,
    pinshape_clock_fall_xpm,
    pinshape_nonlogic_xpm
};


#define PIN_STYLE_CNT  ( sizeof( pin_style_names ) / sizeof( wxString ) )


static const int pin_style_codes[] =
{
    NONE,
    INVERT,
    CLOCK,
    CLOCK | INVERT,
    LOWLEVEL_IN,
    LOWLEVEL_IN | CLOCK,
    LOWLEVEL_OUT,
    CLOCK_FALL,
    NONLOGIC
};


static const wxString pin_electrical_type_names[] =
{
    _( "Input" ),
    _( "Output" ),
    _( "Bidirectional" ),
    _( "Tri-state" ),
    _( "Passive" ),
    _( "Unspecified" ),
    _( "Power input" ),
    _( "Power output" ),
    _( "Open collector" ),
    _( "Open emitter" ),
    _( "Not connected" )
};

// bitmaps to show pins electrical type in dialog editor
// must have same order than pin_electrical_type_names
static const char ** s_icons_Pins_Electrical_Type[] =
{
    pintype_input_xpm,
    pintype_output_xpm,
    pintype_bidi_xpm,
    pintype_3states_xpm,
    pintype_passive_xpm,
    pintype_notspecif_xpm,
    pintype_powerinput_xpm,
    pintype_poweroutput_xpm,
    pintype_opencoll_xpm,
    pintype_openemit_xpm,
    pintype_noconnect_xpm
};


#define PIN_ELECTRICAL_TYPE_CNT ( sizeof( pin_electrical_type_names ) / sizeof( wxString ) )


const wxChar* MsgPinElectricType[] =
{
    wxT( "input" ),
    wxT( "output" ),
    wxT( "BiDi" ),
    wxT( "3state" ),
    wxT( "passive" ),
    wxT( "unspc" ),
    wxT( "power_in" ),
    wxT( "power_out" ),
    wxT( "openCol" ),
    wxT( "openEm" ),
    wxT( "NotConnected" ),
    wxT( "?????" )
};


extern void PlotPinSymbol( PLOTTER* plotter, const wxPoint& pos,
                           int len, int orient, int Shape );


LIB_PIN::LIB_PIN( LIB_COMPONENT * aParent ) :
    LIB_DRAW_ITEM( LIB_PIN_T, aParent )
{
    m_length             = 300;              /* default Pin len */
    m_orientation        = PIN_RIGHT;        /* Pin orient: Up, Down, Left, Right */
    m_shape              = NONE;             /* Pin shape, bitwise. */
    m_type               = PIN_UNSPECIFIED;  /* electrical type of pin */
    m_attributes         = 0;                /* bit 0 != 0: pin invisible */
    m_number             = 0;                /* pin number ( i.e. 4 codes ASCII ) */
    m_PinNumSize         = 50;
    m_PinNameSize        = 50;               /* Default size for pin name and num */
    m_width              = 0;
    m_typeName           = _( "Pin" );
    m_PinNumShapeOpt     = 0;
    m_PinNameShapeOpt    = 0;
    m_PinNumPositionOpt  = 0;
    m_PinNamePositionOpt = 0;
}


LIB_PIN::LIB_PIN( const LIB_PIN& pin ) : LIB_DRAW_ITEM( pin )
{
    m_position           = pin.m_position;
    m_length             = pin.m_length;
    m_orientation        = pin.m_orientation;
    m_shape              = pin.m_shape;
    m_type               = pin.m_type;
    m_attributes         = pin.m_attributes;
    m_number             = pin.m_number;
    m_PinNumSize         = pin.m_PinNumSize;
    m_PinNameSize        = pin.m_PinNameSize;
    m_PinNumShapeOpt     = pin.m_PinNumShapeOpt;
    m_PinNameShapeOpt    = pin.m_PinNameShapeOpt;
    m_PinNumPositionOpt  = pin.m_PinNumPositionOpt;
    m_PinNamePositionOpt = pin.m_PinNamePositionOpt;
    m_width              = pin.m_width;
    m_name               = pin.m_name;
}


void LIB_PIN::SetName( const wxString& aName )
{
    wxString tmp = ( aName.IsEmpty() ) ? wxT( "~" ) : aName;
    tmp.Replace( wxT( " " ), wxT( "_" ) );

    if( m_name != tmp )
    {
        m_name = tmp;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PIN_LIST pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->m_name == m_name )
            continue;

        pinList[i]->m_name = m_name;
        SetModified();
    }
}


void LIB_PIN::SetNameTextSize( int size )
{
    if( size != m_PinNameSize )
    {
        m_PinNameSize = size;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PIN_LIST pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->m_PinNameSize == size )
            continue;

        pinList[i]->m_PinNameSize = size;
        SetModified();
    }
}


void LIB_PIN::SetNumber( const wxString& number )
{
    wxString tmp = ( number.IsEmpty() ) ? wxT( "~" ) : number;
    tmp.Replace( wxT( " " ), wxT( "_" ) );
    long oldNumber = m_number;
    SetPinNumFromString( tmp );

    if( m_number != oldNumber )
    {
        m_Flags |= IS_CHANGED;
    }

    /* Others pin numbers marked by EnableEditMode() are not modified
     * because each pin has its own number
     */
}


void LIB_PIN::SetNumberTextSize( int size )
{
    if( size != m_PinNumSize )
    {
        m_PinNumSize = size;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PIN_LIST pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->m_PinNumSize == size )
            continue;

        pinList[i]->m_PinNumSize = size;
        SetModified();
    }
}


void LIB_PIN::SetOrientation( int orientation )
{
    if( m_orientation != orientation )
    {
        m_orientation = orientation;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PIN_LIST pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->m_orientation == orientation )
            continue;

        pinList[i]->m_orientation = orientation;
        SetModified();
    }
}


void LIB_PIN::SetShape( int aShape )
{
    if( m_shape != aShape )
    {
        m_shape = aShape;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PIN_LIST pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0
            || pinList[i]->m_Convert != m_Convert
            || pinList[i]->m_shape == aShape )
            continue;

        pinList[i]->m_shape = aShape;
        SetModified();
    }
}


void LIB_PIN::SetType( int aType )
{
    if( m_type != aType )
    {
        m_type = aType;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PIN_LIST pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->m_type == aType )
            continue;

        pinList[i]->m_type = aType;
        SetModified();
    }
}


void LIB_PIN::SetLength( int length )
{
    if( m_length != length )
    {
        m_length = length;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PIN_LIST pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0
            || pinList[i]->m_Convert != m_Convert
            || pinList[i]->m_length == length )
            continue;

        pinList[i]->m_length = length;
        SetModified();
    }
}


void LIB_PIN::SetPartNumber( int part )
{
    if( m_Unit == part )
        return;

    m_Unit = part;
    SetModified();

    if( m_Unit == 0 )
    {
        LIB_PIN* pin;
        LIB_PIN* tmp = GetParent()->GetNextPin();

        while( tmp != NULL )
        {
            pin = tmp;
            tmp = GetParent()->GetNextPin( pin );

            if( pin->m_Flags == 0 || pin == this
                || ( m_Convert && ( m_Convert != pin->m_Convert ) )
                || ( m_position != pin->m_position )
                || ( pin->m_orientation != m_orientation ) )
                continue;

            GetParent()->RemoveDrawItem( (LIB_DRAW_ITEM*) pin );
        }
    }
}


void LIB_PIN::SetConversion( int style )
{
    if( m_Convert == style )
        return;

    m_Convert = style;
    m_Flags |= IS_CHANGED;

    if( style == 0 )
    {
        LIB_PIN* pin;
        LIB_PIN* tmp = GetParent()->GetNextPin();

        while( tmp != NULL )
        {
            pin = tmp;
            tmp = GetParent()->GetNextPin( pin );

            if( ( pin->m_Flags & IS_LINKED ) == 0
                || ( pin == this )
                || ( m_Unit && ( m_Unit != pin->m_Unit ) )
                || ( m_position != pin->m_position )
                || ( pin->m_orientation != m_orientation ) )
                continue;

            GetParent()->RemoveDrawItem( (LIB_DRAW_ITEM*) pin );
        }
    }
}


void LIB_PIN::SetVisible( bool visible )
{
    if( visible == IsVisible() )
        return;

    if( visible )
        m_attributes &= ~PINNOTDRAW;
    else
        m_attributes |= PINNOTDRAW;

    SetModified();

    if( GetParent() == NULL )
        return;

    LIB_PIN_LIST pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->IsVisible() == visible )
            continue;

        if( visible )
            pinList[i]->m_attributes &= ~PINNOTDRAW;
        else
            pinList[i]->m_attributes |= PINNOTDRAW;

        SetModified();
    }
}


void LIB_PIN::EnableEditMode( bool enable, bool editPinByPin )
{
    LIB_PIN_LIST pinList;

    if( GetParent() == NULL )
        return;

    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( pinList[i] == this )
            continue;

        if( ( pinList[i]->m_position == m_position )
            && ( pinList[i]->m_orientation == m_orientation )
            && !IsNew()
            && editPinByPin == false
            && enable )
            pinList[i]->m_Flags |= IS_LINKED | IN_EDIT;
        else
            pinList[i]->m_Flags &= ~( IS_LINKED | IN_EDIT );
    }
}


bool LIB_PIN::HitTest( const wxPoint& aPosition )
{
    int mindist = m_width ? m_width / 2 : g_DrawDefaultLineThickness / 2;

    // Have a minimal tolerance for hit test
    if( mindist < 3 )
        mindist = 3;        // = 3 mils

    return HitTest( aPosition, mindist, DefaultTransform );
}


bool LIB_PIN::HitTest( wxPoint aPosition, int aThreshold, const TRANSFORM& aTransform )
{
    wxPoint pinPos = aTransform.TransformCoordinate( m_position );
    wxPoint pinEnd = aTransform.TransformCoordinate( ReturnPinEndPoint() );

    return TestSegmentHit( aPosition, pinPos, pinEnd, aThreshold );
}


bool LIB_PIN::Save( FILE* ExportFile )
{
    wxString StringPinNum;
    int      Etype;

    switch( m_type )
    {
    default:
    case PIN_INPUT:
        Etype = 'I';
        break;

    case PIN_OUTPUT:
        Etype = 'O';
        break;

    case PIN_BIDI:
        Etype = 'B';
        break;

    case PIN_TRISTATE:
        Etype = 'T';
        break;

    case PIN_PASSIVE:
        Etype = 'P';
        break;

    case PIN_UNSPECIFIED:
        Etype = 'U';
        break;

    case PIN_POWER_IN:
        Etype = 'W';
        break;

    case PIN_POWER_OUT:
        Etype = 'w';
        break;

    case PIN_OPENCOLLECTOR:
        Etype = 'C';
        break;

    case PIN_OPENEMITTER:
        Etype = 'E';
        break;

    case PIN_NC:
        Etype = 'N';
        break;
    }

    ReturnPinStringNum( StringPinNum );

    if( StringPinNum.IsEmpty() )
        StringPinNum = wxT( "~" );

    if( !m_name.IsEmpty() )
    {
        if( fprintf( ExportFile, "X %s", CONV_TO_UTF8( m_name ) ) < 0 )
            return false;
    }
    else
    {
        if( fprintf( ExportFile, "X ~" ) < 0 )
            return false;
    }

    if( fprintf( ExportFile, " %s %d %d %d %c %d %d %d %d %c",
                 CONV_TO_UTF8( StringPinNum ), m_position.x, m_position.y,
                 (int) m_length, (int) m_orientation, m_PinNumSize, m_PinNameSize,
                 m_Unit, m_Convert, Etype ) < 0 )
        return false;

    if( ( m_shape ) || ( m_attributes & PINNOTDRAW ) )
    {
        if( fprintf( ExportFile, " " ) < 0 )
            return false;
    }
    if( m_attributes & PINNOTDRAW
        && fprintf( ExportFile, "N" ) < 0 )
        return false;
    if( m_shape & INVERT
        && fprintf( ExportFile, "I" ) < 0 )
        return false;
    if( m_shape & CLOCK
        && fprintf( ExportFile, "C" ) < 0 )
        return false;
    if( m_shape & LOWLEVEL_IN
        && fprintf( ExportFile, "L" ) < 0 )
        return false;
    if( m_shape & LOWLEVEL_OUT
        && fprintf( ExportFile, "V" ) < 0 )
        return false;
    if( m_shape & CLOCK_FALL
        && fprintf( ExportFile, "F" ) < 0 )
        return false;
    if( m_shape & NONLOGIC
        && fprintf( ExportFile, "X" ) < 0 )
        return false;

    if( fprintf( ExportFile, "\n" ) < 0 )
        return false;

    m_Flags &= ~IS_CHANGED;

    return true;
}


bool LIB_PIN::Load( char* line, wxString& errorMsg )
{
    int  i, j;
    char pinAttrs[64];
    char pinName[256];
    char pinNum[64];
    char pinOrient[64];
    char pinType[64];

    *pinAttrs = 0;

    i = sscanf( line + 2, "%s %s %d %d %d %s %d %d %d %d %s %s", pinName,
                pinNum, &m_position.x, &m_position.y, &m_length, pinOrient, &m_PinNumSize,
                &m_PinNameSize, &m_Unit, &m_Convert, pinType, pinAttrs );

    if( i < 11 )
    {
        errorMsg.Printf( wxT( "pin only had %d parameters of the required 11 or 12" ), i );
        return false;
    }

    m_orientation = pinOrient[0] & 255;
    strncpy( (char*) &m_number, pinNum, 4 );
    m_name = CONV_FROM_UTF8( pinName );

    switch( *pinType & 255 )
    {
    case 'I':
        m_type = PIN_INPUT;
        break;

    case 'O':
        m_type = PIN_OUTPUT;
        break;

    case 'B':
        m_type = PIN_BIDI;
        break;

    case 'T':
        m_type = PIN_TRISTATE;
        break;

    case 'P':
        m_type = PIN_PASSIVE;
        break;

    case 'U':
        m_type = PIN_UNSPECIFIED;
        break;

    case 'W':
        m_type = PIN_POWER_IN;
        break;

    case 'w':
        m_type = PIN_POWER_OUT;
        break;

    case 'C':
        m_type = PIN_OPENCOLLECTOR;
        break;

    case 'E':
        m_type = PIN_OPENEMITTER;
        break;

    case 'N':
        m_type = PIN_NC;
        break;

    default:
        errorMsg.Printf( wxT( "unknown pin type [%c]" ), *pinType & 255 );
        return false;
    }

    if( i == 12 )       /* Special Symbol defined */
    {
        for( j = strlen( pinAttrs ); j > 0; )
        {
            switch( pinAttrs[--j] )
            {
            case '~':
                break;

            case 'N':
                m_attributes |= PINNOTDRAW;
                break;

            case 'I':
                m_shape |= INVERT;
                break;

            case 'C':
                m_shape |= CLOCK;
                break;

            case 'L':
                m_shape |= LOWLEVEL_IN;
                break;

            case 'V':
                m_shape |= LOWLEVEL_OUT;
                break;

            case 'F':
                m_shape |= CLOCK_FALL;
                break;

            case 'X':
                m_shape |= NONLOGIC;
                break;

            default:
                errorMsg.Printf( wxT( "unknown pin attribute [%c]" ), pinAttrs[j] );
                return false;
            }
        }
    }

    return true;
}


/**
 * Function GetPenSize
 * @return the size of the "pen" that be used to draw or plot this item
 */
int LIB_PIN::GetPenSize()
{
    return ( m_width == 0 ) ? g_DrawDefaultLineThickness : m_width;
}


void LIB_PIN::drawGraphic( EDA_DRAW_PANEL*  aPanel,
                           wxDC*            aDC,
                           const wxPoint&   aOffset,
                           int              aColor,
                           int              aDrawMode,
                           void*            aData,
                           const TRANSFORM& aTransform )
{
    // Invisible pins are only drawn on request.  In libedit they are drawn
    // in g_InvisibleItemColor because we must see them.
    SCH_EDIT_FRAME* frame = (SCH_EDIT_FRAME*) wxGetApp().GetTopWindow();

    if( m_attributes & PINNOTDRAW )
    {
        if( frame->m_LibeditFrame && frame->m_LibeditFrame->IsActive() )
            aColor = g_InvisibleItemColor;
        else if( !frame->m_ShowAllPins )
            return;
    }

    LIB_COMPONENT* Entry = GetParent();
    bool DrawPinText = true;

    if( ( aData != NULL ) && ( (bool*) aData == false ) )
        DrawPinText = false;

    /* Calculate pin orient taking in account the component orientation. */
    int orient = ReturnPinDrawOrient( aTransform );

    /* Calculate the pin position */
    wxPoint pos1 = aTransform.TransformCoordinate( m_position ) + aOffset;

    /* Drawing from the pin and the special symbol combination */
    DrawPinSymbol( aPanel, aDC, pos1, orient, aDrawMode, aColor );

    if( DrawPinText )
    {
        DrawPinTexts( aPanel, aDC, pos1, orient, Entry->GetPinNameOffset(),
                      Entry->ShowPinNumbers(), Entry->ShowPinNames(),
                      aColor, aDrawMode );
    }

    /* Set to one (1) to draw bounding box around pin to validate bounding
     * box calculation. */
#if 0
    EDA_Rect bBox = GetBoundingBox();
    bBox.Inflate( 5, 5 );
    GRRect( &aPanel->m_ClipBox, aDC, bBox.GetOrigin().x, bBox.GetOrigin().y,
            bBox.GetEnd().x, bBox.GetEnd().y, 0, LIGHTMAGENTA );
#endif
}


/**
 * Function DrawPinSymbol
 * Draw the pin symbol (without texts)
 *  if Color != 0 draw with Color, else with the normal pin color
 */
void LIB_PIN::DrawPinSymbol( EDA_DRAW_PANEL* aPanel,
                             wxDC*           aDC,
                             const wxPoint&  aPinPos,
                             int             aOrient,
                             int             aDrawMode,
                             int             aColor )
{
    int          MapX1, MapY1, x1, y1;
    int          color;
    int          width  = GetPenSize( );
    int          posX   = aPinPos.x, posY = aPinPos.y, len = m_length;
    BASE_SCREEN* screen = aPanel->GetScreen();

    color = ReturnLayerColor( LAYER_PIN );

    if( aColor < 0 )       // Used normal color or selected color
    {
        if( (m_Selected & IS_SELECTED) )
            color = g_ItemSelectetColor;
    }
    else
        color = aColor;

    GRSetDrawMode( aDC, aDrawMode );

    MapX1 = MapY1 = 0;
    x1 = posX;
    y1 = posY;

    switch( aOrient )
    {
    case PIN_UP:
        y1 = posY - len;
        MapY1 = 1;
        break;

    case PIN_DOWN:
        y1 = posY + len;
        MapY1 = -1;
        break;

    case PIN_LEFT:
        x1 = posX - len;
        MapX1 = 1;
        break;

    case PIN_RIGHT:
        x1 = posX + len;
        MapX1 = -1;
        break;
    }

    if( m_shape & INVERT )
    {
        GRCircle( &aPanel->m_ClipBox, aDC, MapX1 * INVERT_PIN_RADIUS + x1,
                  MapY1 * INVERT_PIN_RADIUS + y1,
                  INVERT_PIN_RADIUS, width, color );

        GRMoveTo( MapX1 * INVERT_PIN_RADIUS * 2 + x1,
                  MapY1 * INVERT_PIN_RADIUS * 2 + y1 );
        GRLineTo( &aPanel->m_ClipBox, aDC, posX, posY, width, color );
    }
    else if( m_shape & CLOCK_FALL ) /* an alternative for Inverted Clock */
    {
        GRMoveTo( x1 + MapY1 * CLOCK_PIN_DIM,
                  y1 - MapX1 * CLOCK_PIN_DIM );
        GRLineTo( &aPanel->m_ClipBox,
                  aDC,
                  x1 + MapX1 * CLOCK_PIN_DIM,
                  y1 + MapY1 * CLOCK_PIN_DIM,
                  width,
                  color );
        GRLineTo( &aPanel->m_ClipBox,
                  aDC,
                  x1 - MapY1 * CLOCK_PIN_DIM,
                  y1 + MapX1 * CLOCK_PIN_DIM,
                  width,
                  color );
        GRMoveTo( MapX1 * CLOCK_PIN_DIM + x1,
                  MapY1 * CLOCK_PIN_DIM + y1 );
        GRLineTo( &aPanel->m_ClipBox, aDC, posX, posY, width, color );
    }
    else
    {
        GRMoveTo( x1, y1 );
        GRLineTo( &aPanel->m_ClipBox, aDC, posX, posY, width, color );
    }

    if( m_shape & CLOCK )
    {
        if( MapY1 == 0 ) /* MapX1 = +- 1 */
        {
            GRMoveTo( x1, y1 + CLOCK_PIN_DIM );
            GRLineTo( &aPanel->m_ClipBox,
                      aDC,
                      x1 - MapX1 * CLOCK_PIN_DIM,
                      y1,
                      width,
                      color );
            GRLineTo( &aPanel->m_ClipBox,
                      aDC,
                      x1,
                      y1 - CLOCK_PIN_DIM,
                      width,
                      color );
        }
        else    /* MapX1 = 0 */
        {
            GRMoveTo( x1 + CLOCK_PIN_DIM, y1 );
            GRLineTo( &aPanel->m_ClipBox,
                      aDC,
                      x1,
                      y1 - MapY1 * CLOCK_PIN_DIM,
                      width,
                      color );
            GRLineTo( &aPanel->m_ClipBox,
                      aDC,
                      x1 - CLOCK_PIN_DIM,
                      y1,
                      width,
                      color );
        }
    }

    if( m_shape & LOWLEVEL_IN )  /* IEEE symbol "Active Low Input" */
    {
        if( MapY1 == 0 )            /* MapX1 = +- 1 */
        {
            GRMoveTo( x1 + MapX1 * IEEE_SYMBOL_PIN_DIM * 2, y1 );
            GRLineTo( &aPanel->m_ClipBox,
                      aDC,
                      x1 + MapX1 * IEEE_SYMBOL_PIN_DIM * 2,
                      y1 - IEEE_SYMBOL_PIN_DIM,
                      width,
                      color );
            GRLineTo( &aPanel->m_ClipBox, aDC, x1, y1, width, color );
        }
        else    /* MapX1 = 0 */
        {
            GRMoveTo( x1, y1 + MapY1 * IEEE_SYMBOL_PIN_DIM * 2 );
            GRLineTo( &aPanel->m_ClipBox, aDC, x1 - IEEE_SYMBOL_PIN_DIM,
                      y1 + MapY1 * IEEE_SYMBOL_PIN_DIM * 2, width, color );
            GRLineTo( &aPanel->m_ClipBox, aDC, x1, y1, width, color );
        }
    }


    if( m_shape & LOWLEVEL_OUT ) /* IEEE symbol "Active Low Output" */
    {
        if( MapY1 == 0 )            /* MapX1 = +- 1 */
        {
            GRMoveTo( x1, y1 - IEEE_SYMBOL_PIN_DIM );
            GRLineTo( &aPanel->m_ClipBox,
                      aDC,
                      x1 + MapX1 * IEEE_SYMBOL_PIN_DIM * 2,
                      y1,
                      width,
                      color );
        }
        else    /* MapX1 = 0 */
        {
            GRMoveTo( x1 - IEEE_SYMBOL_PIN_DIM, y1 );
            GRLineTo( &aPanel->m_ClipBox,
                      aDC,
                      x1,
                      y1 + MapY1 * IEEE_SYMBOL_PIN_DIM * 2,
                      width,
                      color );
        }
    }
    else if( m_shape & NONLOGIC ) /* NonLogic pin symbol */
    {
        GRMoveTo( x1 - (MapX1 + MapY1) * NONLOGIC_PIN_DIM,
                  y1 - (MapY1 - MapX1) * NONLOGIC_PIN_DIM );
        GRLineTo( &aPanel->m_ClipBox,
                  aDC,
                  x1 + (MapX1 + MapY1) * NONLOGIC_PIN_DIM,
                  y1 + (MapY1 - MapX1) * NONLOGIC_PIN_DIM,
                  width,
                  color );
        GRMoveTo( x1 - (MapX1 - MapY1) * NONLOGIC_PIN_DIM,
                  y1 - (MapY1 + MapX1) * NONLOGIC_PIN_DIM );
        GRLineTo( &aPanel->m_ClipBox,
                  aDC,
                  x1 + (MapX1 - MapY1) * NONLOGIC_PIN_DIM,
                  y1 + (MapY1 + MapX1) * NONLOGIC_PIN_DIM,
                  width,
                  color );
    }

    /* Draw the pin end target (active end of the pin)
     */
    #define NCSYMB_PIN_DIM TARGET_PIN_DIAM
    if( m_type == PIN_NC )   // Draw a N.C. symbol
    {
        GRLine( &aPanel->m_ClipBox, aDC,
                posX-NCSYMB_PIN_DIM, posY-NCSYMB_PIN_DIM,
                posX+NCSYMB_PIN_DIM, posY+NCSYMB_PIN_DIM,
                width, color);
        GRLine( &aPanel->m_ClipBox, aDC,
                posX+NCSYMB_PIN_DIM, posY-NCSYMB_PIN_DIM,
                posX-NCSYMB_PIN_DIM, posY+NCSYMB_PIN_DIM,
                width, color);
    }
    /* Draw but do not print the pin end target 1 pixel width
    */
    else if( !screen->m_IsPrinting )
    {
        GRCircle( &aPanel->m_ClipBox, aDC, posX, posY, TARGET_PIN_DIAM, 0, color );
    }
}


/*****************************************************************************
*  Put out pin number and pin text info, given the pin line coordinates.
*  The line must be vertical or horizontal.
*  If PinText == NULL nothing is printed. If PinNum = 0 no number is printed.
*  Current Zoom factor is taken into account.
*  If TextInside then the text is been put inside,otherwise all is drawn outside.
*  Pin Name:    substring beteween '~' is negated
*  DrawMode = GR_OR, XOR ...
*****************************************************************************/
void LIB_PIN::DrawPinTexts( EDA_DRAW_PANEL* panel,
                            wxDC*           DC,
                            wxPoint&        pin_pos,
                            int             orient,
                            int             TextInside,
                            bool            DrawPinNum,
                            bool            DrawPinName,
                            int             Color,
                            int             DrawMode )
{
    int        x, y, x1, y1;
    wxString   StringPinNum;
    EDA_Colors NameColor, NumColor;

    wxSize     PinNameSize( m_PinNameSize, m_PinNameSize );
    wxSize     PinNumSize( m_PinNumSize, m_PinNumSize );

    int        nameLineWidth = GetPenSize( );

    nameLineWidth = Clamp_Text_PenSize( nameLineWidth, m_PinNameSize, false );
    int        numLineWidth = GetPenSize( );
    numLineWidth = Clamp_Text_PenSize( numLineWidth, m_PinNumSize, false );

    GRSetDrawMode( DC, DrawMode );

    /* Get the num and name colors */
    if( (Color < 0) && (m_Selected & IS_SELECTED) )
        Color = g_ItemSelectetColor;
    NameColor = (EDA_Colors) ( Color == -1 ? ReturnLayerColor( LAYER_PINNAM ) : Color );
    NumColor  = (EDA_Colors) ( Color == -1 ? ReturnLayerColor( LAYER_PINNUM ) : Color );

    /* Create the pin num string */
    ReturnPinStringNum( StringPinNum );

    x1 = pin_pos.x;
    y1 = pin_pos.y;

    switch( orient )
    {
    case PIN_UP:
        y1 -= m_length;
        break;

    case PIN_DOWN:
        y1 += m_length;
        break;

    case PIN_LEFT:
        x1 -= m_length;
        break;

    case PIN_RIGHT:
        x1 += m_length;
        break;
    }

    if( m_name.IsEmpty() )
        DrawPinName = FALSE;

    if( TextInside )  /* Draw the text inside, but the pin numbers outside. */
    {
        if( (orient == PIN_LEFT) || (orient == PIN_RIGHT) )
        {
            // It is an horizontal line
            if( DrawPinName )
            {
                if( orient == PIN_RIGHT )
                {
                    x = x1 + TextInside;
                    DrawGraphicText( panel, DC, wxPoint( x, y1 ), NameColor,
                                     m_name,
                                     TEXT_ORIENT_HORIZ,
                                     PinNameSize,
                                     GR_TEXT_HJUSTIFY_LEFT,
                                     GR_TEXT_VJUSTIFY_CENTER, nameLineWidth,
                                     false, false );
                }
                else    // Orient == PIN_LEFT
                {
                    x = x1 - TextInside;
                    DrawGraphicText( panel, DC, wxPoint( x, y1 ), NameColor,
                                     m_name,
                                     TEXT_ORIENT_HORIZ,
                                     PinNameSize,
                                     GR_TEXT_HJUSTIFY_RIGHT,
                                     GR_TEXT_VJUSTIFY_CENTER, nameLineWidth,
                                     false, false );
                }
            }

            if( DrawPinNum )
            {
                DrawGraphicText( panel, DC,
                                 wxPoint( (x1 + pin_pos.x) / 2,
                                          y1 - TXTMARGE ), NumColor,
                                 StringPinNum,
                                 TEXT_ORIENT_HORIZ, PinNumSize,
                                 GR_TEXT_HJUSTIFY_CENTER,
                                 GR_TEXT_VJUSTIFY_BOTTOM, numLineWidth,
                                 false, false );
            }
        }
        else            /* Its a vertical line. */
        {
            // Text is drawn from bottom to top (i.e. to negative value for Y axis)
            if( orient == PIN_DOWN )
            {
                y = y1 + TextInside;

                if( DrawPinName )
                    DrawGraphicText( panel, DC, wxPoint( x1, y ), NameColor,
                                     m_name,
                                     TEXT_ORIENT_VERT, PinNameSize,
                                     GR_TEXT_HJUSTIFY_RIGHT,
                                     GR_TEXT_VJUSTIFY_CENTER, nameLineWidth,
                                     false, false );
                if( DrawPinNum )
                    DrawGraphicText( panel, DC,
                                     wxPoint( x1 - TXTMARGE,
                                              (y1 + pin_pos.y) / 2 ), NumColor,
                                     StringPinNum,
                                     TEXT_ORIENT_VERT, PinNumSize,
                                     GR_TEXT_HJUSTIFY_CENTER,
                                     GR_TEXT_VJUSTIFY_BOTTOM, numLineWidth,
                                     false, false );
            }
            else        /* PIN_UP */
            {
                y = y1 - TextInside;

                if( DrawPinName )
                    DrawGraphicText( panel, DC, wxPoint( x1, y ), NameColor,
                                     m_name,
                                     TEXT_ORIENT_VERT, PinNameSize,
                                     GR_TEXT_HJUSTIFY_LEFT,
                                     GR_TEXT_VJUSTIFY_CENTER, nameLineWidth,
                                     false, false );
                if( DrawPinNum )
                    DrawGraphicText( panel, DC,
                                     wxPoint( x1 - TXTMARGE,
                                              (y1 + pin_pos.y) / 2 ), NumColor,
                                     StringPinNum,
                                     TEXT_ORIENT_VERT, PinNumSize,
                                     GR_TEXT_HJUSTIFY_CENTER,
                                     GR_TEXT_VJUSTIFY_BOTTOM, numLineWidth,
                                     false, false );
            }
        }
    }
    else     /**** Draw num & text pin outside  ****/
    {
        if( (orient == PIN_LEFT) || (orient == PIN_RIGHT) )
        {
            /* Its an horizontal line. */
            if( DrawPinName )
            {
                x = (x1 + pin_pos.x) / 2;
                DrawGraphicText( panel, DC, wxPoint( x, y1 - TXTMARGE ),
                                 NameColor, m_name,
                                 TEXT_ORIENT_HORIZ, PinNameSize,
                                 GR_TEXT_HJUSTIFY_CENTER,
                                 GR_TEXT_VJUSTIFY_BOTTOM, nameLineWidth,
                                 false, false );
            }
            if( DrawPinNum )
            {
                x = (x1 + pin_pos.x) / 2;
                DrawGraphicText( panel, DC, wxPoint( x, y1 + TXTMARGE ),
                                 NumColor, StringPinNum,
                                 TEXT_ORIENT_HORIZ, PinNumSize,
                                 GR_TEXT_HJUSTIFY_CENTER,
                                 GR_TEXT_VJUSTIFY_TOP, numLineWidth,
                                 false, false );
            }
        }
        else     /* Its a vertical line. */
        {
            if( DrawPinName )
            {
                y = (y1 + pin_pos.y) / 2;
                DrawGraphicText( panel, DC, wxPoint( x1 - TXTMARGE, y ),
                                 NameColor, m_name,
                                 TEXT_ORIENT_VERT, PinNameSize,
                                 GR_TEXT_HJUSTIFY_CENTER,
                                 GR_TEXT_VJUSTIFY_BOTTOM, nameLineWidth,
                                 false, false );
            }

            if( DrawPinNum )
            {
                DrawGraphicText( panel, DC,
                                 wxPoint( x1 + TXTMARGE, (y1 + pin_pos.y) / 2 ),
                                 NumColor, StringPinNum,
                                 TEXT_ORIENT_VERT, PinNumSize,
                                 GR_TEXT_HJUSTIFY_CENTER,
                                 GR_TEXT_VJUSTIFY_TOP, numLineWidth,
                                 false, false );
            }
        }
    }
}


/*****************************************************************************
* Plot pin number and pin text info, given the pin line coordinates.      *
* Same as DrawPinTexts((), but output is the plotter
* The line must be vertical or horizontal.                        *
* If PinNext == NULL nothing is printed.                                    *
* Current Zoom factor is taken into account.                     *
* If TextInside then the text is been put inside (moving from x1, y1 in      *
* the opposite direction to x2,y2), otherwise all is drawn outside.      *
*****************************************************************************/
void LIB_PIN::PlotPinTexts( PLOTTER *plotter,
                            wxPoint& pin_pos,
                            int      orient,
                            int      TextInside,
                            bool     DrawPinNum,
                            bool     DrawPinName,
                            int      aWidth )
{
    int        x, y, x1, y1;
    wxString   StringPinNum;
    EDA_Colors NameColor, NumColor;
    wxSize     PinNameSize = wxSize( m_PinNameSize, m_PinNameSize );
    wxSize     PinNumSize  = wxSize( m_PinNumSize, m_PinNumSize );

    /* Get the num and name colors */
    NameColor = ReturnLayerColor( LAYER_PINNAM );
    NumColor  = ReturnLayerColor( LAYER_PINNUM );

    /* Create the pin num string */
    ReturnPinStringNum( StringPinNum );
    x1 = pin_pos.x;
    y1 = pin_pos.y;

    switch( orient )
    {
    case PIN_UP:
        y1 -= m_length;
        break;

    case PIN_DOWN:
        y1 += m_length;
        break;

    case PIN_LEFT:
        x1 -= m_length;
        break;

    case PIN_RIGHT:
        x1 += m_length;
        break;
    }

    if( m_name.IsEmpty() )
        DrawPinName = FALSE;

    /* Draw the text inside, but the pin numbers outside. */
    if( TextInside )
    {
        if( (orient == PIN_LEFT) || (orient == PIN_RIGHT) ) /* Its an horizontal line. */
        {
            if( DrawPinName )
            {
                if( orient == PIN_RIGHT )
                {
                    x = x1 + TextInside;
                    plotter->text( wxPoint( x, y1 ), NameColor,
                                   m_name,
                                   TEXT_ORIENT_HORIZ,
                                   PinNameSize,
                                   GR_TEXT_HJUSTIFY_LEFT,
                                   GR_TEXT_VJUSTIFY_CENTER,
                                   aWidth, false, false );
                }
                else    // orient == PIN_LEFT
                {
                    x = x1 - TextInside;

                    if( DrawPinName )
                        plotter->text( wxPoint( x, y1 ),
                                       NameColor, m_name, TEXT_ORIENT_HORIZ,
                                       PinNameSize,
                                       GR_TEXT_HJUSTIFY_RIGHT,
                                       GR_TEXT_VJUSTIFY_CENTER,
                                       aWidth, false, false );
                }
            }
            if( DrawPinNum )
            {
                plotter->text( wxPoint( (x1 + pin_pos.x) / 2,
                                        y1 - TXTMARGE ),
                               NumColor, StringPinNum,
                               TEXT_ORIENT_HORIZ, PinNumSize,
                               GR_TEXT_HJUSTIFY_CENTER,
                               GR_TEXT_VJUSTIFY_BOTTOM,
                               aWidth, false, false );
            }
        }
        else         /* Its a vertical line. */
        {
            if( orient == PIN_DOWN )
            {
                y = y1 + TextInside;

                if( DrawPinName )
                    plotter->text( wxPoint( x1, y ), NameColor,
                                   m_name,
                                   TEXT_ORIENT_VERT, PinNameSize,
                                   GR_TEXT_HJUSTIFY_RIGHT,
                                   GR_TEXT_VJUSTIFY_CENTER,
                                   aWidth, false, false );
                if( DrawPinNum )
                {
                    plotter->text( wxPoint( x1 - TXTMARGE, (y1 + pin_pos.y) / 2 ),
                                   NumColor, StringPinNum,
                                   TEXT_ORIENT_VERT, PinNumSize,
                                   GR_TEXT_HJUSTIFY_CENTER,
                                   GR_TEXT_VJUSTIFY_BOTTOM,
                                   aWidth, false, false );
                }
            }
            else        /* PIN_UP */
            {
                y = y1 - TextInside;

                if( DrawPinName )
                    plotter->text( wxPoint( x1, y ), NameColor,
                                   m_name,
                                   TEXT_ORIENT_VERT, PinNameSize,
                                   GR_TEXT_HJUSTIFY_LEFT,
                                   GR_TEXT_VJUSTIFY_CENTER,
                                   aWidth, false, false );
                if( DrawPinNum )
                {
                    plotter->text( wxPoint( x1 - TXTMARGE, (y1 + pin_pos.y) / 2 ),
                                   NumColor, StringPinNum,
                                   TEXT_ORIENT_VERT, PinNumSize,
                                   GR_TEXT_HJUSTIFY_CENTER,
                                   GR_TEXT_VJUSTIFY_BOTTOM,
                                   aWidth, false, false );
                }
            }
        }
    }
    else     /* Draw num & text pin outside */
    {
        if( (orient == PIN_LEFT) || (orient == PIN_RIGHT) )
        {
            /* Its an horizontal line. */
            if( DrawPinName )
            {
                x = (x1 + pin_pos.x) / 2;
                plotter->text( wxPoint( x, y1 - TXTMARGE ),
                               NameColor, m_name,
                               TEXT_ORIENT_HORIZ, PinNameSize,
                               GR_TEXT_HJUSTIFY_CENTER,
                               GR_TEXT_VJUSTIFY_BOTTOM,
                               aWidth, false, false );
            }
            if( DrawPinNum )
            {
                x = ( x1 + pin_pos.x ) / 2;
                plotter->text( wxPoint( x, y1 + TXTMARGE ),
                               NumColor, StringPinNum,
                               TEXT_ORIENT_HORIZ, PinNumSize,
                               GR_TEXT_HJUSTIFY_CENTER,
                               GR_TEXT_VJUSTIFY_TOP,
                               aWidth, false, false );
            }
        }
        else     /* Its a vertical line. */
        {
            if( DrawPinName )
            {
                y = ( y1 + pin_pos.y ) / 2;
                plotter->text( wxPoint( x1 - TXTMARGE, y ),
                               NameColor, m_name,
                               TEXT_ORIENT_VERT, PinNameSize,
                               GR_TEXT_HJUSTIFY_CENTER,
                               GR_TEXT_VJUSTIFY_BOTTOM,
                               aWidth, false, false );
            }

            if( DrawPinNum )
            {
                plotter->text( wxPoint( x1 + TXTMARGE, ( y1 + pin_pos.y ) / 2 ),
                               NumColor, StringPinNum,
                               TEXT_ORIENT_VERT, PinNumSize,
                               GR_TEXT_HJUSTIFY_CENTER,
                               GR_TEXT_VJUSTIFY_TOP,
                               aWidth, false, false );
            }
        }
    }
}


/* return the pin end position, for a component in normal orient */
wxPoint LIB_PIN::ReturnPinEndPoint() const
{
    wxPoint pos = m_position;

    switch( m_orientation )
    {
    case PIN_UP:
        pos.y += m_length;
        break;

    case PIN_DOWN:
        pos.y -= m_length;
        break;

    case PIN_LEFT:
        pos.x -= m_length;
        break;

    case PIN_RIGHT:
        pos.x += m_length;
        break;
    }

    return pos;
}


int LIB_PIN::ReturnPinDrawOrient( const TRANSFORM& aTransform )
{
    int     orient;
    wxPoint end;   // position of pin end starting at 0,0 according to its orientation, length = 1

    switch( m_orientation )
    {
    case PIN_UP:
        end.y = 1;
        break;

    case PIN_DOWN:
        end.y = -1;
        break;

    case PIN_LEFT:
        end.x = -1;
        break;

    case PIN_RIGHT:
        end.x = 1;
        break;
    }

    // = pos of end point, according to the component orientation
    end = aTransform.TransformCoordinate( end );
    orient = PIN_UP;

    if( end.x == 0 )
    {
        if( end.y > 0 )
            orient = PIN_DOWN;
    }
    else
    {
        orient = PIN_RIGHT;

        if( end.x < 0 )
            orient = PIN_LEFT;
    }

    return orient;
}


/**
 * Function ReturnPinStringNum
 * fill a buffer with pin num as a wxString
 *  Pin num is coded as a long or 4 ascii chars
 *  Used to print/draw the pin num
 * @param aStringBuffer = the wxString to store the pin num as an unicode string
 */
void LIB_PIN::ReturnPinStringNum( wxString& aStringBuffer ) const
{
    aStringBuffer = ReturnPinStringNum( m_number );
}


/**
 * Function ReturnPinStringNum (static function)
 *  Pin num is coded as a long or 4 ascii chars
 * @param aPinNum = a long containing a pin num
 * @return aStringBuffer = the wxString to store the pin num as an unicode string
 */
wxString LIB_PIN::ReturnPinStringNum( long aPinNum )
{
    char ascii_buf[5];

    memcpy( ascii_buf, &aPinNum, 4 );
    ascii_buf[4] = 0;

    wxString buffer = CONV_FROM_UTF8( ascii_buf );

    return buffer;
}


/**
 * Function SetPinNumFromString
 * fill the buffer with pin num as a wxString
 *  Pin num is coded as a long
 *  Used to print/draw the pin num
 */
void LIB_PIN::SetPinNumFromString( wxString& buffer )
{
    char     ascii_buf[4];
    unsigned ii, len = buffer.Len();

    ascii_buf[0] = ascii_buf[1] = ascii_buf[2] = ascii_buf[3] = 0;

    if( len > 4 )
        len = 4;

    for( ii = 0; ii < len; ii++ )
    {
        ascii_buf[ii]  = buffer.GetChar( ii );
        ascii_buf[ii] &= 0xFF;
    }

    strncpy( (char*) &m_number, ascii_buf, 4 );
}


LIB_DRAW_ITEM* LIB_PIN::DoGenCopy()
{
    LIB_PIN* newpin = new LIB_PIN( GetParent() );

    newpin->m_position           = m_position;
    newpin->m_length             = m_length;
    newpin->m_orientation        = m_orientation;
    newpin->m_shape              = m_shape;
    newpin->m_type               = m_type;
    newpin->m_attributes         = m_attributes;
    newpin->m_number             = m_number;
    newpin->m_PinNumSize         = m_PinNumSize;
    newpin->m_PinNameSize        = m_PinNameSize;
    newpin->m_PinNumShapeOpt     = m_PinNumShapeOpt;
    newpin->m_PinNameShapeOpt    = m_PinNameShapeOpt;
    newpin->m_PinNumPositionOpt  = m_PinNumPositionOpt;
    newpin->m_PinNamePositionOpt = m_PinNamePositionOpt;
    newpin->m_Unit               = m_Unit;
    newpin->m_Convert            = m_Convert;
    newpin->m_Flags              = m_Flags;
    newpin->m_width              = m_width;
    newpin->m_name               = m_name;

    return (LIB_DRAW_ITEM*) newpin;
}


int LIB_PIN::DoCompare( const LIB_DRAW_ITEM& other ) const
{
    wxASSERT( other.Type() == LIB_PIN_T );

    const LIB_PIN* tmp = ( LIB_PIN* ) &other;

    if( m_number != tmp->m_number )
        return m_number - tmp->m_number;

    int result = m_name.CmpNoCase( tmp->m_name );

    if( result != 0 )
        return result;

    if( m_position.x != tmp->m_position.x )
        return m_position.x - tmp->m_position.x;

    if( m_position.y != tmp->m_position.y )
        return m_position.y - tmp->m_position.y;

    return 0;
}


void LIB_PIN::DoOffset( const wxPoint& offset )
{
    m_position += offset;
}


bool LIB_PIN::DoTestInside( EDA_Rect& rect ) const
{
    wxPoint end = ReturnPinEndPoint();

    return rect.Contains( m_position.x, -m_position.y ) || rect.Contains( end.x, -end.y );
}


void LIB_PIN::DoMove( const wxPoint& newPosition )
{
    if( m_position != newPosition )
    {
        m_position = newPosition;
        SetModified();
    }
}


void LIB_PIN::DoMirrorHorizontal( const wxPoint& center )
{
    m_position.x -= center.x;
    m_position.x *= -1;
    m_position.x += center.x;

    if( m_orientation == PIN_RIGHT )
        m_orientation = PIN_LEFT;
    else if( m_orientation == PIN_LEFT )
        m_orientation = PIN_RIGHT;
}


void LIB_PIN::DoPlot( PLOTTER* plotter, const wxPoint& offset, bool fill,
                      const TRANSFORM& aTransform )
{
    if( m_attributes & PINNOTDRAW )
        return;

    int orient = ReturnPinDrawOrient( aTransform );

    wxPoint pos = aTransform.TransformCoordinate( m_position ) + offset;

    plotter->set_current_line_width( GetPenSize() );
    PlotPinSymbol( plotter, pos, m_length, orient, m_shape );
    PlotPinTexts( plotter, pos, orient, GetParent()->GetPinNameOffset(),
                  GetParent()->ShowPinNumbers(), GetParent()->ShowPinNames(),
                  GetPenSize() );
}


void LIB_PIN::DoSetWidth( int aWidth )
{
    if( m_width != aWidth )
    {
        m_width = aWidth;
        SetModified();
    }
}


/**
 * Function DisplayInfo
 * Displays info (pin num and name, orientation ...
 * on the Info window
 */
void LIB_PIN::DisplayInfo( EDA_DRAW_FRAME* frame )
{
    wxString Text;

    LIB_DRAW_ITEM::DisplayInfo( frame );

    frame->AppendMsgPanel( _( "Name" ), m_name, DARKCYAN );

    if( m_number == 0 )
        Text = wxT( "?" );
    else
        ReturnPinStringNum( Text );

    frame->AppendMsgPanel( _( "Number" ), Text, DARKCYAN );

    frame->AppendMsgPanel( _( "Type" ),
                           wxGetTranslation( pin_electrical_type_names[ m_type ] ),
                           RED );
    Text = wxGetTranslation(pin_style_names[ GetStyleCodeIndex( m_shape ) ]);
    frame->AppendMsgPanel( _( "Style" ), Text, BLUE );
    if( IsVisible() )
        Text = _( "Yes" );
    else
        Text = _( "No" );
    frame->AppendMsgPanel( _( "Visible" ), Text, DARKGREEN );

    /* Display pin length */
    Text = ReturnStringFromValue( g_UserUnit, m_length, EESCHEMA_INTERNAL_UNIT, true );
    frame->AppendMsgPanel( _( "Length" ), Text, MAGENTA );

    Text = wxGetTranslation(pin_orientation_names[ GetOrientationCodeIndex( m_orientation ) ]);
    frame->AppendMsgPanel( _( "Orientation" ), Text, DARKMAGENTA );
}


/**
 * Function GetBoundingBox
 * @return the boundary box for this, in schematic coordinates
 */
EDA_Rect LIB_PIN::GetBoundingBox() const
{
    wxPoint pt = m_position;

    pt.y *= -1;     // Reverse the Y axis, according to the schematic orientation

    return EDA_Rect( pt, wxSize( 1, 1 ) );
}


wxArrayString LIB_PIN::GetOrientationNames( void )
{
    wxArrayString tmp;

    for( unsigned ii = 0; ii < PIN_ORIENTATION_CNT; ii++ )
        tmp.Add( wxGetTranslation( pin_orientation_names[ii] ) );

    return tmp;
}


int LIB_PIN::GetOrientationCode( int index )
{
    if( index >= 0 && index < (int) PIN_ORIENTATION_CNT )
        return pin_orientation_codes[ index ];

    return PIN_RIGHT;
}


int LIB_PIN::GetOrientationCodeIndex( int code )
{
    size_t i;

    for( i = 0; i < PIN_ORIENTATION_CNT; i++ )
    {
        if( pin_orientation_codes[i] == code )
            return (int) i;
    }

    return wxNOT_FOUND;
}


wxArrayString LIB_PIN::GetStyleNames( void )
{
    wxArrayString tmp;

    for( unsigned ii = 0; ii < PIN_STYLE_CNT; ii++ )
        tmp.Add( wxGetTranslation( pin_style_names[ii] ) );

    return tmp;
}


int LIB_PIN::GetStyleCode( int index )
{
    if( index >= 0 && index < (int) PIN_STYLE_CNT )
        return pin_style_codes[ index ];

    return NONE;
}


int LIB_PIN::GetStyleCodeIndex( int code )
{
    size_t i;

    for( i = 0; i < PIN_STYLE_CNT; i++ )
    {
        if( pin_style_codes[i] == code )
            return (int) i;
    }

    return wxNOT_FOUND;
}


wxArrayString LIB_PIN::GetElectricalTypeNames( void )
{
    wxArrayString tmp;

    for( unsigned ii = 0; ii < PIN_ELECTRICAL_TYPE_CNT; ii++ )
        tmp.Add( wxGetTranslation( pin_electrical_type_names[ii] ) );

    return tmp;
}


const char*** LIB_PIN::GetElectricalTypeSymbols( void )
{
    return s_icons_Pins_Electrical_Type;
}


const char*** LIB_PIN::GetOrientationSymbols()
{
    return s_icons_Pins_Orientations;
}


const char*** LIB_PIN::GetStyleSymbols()
{
    return s_icons_Pins_Shapes;
}


#if defined(DEBUG)

void LIB_PIN::Show( int nestLevel, std::ostream& os )
{
    NestedSpace( nestLevel, os ) << '<' << GetClass().Lower().mb_str()
                                 << " num=\"" << GetNumberString().mb_str()
                                 << '"' << "/>\n";


//    NestedSpace( nestLevel, os ) << "</" << GetClass().Lower().mb_str() << ">\n";
}

#endif
