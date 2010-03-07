
#include <QObject>
#include <QTimer>

#include "clientsocket.h"

#include "misc.h"
#include "clientsocketnotify.h"
#include "socket.h"
#include "socketfactory.h"

#ifdef WIN32
	#include <winsock.h>
#else
#ifndef Q_OS_MAC
	#include <net/if.h>
#endif
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
#endif

namespace SIM
{

	ClientSocket::ClientSocket(ClientSocketNotify *notify, Socket *sock)
	{
		m_notify = notify;
		bRawMode = false;
		bClosed  = false;
		m_sock   = sock;
		if (m_sock == NULL)
			m_sock = getSocketFactory()->createSocket();
		m_sock->setNotify(this);
	}

	ClientSocket::~ClientSocket()
	{
		getSocketFactory()->erase(this);
		delete m_sock;
	}

	void ClientSocket::close()
	{
		m_sock->close();
		bClosed = true;
	}

	const QString &ClientSocket::errorString() const
	{
		return errString;
	}

	void ClientSocket::connect(const QString &host, unsigned short port, TCPClient *client)
	{
		if (client){
			EventSocketConnect e(this, client, host, port);
			e.process();
		}
		m_sock->connect(host, port);
	}

	void ClientSocket::connect(unsigned long ip, unsigned short port, TCPClient* /* client */)
	{
		struct in_addr addr;
		addr.s_addr = ip;
		this->connect(inet_ntoa(addr), port, NULL);
	}

	void ClientSocket::write()
	{
		if (writeBuffer().size() == 0)
			return;
		m_sock->write(writeBuffer().data(), writeBuffer().size());
		writeBuffer().init(0);
	}

	bool ClientSocket::created()
	{
		return (m_sock != NULL);
	}

        void ClientSocket::resolve_ready(QHostAddress ip)
	{
		m_notify->resolve_ready(ip);
	}

	void ClientSocket::connect_ready()
	{
		m_notify->connect_ready();
		bClosed = false;
	}

	void ClientSocket::setRaw(bool mode)
	{
		bRawMode = mode;
		read_ready();
	}

	void ClientSocket::read_ready()
	{
		if (bRawMode){
			for (;;){
				char b[2048];
				int readn = m_sock->read(b, sizeof(b));
				if (readn < 0){
					error_state(I18N_NOOP("Read socket error"));
					return;
				}
				if (readn == 0)
					break;
				unsigned pos = readBuffer().writePos();
				readBuffer().setWritePos(readBuffer().writePos() + readn);
				memcpy((void*)readBuffer().data(pos), b, readn);
			}
			if (m_notify)
				m_notify->packet_ready();
			return;
		}
		for (;;){
			if (bClosed || errString.length())
				break;
			int readn = m_sock->read((char*)readBuffer().data(readBuffer().writePos()), (readBuffer().size() - readBuffer().writePos()));
			if (readn < 0){
				error_state(I18N_NOOP("Read socket error"));
				return;
			}
			if (readn == 0)
				break;
			readBuffer().setWritePos(readBuffer().writePos() + readn);
			if(readBuffer().writePos() < (unsigned)readBuffer().size())
				break;
			if (m_notify)
				m_notify->packet_ready();
		}
	}

	void ClientSocket::write_ready()
	{
		if (m_notify)
			m_notify->write_ready();
	}

	unsigned long ClientSocket::localHost()
	{
		return m_sock->localHost();
	}

	void ClientSocket::pause(unsigned n)
	{
		m_sock->pause(n);
	}

	void ClientSocket::setSocket(Socket *s, bool bClearError)
	{
		if (m_sock){
			if (m_sock->getNotify() == this)
				m_sock->setNotify(NULL);
			if (bClearError){
				getSocketFactory()->erase(this);
			}
		}
		m_sock = s;
		if (s)
			s->setNotify(this);
	}

	void ClientSocket::error_state(const QString &err, unsigned code)
	{
		// -> false -> already there
		if(!getSocketFactory()->add(this))
			return;

		errString = err;
		errCode = code;
		QTimer::singleShot(0, getSocketFactory(), SLOT(idle()));
	}
}

// vim: set expandtab:
