#ifndef QZIP_H
#define QZIP_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#if defined(QTZIP_LIBRARY)
#  define QTZIPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QTZIPSHARED_EXPORT Q_DECL_IMPORT
#endif

class QuaZipFile;
class QTZIPSHARED_EXPORT QZip : public QObject
{
public:
    QZip(const QString &fromFileName,const QString &toFileName);
    bool unzip();
    bool zip();
    void setZipFileName(const QString &zipFileName);
    void setZipPath(const QString &zipPath);
private:
    bool unzipRead();
    void zipDir(QuaZipFile &outFile,const QString &zipPath,const QString &zipTPath);
    //bool unzipPos();
    QString _zipFileName;
    QString _zipPathName;
};

#endif // QZIP_H
