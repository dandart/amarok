/***************************************************************************
 *   Plasma applet for showing video in the context view.                  *
 *                                                                         *
 *   Copyright (c) 2008 Mark Kretschmann <kretschmann@kde.org              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "Video.h"

#include "Amarok.h"
#include "Debug.h"
#include "EngineController.h"
#include "context/ContextView.h"
#include "context/Svg.h"

#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QPainter>


Video::Video( QObject* parent, const QVariantList& args )
    : Context::Applet( parent, args )
{
    DEBUG_BLOCK

    setHasConfigurationInterface( false );

    Phonon::MediaObject* mediaObject = const_cast<Phonon::MediaObject*>( The::engineController()->phononMediaObject() );
     
    m_videoProxy = new QGraphicsProxyWidget( this );
    m_videoWidget = new Phonon::VideoWidget();
    setPreferredSize( 400, 500 ); // take up all the current containment space
    
    m_videoProxy->setWidget( m_videoWidget );
    m_videoWidget->show();

    debug() << "Creating video path.";
    Phonon::Path path = Phonon::createPath( mediaObject, m_videoWidget );

    if( !path.isValid() )
        warning() << "Phonon path is invalid.";

    constraintsEvent();
}

Video::~Video()
{
    DEBUG_BLOCK

    delete m_videoWidget;
} 

void
EngineNewTrackPlaying()
{
    DEBUG_BLOCK
}

void
Video::constraintsEvent( Plasma::Constraints constraints )
{
    prepareGeometryChange();
    m_videoProxy->setMinimumSize( size() );
    m_videoProxy->setMaximumSize( size() );
}

void
Video::paintInterface( QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect )
{
    Q_UNUSED( option );
}

QSizeF
Video::effectiveSizeHint( Qt::SizeHint which, const QSizeF & constraint) const
{
    DEBUG_BLOCK
    Q_UNUSED( which )

    return constraint;
}

#include "Video.moc"
