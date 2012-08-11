/****************************************************************************************
 * Copyright (c) 2012 Matěj Laitl <matej@laitl.cz>                                      *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "Controller.h"

#include "amarokconfig.h"
#include "EngineController.h"
#include "MainWindow.h"
#include "core/interfaces/Logger.h"
#include "core/support/Amarok.h"
#include "core/support/Components.h"
#include "core/support/Debug.h"
#include "statsyncing/Config.h"
#include "statsyncing/Process.h"
#include "statsyncing/ScrobblingService.h"
#include "statsyncing/collection/CollectionProvider.h"

#include <QTimer>
#include <KMessageBox>

using namespace StatSyncing;

Controller::Controller( QObject* parent )
    : QObject( parent )
    , m_startSyncingTimer( new QTimer( this ) )
    , m_config( new Config( this ) )
    , m_updateNowPlayingTimer( new QTimer( this ) )
{
    m_startSyncingTimer->setSingleShot( true );
    connect( m_startSyncingTimer, SIGNAL(timeout()), SLOT(startNonInteractiveSynchronization()) );
    CollectionManager *manager = CollectionManager::instance();
    Q_ASSERT( manager );
    connect( manager, SIGNAL(collectionAdded(Collections::Collection*,CollectionManager::CollectionStatus)),
             SLOT(slotCollectionAdded(Collections::Collection*,CollectionManager::CollectionStatus)) );
    connect( manager, SIGNAL(collectionRemoved(QString)), SLOT(slotCollectionRemoved(QString)) );
    delayedStartSynchronization();

    EngineController *engine = Amarok::Components::engineController();
    Q_ASSERT( engine );
    connect( engine, SIGNAL(trackFinishedPlaying(Meta::TrackPtr,double)),
             SLOT(slotTrackFinishedPlaying(Meta::TrackPtr,double)) );

    m_updateNowPlayingTimer->setSingleShot( true );
    m_updateNowPlayingTimer->setInterval( 10000 ); // wait 10s before updating
    // We connect the signals to (re)starting the timer to postpone the submission a
    // little to prevent frequent updates of rapidly - changing metadata
    connect( engine, SIGNAL(trackChanged(Meta::TrackPtr)),
             m_updateNowPlayingTimer, SLOT(start()) );
    // following is needed for streams that don't emit newTrackPlaying on song change
    connect( engine, SIGNAL(trackMetadataChanged(Meta::TrackPtr)),
             m_updateNowPlayingTimer, SLOT(start()) );
    connect( m_updateNowPlayingTimer, SIGNAL(timeout()),
             SLOT(slotUpdateNowPlayingWithCurrentTrack()) );
    // we need to reset m_lastSubmittedNowPlayingTrack when a track is played twice
    connect( engine, SIGNAL(trackChanged(Meta::TrackPtr)),
             SLOT(slotResetLastSubmittedNowPlayingTrack()) );
}

Controller::~Controller()
{
}

QList<qint64>
Controller::availableFields()
{
    return QList<qint64>() << Meta::valRating << Meta::valFirstPlayed
            << Meta::valLastPlayed << Meta::valPlaycount << Meta::valLabel;
}

void
Controller::registerProvider( const ProviderPtr &provider )
{
    QString id = provider->id();
    bool enabled;
    if( m_config->providerKnown( id ) )
        enabled = m_config->providerEnabled( id, false );
    else
    {
        switch( provider->defaultPreference() )
        {
            case Provider::Never:
            case Provider::NoByDefault:
                enabled = false;
                break;
            case Provider::Ask:
            {
                QString text = i18nc( "%1 is collection name", "%1 has an ability to "
                    "synchronize track meta-data such as play count or rating "
                    "with other collections. Do you want to keep %1 synchronized?\n\n"
                    "You can always change the decision in Amarok configuration.",
                    provider->prettyName() );
                enabled = KMessageBox::questionYesNo( The::mainWindow(), text ) == KMessageBox::Yes;
                break;
            }
            case Provider::YesByDefault:
                enabled = true;
                break;
        }
    }

    // don't tell config about Never-by-default providers
    if( provider->defaultPreference() != Provider::Never )
    {
        m_config->updateProvider( id, provider->prettyName(), provider->icon(), true, enabled );
        m_config->save();
    }
    m_providers.append( provider );
    connect( provider.data(), SIGNAL(updated()), SLOT(slotProviderUpdated()) );
    if( enabled )
        delayedStartSynchronization();
}

void
Controller::unregisterProvider( const ProviderPtr &provider )
{
    disconnect( provider.data(), 0, this, 0 );
    if( m_config->providerKnown( provider->id() ) )
    {
        m_config->updateProvider( provider->id(), provider->prettyName(),
                                  provider->icon(), /* online */ false );
        m_config->save();
    }
    m_providers.removeAll( provider );
}

void
Controller::registerScrobblingService( const ScrobblingServicePtr &service )
{
    if( m_scrobblingServices.contains( service ) )
    {
        warning() << __PRETTY_FUNCTION__ << "scrobbling service" << service << "already registered";
        return;
    }
    m_scrobblingServices << service;
}

void
Controller::unregisterScrobblingService( const ScrobblingServicePtr &service )
{
    m_scrobblingServices.removeAll( service );
}

Config *
Controller::config()
{
    return m_config;
}

void
Controller::synchronize()
{
    synchronize( Process::Interactive );
}

void
Controller::slotProviderUpdated()
{
    QObject *updatedProvider = sender();
    Q_ASSERT( updatedProvider );
    foreach( const ProviderPtr &provider, m_providers )
    {
        if( provider.data() == updatedProvider )
        {
            m_config->updateProvider( provider->id(), provider->prettyName(),
                                      provider->icon(), true );
            m_config->save();
        }
    }
}

void
Controller::delayedStartSynchronization()
{
    if( m_startSyncingTimer->isActive() )
        m_startSyncingTimer->start( 5000 ); // reset the timeout
    else
    {
        m_startSyncingTimer->start( 5000 );
        // we could as well connect to all m_providers updated signals, but this serves
        // for now
        CollectionManager *manager = CollectionManager::instance();
        Q_ASSERT( manager );
        connect( manager, SIGNAL(collectionDataChanged(Collections::Collection*)),
                 SLOT(delayedStartSynchronization()) );
    }
}

void
Controller::slotCollectionAdded( Collections::Collection *collection,
                                 CollectionManager::CollectionStatus status )
{
    if( status != CollectionManager::CollectionEnabled )
        return;
    ProviderPtr provider( new CollectionProvider( collection ) );
    registerProvider( provider );
}

void
Controller::slotCollectionRemoved( const QString &id )
{
    foreach( const ProviderPtr &provider, m_providers )
    {
        // here we depend on StatSyncing::CollectionProvider returning identical id
        // as collection
        if( provider->id() == id )
            unregisterProvider( provider );
    }
}

void
Controller::startNonInteractiveSynchronization()
{
    CollectionManager *manager = CollectionManager::instance();
    Q_ASSERT( manager );
    disconnect( manager, SIGNAL(collectionDataChanged(Collections::Collection*)),
                this, SLOT(delayedStartSynchronization()) );
    synchronize( Process::NonInteractive );
}

void Controller::synchronize( int intMode )
{
    Process::Mode mode = Process::Mode( intMode );
    if( m_currentProcess )
    {
        if( mode == StatSyncing::Process::Interactive )
            m_currentProcess.data()->raise();
        return;
    }

    if( m_providers.count() <= 1 )
    {
        if( mode == StatSyncing::Process::Interactive )
        {
            // the text intentionally doesn't cope with 0 collections
            QString text = i18n( "You only seem to have one collection. Statistics "
                "synchronization only makes sense if there is more than one collection." );
            Amarok::Components::logger()->longMessage( text );
        }
        return;
    }

    // read saved config
    qint64 fields = m_config->checkedFields();
    ProviderPtrSet checkedProviders;
    foreach( ProviderPtr provider, m_providers )
    {
        if( m_config->providerEnabled( provider->id(), false ) )
            checkedProviders.insert( provider );
    }

    if( mode == StatSyncing::Process::NonInteractive &&
        ( checkedProviders.count() <= 1 || fields == 0 ) )
    {
        return;
    }

    m_currentProcess = new Process( m_providers, checkedProviders, fields, mode, this );
    m_currentProcess.data()->start();
}

void
Controller::slotTrackFinishedPlaying( const Meta::TrackPtr &track, double playedFraction )
{
    if( !AmarokConfig::submitPlayedSongs() )
        return;
    Q_ASSERT( track );
    scrobble( track, playedFraction );
}

void
Controller::scrobble( const Meta::TrackPtr &track, double playedFraction, const QDateTime &time )
{
    foreach( ScrobblingServicePtr service, m_scrobblingServices )
    {
        service->scrobble( track, playedFraction, time );
    }
}

void
Controller::slotResetLastSubmittedNowPlayingTrack()
{
    m_lastSubmittedNowPlayingTrack = Meta::TrackPtr();
}

void
Controller::slotUpdateNowPlayingWithCurrentTrack()
{
    EngineController *engine = Amarok::Components::engineController();
    if( !engine )
        return;

    Meta::TrackPtr track = engine->currentTrack(); // null track is okay
    if( tracksVirtuallyEqual( track, m_lastSubmittedNowPlayingTrack ) )
    {
        debug() << __PRETTY_FUNCTION__ << "this track already recently submitted, ignoring";
        return;
    }
    foreach( ScrobblingServicePtr service, m_scrobblingServices )
    {
        service->updateNowPlaying( track );
    }

    m_lastSubmittedNowPlayingTrack = track;
}

bool
Controller::tracksVirtuallyEqual( const Meta::TrackPtr &first, const Meta::TrackPtr &second )
{
    if( !first && !second )
        return true; // both null
    if( !first || !second )
        return false; // exactly one is null
    const QString firstAlbum = first->album() ? first->album()->name() : QString();
    const QString secondAlbum = second->album() ? second->album()->name() : QString();
    const QString firstArtist = first->artist() ? first->artist()->name() : QString();
    const QString secondArtist = second->artist() ? second->artist()->name() : QString();
    return first->name() == second->name() &&
           firstAlbum == secondAlbum &&
           firstArtist == secondArtist;
}
