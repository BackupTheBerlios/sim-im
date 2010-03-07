
#ifndef ICQICMB_H
#define ICQICMB_H

#include <list>
#include <QObject>
#include "snac.h"
#include "message.h"

class ICQBuffer;
class ICQClientSocket;
class DirectSocket;
class TlvList;
class QTimer;

struct ICQUserData;


struct MessageId
{
    unsigned long    id_l;
    unsigned long    id_h;
    MessageId() : id_l(0), id_h(0) {}
};

bool operator == (const MessageId &m1, const MessageId &m2);
namespace SIM
{
	class Message;
}

struct SendMsg
{
    QString 		screen;
    MessageId		id;
    SIM::Message	*msg;
    QString			text;
    QString			part;
    unsigned		flags;
    DirectSocket    *socket;
    SendMsg() : msg(NULL), socket(NULL) {}
};

typedef unsigned char plugin[0x12];

class SnacIcqICBM : public QObject, public SnacHandler
{
	Q_OBJECT
public:
	SnacIcqICBM(ICQClient* client);
	virtual ~SnacIcqICBM();

	virtual bool process(unsigned short subtype, ICQBuffer* buf, unsigned short seq);
	void rightsRequest();
	void sendICMB(unsigned short channel, unsigned long flags);
	void sendThroughServer(const QString &screen, unsigned short channel, ICQBuffer &b, const MessageId &id, bool bOffline, bool bReqAck);
	void sendType2(const QString &screen, ICQBuffer &msgBuf, const MessageId &id, unsigned cap, bool bOffline, unsigned short port, TlvList *tlvs = NULL, unsigned short type = 1);
	bool ackMessage(SIM::Message *msg, unsigned short ackFlags, const QByteArray &msg_str);
	void sendType1(const QString &text, bool bWide, ICQUserData *data);
	void sendAdvMessage(const QString &screen, ICQBuffer &msgText, unsigned plugin_index,
			const MessageId &id, bool bOffline, bool bDirect, unsigned short cookie1 = 0,
			unsigned short cookie2 = 0, unsigned short type = 1);
	void ackMessage(SendMsg &s);
	bool sendThruServer(SIM::Message *msg, void *_data);
	void clearMsgQueue();
	void sendFile(TlvList& tlv, unsigned long primary_ip, unsigned long secondary_ip, unsigned short port,const QString &screen, MessageId const& id);
	void sendAutoReply(const QString &screen, MessageId id,
                              const plugin p, unsigned short cookie1, unsigned short cookie2,
                              unsigned short msgType, char msgFlags, unsigned short msgState,
                              const QString &response, unsigned short response_type, ICQBuffer &copy);
	void sendMTN(const QString &screen, unsigned short type);
	void accept(SIM::Message *msg, ICQUserData *data);
	void accept(SIM::Message *msg, const QString &dir, SIM::OverwriteMode overwrite);
	void decline(SIM::Message *msg, const QString &reason);
	void requestReverseConnection(const QString &screen, DirectSocket *socket);
	void parseAdvancedMessage(const QString &screen, ICQBuffer &m, bool needAck, MessageId id);
	bool processMsg();
	bool cancelMessage(SIM::Message* msg);
	void sendSMS(SendMsg msg);
	void pluginInfoRequest(unsigned long, unsigned plugin_index);

	QTimer* getSendTimer() { return m_sendTimer; }
	
public slots:
	void processSendQueue();

protected:
    virtual ICQClientSocket *socket();

protected slots:
    void sendTimeout();

private:
    QTimer *m_sendTimer;
    SendMsg  m_send;
    std::list<SendMsg>		sendFgQueue;
    std::list<SendMsg>		sendBgQueue;
    std::list<SendMsg>		replyQueue;
    std::list<SendMsg>		smsQueue;

	friend class SMSRequest;
	friend class ICQClient;
};

#endif

