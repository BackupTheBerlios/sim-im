
#ifndef _ICQDIRECT_H_
#define _ICQDIRECT_H_

#include <QObject>
#include <QTimer>
#include <QWaitCondition>

#include "message.h"
#include "socket/socket.h"
#include "icqclient.h"
#include "icqicmb.h"

#define AOL_PROXY_HOST "ars.oscar.aol.com"
#define AOL_PROXY_PORT 5190

class ICQUserData;
class ICQClient;
struct OftData
{
	unsigned long magic;
	unsigned short unknown;
	unsigned short type;
	char cookie[8];
	unsigned short encrypt;
	unsigned short compress;
	unsigned short total_files;
	unsigned short files_left;
	unsigned short total_parts;
	unsigned short parts_left;
	unsigned long total_size;
	unsigned long size;
	unsigned long mod_time;
	unsigned long checksum;
	unsigned long rfrcsum;
	unsigned long rfsize;
	unsigned long cretime;
	unsigned long rfcsum;
	unsigned long nrecvd;
	unsigned long recvcsum;
	char idstring[32];
	unsigned char flags;
	unsigned char lnameoffset;
	unsigned char lsizeoffset;
	char dummy[69];
	char macfileinfo[16];
	unsigned short nencode;
	unsigned short nlanguage;
	QByteArray name;
};

class AIMFileTransfer : public SIM::FileTransfer, public SIM::ClientSocketNotify, public SIM::ServerSocketNotify
{
public:
	typedef enum
	{
		tdInput,
		tdOutput
	} tTransferDirection;

    AIMFileTransfer(SIM::FileMessage *msg, ICQUserData *data, ICQClient *client);
    ~AIMFileTransfer();
    virtual void accept();
	void setPort(unsigned short port) {m_port = port;}
    unsigned short remotePort();
	void setICBMCookie(MessageId const& cookie);
	void setICBMCookie2(unsigned short cookie2);
	MessageId& getICBMCookie() {return m_cookie; }
	void setProxyActive(bool proxyActive) { m_proxyActive = proxyActive; }
	virtual void connect(unsigned long ip, unsigned short port);
	void setStage(int stage) { m_stage = stage; }
	void forceProxyConnection() { m_proxy = true;}
	void requestFT();

	virtual void connectThroughProxy(const QString& host, uint16_t port, uint16_t cookie2);
	virtual tTransferDirection getDirection() = 0;

	static const unsigned long OFT_magic = 0x3254464f;
	static const int OFT_fileInfo = 0x0101;
	static const int OFT_answer = 0x0202;
	static const int OFT_success = 0x0402;
	static const int OFT_continue = 0x0502;

	static const unsigned short Chunk_status = 0x044a;
	static const unsigned short Chunk_uin = 0x0000;
	static const unsigned short Chunk_cap = 0x0001;
 
protected:

    virtual void processPacket();
    virtual bool error_state(const QString &err, unsigned code);
    virtual void resolve_ready(QHostAddress ip);
    virtual void startReceive(unsigned pos);
    virtual void bind_ready(unsigned short port);
    virtual bool error(const QString &err);
    void negotiateWithProxy();

	bool readOFT(OftData* oft);
	bool writeOFT(OftData* oft);
	unsigned long calculateChecksum();
	unsigned long checksumChunk(QByteArray* filechunk, unsigned int chunklength, unsigned int start);
	
	int m_stage;
	bool m_proxy;
	bool m_proxyActive;
	MessageId m_cookie;
	uint16_t m_cookie2;
	OftData m_oft;
	unsigned long m_packetLength;
	bool bcontinue;
    ICQUserData *m_data;
    ICQClient *m_client;
    ICQClientSocket *m_socket;
    unsigned short m_port;
    unsigned long m_ip;

    friend class ICQClient;
};

class AIMIncomingFileTransfer : public QObject, public AIMFileTransfer
{
	Q_OBJECT
public:
	AIMIncomingFileTransfer(SIM::FileMessage *msg, ICQUserData *data, ICQClient *client);
	virtual ~AIMIncomingFileTransfer();
    virtual void accept();
	virtual void connect_ready();
	virtual void packet_ready();
	virtual void write_ready();
    virtual bool accept(SIM::Socket *s, unsigned long ip);

	void receiveNextBlock(long size);
	void ackOFT();
	virtual void connectThroughProxy(const QString& host, uint16_t port, uint16_t cookie2);
	virtual tTransferDirection getDirection();

protected slots:
	virtual void connect_timeout();
protected:
    virtual void startReceive(unsigned pos);
	virtual bool error_state(const QString &err, unsigned code);
    enum State
    {
        None,
		Connecting,
		ReverseConnection,
		ProxyConnection,
		ProxyNegotiation,
		OFTNegotiation,
		Reading,
		Done
    };
	State m_state;
	QTimer m_connectTimer;
};

class AIMOutcomingFileTransfer : public QObject, public AIMFileTransfer
{
	Q_OBJECT
public:
	AIMOutcomingFileTransfer(SIM::FileMessage *msg, ICQUserData *data, ICQClient *client);
	virtual ~AIMOutcomingFileTransfer();

	void listen();
	virtual void connect(unsigned long ip, unsigned short port);
    //void connect(unsigned short port);
	virtual tTransferDirection getDirection();
	virtual void connectThroughProxy(const QString& host, uint16_t port, uint16_t cookie2);
protected slots:
	virtual void connect_timeout();
protected:
    enum State
    {
        None,
		Listen,
		ReverseConnection,
		ProxyConnection,
		ProxyNegotiation,
		OFTNegotiation,
		Writing,
		Done
    };
    virtual bool accept(SIM::Socket *s, unsigned long ip);
    virtual void write_ready();
    virtual void packet_ready();
    virtual void connect_ready();

	//void read_ready();
	bool sendNextBlock();

	void initOFTSending();

	State m_state;
	QTimer m_connectTimer;
};


#endif

