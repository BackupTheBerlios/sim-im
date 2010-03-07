/***************************************************************************
                          aboutdata.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ABOUTDATA_H
#define _ABOUTDATA_H

#ifdef USE_KDE
#include <kaboutdata.h>
#else

/*
* This file is part of the KDE Libraries
* Copyright (C) 2000 Espen Sand (espen@kde.org)
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
*/

#include "simapi.h"

#include <QList>
#include <QString>

/**
* This structure is used to store information about a person or developer.
* It can store the person's name, a task, an email address and a
* link to a home page. This class is intended for use in the
* @ref KAboutData class, but it can be used elsewhere as well.
* Normally you should at least define the person's name.
*/

class EXPORT KAboutPerson
{
public:
    KAboutPerson( const char *name, const char *task,
                  const char *emailAddress, const char *webAddress )
    {
        mName = name;
        mTask = task;
        mEmailAddress = emailAddress;
        mWebAddress = webAddress;
    }
    KAboutPerson() {}

    QString name() const;

    QString task() const;

    QString emailAddress() const;

    QString webAddress() const;

private:
    const char *mName;
    const char *mTask;
    const char *mEmailAddress;
    const char *mWebAddress;
};


/**
* This structure is used to store information about a translator.
* It can store the translator's name and an email address.
* This class is intended for use in the @ref KAboutData class,
* but it can be used elsewhere as well.
* Normally you should at least define the translator's name.
*
* It's not possible to use @ref KAboutPerson for this, because
* @ref KAboutPerson stores internally only const char* pointers, but the
* translator information is generated dynamically from the translation
* of a dummy string.
*/

class EXPORT KAboutTranslator
{
public:
    KAboutTranslator(const QString & name=QString::null,
                     const QString & emailAddress=QString::null);

    QString name() const;

    QString emailAddress() const;

private:
    QString mName;
    QString mEmail;
};



/**
* This class is used to store information about a program. It can store
* such values as version number, program name, home page, email address
* for bug reporting, multiple authors and contributors
* (using @ref KAboutPerson), license and copyright information.
*
* Currently, the values set here are shown by the "About" box
* (see @ref KAboutDialog), used by the bug report dialog (see @ref KBugReport),
* and by the help shown on command line (see @ref KCmdLineArgs).
*
* @short Holds information needed by the "About" box and other
* classes.
* @author Espen Sand (espen@kde.org), David Faure (faure@kde.org)
* @version $Id: aboutdata.h,v 1.2 2004/06/26 08:16:59 shutoff Exp $
*/

class EXPORT KAboutData
{
public:
    enum LicenseKey
    {
        License_Custom = -2,
        License_File = -1,
        License_Unknown = 0,
        License_GPL  = 1,
        License_GPL_V2 = 1,
        License_LGPL = 2,
        License_LGPL_V2 = 2,
        License_BSD  = 3,
        License_Artistic = 4,
        License_QPL = 5,
        License_QPL_V1_0 = 5
    };

public:
    KAboutData( const char *appName,
                const char *programName,
                const char *version,
                const char *shortDescription = 0,
                int licenseType = License_Unknown,
                const char *copyrightStatement = 0,
                const char *text = 0,
                const char *homePageAddress = 0,
                const char *bugsEmailAddress = "sim-im-main@lists.sim-im.org"
              );

    ~KAboutData();

    void addAuthor( const char *name,
                    const char *task=0,
                    const char *emailAddress=0,
                    const char *webAddress=0 );

    void addCredit( const char *name,
                    const char *task=0,
                    const char *emailAddress=0,
                    const char *webAddress=0 );

    void setTranslator(const char* name, const char* emailAddress);

    void setLicenseText( const char *license );

    void setLicenseTextFile( const QString &file );

    const char *appName() const;

    QString programName() const;

    QString version() const;

    QString shortDescription() const;

    QString homepage() const;

    QString bugAddress() const;

    const QList<KAboutPerson> authors() const;

    const QList<KAboutPerson> credits() const;

    const QList<KAboutTranslator> translators() const;

    static QString aboutTranslationTeam();


    QString otherText() const;

    QString license() const;

    QString copyrightStatement() const { return( QString::fromLatin1(mCopyrightStatement )); }


private:
    const char *mAppName;
    const char *mProgramName;
    const char *mVersion;
    const char *mShortDescription;
    int mLicenseKey;
    const char *mCopyrightStatement;
    const char *mOtherText;
    const char *mHomepageAddress;
    const char *mBugEmailAddress;
    const char *mLicenseText;

    class KAboutDataPrivate *d;

    COPY_RESTRICTED(KAboutData)
};


#endif

#endif

