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

#include "TranscodingAacFormat.h"

#include <KLocale>

#include <QVariant>

namespace Transcoding
{

AacFormat::AacFormat()
{
    m_encoder = AAC;
    m_fileExtension = "m4a";
    QString description1 =
        i18n( "The quality rating is an integer value between 0 and 255 that represents the "
              "tradeoff between file size and sound quality. While it does not directly "
              "translate into a bitrate, a higher quality rating generally raises the "
              "<a href=http://www.ffmpeg.org/faq.html#SEC21>average bitrate</a>.<br/>"
              "150 is a good choice for music listening on a portable player.<br/>"
              "Anything below 120 might be unsatisfactory for music and anything above"
              "220 is probably overkill." );
    m_propertyList << Property::Numeric( "quality", i18n( "Quality" ), description1, 0, 255, 150 );
}

QString
AacFormat::prettyName() const
{
    return i18n( "AAC (Non-Free)" );
}

QString
AacFormat::description() const
{
    return i18nc( "Feel free to redirect the english Wikipedia link to a local version, if "
                  "it exists.",
                  "<a href=http://en.wikipedia.org/wiki/Advanced_Audio_Coding>Advanced Audio "
                  "Coding</a> (AAC) is a patented lossy codec for digital audio.<br>AAC "
                  "generally achieves better sound quality than MP3 at similar bit rates. "
                  "It is a reasonable choice for the iPod and some other portable music "
                  "players. Non-Free implementation." );

}

KIcon
AacFormat::icon() const
{
    return KIcon( "audio-ac3" ); //TODO: get a *real* icon!
}

QStringList
AacFormat::ffmpegParameters( const Configuration &configuration ) const
{
    QStringList parameters;
    parameters << "-acodec" << "libfaac"; /* libfaac seems to be the only decent AAC encoder
                                             for GNU/Linux and it's a proprietary freeware
                                             with LGPL portions. Hopefully in the future
                                             FFmpeg's native aac implementation should get
                                             better so libfaac won't be necessary any more.
                                                            -- Teo 5/aug/2010 */
    foreach( Property property, m_propertyList )
    {
        if( !configuration.property( property.name() ).isNull()
            && configuration.property( property.name() ).type() == property.variantType() )
        {
            if( property.name() == "quality" )
            {
                parameters << "-aq"
                           << QString::number( configuration.property( "quality" ).toInt() );
            }
        }
    }
    return parameters;
}

bool
AacFormat::verifyAvailability( const QString &ffmpegOutput ) const
{
    return ffmpegOutput.contains( QRegExp( ".EA... libfaac" ) );
}

}