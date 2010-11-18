#include "qzip.h"
#include "quazip.h"
#include "quazipfile.h"
QZip::QZip(const QString &zipFileName,const QString &zipPathName)
{
    _zipFileName = zipFileName;
    _zipPathName = zipPathName;

    QDir dir1(".");
    dir1.setCurrent(".");
    _zipFileName = dir1.absoluteFilePath(_zipFileName);

    QDir dir2(_zipPathName);
    if(!dir2.isAbsolute()){
        _zipPathName = dir2.absolutePath();
    }
}
bool QZip::unzip()
{
    QDir dir;
    if(!dir.exists(_zipPathName)){
        dir.mkpath(_zipPathName);
    }
    if(!unzipRead()){
        return false;
    }
    return true;
}
void QZip::zipDir(QuaZipFile &outFile,const QString &zipPath,const QString &zipTPath){
    QDir dir(zipPath);
    dir.setCurrent(zipPath);
    QFileInfoList files=dir.entryInfoList();

    QFile inFile;
    char c;
    foreach(QFileInfo file, files) {
        if(file.isDir()){
            if(!file.absoluteFilePath().endsWith(".")){
                QString zipDirecName = zipTPath==""?file.fileName():zipTPath+"\\"+file.fileName();
                zipDir(outFile,file.absoluteFilePath(),zipDirecName);
                dir.setCurrent(zipPath);
            }
            continue;//
        }
        inFile.setFileName(file.fileName());
        if(!inFile.open(QIODevice::ReadOnly)) {
            qWarning("testCreate(): inFile.open(): %s", inFile.errorString().toLocal8Bit().constData());
            return;
        }
        QString zipDirName = zipTPath==""?inFile.fileName():zipTPath+"\\"+inFile.fileName();
        if(!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(zipDirName, inFile.fileName()))) {
            qWarning("testCreate(): outFile.open(): %d", outFile.getZipError());
            return;
        }
        while(inFile.getChar(&c)&&outFile.putChar(c));
        if(outFile.getZipError()!=UNZ_OK) {
            qWarning("testCreate(): outFile.putChar(): %d", outFile.getZipError());
            return;
        }
        outFile.close();
        if(outFile.getZipError()!=UNZ_OK) {
            qWarning("testCreate(): outFile.close(): %d", outFile.getZipError());
            return;
        }
        inFile.close();
    }
}
bool QZip::zip()
{
    QuaZip zip1(_zipFileName);
    if(!zip1.open(QuaZip::mdCreate)) {
        qWarning("Zip File Create Failure: %s", _zipFileName.toLocal8Bit().data());
        return false;
    }
    QuaZipFile outFile(&zip1);

    zipDir(outFile,_zipPathName,"");

    zip1.close();
    if(zip1.getZipError()!=0) {
        qWarning("Zip File Close Failure: %s", _zipFileName.toLocal8Bit().data());
        return false;
    }
    return true;
}

bool QZip::unzipRead()
{
    QuaZip zip(_zipFileName);
    if(!zip.open(QuaZip::mdUnzip)) {
        qWarning("File Can't be Open:%s",_zipFileName.toLocal8Bit().data());
        return false;
    }
    zip.setFileNameCodec("IBM866");
    printf("%d entries\n", zip.getEntriesCount());
    printf("Global comment: %s\n", zip.getComment().toLocal8Bit().constData());
    QuaZipFileInfo info;
    printf("name\tcver\tnver\tflags\tmethod\tctime\tCRC\tcsize\tusize\tdisknum\tIA\tEA\tcomment\textra\n");
    QuaZipFile file(&zip);
    QFile out;
    QString name;
    char c;
    for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {
        if(!zip.getCurrentFileInfo(&info)) {
            qWarning("testRead(): getCurrentFileInfo(): %d\n", zip.getZipError());
            return false;
        }
        printf("%s\t%hu\t%hu\t%hu\t%hu\t%s\t%u\t%u\t%u\t%hu\t%hu\t%u\t%s\t%s\n",
               info.name.toLocal8Bit().constData(),
               info.versionCreated, info.versionNeeded, info.flags, info.method,
               info.dateTime.toString(Qt::ISODate).toLocal8Bit().constData(),
               info.crc, info.compressedSize, info.uncompressedSize, info.diskNumberStart,
               info.internalAttr, info.externalAttr,
               info.comment.toLocal8Bit().constData(), info.extra.constData());
        if(!file.open(QIODevice::ReadOnly)) {
            qWarning("testRead(): file.open(): %d", file.getZipError());
            return false;
        }

        QStringList list;
        if(info.name.contains("\\")){
            list = info.name.split("\\");
        }else if(info.name.contains("/")){
            list = info.name.split("/");
        }
        QDir dir;
        dir.setCurrent(_zipPathName);
        QString tempDir = _zipPathName;
        for(int ccc=0;ccc<list.count()-1;ccc++){
            tempDir+="\\"+list.at(ccc);
            if(!dir.exists(list.at(ccc))){
                dir.mkdir(tempDir);
            }
            dir.setCurrent(tempDir);
        }


        name=file.getActualFileName();
        if(file.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return false;
        }
        out.setFileName(_zipPathName+"\\"+name);
        // this will fail if "name" contains subdirectories, but we don't mind that
        out.open(QIODevice::WriteOnly);
        // Slow like hell (on GNU/Linux at least), but it is not my fault.
        // Not ZIP/UNZIP package's fault either.
        // The slowest thing here is out.putChar(c).
        while(file.getChar(&c)) out.putChar(c);
        out.close();
        if(file.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return false;
        }
        if(!file.atEnd()) {
            qWarning("testRead(): read all but not EOF");
            return false;
        }
        file.close();
        if(file.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.close(): %d", file.getZipError());
            return false;
        }
    }
    zip.close();
    if(zip.getZipError()!=UNZ_OK) {
        qWarning("testRead(): zip.close(): %d", zip.getZipError());
        return false;
    }
    return true;
}

void QZip::setZipPath(const QString &zipPath){
    this->_zipPathName = zipPath;
}
void QZip::setZipFileName(const QString &zipPath){
    this->_zipFileName = zipPath;
}
