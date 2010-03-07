/***************************************************************************
                          aboutdata.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "aboutdata.h"
#include "misc.h"
#include "log.h"

using namespace SIM;

#ifndef USE_KDE4

#include <qglobal.h>
#include <QFile>
#include <QStringList>
#include <QList>

QString
KAboutPerson::name() const
{
    return QString::fromUtf8(mName);
}

QString
KAboutPerson::task() const
{
    if (mTask)
        return i18n(mTask);
    else
        return QString::null;
}

QString
KAboutPerson::emailAddress() const
{
    return QString::fromUtf8(mEmailAddress);
}


QString
KAboutPerson::webAddress() const
{
    return QString::fromUtf8(mWebAddress);
}


KAboutTranslator::KAboutTranslator(const QString & name,
                                   const QString & emailAddress)
{
    mName=name;
    mEmail=emailAddress;
}

QString KAboutTranslator::name() const
{
    return mName;
}

QString KAboutTranslator::emailAddress() const
{
    return mEmail;
}

class KAboutDataPrivate
{
public:
    KAboutDataPrivate()
: translatorName("_: NAME OF TRANSLATORS\nYour names")
    , translatorEmail("_: EMAIL OF TRANSLATORS\nYour emails")
    {};
    const char *translatorName;
    const char *translatorEmail;
    QList<KAboutPerson> mAuthorList;
    QList<KAboutPerson> mCreditList;
};



KAboutData::KAboutData( const char *appName,
                        const char *programName,
                        const char *version,
                        const char *shortDescription,
                        int licenseType,
                        const char *copyrightStatement,
                        const char *text,
                        const char *homePageAddress,
                        const char *bugsEmailAddress
                      ) :
        mProgramName( programName ),
        mVersion( version ),
        mShortDescription( shortDescription ),
        mLicenseKey( licenseType ),
        mCopyrightStatement( copyrightStatement ),
        mOtherText( text ),
        mHomepageAddress( homePageAddress ),
        mBugEmailAddress( bugsEmailAddress )
{
    d = new KAboutDataPrivate;

    if( appName ) {
        const char *p = strrchr(appName, '/');
        if( p )
            mAppName = p+1;
        else
            mAppName = appName;
    } else
        mAppName = 0;
}

KAboutData::~KAboutData()
{
    //delete d;
}

void
KAboutData::addAuthor( const char *name, const char *task,
                       const char *emailAddress, const char *webAddress )
{
    d->mAuthorList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

void
KAboutData::addCredit( const char *name, const char *task,
                       const char *emailAddress, const char *webAddress )
{
    d->mCreditList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

void
KAboutData::setTranslator( const char *name, const char *emailAddress)
{
    d->translatorName=name;
    d->translatorEmail=emailAddress;
}

void
KAboutData::setLicenseText( const char *licenseText )
{
    mLicenseText = licenseText;
    mLicenseKey = License_Custom;
}

void
KAboutData::setLicenseTextFile( const QString &file )
{
    mLicenseText = qstrdup(QFile::encodeName(file));
    mLicenseKey = License_File;
}


const char *
KAboutData::appName() const
{
    return mAppName;
}

QString
KAboutData::programName() const
{
    if (mProgramName && *mProgramName)
        return i18n(mProgramName);
    else
        return QString::null;
}

QString
KAboutData::version() const
{
    if (this)
        return QString::fromLatin1(mVersion);
    else
        log(L_ERROR, "A Plugin requires About-Plugin to be loaded, but it isn't.");
    return i18n("Error: About-Plugin is not loaded.");
}

QString
KAboutData::shortDescription() const
{
    if (mShortDescription)
        return i18n(mShortDescription);
    else
        return QString::null;
}

QString
KAboutData::homepage() const
{
    return QString::fromLatin1(mHomepageAddress);
}

QString
KAboutData::bugAddress() const
{
    return QString::fromLatin1(mBugEmailAddress);
}

const QList<KAboutPerson>
KAboutData::authors() const
{
    return d->mAuthorList;
}

const QList<KAboutPerson>
KAboutData::credits() const
{
    return d->mCreditList;
}

const QList<KAboutTranslator>
KAboutData::translators() const
{
    QList<KAboutTranslator> personList;

    if(d->translatorName == 0)
        return personList;

    QStringList nameList;
    QStringList emailList;

    QString names = i18n(d->translatorName);
    if(names != QString::fromUtf8(d->translatorName))
    {
        nameList = names.split(',');
    }


    if(d->translatorEmail)
    {
        QString emails = i18n(d->translatorEmail);

        if(emails != QString::fromUtf8(d->translatorEmail))
        {
            emailList = emails.split(',');
        }
    }


    QStringList::Iterator nit;
    QStringList::Iterator eit=emailList.begin();

    for(nit = nameList.begin(); nit != nameList.end(); ++nit)
    {
        QString email;
        if(eit != emailList.end())
        {
            email=*eit;
            ++eit;
        }

        QString name=*nit;

        personList.append(KAboutTranslator( name, email));
    }

    return personList;
}

QString
KAboutData::aboutTranslationTeam()
{
    return i18n("replace this with information about your translation team");
}

QString
KAboutData::otherText() const
{
    if (mOtherText)
        return i18n(mOtherText);
    else
        return QString::null;
}


QString
KAboutData::license() const
{
    return i18n("This program is distributed under the terms of the GPL v2.");
}

#endif

static KAboutData *appAbout = NULL;

void SIM::setAboutData(KAboutData *data)
{
    appAbout = data;
}

KAboutData *SIM::getAboutData()
{
    return appAbout;
}





