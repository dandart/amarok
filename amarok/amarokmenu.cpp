// Maintainer: Max Howell <max.howell@methylblue.com>, (C) 2004
// Copyright:  See COPYING file that comes with this distribution

#include "amarokconfig.h"
#include "amarokmenu.h"
#include "playerapp.h"
#include "enginecontroller.h"

#include <kaction.h>
#include <khelpmenu.h>
#include <klocale.h>


using namespace amaroK;


//static member
KHelpMenu *Menu::HelpMenu = 0;


static void
safePlug( KActionCollection *ac, const char *name, QWidget *w )
{
    if( ac )
    {
        KAction *a = ac->action( name );
        if( a ) a->plug( w );
    }
}


Menu::Menu( QWidget *parent )
  : QPopupMenu( parent )
{
    KActionCollection *ac = pApp->actionCollection();

    setCheckable( true );

    insertItem( i18n( "Repeat &Track" ),    ID_REPEAT_TRACK );
    insertItem( i18n( "Repeat &Playlist" ), ID_REPEAT_PLAYLIST );
    insertItem( i18n( "Random &Mode" ),     ID_RANDOM_MODE );

    insertSeparator();

    insertItem( i18n( "Configure &Effects..." ), pApp, SLOT( showEffectWidget() ) );
    insertItem( i18n( "Configure &Decoder..." ), ID_CONF_DECODER );

    insertSeparator();

    safePlug( ac, KStdAction::name(KStdAction::ConfigureToolbars), this );
    safePlug( ac, KStdAction::name(KStdAction::KeyBindings), this );
    safePlug( ac, "options_configure_globals", this ); //we created this one
    safePlug( ac, KStdAction::name(KStdAction::Preferences), this );

    insertSeparator();

    insertItem( i18n( "&Help" ), helpMenu( parent ) );

    insertSeparator();

    safePlug( ac, KStdAction::name(KStdAction::Quit), this );

    connect( this, SIGNAL( aboutToShow() ), SLOT( slotAboutToShow() ) );
    connect( this, SIGNAL( activated(int) ), SLOT( slotActivated(int) ) );
}

KPopupMenu*
Menu::helpMenu( QWidget *parent ) //STATIC
{
    if( HelpMenu == 0 )
        HelpMenu = new KHelpMenu( parent, KGlobal::instance()->aboutData(), pApp->actionCollection() );

    return HelpMenu->menu();
}

void
Menu::slotAboutToShow()
{
    setItemChecked( ID_REPEAT_TRACK,    AmarokConfig::repeatTrack() );
    setItemChecked( ID_REPEAT_PLAYLIST, AmarokConfig::repeatPlaylist() );
    setItemChecked( ID_RANDOM_MODE,     AmarokConfig::randomMode() );
    setItemEnabled( ID_CONF_DECODER,    EngineController::instance()->engine()->decoderConfigurable() );
}

void
Menu::slotActivated( int index )
{
    switch( index ) {
    case ID_REPEAT_TRACK:
        AmarokConfig::setRepeatTrack( !isItemChecked(ID_REPEAT_TRACK) );
        break;
    case ID_REPEAT_PLAYLIST:
        AmarokConfig::setRepeatPlaylist( !isItemChecked(ID_REPEAT_PLAYLIST) );
        break;
    case ID_RANDOM_MODE:
        AmarokConfig::setRandomMode( !isItemChecked(ID_RANDOM_MODE) );
        break;
    case ID_CONF_DECODER:
        EngineController::engine()->configureDecoder();
        break;
    }
}



#include <ktoolbar.h>
#include <ktoolbarbutton.h>

MenuAction::MenuAction( KActionCollection *ac )
  : KAction( i18n( "amaroK Menu" ), 0, ac, "amarok_menu" )
{}

int
MenuAction::plug( QWidget *w, int index )
{
    KToolBar *bar = dynamic_cast<KToolBar*>(w);

    if( bar && kapp->authorizeKAction( name() ) )
    {
        const int id = KAction::getToolButtonID();

        addContainer( w, id );
        connect( w, SIGNAL( destroyed() ), SLOT( slotDestroyed() ) );

        //TODO create menu on demand
        //TODO create menu above and aligned within window
        //TODO make the arrow point upwards!
        bar->insertButton( QString::null, id, true, i18n( "Menu" ), index );
        bar->alignItemRight( id );

        KToolBarButton *button = bar->getButton( id );
        button->setPopup( new amaroK::Menu( bar ) );
        button->setName( "toolbutton_amarok_menu" );

        return containerCount() - 1;
    }
    else return -1;
}



PlayPauseAction::PlayPauseAction( KActionCollection *ac )
  : KAction( i18n( "Play/Pause" ), 0, ac, "play_pause" )
{
    EngineController* const ec = EngineController::instance();

    ec->attach( this );
    connect( this, SIGNAL( activated() ), ec, SLOT( pause() ) );
}

void
PlayPauseAction::engineStateChanged( EngineBase::EngineState state )
{
    switch( state )
    {
    case EngineBase::Playing:
        setIcon( "player_pause" );
        break;
    default:
        setIcon( "player_play" );
        break;
    }
}



#include "blockanalyzer.h"

AnalyzerAction::AnalyzerAction( KActionCollection *ac )
  : KAction( i18n( "Analyzer" ), 0, ac, "toolbar_analyzer" )
{}

int
AnalyzerAction::plug( QWidget *w, int index )
{
    KToolBar *bar = dynamic_cast<KToolBar*>(w);

    if( bar && kapp->authorizeKAction( name() ) )
    {
        const int id = KAction::getToolButtonID();

        addContainer( w, id );
        connect( w, SIGNAL( destroyed() ), SLOT( slotDestroyed() ) );

        QWidget *block = new BlockAnalyzer( w );
        block->setBackgroundColor( w->backgroundColor().dark( 110 ) );

        bar->insertWidget( id, 0, block, index );

        return containerCount() - 1;
    }
    else return -1;
}

#include "amarokmenu.moc"
