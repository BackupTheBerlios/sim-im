#ifndef ICQSSBI_H
#define ICQSSBI_H

#include <QObject>
#include <QImage>
#include "icqclient.h"

class SSBISocket : public QObject, public ServiceSocket
{
    Q_OBJECT
public:
    SSBISocket(ICQClient *client);
    ~SSBISocket();
    void requestBuddy(const QString &screen, unsigned short buddyID, const QByteArray &buddyHash);
    void uploadBuddyIcon(unsigned short refNumber, const QImage &img);
protected:
    virtual bool error_state(const QString &err, unsigned code);
    virtual const char *serviceSocketName() { return "SSBISocket"; }
    virtual void data(unsigned short food, unsigned short type, unsigned short seq);
    void snac_service(unsigned short type, unsigned short seq);
    void snac_ssbi(unsigned short type, unsigned short seq);
    void process();

    QStringList m_buddyRequests;
    QImage m_img;   // image to upload
    unsigned short m_refNumber; // the ref number for the image
    unsigned m_retryCount;
protected slots:
    void requestService() { m_client->snacService()->requestService(this); }
};

#endif // ICQSSBI_H
