/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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
 
#include "Mp3tunesConfig.h"

#include <kdebug.h>
#include <KConfig>
#include <KConfigGroup>
#include <KGlobal>

#include <QNetworkInterface>

Mp3tunesConfig::Mp3tunesConfig()
{
    m_hasChanged = false;
    load();
}


Mp3tunesConfig::~Mp3tunesConfig()
{
}

void Mp3tunesConfig::load()
{
    kDebug( 14310 ) << "load";
    KConfigGroup config = KGlobal::config()->group( "Service_Mp3tunes" );
    m_email = config.readEntry( "email", QString() );
    m_password = config.readEntry( "password", QString() );
    m_hardwareAddress = config.readEntry( "hardwareAddress", QString() );
    m_harmonyEnabled = config.readEntry( "harmonyEnabled", false );

    if( m_hardwareAddress == QString() )
    {
        foreach( QNetworkInterface iface, QNetworkInterface::allInterfaces() )
        {
            QString addr = iface.hardwareAddress();
            if( addr != "00:00:00:00:00:00" ) {
                kDebug( 14310 ) << "Using iface \"" << iface.name() << " addr: " << iface.hardwareAddress();
                setHardwareAddress( addr );
                save();
                break;
            }
        }
    }
}

void Mp3tunesConfig::save()
{
    kDebug( 14310 ) << "save";
    if ( m_hasChanged ) {
        KConfigGroup config = KGlobal::config()->group( "Service_Mp3tunes" );
        config.writeEntry( "email", m_email );
        config.writeEntry( "password", m_password );
        config.writeEntry( "hardwareAddress", m_hardwareAddress );
        config.writeEntry( "harmonyEnabled", m_harmonyEnabled );
    }
}

QString Mp3tunesConfig::email()
{
    return m_email;
}

QString Mp3tunesConfig::password()
{
    return m_password;
}

QString Mp3tunesConfig::hardwareAddress()
{
    return m_hardwareAddress;
}

bool Mp3tunesConfig::harmonyEnabled()
{
   return m_harmonyEnabled;
}

void Mp3tunesConfig::setHarmonyEnabled( bool enabled )
{    
    kDebug( 14310 ) << "set harmony";
    if ( enabled != m_harmonyEnabled ) {
        m_harmonyEnabled = enabled;
        m_hasChanged = true;
    }
}

void Mp3tunesConfig::setHardwareAddress( const QString &address )
{
    kDebug( 14310 ) << "set hwaddress";
    if ( address != m_hardwareAddress ) {
        m_hardwareAddress = address;
        m_hasChanged = true;
    }
}

void Mp3tunesConfig::setEmail( const QString &email )
{
    kDebug( 14310 ) << "set email";
    if ( email != m_email ) {
        m_email = email;
        m_hasChanged = true;
    }
}

void Mp3tunesConfig::setPassword( const QString &password )
{
    kDebug( 14310 ) << "set Password";
    if( password != m_password ) {
        m_password = password;
        m_hasChanged = true;
    }
}


