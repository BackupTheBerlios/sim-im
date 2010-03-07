#ifndef SIMFS_H
#define SIMFS_H

#include <QAbstractFileEngine>
#include <QAbstractFileEngineHandler>
#include <QBuffer>
#include "simapi.h"

class EXPORT SimFileEngine
    : public QAbstractFileEngine
{
public:
    SimFileEngine( const QString &fileName );
    virtual ~SimFileEngine();

    virtual Iterator * beginEntryList ( QDir::Filters filters, const QStringList & filterNames );
    virtual bool caseSensitive () const;
    virtual bool close ();
    virtual bool copy ( const QString & newName );
    virtual QStringList entryList ( QDir::Filters filters, const QStringList & filterNames ) const;
    virtual bool extension ( Extension extension, const ExtensionOption * option = 0, ExtensionReturn * output = 0 );
    virtual FileFlags fileFlags ( FileFlags type = FileInfoAll ) const;
    virtual QString fileName ( FileName file = DefaultName ) const;
    virtual QDateTime fileTime ( FileTime time ) const;
    virtual bool flush ();
    virtual int handle () const;
    virtual bool isRelativePath () const;
    virtual bool isSequential () const;
    virtual bool link ( const QString & newName );
    virtual bool mkdir ( const QString & dirName, bool createParentDirectories ) const;
    virtual bool open ( QIODevice::OpenMode mode );
    virtual QString owner ( FileOwner owner ) const;
    virtual uint ownerId ( FileOwner owner ) const;
    virtual qint64 pos () const;
    virtual qint64 read ( char * data, qint64 maxlen );
    virtual qint64 readLine ( char * data, qint64 maxlen );
    virtual bool remove ();
    virtual bool rename ( const QString & newName );
    virtual bool rmdir ( const QString & dirName, bool recurseParentDirectories ) const;
    virtual bool seek ( qint64 offset );
    virtual void setFileName ( const QString & file );
    virtual bool setPermissions ( uint perms );
    virtual bool setSize ( qint64 size );
    virtual qint64 size () const;
    virtual bool supportsExtension ( Extension extension ) const;
    virtual qint64 write ( const char * data, qint64 len );

protected:
    QString m_fileName;
    QBuffer m_buffer;
    QByteArray m_data;
};

class EXPORT SimFileEngineHandler
    : public QAbstractFileEngineHandler
{
public:
    SimFileEngineHandler();
    virtual ~SimFileEngineHandler();

    virtual QAbstractFileEngine *create( const QString &fileName ) const;
};

#endif // SIMFS_H
