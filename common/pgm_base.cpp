/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2008-2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/**
 * @file pgm_base.cpp
 *
 * @brief For the main application: init functions, and language selection
 *        (locale handling)
 */

#include <fctsys.h>
#include <wx/html/htmlwin.h>
#include <wx/fs_zip.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/snglinst.h>
#include <wx/stdpaths.h>

#include <pgm_base.h>
#include <wxstruct.h>
#include <macros.h>
#include <config_params.h>
#include <id.h>
#include <build_version.h>
#include <hotkeys_basic.h>
#include <online_help.h>
#include <gestfich.h>
#include <menus_helpers.h>
#include <confirm.h>


#define KICAD_COMMON                    wxT( "kicad_common" )

// some key strings used to store parameters in KICAD_COMMON

const wxChar PGM_BASE::workingDirKey[]  =  wxT( "WorkingDir" );     // public

static const wxChar languageCfgKey[] =  wxT( "LanguageID" );
static const wxChar kicadFpLibPath[] =  wxT( "KicadFootprintLibraryPath" );


/**
 *   A small class to handle the list of existing translations.
 *   The locale translation is automatic.
 *   The selection of languages is mainly for maintainer's convenience
 *   To add a support to a new translation:
 *   create a new icon (flag of the country) (see Lang_Fr.xpm as an example)
 *   add a new item to s_Languages[].
 */
struct LANGUAGE_DESCR
{
    /// wxWidgets locale identifier (See wxWidgets doc)
    int         m_WX_Lang_Identifier;

    /// KiCad identifier used in menu selection (See id.h)
    int         m_KI_Lang_Identifier;

    /// The menu language icons
    BITMAP_DEF  m_Lang_Icon;

    /// Labels used in menus
    wxString    m_Lang_Label;

    /// Set to true if the m_Lang_Label must not be translated
    bool        m_DoNotTranslate;
};


/**
 * Variable s_Languages
 * Note: because this list is not created on the fly, wxTranslation
 * must be called when a language name must be displayed after translation.
 * Do not change this behavior, because m_Lang_Label is also used as key in config
 */
static LANGUAGE_DESCR s_Languages[] =
{
    // Default language
    {
        wxLANGUAGE_DEFAULT,
        ID_LANGUAGE_DEFAULT,
        lang_def_xpm,
        _( "Default" )
    },

    // English language
    {
        wxLANGUAGE_ENGLISH,
        ID_LANGUAGE_ENGLISH,
        lang_en_xpm,
        wxT( "English" ),
        true
    },

    // French language
    {
        wxLANGUAGE_FRENCH,
        ID_LANGUAGE_FRENCH,
        lang_fr_xpm,
        _( "French" )
    },

    // Finnish language
    {
        wxLANGUAGE_FINNISH,
        ID_LANGUAGE_FINNISH,
        lang_fi_xpm,
        _( "Finnish" )
    },

    // Spanish language
    {
        wxLANGUAGE_SPANISH,
        ID_LANGUAGE_SPANISH,
        lang_es_xpm,
        _( "Spanish" )
    },

    // Portuguese language
    {
        wxLANGUAGE_PORTUGUESE,
        ID_LANGUAGE_PORTUGUESE,
        lang_pt_xpm,
        _( "Portuguese" )
    },

    // Italian language
    {
        wxLANGUAGE_ITALIAN,
        ID_LANGUAGE_ITALIAN,
        lang_it_xpm,
        _( "Italian" )
    },

    // German language
    {
        wxLANGUAGE_GERMAN,
        ID_LANGUAGE_GERMAN,
        lang_de_xpm,
        _( "German" )
    },

    // Greek language
    {
        wxLANGUAGE_GREEK,
        ID_LANGUAGE_GREEK,
        lang_gr_xpm,
        _( "Greek" )
    },

    // Slovenian language
    {
        wxLANGUAGE_SLOVENIAN,
        ID_LANGUAGE_SLOVENIAN,
        lang_sl_xpm,
        _( "Slovenian" )
    },

    // Hungarian language
    {
        wxLANGUAGE_HUNGARIAN,
        ID_LANGUAGE_HUNGARIAN,
        lang_hu_xpm,
        _( "Hungarian" )
    },

    // Polish language
    {
        wxLANGUAGE_POLISH,
        ID_LANGUAGE_POLISH,
        lang_pl_xpm,
        _( "Polish" )
    },

    // Czech language
    {
        wxLANGUAGE_CZECH,
        ID_LANGUAGE_CZECH,
        lang_cs_xpm,
        _( "Czech" )
    },

    // Russian language
    {
        wxLANGUAGE_RUSSIAN,
        ID_LANGUAGE_RUSSIAN,
        lang_ru_xpm,
        _( "Russian" )
    },

    // Korean language
    {
        wxLANGUAGE_KOREAN,
        ID_LANGUAGE_KOREAN,
        lang_ko_xpm,
        _( "Korean" )
    },

    // Chinese simplified
    {
        wxLANGUAGE_CHINESE_SIMPLIFIED,
        ID_LANGUAGE_CHINESE_SIMPLIFIED,
        lang_chinese_xpm,
        _( "Chinese simplified" )
    },

    // Catalan language
    {
        wxLANGUAGE_CATALAN,
        ID_LANGUAGE_CATALAN,
        lang_catalan_xpm,
        _( "Catalan" )
    },

    // Dutch language
    {
        wxLANGUAGE_DUTCH,
        ID_LANGUAGE_DUTCH,
        lang_nl_xpm,
        _( "Dutch" )
    },

    // Japanese language
    {
        wxLANGUAGE_JAPANESE,
        ID_LANGUAGE_JAPANESE,
        lang_jp_xpm,
        _( "Japanese" )
    },

    // Bulgarian language
    {
        wxLANGUAGE_BULGARIAN,
        ID_LANGUAGE_BULGARIAN,
        lang_bg_xpm,
        _( "Bulgarian" )
    }
};


PGM_BASE::PGM_BASE()
{
    m_pgm_checker = NULL;
    m_file_checker = NULL;
    m_html_ctrl = NULL;
    m_locale = NULL;
    m_common_settings = NULL;

    m_wx_app = NULL;

    setLanguageId( wxLANGUAGE_DEFAULT );

    ForceSystemPdfBrowser( false );
}


PGM_BASE::~PGM_BASE()
{
    destroy();
}


void PGM_BASE::destroy()
{
    // unlike a normal destructor, this is designed to be called more than once safely:

    delete m_common_settings;
    m_common_settings = 0;

    delete m_pgm_checker;
    m_pgm_checker = 0;

    delete m_file_checker;
    m_file_checker = 0;

    delete m_locale;
    m_locale = 0;

    delete m_html_ctrl;
    m_html_ctrl = 0;
}


void PGM_BASE::SetEditorName( const wxString& aFileName )
{
    m_editor_name = aFileName;
    wxASSERT( m_common_settings );
    m_common_settings->Write( wxT( "Editor" ), aFileName );
}


const wxString& PGM_BASE::GetEditorName()
{
    wxString editorname = m_editor_name;

    if( !editorname )
    {
        // Get the preferred editor name from environment variable first.
        if(!wxGetEnv( wxT( "EDITOR" ), &editorname ))
        {
            // If there is no EDITOR variable set, try the desktop default
#ifdef __WXMAC__
            editorname = "/usr/bin/open";
#elif __WXX11__
            editorname = "/usr/bin/xdg-open";
#endif
        }
    }

    if( !editorname )       // We must get a preferred editor name
    {
        DisplayInfoMessage( NULL,
                            _( "No default editor found, you must choose it" ) );

        wxString mask( wxT( "*" ) );

#ifdef __WINDOWS__
        mask += wxT( ".exe" );
#endif
        editorname = EDA_FileSelector( _( "Preferred Editor:" ), wxEmptyString,
                                       wxEmptyString, wxEmptyString, mask,
                                       NULL, wxFD_OPEN, true );
    }

    if( !editorname.IsEmpty() )
    {
        m_editor_name = editorname;
        m_common_settings->Write( wxT( "Editor" ), m_editor_name );
    }

    return m_editor_name;
}


bool PGM_BASE::initPgm()
{
    wxFileName pgm_name( App().argv[0] );

    wxInitAllImageHandlers();

    m_pgm_checker = new wxSingleInstanceChecker( pgm_name.GetName().Lower() + wxT( "-" ) + wxGetUserId() );

    if( m_pgm_checker->IsAnotherRunning() )
    {
        wxString quiz = wxString::Format(
            _( "%s is already running, Continue?" ),
            GetChars( pgm_name.GetName() )
            );
        if( !IsOK( NULL, quiz ) )
            return false;
    }

    // Init KiCad environment
    // the environment variable KICAD (if exists) gives the kicad path:
    // something like set KICAD=d:\kicad
    bool isDefined = wxGetEnv( wxT( "KICAD" ), &m_kicad_env );

    if( isDefined )    // ensure m_kicad_env ends by "/"
    {
        m_kicad_env.Replace( WIN_STRING_DIR_SEP, UNIX_STRING_DIR_SEP );

        if( !m_kicad_env.IsEmpty() && m_kicad_env.Last() != '/' )
            m_kicad_env += UNIX_STRING_DIR_SEP;
    }

    // Init parameters for configuration
    App().SetVendorName( wxT( "KiCad" ) );
    App().SetAppName( pgm_name.GetName().Lower() );

    // Install some image handlers, mainly for help
    wxImage::AddHandler( new wxPNGHandler );
    wxImage::AddHandler( new wxGIFHandler );
    wxImage::AddHandler( new wxJPEGHandler );
    wxFileSystem::AddHandler( new wxZipFSHandler );

    // Analyze the command line & initialize the binary path
    setExecutablePath();

    SetLanguagePath();

    // OS specific instantiation of wxConfigBase derivative:
    m_common_settings = new wxConfig( KICAD_COMMON );

    ReadPdfBrowserInfos();      // needs m_common_settings

    loadCommonSettings();


    bool succes = SetLanguage( true );

    if( !succes )
    {
    }

    // Set locale option for separator used in float numbers
    SetLocaleTo_Default();

    return true;
}


void PGM_BASE::SetHtmlHelpController( wxHtmlHelpController* aController )
{
    delete m_html_ctrl;
    m_html_ctrl = aController;
}


void PGM_BASE::InitOnLineHelp()
{
    wxString fullfilename = FindKicadHelpPath();

#if defined ONLINE_HELP_FILES_FORMAT_IS_HTML
    m_HelpFileName = fullfilename + wxT( ".html" );
    fullfilename  += wxT( "kicad.hhp" );

    if( wxFileExists( fullfilename ) )
    {
        m_html_ctrl = new wxHtmlHelpController( wxHF_TOOLBAR | wxHF_CONTENTS |
                                               wxHF_PRINT | wxHF_OPEN_FILES
                                               /*| wxHF_SEARCH */ );
        m_html_ctrl->UseConfig( m_common_settings );
        m_html_ctrl->SetTitleFormat( wxT( "KiCad Help" ) );
        m_html_ctrl->AddBook( fullfilename );
    }

#elif defined ONLINE_HELP_FILES_FORMAT_IS_PDF
    m_html_ctrl = NULL;

#else
    #error Help files format not defined
#endif
}


bool PGM_BASE::setExecutablePath()
{
// Apple MacOSx
#ifdef __APPLE__

    // Derive path from location of the app bundle
    CFBundleRef mainBundle = CFBundleGetMainBundle();

    if( mainBundle == NULL )
        return false;

    CFURLRef urlref = CFBundleCopyBundleURL( mainBundle );

    if( urlref == NULL )
        return false;

    CFStringRef str = CFURLCopyFileSystemPath( urlref, kCFURLPOSIXPathStyle );

    if( str == NULL )
        return false;

    char* native_str = NULL;
    int   len = CFStringGetMaximumSizeForEncoding( CFStringGetLength( str ),
                                                   kCFStringEncodingUTF8 ) + 1;
    native_str = new char[len];

    CFStringGetCString( str, native_str, len, kCFStringEncodingUTF8 );
    m_bin_dir = FROM_UTF8( native_str );
    delete[] native_str;

#else
    m_bin_dir = wxStandardPaths::Get().GetExecutablePath();

#endif

    // Use unix notation for paths. I am not sure this is a good idea,
    // but it simplifies compatibility between Windows and Unices.
    // However it is a potential problem in path handling under Windows.
    m_bin_dir.Replace( WIN_STRING_DIR_SEP, UNIX_STRING_DIR_SEP );

    // Remove file name form command line:
    while( m_bin_dir.Last() != '/' && !m_bin_dir.IsEmpty() )
        m_bin_dir.RemoveLast();

    return true;
}


void PGM_BASE::loadCommonSettings()
{
    wxASSERT( m_common_settings );

    m_help_size.x = 500;
    m_help_size.y = 400;

    wxString languageSel;

    m_common_settings->Read( languageCfgKey, &languageSel );
    setLanguageId( wxLANGUAGE_DEFAULT );

    // Search for the current selection
    for( unsigned ii = 0; ii < DIM( s_Languages ); ii++ )
    {
        if( s_Languages[ii].m_Lang_Label == languageSel )
        {
            setLanguageId( s_Languages[ii].m_WX_Lang_Identifier );
            break;
        }
    }

    m_editor_name = m_common_settings->Read( wxT( "Editor" ) );
}


void PGM_BASE::saveCommonSettings()
{
    // m_common_settings is not initialized until fairly late in the
    // process startup: initPgm(), so test before using:
    if( m_common_settings )
    {
        m_common_settings->Write( workingDirKey, wxGetCwd() );
    }
}


bool PGM_BASE::SetLanguage( bool first_time )
{
    bool     retv = true;

    // dictionary file name without extend (full name is kicad.mo)
    wxString dictionaryName( wxT( "kicad" ) );

    delete m_locale;
    m_locale = new wxLocale;

#if wxCHECK_VERSION( 2, 9, 0 )
    if( !m_locale->Init( m_language_id ) )
#else
    if( !m_locale->Init( m_language_id, wxLOCALE_CONV_ENCODING ) )
#endif
    {
        wxLogDebug( wxT( "This language is not supported by the system." ) );

        setLanguageId( wxLANGUAGE_DEFAULT );
        delete m_locale;

        m_locale = new wxLocale;
        m_locale->Init();
        retv = false;
    }
    else if( !first_time )
    {
        wxLogDebug( wxT( "Search for dictionary %s.mo in %s" ),
                    GetChars( dictionaryName ), GetChars( m_locale->GetName() ) );
    }

    // how about a meaningful comment here.
    if( !first_time )
    {
        wxString languageSel;

        // Search for the current selection
        for( unsigned ii = 0; ii < DIM( s_Languages ); ii++ )
        {
            if( s_Languages[ii].m_WX_Lang_Identifier == m_language_id )
            {
                languageSel = s_Languages[ii].m_Lang_Label;
                break;
            }
        }

        m_common_settings->Write( languageCfgKey, languageSel );
    }

    // Test if floating point notation is working (bug in cross compilation, using wine)
    // Make a conversion double <=> string
    double dtst = 0.5;
    wxString msg;

    extern bool g_DisableFloatingPointLocalNotation;    // See common.cpp

    g_DisableFloatingPointLocalNotation = false;

    msg << dtst;
    double result;
    msg.ToDouble( &result );

    if( result != dtst )  // string to double encode/decode does not work! Bug detected
    {
        // Disable floating point localization:
        g_DisableFloatingPointLocalNotation = true;
        SetLocaleTo_C_standard( );
    }

    if( !m_locale->IsLoaded( dictionaryName ) )
        m_locale->AddCatalog( dictionaryName );

    if( !retv )
        return retv;

    return m_locale->IsOk();
}


void PGM_BASE::SetLanguageIdentifier( int menu_id )
{
    wxLogDebug( wxT( "Select language ID %d from %zd possible languages." ),
                menu_id, DIM( s_Languages ) );

    for( unsigned ii = 0; ii < DIM( s_Languages ); ii++ )
    {
        if( menu_id == s_Languages[ii].m_KI_Lang_Identifier )
        {
            setLanguageId( s_Languages[ii].m_WX_Lang_Identifier );
            break;
        }
    }
}


void PGM_BASE::SetLanguagePath()
{
    SEARCH_STACK    guesses;

    SystemDirsAppend( &guesses );

    // Add our internat dir to the wxLocale catalog of paths
    for( unsigned i = 0; i < guesses.GetCount(); i++ )
    {
        wxFileName fn( guesses[i], wxEmptyString );

        // Append path for Windows and unix KiCad package install
        fn.AppendDir( wxT( "share" ) );
        fn.AppendDir( wxT( "internat" ) );

        if( fn.IsDirReadable() )
        {
            wxLogDebug( wxT( "Adding locale lookup path: " ) + fn.GetPath() );
            wxLocale::AddCatalogLookupPathPrefix( fn.GetPath() );
        }

        // Append path for unix standard install
        fn.RemoveLastDir();
        fn.AppendDir( wxT( "kicad" ) );
        fn.AppendDir( wxT( "internat" ) );

        if( fn.IsDirReadable() )
        {
            wxLogDebug( wxT( "Adding locale lookup path: " ) + fn.GetPath() );
            wxLocale::AddCatalogLookupPathPrefix( fn.GetPath() );
        }
    }
}


void PGM_BASE::AddMenuLanguageList( wxMenu* MasterMenu )
{
    wxMenu*      menu = NULL;
    wxMenuItem*  item = MasterMenu->FindItem( ID_LANGUAGE_CHOICE );

    if( item )     // This menu exists, do nothing
        return;

    menu = new wxMenu;

    for( unsigned ii = 0; ii < DIM( s_Languages ); ii++ )
    {
        wxString label;

        if( s_Languages[ii].m_DoNotTranslate )
            label = s_Languages[ii].m_Lang_Label;
        else
            label = wxGetTranslation( s_Languages[ii].m_Lang_Label );

        AddMenuItem( menu, s_Languages[ii].m_KI_Lang_Identifier,
                     label, KiBitmap(s_Languages[ii].m_Lang_Icon ),
                     wxITEM_CHECK );
    }

    AddMenuItem( MasterMenu, menu,
                 ID_LANGUAGE_CHOICE,
                 _( "Language" ),
                 _( "Select application language (only for testing!)" ),
                 KiBitmap( language_xpm ) );

    // Set Check mark on current selected language
    for( unsigned ii = 0;  ii < DIM( s_Languages );  ii++ )
    {
        if( m_language_id == s_Languages[ii].m_WX_Lang_Identifier )
            menu->Check( s_Languages[ii].m_KI_Lang_Identifier, true );
        else
            menu->Check( s_Languages[ii].m_KI_Lang_Identifier, false );
    }
}


bool PGM_BASE::LockFile( const wxString& aFileName )
{
    // first make absolute and normalize, to avoid that different lock files
    // for the same file can be created
    wxFileName fn( aFileName );

    fn.MakeAbsolute();

    // semaphore to protect the edition of the file by more than one instance
    if( m_file_checker != NULL )
    {
        // it means that we had an open file and we are opening a different one
        delete m_file_checker;
    }

    wxString lockFileName = fn.GetFullPath() + wxT( ".lock" );

    lockFileName.Replace( wxT( "/" ), wxT( "_" ) );

    // We can have filenames coming from Windows, so also convert Windows separator
    lockFileName.Replace( wxT( "\\" ), wxT( "_" ) );

    m_file_checker = new wxSingleInstanceChecker( lockFileName );

    if( m_file_checker &&
        m_file_checker->IsAnotherRunning() )
    {
        return false;
    }

    return true;
}
