/***************************************************************************
                          cfg.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "cfg.h"

#include <stdio.h>
#include <errno.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <QFile>
#include <QToolBar>
#include <QMainWindow>
#include <QStringList>
#include <QApplication>
#include <QDir>
#include <QStyle>
#ifdef _DEBUG
# include <QMessageBox>
#endif
#include <QByteArray>
#include <QDesktopWidget>

#ifdef USE_KDE
#include <kglobal.h>
#include <kstddirs.h>
#include <kwin.h>
#include "kdeisversion.h"
#endif

#include "log.h"
#include "misc.h"

namespace SIM
{

Config::Config(const QString& filename) : m_filename(filename),
	m_roothub(PropertyHub::create()), m_changed(false)
{
}

Config::~Config()
{
}

bool Config::addPropertyHub(PropertyHubPtr hub)
{
	if(hub->getNamespace().isEmpty())
		return false;
	m_hubs.insert(hub->getNamespace(), hub);
	return true;
}

PropertyHubPtr Config::propertyHub(const QString& hubNamespace)
{
	PropertyHubMap::iterator it = m_hubs.find(hubNamespace);
	if(it == m_hubs.end())
		return PropertyHubPtr();
	return it.value();
}

void Config::clearPropertyHubs()
{
	m_hubs.clear();
}

PropertyHubPtr Config::rootPropertyHub()
{
	return m_roothub;
}

QByteArray Config::serialize()
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"utf-8\"" ) );
    QDomElement root = doc.createElement( "config" );
    doc.appendChild(root);

    QDomElement hubelement = doc.createElement( "propertyhub" );
    if( m_roothub->serialize( hubelement ) )
        root.appendChild( hubelement );

    foreach(PropertyHubPtr hub, m_hubs)
    {
        QDomElement hubelement = doc.createElement( "propertyhub" );
        hubelement.setAttribute( "name", hub->getNamespace() );
        if( hub->serialize( hubelement ) )
            root.appendChild( hubelement );
    }
    return doc.toByteArray();
}

bool Config::deserialize(const QByteArray& arr)
{
    QDomDocument doc;
    if( !doc.setContent( arr ) )
        return false;

    QDomElement root = doc.elementsByTagName( "config" ).at(0).toElement();
    if(root.isNull())
        return false;

    QDomNodeList list = root.elementsByTagName( "propertyhub" );
    for( int i = 0 ; i < list.size() ; i++ ) {
        QDomElement propertyhub = list.at(i).toElement();
        if( !propertyhub.isNull() ) {
            QString name = propertyhub.attribute( "name" );
            PropertyHubPtr hub;
            if( name.isEmpty() )
                hub = m_roothub;
            else
                hub = PropertyHub::create( name );
            if( !hub->deserialize( propertyhub ) )
                return false;
            addPropertyHub( hub );
        }
    }
    return true;
}

bool Config::mergeOldConfig(const QString& filename)
{
    QFile f(filename);
    if( !f.open(QIODevice::ReadOnly) )
        return false;
    log(L_DEBUG, "Merging old config: %s", qPrintable(filename));
    QString config = QString(f.readAll());
    QRegExp re("\\[([^\\]]+)\\]\n([^\\[]+)");
    int pos = 0;
    while((pos = re.indexIn(config, pos)) != -1)
    {
        pos += re.matchedLength();
        QString ns = re.cap(1);
        QStringList lines = re.cap(2).split('\n');
        for(QStringList::iterator it = lines.begin(); it != lines.end(); ++it)
        {
            QStringList line = it->split('=');
            if(line.size() != 2)
                continue;
            PropertyHubPtr hub = propertyHub(ns);
            if(hub.isNull())
            {
                    hub = PropertyHub::create(ns);
                    addPropertyHub(hub);
            }

            // Merge if only there's no setting in a new config:
            if(!hub->value(line[0]).isValid())
            {
                if(line[1].startsWith('"') && line[1].endsWith('"'))
                    hub->setValue(line[0], line[1].mid(1, line[1].length() - 2));
                else
                    hub->setValue(line[0], line[1]);
            }
        }
    }
    f.close();
    return true;
}

// ______________________________________________________________________________________

#ifdef WIN32

EXPORT bool makedir(const QString &p)
{
    QDir path;
    // TODO: still needed?
    if(p.endsWith('/') || p.endsWith('\\')) {
        QFileInfo fi(p + "dummy.txt");
        path = fi.absoluteDir();
    } else {
        QFileInfo fi(p);
        path = fi.absoluteDir();
    }

    if(path.exists())
        return true;
    QString r = QDir::convertSeparators(path.absolutePath());

    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    if(QSysInfo::windowsVersion()&QSysInfo::WV_NT_based){
        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
        sa.lpSecurityDescriptor = &sd;
    }
    CreateDirectoryW((LPCWSTR)p.utf16(), &sa);
    DWORD dwAttr = GetFileAttributesW((LPCWSTR)p.utf16());
    if (dwAttr & FILE_ATTRIBUTE_READONLY)
        SetFileAttributesW((LPCWSTR)p.utf16(), dwAttr & ~FILE_ATTRIBUTE_READONLY);
    return true;
}

#else

EXPORT bool makedir(const QString &p)
{
    QDir path;
    if(p.endsWith("/") || p.endsWith("\\")) {
        QFileInfo fi(p + "dummy.txt");
        path = fi.dir();
    } else {
        QFileInfo fi(p);
        path = fi.dir();
    }

    if(path.exists())
        return true;
    QString r = QDir::convertSeparators(path.absolutePath());

    struct stat st;
    if (stat(QFile::encodeName(r).data(), &st) != 0){
#ifdef __OS2__
        int idx = r.lastIndexOf('\\');
#else
        int idx = r.lastIndexOf('/');
#endif
        if(idx == -1)
            return false;
        if (makedir(r.left(idx))){
            if (mkdir(QFile::encodeName(r).data(), 0700)){
                log(L_ERROR, "Can't create %s: %s", QFile::encodeName(r).data(), strerror(errno));
                return false;
            }
        }
        return false;
    }
    if ((st.st_mode & S_IFMT) != S_IFDIR){
        log(L_ERROR, "%s no directory", qPrintable(p));
        return false;
    }
    return true;
}

#endif

// ______________________________________________________________________________________

// ______________________________________________________________________________________

char fromHex(char c)
{
    if ((c >= '0') && (c <= '9')) return (char)(c - '0');
    if ((c >= 'A') && (c <= 'F')) return (char)(c + 10 - 'A');
    if ((c >= 'a') && (c <= 'f')) return (char)(c + 10 - 'a');
    return (char)0;
}

static unsigned char toHex(unsigned char c)
{
    c &= 0x0F;
    if (c < 10)
        return (unsigned char)(c + '0');
    return (unsigned char)(c - 10 + 'a');
}


QString quoteChars(const QString &from, const char *chars, bool bQuoteSlash)
{
    QString     res;
    QString     quote_chars;

    quote_chars = chars;
    if (bQuoteSlash) {
        quote_chars += '\\';
    }
    for (int i = 0; i < from.length (); i++) {
        const QChar c = from[i];
        if (quote_chars.contains (c)) {
            res += '\\';
        }
        res += c;
    }
    return res;
}

QString unquoteChars(const QString &from, const QString chars, bool bQuoteSlash)
{
    QString     res;
    QString     quote_chars;

    quote_chars = chars;
    if (bQuoteSlash) {
        quote_chars += '\\';
    }
    for (int i = 0; i < from.length(); i++) {
        if ( (from[i] == '\\') && (i+1 < from.length()) ) {
          if (quote_chars.contains (from[i+1])) {
                i++; // If the char after the slash is part of quote_chars, then we will skip that slash
          } else
          {
            if (bQuoteSlash) {
                // There should not be slashes with characters other than quote_chars after it, when bQuoteSlash is true
                // So will warn about it
              log(L_WARN,"Single slash found while unquoting chars '%s' in string '%s'",
                  qPrintable(chars), qPrintable(from));
            }
          }
        }
        if ( bQuoteSlash && (from[i] == '\\') && (i+1 == from.length()) ) {
          // There should not be slashes at the end of the string if bQuoteSlash is true
          log(L_WARN,"Single slash found at the end of string while unquoting chars '%s' in string '%s'",
              qPrintable(chars), qPrintable(from));
        }
        res += from[i];
    }
    return res;
}
EXPORT QString getToken(QString &from, char c, bool bUnEscape)
{
    QString res;
    int i;
    for (i = 0; i < from.length(); i++){
        if (from[i] == c)
            break;
        if (from[i] == '\\'){
            i++;
            if (i >= from.length())
                break;
            if (!bUnEscape)
                res += '\\';
        }
        res += from[i];
    }
    if (i < from.length()){
        from = from.mid(i + 1);
    }else{
        from.clear();
    }
    return res;
}

EXPORT QByteArray getToken(QByteArray &from, char c, bool bUnEscape)
{
    QByteArray res;
    int i;
    for (i = 0; i < from.length(); i++){
        if (from[i] == c)
            break;
        if (from[i] == '\\'){
            i++;
            if (i >= from.length())
                break;
            if (!bUnEscape)
                res += '\\';
        }
        res += from[i];
    }
    if (i < from.length()){
        from = from.mid(i + 1);
    }else{
        from.clear();
    }
    return res;  
}


static QByteArray quoteInternal(const QByteArray &str)
{
    QByteArray res("\"");
    if (!str.isEmpty()){
        for (int i = 0; i < str.length(); i++){
            unsigned char p = str[i];
            switch (p){
            case '\\':
                res += "\\\\";
                break;
            case '\r':
                break;
            case '\n':
                res += "\\n";
                break;
            case '\"': {
                res += "\\x";
                res += toHex((unsigned char)(p >> 4));
                res += toHex(p);
                break;
            }
            default:
                if (p >= ' '){
                    res += p;
                }else if (p){
                    res += "\\x";
                    res += toHex((unsigned char)(p >> 4));
                    res += toHex(p);
                }
            }
        }
    }
    res += '\"';
    return res;
}

static bool unquoteInternal(QByteArray &val, QByteArray &str)
{
    int idx1 = val.indexOf('\"');
    if(idx1 == -1)
        return false;
    idx1++;
    int idx2 = val.lastIndexOf('\"');
    if(idx2 == -1)
        return false;
    str = val.mid(idx1, idx2 - idx1);
    val = val.mid(idx2 + 1);
    // now unquote
    idx1 = 0;
    while((idx1 = str.indexOf('\\', idx1)) != -1) {
        char c = str[idx1 + 1];
        switch(c) {
            case '\\':
                str = str.left(idx1) + '\\' + str.mid(idx1 + 2);
                break;
            case 'n':
                str = str.left(idx1) + '\n' + str.mid(idx1 + 2);
                break;
            case 'x': {
                char c1 = str[idx1 + 2];
                char c2 = c1 ? str[idx1 + 3] : 0;
                if(!c1 || !c2)
                    return false;
                c = (fromHex(c1) << 4) | (fromHex(c2));
                str = str.left(idx1) + c + str.mid(idx1 + 4);
                break;
            }
            default:
                break;
        }
        idx1++;
    }
    return true;
}

}   // namespace SIM
