/***************************************************************************
                          cfg.h  -  description
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

#ifndef _CFG_H
#define _CFG_H

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QSharedPointer>

#include "propertyhub.h"
#include "simapi.h"

class QObject;
class QWidget;
class QToolBar;

class Buffer;
class IP;

namespace SIM {

class EXPORT Config
{
public:
    Config(const QString& filename);
    virtual ~Config();

    bool mergeOldConfig(const QString& filename);

    bool addPropertyHub(PropertyHubPtr hub);
    PropertyHubPtr propertyHub(const QString& hubNamespace);
    int propertyHubCount();
    void clearPropertyHubs();
    PropertyHubPtr rootPropertyHub();

    QByteArray serialize();
    bool deserialize(const QByteArray& arr);
private:
    typedef QMap<QString, PropertyHubPtr> PropertyHubMap;
    PropertyHubMap m_hubs;
    QString m_group;
    QVariantMap m_data;
    QString m_filename;
    PropertyHubPtr m_roothub;
    bool m_changed;
};

typedef QSharedPointer<Config> ConfigPtr;

enum DataType {
    DATA_UNKNOWN = 0,
    DATA_STRING,
    DATA_LONG,
    DATA_ULONG,
    DATA_BOOL,
    DATA_STRLIST,
    DATA_STRMAP = DATA_STRLIST,
    DATA_UTF,
    DATA_IP,
    DATA_STRUCT,
    DATA_UTFLIST,
    DATA_OBJECT,
    DATA_BINARY,    // QByteArray
    DATA_CSTRING    // to store data in an unknown encoding (icq's ServerText)
};

struct DataDef
{
    const char  *name;
    DataType    type;
    unsigned    n_values;
    const char  *def_value;
};

class IP;
class EXPORT Data
{
public:
    typedef QMap<unsigned, QString> STRING_MAP;
public:
    Data();
    Data(const Data& d);
    Data &operator =(const Data &);

    ~Data() { clear(false); }

    // DataName
    void setName(const QString &name);
    const QString &name() const;

    // DataType
    void setType(DataType eType);
    DataType type() const;

    void clear(bool bNew = true);

    // QString
    const QString &str() const;
    QString &str();
    bool setStr(const QString &s);
    // StringMap
    const STRING_MAP &strMap() const;
    STRING_MAP &strMap();
    bool setStrMap(const STRING_MAP &s);
    // Long
    long toLong() const;
    long &asLong();
    bool setLong(long d);
    // ULong
    unsigned long toULong() const;
    unsigned long &asULong();
    bool setULong(unsigned long d);
    // Bool
    bool toBool() const;
    bool &asBool();
    bool setBool(bool d);
    // class QObject
    const QObject* object() const;
    QObject* object();
    bool setObject(const QObject *);
    // class IP
    const IP* ip() const;
    IP* ip();
    bool setIP(const IP *);
    // Binary
    const QByteArray &toBinary() const;
    QByteArray &asBinary();
    bool setBinary(const QByteArray &d);
    // QString
    const QByteArray &cstr() const;
    QByteArray &cstr();
    bool setCStr(const QByteArray &s);

protected:
    bool checkType(DataType type) const;
    DataType m_type;
    QString m_name;
    class DataPrivate *data;
};

#define DATA(A) ((const char*)(A))

EXPORT void free_data(const DataDef *def, void *data);
EXPORT void load_data(const DataDef *def, void *data, Buffer *config);
EXPORT QByteArray save_data(const DataDef *def, const void *data);

EXPORT const QString get_str(const Data &strlist, unsigned index);
EXPORT void set_str(Data *strlist, unsigned index, const QString &value);
EXPORT unsigned long get_ip(const Data &ip);
EXPORT QString get_host(const Data &ip);
EXPORT bool set_ip(Data *ip, unsigned long value, const QString &host=QString::null);

#define PROP_STRLIST(A) \
    QString get##A(unsigned index) { return SIM::get_str(data.A, index); } \
    void set##A(unsigned index, const QString &value) { SIM::set_str(&data.A, index, value); } \
    void clear##A()  { data.A.clear(); }

#define PROP_UTFLIST(A) \
    QString get##A(unsigned index) { return SIM::get_str(data.A, index); } \
    void set##A(unsigned index, const QString &value) { SIM::set_str(&data.A, index, value); } \
    void clear##A()  { data.A.clear(); }

#define PROP_STR(A) \
    QString get##A() const { return data.A.str(); } \
    bool set##A(const QString &r) { return data.A.setStr( r ); }

#define PROP_UTF8(A) \
    QString get##A() const { return data.A.str(); } \
    bool set##A(const QString &r) { return data.A.setStr( r ); }

#define VPROP_UTF8(A) \
    virtual QString get##A() const { return data.A.str(); } \
    virtual bool set##A(const QString &r) { return data.A.setStr( r ); }

#define PROP_LONG(A) \
    long get##A() const { return data.A.toLong(); } \
    void set##A(long r) { data.A.setLong(r); }

#define PROP_ULONG(A) \
    unsigned long get##A() const { return data.A.toULong(); } \
    void set##A(unsigned long r) { data.A.setULong(r); }

#define PROP_USHORT(A) \
    unsigned short get##A() const { return (unsigned short)(data.A.toULong()); } \
    void set##A(unsigned short r) { data.A.setULong(r); }

#define PROP_BOOL(A) \
    bool get##A() const { return data.A.toBool(); } \
    void set##A(bool r) { data.A.setBool(r); }

#define VPROP_BOOL(A) \
    virtual bool get##A() const { return data.A.toBool(); } \
    virtual void set##A(bool r) { data.A.setBool(r); }

#define PROP_CSTR(A) \
    QByteArray get##A() const { return data.A.cstr(); } \
    bool set##A(const QByteArray &r) { return data.A.setCStr( r ); }

const int LEFT      = 0;
const int TOP       = 1;
const int WIDTH     = 2;
const int HEIGHT    = 3;
const int DESKTOP   = 4;

typedef Data Geometry[5];
EXPORT void saveGeometry(QWidget*, Geometry);
EXPORT void restoreGeometry(QWidget*, Geometry, bool bPos, bool bSize);

EXPORT void saveToolbar(QToolBar*, Data[7]);
EXPORT void restoreToolbar(QToolBar*, Data[7]);

/* Get full path */
EXPORT QString app_file(const QString &f);

/* Get user file */
EXPORT QString user_file(const QString &f);

/* Make directory */
EXPORT bool makedir(const QString &dir);

/* Save state */
EXPORT void save_state();

EXPORT QString getToken(QString &from, char c, bool bUnEsacpe=true);
EXPORT QByteArray getToken(QByteArray &from, char c, bool bUnEsacpe=true);
//EXPORT QByteArray getToken(QByteArray &from, char c, bool bUnEsacpe=true);
EXPORT QString quoteChars(const QString &from, const char *chars, bool bQuoteSlash=true);
EXPORT QString unquoteChars(const QString &from, const QString chars, bool bQuoteSlash = true);

EXPORT char fromHex(char);

} // namespace SIM

#endif
