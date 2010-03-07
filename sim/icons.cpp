/***************************************************************************
                          icons.cpp  -  description
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <QCoreApplication>
#include <QIcon>
#include <QImage>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMap>
#include <QList>
#include <QPixmap>
#include <QImageReader>
#include <QDomDocument>

#ifdef USE_KDE
# include <kapp.h>
# include <kipc.h>
# include <kiconloader.h>
#endif

#include "contacts.h"
#include "icons.h"
#include "log.h"
#include "qzip/qzip.h"
#include "unquot.h"
#include "contacts/client.h"

const unsigned ICON_COLOR_MASK  = 0x000000FF;

namespace SIM
{

typedef QMap<QString, PictDef> PIXMAP_MAP;
unsigned Icons::nSmile = 0;

struct smileDef
{
    QString smile;
    QString name;
};

/*********************
 * base IconSet class
**********************/
class IconSet
{
public:
    IconSet() {};
    virtual ~IconSet() {};
    virtual PictDef *getPict(const QString &name) = 0;
    virtual void clear() = 0;
    void parseSmiles(const QString&, unsigned &start, unsigned &size, QString &name);
    QStringList getSmile(const QString &name);
    QString getSmileName(const QString &name);
    void getSmiles(QStringList &smiles, QStringList &used);
protected:
    PIXMAP_MAP      m_icons;
    QList<smileDef>    m_smiles;
};

class WrkIconSet : public IconSet
{
public:
    WrkIconSet() {};
    PictDef *getPict(const QString &name);
    void clear();
    PictDef *add(const QString &name, const QImage &pict, unsigned flags);
protected:
};

class FileIconSet : public IconSet
{
public:
    FileIconSet(const QString &file);
    ~FileIconSet();
    PictDef *getPict(const QString &name);
    void clear();
protected:
	void parse(const QByteArray &data);
    QString     m_name;
    QString     m_file;
    QString     m_smile;
    QString     *m_data;
#ifdef USE_KDE
    QString     m_system;
#endif
    unsigned    m_flags;
    UnZip      *m_zip;
};

class IconsPrivate
{
public:
    QList<IconSet*>    defSets;
    QList<IconSet*>    customSets;
	WrkIconSet* m_workset;

    static Icons *instance;
    static void cleanupIcons()
    {
        delete instance;
        instance = 0;
    }
};
Icons *IconsPrivate::instance = NULL;

/*************************************
 * functions for easy external access
 *************************************/
PictDef *getPict(const QString &name)
{
    return getIcons()->getPict(name);
}

static QPixmap getPixmap(PictDef *d)
{
    return QPixmap::fromImage(d->image);
}

QIcon Icon(const QString &name)
{
    PictDef *pict = getPict(name);
    if (pict == NULL)
        return QIcon();
    QIcon res(getPixmap(pict));
    const QString bigName = "big." + name;
    pict = getPict(bigName);
    if (pict)
        res.addPixmap(getPixmap(pict));
    return res;
}

QImage Image(const QString &name)
{
    PictDef *p = getPict(name);
    if (p == NULL)
        return QImage();
    return p->image;
}

QPixmap Pict(const QString &name)
{
    PictDef *p = getPict(name);
    if (p == NULL)
        return QPixmap();
    return getPixmap(p);
}

/*********************
 * IconSet
**********************/
void IconSet::parseSmiles(const QString &text, unsigned &start, unsigned &size, QString &name)
{
    QList<smileDef>::iterator it;
    for (it = m_smiles.begin(); it != m_smiles.end(); ++it){
        const QString pat = it->smile;
        int n = text.indexOf(pat);
        if (n < 0)
            continue;
        if (((unsigned)n < start) || (((unsigned)n == start) && ((unsigned)pat.length() > size))){
            start = n;
            size  = pat.length();
            name  = it->name;
        }
    }
}

void IconSet::getSmiles(QStringList &smiles, QStringList &used)
{
    QString name;
    bool bOK = false;
    QList<smileDef>::iterator it;
    for (it = m_smiles.begin(); it != m_smiles.end(); ++it){
        if (name != it->name){
            if (bOK && !name.isEmpty())
                smiles.push_back(name);
            name = it->name;
            bOK = true;
        }
        if (!used.contains(it->smile)){
            used.append(it->smile);
        }else{
            bOK = false;
        }
    }
    if (bOK && !name.isEmpty())
        smiles.append(name);
}

QStringList IconSet::getSmile(const QString &name)
{
    QStringList res;
    PIXMAP_MAP::iterator it = m_icons.find(name);
    if (it == m_icons.end())
        return res;
    QList<smileDef>::iterator its;
    for (its = m_smiles.begin(); its != m_smiles.end(); ++its){
        if ((*its).name != name)
            continue;
        res.append((*its).smile);
    }
    return res;
}

QString IconSet::getSmileName(const QString &name)
{
    PIXMAP_MAP::iterator it = m_icons.find(name);
    if (it == m_icons.end())
        return QString();
    QString res = it.value().file;
    int n = res.indexOf('.');
    if (n > 0)
        res = res.left(n);
    return res;
}

/****************************************************
 * helper for WrkIconSet - move to class WrkIconSet?
 ****************************************************/
static QImage makeOffline(unsigned flags, const QImage &p)
{
    QImage image = p.copy();
    unsigned swapColor = flags & ICON_COLOR_MASK;
    unsigned int *data = (image.depth() > 8) ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable().data();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();

    for (int i = 0; i < pixels; i++){
        QColor c(qRed(data[i]), qGreen(data[i]), qBlue(data[i]));
        int a = qAlpha(data[i]);
        int h, s, v;
        c.getHsv(&h, &s, &v);
        if (swapColor){
            h = (swapColor * 2 - h) & 0xFF;
            c.setHsv(h, s, v);
        }else{
            c.setHsv(h, 0, v * 3 / 4);
        }
        data[i] = qRgba(c.red(), c.green(), c.blue(), a);
    }
    return image;
}

static QImage makeInactive(const QImage &p)
{
    QImage image = p.copy();
    unsigned int *data = (image.depth() > 8) ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable().data();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();

    for (int i = 0; i < pixels; i++){
        QColor c(qRed(data[i]), qGreen(data[i]), qBlue(data[i]));
        int a = qAlpha(data[i]);
        int h, s, v;
        c.getHsv(&h, &s, &v);
        c.setHsv(h, s / 8, v);
        data[i] = qRgba(c.red(), c.green(), c.blue(), a);
    }
    return image;
}

static QImage makeInvisible(unsigned flags, const QImage &p)
{
    QImage image = (p.depth() != 32) ? p.convertToFormat(QImage::Format_ARGB32) : p.copy();
    unsigned swapColor = flags & ICON_COLOR_MASK;
    char shift = (flags >> 8) & 0xFF;
    unsigned int *data = (unsigned int*)image.bits();

    for (int y = 0; y < image.width(); y++){
        int x = image.width() / 2 - (y - image.height() / 2) * 2 / 3 + shift;
        if (x < 0)
            x = 0;
        if (x > image.width())
            x = image.width();
        unsigned int *line = data + y * (image.width()) + x;
        for (; x < image.width(); x++, line++){
            QColor c(qRed(*line), qGreen(*line), qBlue(*line));
            int a = qAlpha(*line);
            int h, s, v;
            c.getHsv(&h, &s, &v);
            if (swapColor){
                h = (swapColor * 2 - h) & 0xFF;
                c.setHsv(h, s / 2, v * 3 / 4);
            }else{
                c.setHsv(h, s / 2, v * 3 / 4);
            }
            *line = qRgba(c.red(), c.green(), c.blue(), a);
        }
    }
    return image;
}

static QImage merge(const QImage &p1, const QImage &p2)
{
    QImage img1 = (p1.depth() != 32) ? p1.convertToFormat(QImage::Format_ARGB32) : p1.copy();
    QImage img2 = (p2.depth() != 32) ? p2.convertToFormat(QImage::Format_ARGB32) : p2.copy();

    unsigned int *data1 = (unsigned int*)img1.bits();
    unsigned int *data2 = (unsigned int*)img2.bits();
    for (int y1 = 0; y1 < img1.height(); y1++){
        int y2 = y1 - (img1.height() - img2.height()) / 2;
        if ((y2 < 0) || (y2 >= img2.height()))
            continue;
        unsigned int *line1 = data1 + y1 * img1.width();
        unsigned int *line2 = data2 + y2 * img2.width();
        int w = img1.width();
        if (w < img2.width()){
            line2 += (img2.width() - w) / 2;
        }else if (w > img2.width()){
            w = img2.width();
            line1 += (img1.width() - w) / 2;
        }
        for (int i = 0; i < w; i++, line1++, line2++){
            int r1 = qRed(*line1);
            int g1 = qGreen(*line1);
            int b1 = qBlue(*line1);
            int a1 = qAlpha(*line1);
            int r2 = qRed(*line2);
            int g2 = qGreen(*line2);
            int b2 = qBlue(*line2);
            int a2 = qAlpha(*line2);
            r1 = (r2 * a2 + r1 * (255 - a2)) / 255;
            g1 = (g2 * a2 + g1 * (255 - a2)) / 255;
            b1 = (b2 * a2 + b1 * (255 - a2)) / 255;
            if (a2 > a1)
                a1 = a2;
            *line1 = qRgba(r1, g1, b1, a1);
        }
    }
    return img1;
}

/**************
 * WrkIconSet
 **************/
PictDef *WrkIconSet::getPict(const QString &name)
{
    if (name.isEmpty())
        return NULL;
    PIXMAP_MAP::iterator it = m_icons.find(name);
    if (it != m_icons.end())
        return &it.value();

    if (name == "online"){
        PictDef *p = NULL;
        for (unsigned i = 0; i < getContacts()->nClients(); i++){
            Client *client = getContacts()->getClient(i);
            p = SIM::getPict(client->protocol()->description()->icon);
            if (p)
                break;
        }
        if (p == NULL){
            p = SIM::getPict("SIM");
        }
        if (p == NULL)
            return NULL;
        return add(name, p->image, p->flags);
    }
    if (name == "offline"){
        PictDef *p = SIM::getPict("online");
        if (p == NULL)
            return NULL;
        return add(name, makeOffline(p->flags, p->image), p->flags);
    }
    if (name == "inactive"){
        PictDef *p = SIM::getPict("online");
        if (p == NULL)
            return NULL;
        return add(name, makeInactive(p->image), p->flags);
    }
    if (name == "invisible"){
        PictDef *p = SIM::getPict("online");
        if (p == NULL)
            return NULL;
        return add(name, makeInvisible(p->flags, p->image), p->flags);
    }
    int pos = name.indexOf('_');
    if (pos > 0){
        PictDef *p = SIM::getPict(name.left(pos));
        QImage res;
        if (p){
            if(p->image.isNull()) {
                log(L_ERROR, "Null Image!");
                return false;
            }
            QString s = name.mid(pos + 1);
            if (s == "online"){
                res = p->image;
            }else if (s == "offline"){
                res = makeOffline(p->flags, p->image);
            }else if (s == "invisible"){
                res = makeInvisible(p->flags, p->image);
            }else if (s == "inactive"){
                res = makeInactive(p->image);
            }else{
                PictDef *pp = SIM::getPict(s);
                if (pp)
                    res = merge(p->image, pp->image);
            }
            return add(name, res, p->flags);
        }
    }
    if (!name.startsWith("big."))
        log(L_DEBUG, "Icon %s not found", qPrintable(name));
    return NULL;
}

void WrkIconSet::clear()
{
    m_icons.clear();
}

PictDef *WrkIconSet::add(const QString &name, const QImage &pict, unsigned flags)
{
    PictDef p;
    p.image = pict;
    p.flags = flags;
    m_icons.insert(name, p);
    return &m_icons.find(name).value();
}

/**************
 * FileIconSet
 **************/
FileIconSet::FileIconSet(const QString &file)
{
    m_zip = new UnZip(app_file(file));
    QByteArray arr;
    m_data = NULL;
    if (m_zip->open() && (m_zip->readFile("icondef.xml", &arr) || m_zip->readFile(QFileInfo(m_zip->name()).baseName() + "/icondef.xml", &arr)))
        parse(arr);
}

FileIconSet::~FileIconSet()
{
    delete m_zip;
}

PictDef *FileIconSet::getPict(const QString &name)
{
    if (name.isEmpty())
        return NULL;
    PIXMAP_MAP::iterator it = m_icons.find(name);
    if (it == m_icons.end())
        return NULL;
    if (it.value().image.isNull()){
#ifdef USE_KDE
        if (!it.data().system.isEmpty()){
            QPixmap pict;
             if (!name.startsWith("big.")){
                pict = SmallIconSet(it.data().system).pixmap(QIcon::Small, QIcon::Normal);
            }else{
                pict = DesktopIconSet(it.data().system).pixmap(QIcon::Large, QIcon::Normal);
            }
            if (!pict.isNull()){
                it.data().image = pict.convertToImage();
                return &(it.data());
            }
        }
#endif
        if (it.value().file.isEmpty())
            return NULL;
        QByteArray arr;
        if (!m_zip->readFile(it->file, &arr) && !m_zip->readFile(QFileInfo(m_zip->name()).baseName() + '/' + it->file, &arr))
            return NULL;
        it.value().image = QImage::fromData(arr).convertToFormat(QImage::Format_ARGB32);
    }
    return &(it.value());
}

void FileIconSet::clear()
{
    for (PIXMAP_MAP::iterator it = m_icons.begin(); it != m_icons.end(); ++it){
        it.value().image = QImage();
    }
}

void FileIconSet::parse(const QByteArray &data)
{
    QDomDocument doc;
    doc.setContent( data );
    QDomElement icondef = doc.firstChildElement( "icondef" );
    QDomElement icon = icondef.firstChildElement( "icon" );
    while( !icon.isNull() ) {
        PictDef pict;
        smileDef smile;
        QString sName = icon.attribute( "name", "s_" + QString::number(++Icons::nSmile) );
        pict.flags = icon.attribute( "flags" ).toUInt();
        QDomElement object = icon.firstChildElement( "object" );
        pict.file = object.text();
#ifdef USE_KDE
        pict.system = icon.attribute( "kicon" );
        if (!sName.startsWith("big.")){
            PictDef bigpict;
            QString big_name = "big." + sName;
            p.file.clear();
            bigpict.flags  = pict.flags;
            bigpict.system = pict.system;
            it = m_icons.indexOf(big_name);
            if (it == m_icons.end())
                m_icons.insert(PIXMAP_MAP::value_type(big_name, bigpict));
        }
#endif
        QDomElement text = icon.firstChildElement( "text" );
        m_icons.insert( sName, pict );
        while( !text.isNull() ) {
            smile.name = sName;
            smile.smile = text.text();
            m_smiles.push_back( smile );
            text = text.nextSiblingElement( "text" );
        }
        icon = icon.nextSiblingElement( "icon" );
    }
}

/*****************
 * Icons
 *****************/
Icons *Icons::instance()
{
    if(!IconsPrivate::instance) {
        IconsPrivate::instance = new Icons();
        qAddPostRoutine(IconsPrivate::cleanupIcons);
    }
    return IconsPrivate::instance;
}

Icons::Icons()
: d(new IconsPrivate())
{
    addIconSet("icons/sim.jisp", true);
	d->m_workset = new WrkIconSet;
    d->defSets.append(d->m_workset);
    addIconSet("icons/smiles.jisp", false);
    addIconSet("icons/icqlite.jisp", false);
    addIconSet("icons/additional.jisp", false);	
}

Icons::~Icons()
{
    QList<IconSet*>::iterator it;
    for (it = d->customSets.begin(); it != d->customSets.end(); ++it)
        delete *it;
    for (it = d->defSets.begin(); it != d->defSets.end(); ++it)
        delete *it;
    delete d;
}

void Icons::setPixmap(const QString& name, const QPixmap& pict)
{
    d->m_workset->add(name, pict.toImage(), 0);
}

bool Icons::processEvent(Event *e)
{
    if (e->type() == eEventIconChanged){
        QList<IconSet*>::iterator it;
        for (it = d->customSets.begin(); it != d->customSets.end(); ++it)
            (*it)->clear();
        for (it = d->defSets.begin(); it != d->defSets.end(); ++it)
            (*it)->clear();
    }
    return false;
}

void Icons::iconChanged(int)
{
    EventIconChanged().process();
}

PictDef *Icons::getPict(const QString &name)
{
    QList<IconSet*>::iterator it;
    for (it = d->customSets.begin(); it != d->customSets.end(); ++it){
        PictDef *res = (*it)->getPict(name);
        if (res)
            return res;
    }
    for (it = d->defSets.begin(); it != d->defSets.end(); ++it){
        PictDef *res = (*it)->getPict(name);
        if (res)
            return res;
    }
    return NULL;
}

QStringList Icons::getSmile(const QString &name)
{
    QList<IconSet*>::iterator it;
    for (it = d->customSets.begin(); it != d->customSets.end(); ++it){
        QStringList res = (*it)->getSmile(name);
        if (!res.empty())
            return res;
    }
    return QStringList();
}

QString Icons::getSmileName(const QString &name)
{
    QList<IconSet*>::iterator it;
    for (it = d->customSets.begin(); it != d->customSets.end(); ++it){
        QString res = (*it)->getSmileName(name);
        if (!res.isEmpty())
            return res;
    }
    return QString();
}

void Icons::getSmiles(QStringList &smiles)
{
    QStringList used;
    foreach( IconSet* iconSet, d->customSets ) {
        iconSet->getSmiles(smiles, used);
    }
}

QString Icons::parseSmiles(const QString &str)
{
	QString s = str;
	QString res;
	while (!s.isEmpty()){
		unsigned start = (unsigned)(-1);
		unsigned size  = 0;
		QString smile;
		QList<IconSet*>::iterator it;
		for (it = d->customSets.begin(); it != d->customSets.end(); ++it){
			unsigned pos    = ~0U;
			unsigned length = 0;
			QString n_smile;
			(*it)->parseSmiles(s, pos, length, n_smile);
			if (length == 0)
				continue;
			if (pos < start)
			{
				start = pos;
				size  = length;
				smile = n_smile;
			}
		}
		if (size == 0){
			res += quoteString(s);
			break;
		}
		res += s.left(start);
        res += "<img src=\"sim:icons/";
		res += smile;
		res += "\" alt=\"";
		res += quoteString(s.mid(start, size));
		res += "\">";
		s = s.mid(start + size);
	}
	return res;
}

IconSet *Icons::addIconSet(const QString &name, bool bDefault)
{
	FileIconSet *is = new FileIconSet(name);
	if (bDefault)
	{
		d->defSets.prepend(is);
	}
	else
	{
		d->customSets.append(is);
	}
	return is;
}

void Icons::removeIconSet(IconSet *is)
{
    if(!is) {
        for (QList<IconSet*>::iterator it = d->customSets.begin(); it != d->customSets.end(); ){
            IconSet *set = *it;
            it++;
            delete set;
        }
        d->customSets.clear();
        return;
    }
    if(d->customSets.contains( is )){
        delete is;
        d->customSets.removeAll( is );
        return;
    }
    if(d->defSets.contains( is )){
        delete is;
        d->defSets.removeAll( is );
        return;
    }
}

};

