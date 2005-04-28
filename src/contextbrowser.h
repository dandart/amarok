// (c) 2004 Christian Muehlhaeuser <chris@chris.de>
// See COPYING file for licensing information

#ifndef AMAROK_CONTEXTBROWSER_H
#define AMAROK_CONTEXTBROWSER_H

#include "engineobserver.h"
#include <kurl.h>
#include <qtabwidget.h>

class CollectionDB;
class Color;
class KHTMLPart;
class KTabBar;
class KTempFile;
class MetaBundle;
class QPalette;

namespace KIO { class Job; class TransferJob; }


class ContextBrowser : public QTabWidget, public EngineObserver
{
    Q_OBJECT

        friend class CurrentTrackJob;

        static ContextBrowser *s_instance;

    public:
        ContextBrowser( const char *name );
       ~ContextBrowser();

        static ContextBrowser *instance() { return s_instance; }

        void setFont( const QFont& );
        void setStyleSheet();

    public slots:
        void openURLRequest(const KURL &url );
        void collectionScanStarted();
        void collectionScanDone();
        void renderView();

    protected:
        void engineNewMetaData( const MetaBundle&, bool );
        void engineStateChanged( Engine::State );
        void paletteChange( const QPalette& );

    private slots:
        void tabChanged( QWidget *page );
        void slotContextMenu( const QString& urlString, const QPoint& point );
        void showHome();
        void showCurrentTrack();
        void showLyrics( const QString& hash = QString::null );
        void showLyricSuggestions();
        void showWikipedia( const QString& url = QString::null );

        void lyricsData( KIO::Job* job, const QByteArray& data );
        void wikiData( KIO::Job* job, const QByteArray& data );
        void lyricsResult( KIO::Job* job );
        void wikiResult( KIO::Job* job );
        void coverFetched( const QString &artist, const QString &album );
        void coverRemoved( const QString &artist, const QString &album );
        void similarArtistsFetched( const QString &artist );

    private:
        void setStyleSheet_Default( QString& styleSheet );
        void setStyleSheet_ExternalStyle( QString& styleSheet, QString& themeName );
        void showIntroduction();
        void saveHtmlData();
        void showScanning();
        void showHomeBySongs();
        void showHomeByAlbums();

        KHTMLPart    *m_homePage;
        KHTMLPart    *m_currentTrackPage;
        KHTMLPart    *m_lyricsPage;
        KHTMLPart    *m_wikiPage;
        // These control if is needed to rewrite the html for the pages
        // true -> need rebuild
        bool          m_dirtyHomePage;
        bool          m_dirtyCurrentTrackPage;
        bool          m_dirtyLyricsPage;
        bool          m_dirtyWikiPage;
        bool          m_dirtyWikiFetching;

        QString       m_styleSheet;
        bool          m_emptyDB;
        QString       m_lyrics;
        QStringList   m_lyricSuggestions;
        QStringList   m_lyricHashes;
        QString       m_lyricAddUrl;
        QString       m_lyricSearchUrl;
        KIO::TransferJob* m_lyricJob;
        QString       m_wiki;
        QString       m_wikiCurrentUrl;
        KIO::TransferJob* m_wikiJob;
        QString       m_HTMLSource;
        KTempFile    *m_bgGradientImage;
        KTempFile    *m_headerGradientImage;
        KTempFile    *m_shadowGradientImage;
        QStringList   m_metadataHistory;
        KURL          m_currentURL;

        bool          m_suggestionsOpen;
        bool          m_favouritesOpen;
};

#endif /* AMAROK_CONTEXTBROWSER_H */
