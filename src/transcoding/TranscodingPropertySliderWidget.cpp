/****************************************************************************************
 * Copyright (c) 2010 Téo Mrnjavac <teo@kde.org>                                        *
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

#include "TranscodingPropertySliderWidget.h"

#include "core/support/Debug.h"

#include <QHBoxLayout>

namespace Transcoding
{

PropertySliderWidget::PropertySliderWidget( Property property, QWidget * parent )
    : QWidget( parent )
    , m_property( property )
{
    m_name = property.name();

    QBoxLayout *mainLayout;
    m_mainLabel = new QLabel( m_property.prettyName(), this );
    m_mainLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );

    mainLayout = new QVBoxLayout( this );
    QBoxLayout *secondaryTopLayout = new QHBoxLayout( this );
    QBoxLayout *secondaryBotLayout = new QHBoxLayout( this );
    mainLayout->addWidget( m_mainLabel );
    mainLayout->addLayout( secondaryTopLayout );
    mainLayout->addLayout( secondaryBotLayout );
    secondaryTopLayout->addSpacing( 5 );

    m_mainEdit = new QSlider( this );
    m_mainEdit->setOrientation( Qt::Horizontal );
    m_mainEdit->setRange( m_property.min(), m_property.max() );

    m_mainEdit->setValue( m_property.defaultValue() );
    m_mainEdit->setTickPosition( QSlider::TicksBelow );
    m_mainEdit->setTickInterval( 1 );
    m_mainEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    secondaryTopLayout->addWidget( m_mainEdit, 3 );

    secondaryTopLayout->addSpacing( 5 );

    QLabel *leftLabel = new QLabel( m_property.prettyValues().at( 0 ), this );
    secondaryBotLayout->addWidget( leftLabel, 1 );

    m_midLabel = new QLabel( QString::number( m_mainEdit->value() ), this );
    {
        QFont font = m_midLabel->font();
        font.setBold( true );
        m_midLabel->setFont( font );
    }
    connect( m_mainEdit, SIGNAL( valueChanged( int ) ),
             this, SLOT( onSliderChanged( int ) ) );
    secondaryBotLayout->addWidget( m_midLabel );

    QLabel *rightLabel = new QLabel( m_property.prettyValues().at( 1 ), this );
    rightLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    secondaryBotLayout->addWidget( rightLabel, 1 );

    onSliderChanged( m_property.defaultValue() );

    m_mainEdit->setToolTip( m_property.description() );
    m_mainLabel->setBuddy( m_mainEdit );
    m_midLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
}

void
PropertySliderWidget::onSliderChanged( int value ) //SLOT
{
    QString newText;
    if( !m_property.values().isEmpty() &&
        m_property.values().size() == qAbs( m_property.max() - m_property.min() ) + 1 )
        newText = m_property.values().at( value - qMin( m_property.min(), m_property.max() ) );
    else
        newText = QString::number( value );

    if( value == m_property.defaultValue() )
        newText += QString( "\n" ) + i18n( "(recommended)" );
    else
        newText += QString( "\n " );
    m_midLabel->setText( newText );
}

QVariant
PropertySliderWidget::value() const
{
    return m_mainEdit->value();
}

} //namespace Transcoding